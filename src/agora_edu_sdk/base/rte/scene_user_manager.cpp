//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "scene_user_manager.h"

#include <sstream>

#include "facilities/tools/api_logger.h"
#include "internal/IAgoraRteSceneWithTest.h"
#include "media/media_control.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_checker.h"

#include "interface/base/AgoraRefPtr.h"
#include "src/edu_message_impl.h"
#include "src/refcountedobject.h"

static const char* const MODULE_NAME = "[RTE.SUM]";

namespace agora {
namespace rte {

static const char* kDataParseThread = "RteDataParseWorker";

static const int DELAY_UPDATE_RTM_DATA_INTERVAL_MS = 300;
static const int DELAY_TRY_NEXT_REFRESH_INTERVAL_MS = 3000;

SceneUserManager::SceneUserManager(DataTransferMethod data_transfer_method)
    : user_update_state_(UUS_IDLE),
      data_transfer_method_(data_transfer_method),
      event_handlers_(
          utils::RtcAsyncCallback<ISceneUserManagerEventHandler>::Create()) {
  std::stringstream ss;
  ss << kDataParseThread << "_" << std::hex << this;
  // Fixed thread for data parse
  data_parse_worker_ = utils::minor_worker(ss.str().c_str(), true);
}

void SceneUserManager::RegisterEventHandler(
    ISceneUserManagerEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    return;
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(event_handler);
    return ERR_OK;
  });
}

void SceneUserManager::UnregisterEventHandler(
    ISceneUserManagerEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    return;
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(event_handler);
    return ERR_OK;
  });
}

std::string SceneUserManager::GetSceneUserToken() {
  std::string user_token;
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    user_token = scene_joined_data_.user_token;
    return ERR_OK;
  });

  return user_token;
}

void SceneUserManager::Refresh(bool full_fetch) {
  // TODO(jxm): need to check all the code of using 'shared_this', confirm which
  // part of code will run on major worker and which part will run on callback
  // worker, make sure no race condition. Better follow RTE scene, all the CP
  // code run on major worker and all the DP code run on data parser worker.
  auto shared_this = shared_from_this();

  API_LOGGER_MEMBER("full_fetch: %s", BOOL_TO_STR(full_fetch));

  data_parse_worker_->async_call(LOCATION_HERE, [shared_this, full_fetch] {
    if (shared_this->user_update_state_ == UUS_FETCHING) return;
    shared_this->user_update_state_ = UUS_FETCHING;
    shared_this->last_rtm_update_user_time_ =
        std::chrono::high_resolution_clock::now();

    LOG_INFO("start refresh, full_fetch: %s, [this: %p]",
             full_fetch ? "yes" : "no", shared_this.get());

    shared_this->user_list_refresh_times_++;

    shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
      // TODO(jxm): 'user_list_refresh_times_' is not atomic, race condition
      // between callback worker and data parse worker?
      event_handler->OnRefreshBegin(shared_this->user_list_refresh_times_);
    });

    if (!full_fetch && (shared_this->user_list_refresh_succeed_times_ > 0 &&
                        shared_this->server_data_sequence_ > 0)) {
      // If it has been successfully updated before, request all the latest
      // diffs from the server from the current local update sequence
      LOG_INFO("start refresh: DoRefreshFromCurrentSequenceId");
      shared_this->DoRefreshFromCurrentSequenceId();
    } else {
      LOG_INFO("start refresh: DoFullRefresh");
      shared_this->DoFullRefresh();
    }
  });
}

void SceneUserManager::Reset() {
  ASSERT_IS_UI_THREAD();

  if (data_parse_worker_) {
    data_parse_worker_->sync_call(LOCATION_HERE, [&] {
      user_update_state_ = UUS_IDLE;

      user_list_refresh_times_ = 0;
      user_list_refresh_succeed_times_ = 0;

      server_data_sequence_ = 0;  // 0 is unused by server
      current_fetch_missing_sequence_ = 0;
      fetch_missing_sequence_counter_ = 0;

      last_rtm_update_user_time_ = std::chrono::high_resolution_clock::now();

      waiting_for_try_next_full_refresh_ = false;

      // for test
      test_discard_next_sequence_ = false;

      // reset map
      online_users_.clear();
      online_streams_.clear();
      cached_sequence_data_.clear();

      return 0;
    });
  }
}

void SceneUserManager::SetSceneJoinData(
    const std::string& app_id, const std::string& auth,
    std::shared_ptr<RestfulSceneData> scene_join_data) {
  API_LOGGER_MEMBER("app_id: %s", app_id.c_str());

  if (data_parse_worker_) {
    data_parse_worker_->sync_call(LOCATION_HERE, [&] {
      scene_joined_data_.app_id = app_id;
      scene_joined_data_.auth = auth;
      scene_joined_data_.user_uuid = scene_join_data->user_uuid;
      scene_joined_data_.user_token = scene_join_data->user_token;
      scene_joined_data_.scene_uuid = scene_join_data->scene_uuid;
      scene_joined_data_.sequence_timeout = scene_join_data->sequence_time_out;
      scene_joined_data_.rtc_token = scene_join_data->rtc_token;

      OnlineUser self;
      self.user.user_uuid = scene_join_data->user_uuid;
      self.user.user_name = scene_join_data->user_name;
      self.user.role = scene_join_data->user_role;
      online_users_[self.user.user_uuid] = std::move(self);
      return ERR_OK;
    });
  }
}

void SceneUserManager::OnRtmConnectStateChanged(bool connected) {
  API_LOGGER_MEMBER("connected: %s", BOOL_TO_STR(connected));

  if (connected) {
    LOG_INFO("OnRtmConnectStateChanged refresh: [this:%p]", this);
    Refresh(false);
  } else {
    // Do nothing now
  }
}

void SceneUserManager::OnRTMSceneData(const commons::cjson::JsonWrapper& root,
                                      bool is_channel_msg) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  auto parse_data = RestfulDataParser::RtmParseSequence(root);
  if (parse_data) {
    // Cache data
    seq_id_t sequence_index = parse_data->sequence;

    if (test_discard_next_sequence_) {
      test_discard_next_sequence_ = false;
      LOG_INFO(
          "test_discard_next_sequence_ is set, discard this message [%" PRIu64
          "]for test!!! [this:%p]",
          sequence_index, this);
      return;
    }

    cached_sequence_data_[sequence_index] = std::move(parse_data);
    LOG_INFO("OnRTMSceneData in sequence: %" PRIu64 "[this:%p]", sequence_index,
             this);

    if (CanUpdateRtmDataNow()) {
      // Cache the data that has been added for a short time, and delay the
      // unified update
      auto shared_this = shared_from_this();
      data_parse_worker_->delayed_async_call(
          LOCATION_HERE,
          [shared_this] {
            seq_id_t missing_start;
            shared_this->DelayUpdateRtmData(
                shared_this->cached_sequence_data_.size(), missing_start);
            if (missing_start > 0) {
              shared_this->DelayFetchLostRtmSequenceData(missing_start);
            }
          },
          DELAY_UPDATE_RTM_DATA_INTERVAL_MS);
    }
  }
}

bool SceneUserManager::CanUpdateRtmDataNow() {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  return (user_update_state_ == UUS_FETCH_COMPLETE &&
          !waiting_for_try_next_full_refresh_);
}

void SceneUserManager::DelayUpdateRtmData(int last_count,
                                          seq_id_t& missing_start) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER("last_count: %d", last_count);

  missing_start = 0;

  std::chrono::high_resolution_clock::time_point now =
      std::chrono::high_resolution_clock::now();

  auto time_span = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - last_rtm_update_user_time_);

  // Delay for a period of time to process rtm data in batches, reducing the
  // number of reports to users
  if (last_count < cached_sequence_data_.size() &&
      time_span.count() < DELAY_UPDATE_RTM_DATA_INTERVAL_MS)
    return;

  StreamChangeData stream_change;
  UserChangeData user_change;
  // TODO(jxm): if the above two variables won't be used in this function, maybe
  // you can hide them in UpdateRtmCachedData()
  UpdateRtmCachedData(stream_change, user_change, true, missing_start);
}

void SceneUserManager::UpdateRtmCachedData(StreamChangeData& stream_change,
                                           UserChangeData& user_change,
                                           bool need_notify,
                                           seq_id_t& missing_start) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER("need_notify: %s", BOOL_TO_STR(need_notify));

  missing_start = 0;
  last_rtm_update_user_time_ = std::chrono::high_resolution_clock::now();

  std::list<std::unique_ptr<RtmResponseBase>> batch_sequence;
  auto it = cached_sequence_data_.begin();
  for (seq_id_t sequence_index = server_data_sequence_ + 1;
       it != cached_sequence_data_.end(); sequence_index++) {
    if (sequence_index > it->first) {
      // Old sequence data?
      LOG_INFO("Needed sequence id: %" PRIu64
               ", but cached sequence data is older: %" PRIu64,
               sequence_index, it->first);
      it = cached_sequence_data_.erase(it);
      continue;
    } else if (sequence_index == it->first) {
      // Is the sequence we need now, do update
      batch_sequence.push_back(std::move(it->second));
      it = cached_sequence_data_.erase(it);
      continue;
    } else {
      missing_start = sequence_index;
      break;
    }
  }

  if (!batch_sequence.empty()) {
    if (need_notify) {
      UpdateBatchRtmDataAndNotify(batch_sequence);
    } else {
      UpdateBatchRtmData(batch_sequence, stream_change, user_change);
    }
  }

  if (missing_start != 0) {
    LOG_INFO("UpdateRtmCachedData missing_start sequence id: %" PRIu64
             ", [this:%p]]",
             missing_start, this);
  }
}

void SceneUserManager::DelayTryNextFullRefresh() {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  waiting_for_try_next_full_refresh_ = true;
  LOG_INFO("DelayTryNextFullRefresh %d ms to refresh",
           DELAY_TRY_NEXT_REFRESH_INTERVAL_MS);

  auto shared_this = shared_from_this();
  data_parse_worker_->delayed_async_call(
      LOCATION_HERE,
      [shared_this] {
        shared_this->waiting_for_try_next_full_refresh_ = false;
        LOG_INFO("DelayTryNextFullRefresh refresh: [this:%p]",
                 shared_this.get());
        shared_this->Refresh(true);
      },
      DELAY_TRY_NEXT_REFRESH_INTERVAL_MS);
}

void SceneUserManager::DelayFetchLostRtmSequenceData(seq_id_t start) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER("start: %" PRIu64, start);

  // Delay a timeout to fetch the lost sequence data, because the missing
  // sequence data maybe arrive before the timeout

  LOG_INFO("Delay a timeout to fetch the lost sequence data: %" PRIu64, start);

  auto shared_this = shared_from_this();
  data_parse_worker_->delayed_async_call(
      LOCATION_HERE,
      [shared_this, start] {
        // Check again, missing data is already arrived over RTM?
        if (shared_this->server_data_sequence_ < start) {
          LOG_INFO("Missing sequence[%" PRIu64 "], do fetch...", start);
          shared_this->FetchLostRtmSequenceData(start);
        }
      },
      scene_joined_data_.sequence_timeout);
}

void SceneUserManager::FetchLostRtmSequenceData(seq_id_t start) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER("start: %" PRIu64, start);

  if (current_fetch_missing_sequence_ == start) {
    // Already fetching
    return;
  }

  // Data lost
  int count = 0;
  if (cached_sequence_data_.size() > 0) {
    auto it = cached_sequence_data_.begin();
    for (int i = start; i < it->first; i++) {
      count++;
    }
  }
  if (count == 0) return;

  current_fetch_missing_sequence_ = start;
  ++fetch_missing_sequence_counter_;

  LOG_WARN("FetchLostRtmSequenceData from sequence id: %" PRIu64 ", count: %d",
           start, count);

  auto param = CreateDataParamForSceneSequence(start, count);
  auto shared_this = shared_from_this();

  FetchUtility::CallFetchSceneSequence(
      param, data_transfer_method_.data_request_type, data_parse_worker_,
      [shared_this, start](bool success,
                           std::shared_ptr<RestfulSequenceDataWarpper> data) {
        // Clear this sequence fetching flag
        shared_this->current_fetch_missing_sequence_ = 0;

        if (success) {
          // Make sure data_list has vals!!!
          for (auto& d : data->data_list) {
            seq_id_t id = d->sequence;
            if (shared_this->server_data_sequence_ < id) {
              // Add the sequence data if is needed
              shared_this->cached_sequence_data_[id] = std::move(d);
              LOG_INFO("Add missing sequence[%" PRIu64
                       "], server_data_sequence_:[%" PRIu64 "]",
                       id, shared_this->server_data_sequence_);
            }
          }

          // Do date update now
          seq_id_t missing_start;
          shared_this->DelayUpdateRtmData(0, missing_start);
          // Nothing need to do with missing_start
          LOG_INFO("Missing sequence[%" PRIu64
                   "] fetch is succeed, still have missing? %" PRIu64,
                   start, missing_start);
        } else {
          LOG_INFO("Missing sequence[%" PRIu64
                   "] fetch is failed, delay try next refresh",
                   start);
          shared_this->DelayTryNextFullRefresh();
        }
      });
}

void SceneUserManager::UpdateBatchRtmData(
    std::list<std::unique_ptr<RtmResponseBase>>& batch_sequence,
    StreamChangeData& stream_change, UserChangeData& user_change) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  seq_id_t id = 0;
  for (const auto& s : batch_sequence) {
    id = s->sequence;
    switch (s->cmd) {
      case RTM_CMD_STREAM_CHANGED: {
        RtmSceneStreams* streams = static_cast<RtmSceneStreams*>(s.get());
        UpdateRtmStreamChange(*streams, stream_change);
      } break;

      case RTM_CMD_USER_CHANGED: {
        RtmSceneUsers* users = static_cast<RtmSceneUsers*>(s.get());
        UpdateRtmUserChange(*users, user_change, stream_change);
      } break;

      case RTM_CMD_SCENE_MESSAGE:
      case RTM_CMD_CUSTOM_MESSAGE: {
        RtmPeerMessage* msg_data = static_cast<RtmPeerMessage*>(s.get());
        edu::agora_refptr<edu::IAgoraEduMessage> send_msg =
            new edu::RefCountedObject<edu::AgoraEduMessage>();
        send_msg->SetEduMessage(msg_data->msg.c_str());
        send_msg->SetTimestamp(msg_data->ts);
        RtmMsgCmd cmd = static_cast<RtmMsgCmd>(s->cmd);
        auto remote_user_data = msg_data->from_user;
        event_handlers_->Post(LOCATION_HERE, [send_msg, remote_user_data,
                                              cmd](auto event_handler) {
          event_handler->OnSceneMessageReceived(send_msg, remote_user_data,
                                                cmd);
        });
      } break;

      case RTM_CMD_USER_PROPERTIES_CHANGED: {
        RtmUserPropertiesChange* changes =
            static_cast<RtmUserPropertiesChange*>(s.get());
        auto it = online_users_.find(changes->from_user.user_uuid);
        if (it != online_users_.end()) {
          auto& u = it->second;

          for (auto& p : changes->changed_properties) {
            if (changes->action == 2) {
              // remove
              auto it_property = u.user.properties.find(p.first);
              if (it_property != u.user.properties.end()) {
                u.user.properties.erase(it_property);
              }
            } else {
              // update/add
              u.user.properties[p.first] = p.second;
            }
          }
          auto changes_copy = *changes;
          changes_copy.from_user = u.user;
          event_handlers_->Post(
              LOCATION_HERE, [changes_copy](auto event_handler) {
                event_handler->OnUserPropertiesChanged(changes_copy);
              });
        }
      } break;

      case RTM_CMD_SCENE_PROPERTIES_CHANGED: {
        RtmScenePropertiesChange* changes =
            static_cast<RtmScenePropertiesChange*>(s.get());
        auto changes_copy = *changes;
        event_handlers_->Post(
            LOCATION_HERE, [changes_copy](auto event_handler) {
              event_handler->OnScenePropertiesChanged(changes_copy);
            });
      } break;

      default:
        LOG_INFO("UpdateBatchRtmData batch_sequence unknown cmd: %d", s->cmd);
        break;
    }
  }

  LOG_INFO(
      "UpdateBatchRtmData batch_sequence count: %u, user changes: [add: %u "
      "del: %u] [this: %p]",
      batch_sequence.size(), user_change.add_users.size(),
      user_change.del_users.size(), this);

  UpdateOffineUserStreams(user_change.del_users, stream_change.remove_streams);

  // Update the newest sequence
  server_data_sequence_ = id;
}

void SceneUserManager::UpdateBatchRtmDataAndNotify(
    std::list<std::unique_ptr<RtmResponseBase>>& batch_sequence) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  StreamChangeData stream_change;
  UserChangeData user_change;
  UpdateBatchRtmData(batch_sequence, stream_change, user_change);

  if (!user_change.add_users.empty()) {
    OnCalculatedUserListChanged(user_change.add_users, true);
  }

  OnCalculatedStreamsChanged(stream_change);

  if (!user_change.del_users.empty()) {
    OnCalculatedUserListChanged(user_change.del_users, false);
  }
}

void SceneUserManager::DiscardNextSequenceForTest() {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  test_discard_next_sequence_ = true;
}

void SceneUserManager::GetSceneDebugInfo(AgoraRteSceneDebugInfo& info) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  info.cur_sequence_id = server_data_sequence_;
  info.cached_sequence_size = cached_sequence_data_.size();
  info.fetch_missing_sequence_counter = fetch_missing_sequence_counter_;
}

void SceneUserManager::DoRefreshFromCurrentSequenceId() {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  seq_id_t start = server_data_sequence_;
  int count = 0;

  auto param = CreateDataParamForSceneSequence(start, count);
  auto shared_this = shared_from_this();

  FetchUtility::CallFetchSceneSequence(
      param, data_transfer_method_.data_request_type, data_parse_worker_,
      [shared_this, start](bool success,
                           std::shared_ptr<RestfulSequenceDataWarpper> data) {
        // check result first
        if (!success) {
          LOG_WARN("DoRefreshFromCurrentSequenceId() failed, DoFullRefresh()");
          shared_this->DoFullRefresh();
          return;
        }

        // add sequence data if necessary
        for (auto& resp : data->data_list) {
          auto id = resp->sequence;

          if (shared_this->server_data_sequence_ < id) {
            shared_this->cached_sequence_data_[id] = std::move(resp);
          }
        }

        // delay update RTM data
        seq_id_t missing_start = 0;
        shared_this->DelayUpdateRtmData(0, missing_start);

        // mark fetch complete and notify
        shared_this->user_update_state_ = UUS_FETCH_COMPLETE;
        shared_this->user_list_refresh_succeed_times_++;

        if (shared_this->event_handlers_) {
          shared_this->event_handlers_->Post(
              LOCATION_HERE, [=](auto event_handler) {
                event_handler->OnRefreshComplete(
                    shared_this->user_list_refresh_times_, true);
              });
        }

        // delay fetch lost RTM data
        if (missing_start > 0) {
          shared_this->DelayFetchLostRtmSequenceData(missing_start);
        }
      });
}

void SceneUserManager::DoFullRefresh() {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  auto param = CreateDataParam();
  param->AddString(PARAM_APP_ID, scene_joined_data_.app_id);
  param->AddString(PARAM_AUTH, scene_joined_data_.auth);
  param->AddString(PARAM_SCENE_UUID, scene_joined_data_.scene_uuid);
  param->AddString(PARAM_HTTP_TOKEN, scene_joined_data_.user_token);
  param->AddString(PARAM_USER_UUID, scene_joined_data_.user_uuid);

  auto shared_this = shared_from_this();
  FetchUtility::CallFetchSceneSnapshot(
      param, data_transfer_method_.data_request_type, data_parse_worker_,
      [shared_this](bool success,
                    std::shared_ptr<RestfulSceneSnapshotData> data) {
        if (success) {
          shared_this->OnFetchSceneUsers(data->sequence, data->scene_users);
          shared_this->event_handlers_->Post(
              LOCATION_HERE, [data](auto event_handler) {
                event_handler->OnScenePropertiesFromSnapshot(
                    data->scene_info.properties);
              });
        } else {
          LOG_INFO(
              "DoFullRefresh failed: success[%d] data->scene_users.size()[%d] "
              "[this: %p]",
              success, data->scene_users.size(), shared_this.get());
          shared_this->user_update_state_ = UUS_FETCH_FAILED;
          shared_this->OnFetchSceneUsersFailed();
          shared_this->DelayTryNextFullRefresh();
        }
      });
}

void SceneUserManager::UpdateOffineUserStreams(
    MapOnlineUsersType& users, MapOnlineStreamsType& remove_streams) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  for (auto& user : users) {
    for (const auto& stream_id : user.second.streams) {
      // Remove from online_streams_ log
      auto it = online_streams_.find(stream_id);
      if (it != online_streams_.end()) {
        // Log removed stream
        WithOperator<OnlineStream> stream_with_opertor;
        *(static_cast<OnlineStream*>(&stream_with_opertor)) = it->second;
        remove_streams[stream_id] = std::move(stream_with_opertor);

        online_streams_.erase(it);
      }
    }
    user.second.streams.clear();
  }
}

void SceneUserManager::UpdateRtmStreamChange(RtmSceneStreams& stream,
                                             StreamChangeData& change) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  auto it = online_users_.find(stream.from_user.user_uuid);
  if (it == online_users_.end()) return;  // User must be exist
  auto& user = it->second;

  auto stream_uuid = stream.stream_data.stream_uuid;

  switch (stream.stream_data.action) {
    case STREAM_ADD:
    case STREAM_MODIFY: {
      auto it = user.streams.find(stream_uuid);
      if (it == user.streams.end()) {
        // Make new stream item
        OnlineStream online_stream;
        online_stream.stream = std::move(stream.stream_data);
        online_stream.owner = user;
        online_streams_[stream_uuid] = online_stream;

        // Add to user
        user.streams.insert(stream_uuid);

        // Log added stream
        WithOperator<OnlineStream> stream_with_opertor;
        *(reinterpret_cast<OnlineStream*>(&stream_with_opertor)) =
            online_stream;
        stream_with_opertor.operator_user = stream.operator_user;
        change.add_streams[stream_uuid] = std::move(stream_with_opertor);
      } else {
        auto it2 = online_streams_.find(stream_uuid);
        if (it2 != online_streams_.end()) {
          auto& old_s = it2->second;

          // Object copy
          old_s.stream = std::move(stream.stream_data);

          // Log modified stream
          WithOperator<OnlineStream> stream_with_opertor;
          *(reinterpret_cast<OnlineStream*>(&stream_with_opertor)) = old_s;
          stream_with_opertor.operator_user = stream.operator_user;
          change.modify_streams.push_back(std::move(stream_with_opertor));
        }
      }
    } break;

    case STREAM_REMOVE: {
      if (user.streams.find(stream_uuid) == user.streams.end()) {
        LOG_WARN(
            "UpdateRtmStreamChange trying to remove a not existing stream: %s",
            stream.stream_data.stream_uuid.c_str());
        break;
      }
      auto it = user.streams.find(stream_uuid);
      if (it != user.streams.end()) {
        // Remove from user
        user.streams.erase(it);

        // Remove from online_streams_ log
        auto it2 = online_streams_.find(stream_uuid);
        if (it2 != online_streams_.end()) {
          const auto& old_s = it2->second;

          // Log removed stream
          WithOperator<OnlineStream> stream_with_opertor;
          *(reinterpret_cast<OnlineStream*>(&stream_with_opertor)) = old_s;
          stream_with_opertor.operator_user = stream.operator_user;
          change.remove_streams[stream_uuid] = std::move(stream_with_opertor);

          online_streams_.erase(it2);
        }

        // Remove what added just now
        auto it3 = change.add_streams.find(stream_uuid);
        if (it3 != change.add_streams.end()) change.add_streams.erase(it3);

        // Remove what modified just now
        auto it4 = change.modify_streams.begin();
        for (; it4 != change.modify_streams.end();) {
          if ((*it4).stream.stream_uuid == stream_uuid) {
            it4 = change.modify_streams.erase(it4);
            continue;
          }
          ++it4;
        }
      }
    } break;

    default:
      LOG_WARN("UpdateRtmStreamChange unknown stream action %d: %s",
               stream.stream_data.action, stream_uuid.c_str());
      break;
  }
}

void SceneUserManager::UpdateRtmUserChange(RtmSceneUsers& user,
                                           UserChangeData& change,
                                           StreamChangeData& stream_change) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  // Extract streams from user info
  std::list<RtmSceneStreams> online_user_streams;

  for (auto& one_online_user : user.online_users) {
    LOG_INFO("UpdateRtmUserChange user add: %s",
             one_online_user.user_data.user_uuid.c_str());

    for (auto& s : one_online_user.user_streams) {
      RtmSceneStreams stream;
      stream.stream_data = std::move(s);
      stream.from_user = one_online_user.user_data;
      online_user_streams.push_back(std::move(stream));
    }
    one_online_user.user_streams.clear();

    auto it = online_users_.find(one_online_user.user_data.user_uuid);
    if (it == online_users_.end()) {
      OnlineUser new_user;
      new_user.user = one_online_user.user_data;
      online_users_[new_user.user.user_uuid] = new_user;
      WithOperator<OnlineUser> user_with_operator;
      *(reinterpret_cast<OnlineUser*>(&user_with_operator)) =
          std::move(new_user);
      user_with_operator.operator_user =
          std::move(one_online_user.user_data.operator_user);
      change.add_users[user_with_operator.user.user_uuid] =
          std::move(user_with_operator);
    } else {
      // Update the user
      it->second.user = std::move(one_online_user.user_data);
    }
  }

  for (auto& one_offline_user : user.offline_users) {
    LOG_INFO("UpdateRtmUserChange user left: %s",
             one_offline_user.user_data.user_uuid.c_str());
    auto it = online_users_.find(one_offline_user.user_data.user_uuid);
    if (it != online_users_.end()) {
      // The user just joined and left?
      auto add_it = change.add_users.find(one_offline_user.user_data.user_uuid);
      if (add_it != change.add_users.end()) {
        LOG_INFO(
            "UpdateRtmUserChange user left, but this user just joined this "
            "time: %s",
            one_offline_user.user_data.user_uuid.c_str());
        change.add_users.erase(add_it);
      } else {
        WithOperator<OnlineUser> user_with_operator;
        *(reinterpret_cast<OnlineUser*>(&user_with_operator)) = it->second;
        user_with_operator.operator_user =
            std::move(one_offline_user.user_data.operator_user);
        change.del_users[user_with_operator.user.user_uuid] =
            std::move(user_with_operator);
      }

      online_users_.erase(it);
    } else {
      LOG_INFO(
          "UpdateRtmUserChange user left, but this user does not exist: %s",
          one_offline_user.user_data.user_uuid.c_str());
    }
  }

  for (auto& stream : online_user_streams) {
    UpdateRtmStreamChange(stream, stream_change);
  }
}

void SceneUserManager::OnFetchSceneUsers(
    seq_id_t sequence, std::map<uuid_t, RestfulRemoteUserData>& scene_users) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER("sequence: %" PRIu64, sequence);

  cached_stream_change_.add_streams.clear();
  cached_stream_change_.modify_streams.clear();
  cached_stream_change_.remove_streams.clear();

  cached_user_change_.add_users.clear();
  cached_user_change_.del_users.clear();

  // Is the full refreshed data older than the current sequence? If yes, refresh
  // directly
  if (sequence <= server_data_sequence_) {
    LOG_INFO(
        "The data refreshed in full is older than the current sequence and "
        "refreshed directly, "
        "data_sequence: %" PRIu64 ", local_sequence: %" PRIu64,
        sequence, server_data_sequence_);
  } else {
    std::list<RtmSceneStreams> temp_streams_;
    {
      auto online_users_backup = online_users_;
      for (auto& one_scene_user : scene_users) {
        auto& remote_user_data = one_scene_user.second.user;

        // Strip out the stream information contained in the user information
        for (auto& remote_stream_data : one_scene_user.second.streams) {
          RtmSceneStreams rtm_stream_temp;
          rtm_stream_temp.from_user = one_scene_user.second.user;
          rtm_stream_temp.stream_data = std::move(remote_stream_data);
          temp_streams_.push_back(std::move(rtm_stream_temp));
        }
        one_scene_user.second.streams.clear();

        auto it = online_users_.find(remote_user_data.user_uuid);
        if (it != online_users_.end()) {
          // the user exist, Update the user
          it->second.user = std::move(remote_user_data);

          // remove from backup map
          auto it_exist = online_users_backup.find(it->second.user.user_uuid);
          if (it_exist != online_users_backup.end()) {
            online_users_backup.erase(it_exist);
          }
          continue;
        } else {
          // not exist, new user
          OnlineUser new_user;
          new_user.user = std::move(remote_user_data);
          online_users_[new_user.user.user_uuid] = new_user;

          WithOperator<OnlineUser> user_with_operator;
          *(static_cast<OnlineUser*>(&user_with_operator)) =
              std::move(new_user);
          cached_user_change_.add_users[user_with_operator.user.user_uuid] =
              std::move(user_with_operator);
        }
      }

      // check the offline users
      for (auto& p : online_users_backup) {
        WithOperator<OnlineUser> user_with_operator;
        *(static_cast<OnlineUser*>(&user_with_operator)) = std::move(p.second);
        cached_user_change_.del_users[user_with_operator.user.user_uuid] =
            std::move(user_with_operator);
      }
    }

    // Update streams in user
    for (auto& one_stream : temp_streams_) {
      UpdateRtmStreamChange(one_stream, cached_stream_change_);
    }
    UpdateOffineUserStreams(cached_user_change_.del_users,
                            cached_stream_change_.remove_streams);

    server_data_sequence_ = sequence;
  }

  RefreshBySequenceId(sequence + 1, 0);
}

void SceneUserManager::OnFetchSceneUsersFailed() {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  auto shared_this = shared_from_this();
  event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
    event_handler->OnRefreshComplete(shared_this->user_list_refresh_times_,
                                     false);
  });
}

void SceneUserManager::RefreshBySequenceId(seq_id_t start, int count) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  auto param = CreateDataParamForSceneSequence(start, count);
  auto shared_this = shared_from_this();

  FetchUtility::CallFetchSceneSequence(
      param, data_transfer_method_.data_request_type, data_parse_worker_,
      [shared_this, start](bool success,
                           std::shared_ptr<RestfulSequenceDataWarpper> data) {
        if (success) {
          for (auto& d : data->data_list) {
            seq_id_t id = d->sequence;
            if (shared_this->server_data_sequence_ < id) {
              // Add the sequence data if is needed
              shared_this->cached_sequence_data_[id] = std::move(d);
            }
          }

          // Do date update now
          seq_id_t missing_start;
          shared_this->UpdateRtmCachedData(shared_this->cached_stream_change_,
                                           shared_this->cached_user_change_,
                                           false, missing_start);

          // End of this refresh
          shared_this->user_update_state_ = UUS_FETCH_COMPLETE;
          shared_this->user_list_refresh_succeed_times_++;

          if (shared_this->event_handlers_) {
            shared_this->event_handlers_->Post(
                LOCATION_HERE, [=](auto event_handler) {
                  event_handler->OnRefreshComplete(
                      shared_this->user_list_refresh_times_, true);
                });
          }

          if (!shared_this->cached_user_change_.add_users.empty()) {
            shared_this->OnCalculatedUserListChanged(
                shared_this->cached_user_change_.add_users, true);
          }

          shared_this->OnCalculatedStreamsChanged(
              shared_this->cached_stream_change_);

          if (!shared_this->cached_user_change_.del_users.empty()) {
            shared_this->OnCalculatedUserListChanged(
                shared_this->cached_user_change_.del_users, false);
          }

          if (missing_start > 0) {
            shared_this->DelayFetchLostRtmSequenceData(missing_start);
          }

        } else {
          LOG_INFO("RefreshBySequenceId failed: success[%d][this: %p]", success,
                   shared_this.get());
          shared_this->user_update_state_ = UUS_FETCH_FAILED;
          shared_this->OnFetchSceneUsersFailed();
          shared_this->DelayTryNextFullRefresh();
        }
      });
}

void SceneUserManager::OnCalculatedUserListChanged(MapOnlineUsersType& users,
                                                   bool add) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER("bool: %s", BOOL_TO_STR(add));

  // If from_full_fetch is true, Notification must be asynchronous
  event_handlers_->Post(LOCATION_HERE, [users, add](auto event_handler) {
    event_handler->OnUserListChanged(users, add);
  });
}

void SceneUserManager::OnCalculatedStreamsChanged(StreamChangeData& change) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  API_LOGGER_MEMBER(nullptr);

  if ((!change.add_streams.empty() || !change.modify_streams.empty() ||
       !change.remove_streams.empty())) {
    event_handlers_->Post(LOCATION_HERE, [change](auto event_handler) {
      event_handler->OnStreamsChanged(change.add_streams, change.modify_streams,
                                      change.remove_streams);
    });
  }
}

class RemoteUserInfoCollectionImpl : public RemoteUserInfoCollection {
 public:
  RemoteUserInfoCollectionImpl() {}

  uint32_t NumberOfUsers() override { return user_infos_.size(); }

  bool GetRemoteUserInfo(uint32_t user_index, UserInfo& user_info) override {
    if (user_index >= 0 && user_index < user_infos_.size()) {
      user_info = user_infos_[user_index];
      return true;
    }
    return false;
  }

  void AddUserInfo(const UserInfo& user_info) {
    user_infos_.push_back(user_info);
  }

 private:
  std::vector<UserInfo> user_infos_;
};

class RemoteStreamInfoCollectionImpl : public RemoteStreamInfoCollection {
 public:
  RemoteStreamInfoCollectionImpl() {}

  uint32_t NumberOfStreamInfo() override { return stream_infos_.size(); }

  bool GetRemoteStreamInfo(uint32_t stream_index,
                           MediaStreamInfo& stream_info) override {
    if (stream_index >= 0 && stream_index < stream_infos_.size()) {
      stream_info = stream_infos_[stream_index];
      return true;
    }
    return false;
  }

  void AddStreamInfo(const MediaStreamInfo& stream_info) {
    stream_infos_.push_back(stream_info);
  }

 private:
  std::vector<MediaStreamInfo> stream_infos_;
};

MapOnlineUsers SceneUserManager::GetUsers() {
  MapOnlineUsers online_users;
  data_parse_worker_->sync_call(LOCATION_HERE, [&]() {
    online_users = online_users_;
    return ERR_OK;
  });

  return online_users;
}

MapOnlineStreams SceneUserManager::GetStreams() {
  MapOnlineStreams online_streams;
  data_parse_worker_->sync_call(LOCATION_HERE, [&]() {
    online_streams = online_streams_;
    return ERR_OK;
  });

  return online_streams;
}

std::unique_ptr<OnlineUser> SceneUserManager::FindUser(UserId user_id) {
  std::unique_ptr<OnlineUser> user;
  data_parse_worker_->sync_call(LOCATION_HERE, [&]() {
    auto it = online_users_.find(user_id);
    if (it != online_users_.end()) {
      user.reset(new OnlineUser());
      *user = it->second;
    }

    return ERR_OK;
  });

  return user;
}

agora_refptr<IDataParam> SceneUserManager::CreateDataParamForSceneSequence(
    seq_id_t start, int count) {
  ASSERT_THREAD_IS(data_parse_worker_->getThreadId());

  auto param = CreateDataParam();
  param->AddString(PARAM_APP_ID, scene_joined_data_.app_id);
  param->AddString(PARAM_AUTH, scene_joined_data_.auth);
  param->AddString(PARAM_USER_UUID, scene_joined_data_.user_uuid);
  param->AddString(PARAM_SCENE_UUID, scene_joined_data_.scene_uuid);
  param->AddString(PARAM_HTTP_TOKEN, scene_joined_data_.user_token);
  param->AddUInt64(PARAM_START, start);
  param->AddInt(PARAM_COUNT, count);

  return param;
}

}  // namespace rte
}  // namespace agora
