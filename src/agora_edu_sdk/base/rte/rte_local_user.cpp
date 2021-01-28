//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "rte_local_user.h"

#include <memory>

#include "base/user_id_manager.h"
#include "facilities/tools/api_logger.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/strings/string_util.h"

//#include "api2/IAgoraService.h"
//#include "api2/internal/media_node_factory_i.h"

#include "media/IAgoraRealTimeMessage.h"
//#include "media/IAgoraMediaControl.h"
//#include "media/IAgoraMediaTrack.h"

//#include "media/camera_video_track.h"
//#include "media/microphone_audio_track.h"
//#include "media/screen_video_track.h"
#include "utils/video_frame_observer.h"

//#include "core/rtc_connection.h"
#include "rte_scene.h"
#include "transfer/rest_api_utility.h"

static const char* const MODULE_NAME = "[RTE.RLU]";

namespace agora {
namespace rte {

RteLocalUser::RteLocalUser( RteScene* scene, agora::rtm::IRtmService *& rtm_service,DataTransferMethod data_transfer_method)
    : scene_(scene),
	rtm_service_(rtm_service),
      data_transfer_method_(data_transfer_method),
      event_handlers_(utils::RtcAsyncCallback<IAgoraLocalUserEventHandler>::Create()),
      media_stream_event_handlers_(
          utils::RtcAsyncCallback<IAgoraMediaStreamEventHandler>::Create()) {
 /* if (!engine) {
    LOG_ERR_ASSERT_AND_RET("nullptr RTE engine in CTOR");
  }
*/
  if (!rtm_service) {
    LOG_ERR_ASSERT_AND_RET("nullptr rtm_service in CTOR");
  }

  if (!scene_) {
    LOG_ERR_ASSERT_AND_RET("nullptr scene in CTOR");
  }

//  if (!(media_control_ = engine->CreateAgoraMediaControl())) {
//    LOG_ERR_ASSERT_AND_RET("failed to create media control in CTOR");
//  }
//
//  if (!(media_node_factory_ = service->createMediaNodeFactory())) {
//    LOG_ERR_ASSERT_AND_RET("failed to create media node factory in CTOR");
//  }
}

RteLocalUser::~RteLocalUser() {
  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
	  /*  video_sink_map_.clear();
		remote_video_frame_observer_map_.clear();
		rte_video_frame_observer_ = nullptr;
	*/
    return ERR_OK;
  });
}

AgoraError RteLocalUser::SendPeerMessageToRemoteUser(agora_refptr<IAgoraMessage> message,
                                                     UserId user_id, const char* operate_id) {
  API_LOGGER_MEMBER("message: %p, user_id: %s, operate_id: %s", message.get(),
                    LITE_STR_CONVERT(user_id), LITE_STR_CONVERT(operate_id));

  if (!message) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "nullptr message");
  }

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid user ID");
  }

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
    }

    // prepare fetch parameters
    auto param = CreateDataParam();
    param->AddString(PARAM_APP_ID, scene_->GetAppId());
    param->AddString(PARAM_SCENE_UUID, scene_->GetSceneUuid());
    param->AddString(PARAM_USER_UUID, user_id);
    param->AddString(PARAM_TEXT, message->GetMessage());
    param->AddString(PARAM_HTTP_TOKEN, scene_->GetUserToken());

    agora_refptr<RteLocalUser> shared_this = this;
    std::string user_id_str(user_id);
    std::string operate_id_str(LITE_STR_CAST(operate_id));

    // fetch
    FetchUtility::CallSendPeerMessageToRemoteUser(
        param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
        [=](bool success, std::shared_ptr<RestfulResponseBase> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=]() {
            if (!success) {
              shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
                event_handler->OnSendPeerMessageToRemoteUserCompleted(
                    operate_id_str.c_str(), user_id_str.c_str(),
                    AgoraError(ERR_FAILED, "FetchUtility::CallSendPeerMessageToRemoteUser failed"));
              });

              LOG_ERR_AND_RET_INT(ERR_FAILED,
                                  "FetchUtility::CallSendPeerMessageToRemoteUser failed");
            }

            shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
              event_handler->OnSendPeerMessageToRemoteUserCompleted(
                  operate_id_str.c_str(), user_id_str.c_str(),
                  AgoraError(ERR_OK, "FetchUtility::CallSendPeerMessageToRemoteUser succeeded"));
            });

            return ERR_OK_;
          });
        });

    return ERR_OK_;
  });

  return err;
}

AgoraError RteLocalUser::SendSceneMessageToAllRemoteUsers(agora_refptr<IAgoraMessage> message,
                                                          const char* operate_id) {
  API_LOGGER_MEMBER("message: %p, operate_id: %s", message.get(), LITE_STR_CONVERT(operate_id));

  if (!message) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "nullptr message");
  }

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
    }

    // prepare fetch parameters
    auto param = CreateDataParam();
    param->AddString(PARAM_APP_ID, scene_->GetAppId());
    param->AddString(PARAM_SCENE_UUID, scene_->GetSceneUuid());
    param->AddString(PARAM_TEXT, message->GetMessage());
    param->AddString(PARAM_HTTP_TOKEN, scene_->GetUserToken());

    agora_refptr<RteLocalUser> shared_this = this;
    std::string operate_id_str(LITE_STR_CAST(operate_id));

    // fetch
    FetchUtility::CallSendSceneMessageToAllRemoteUsers(
        param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
        [=](bool success, std::shared_ptr<RestfulResponseBase> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=]() {
            if (!success) {
              shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
                event_handler->OnSendRoomMessageToAllRemoteUsersCompleted(
                    operate_id_str.c_str(),
                    AgoraError(ERR_FAILED,
                               "FetchUtility::CallSendSceneMessageToAllRemoteUsers failed"));
              });

              LOG_ERR_AND_RET_INT(ERR_FAILED,
                                  "FetchUtility::CallSendSceneMessageToAllRemoteUsers failed");
            }

            shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
              event_handler->OnSendRoomMessageToAllRemoteUsersCompleted(
                  operate_id_str.c_str(),
                  AgoraError(ERR_OK,
                             "FetchUtility::CallSendSceneMessageToAllRemoteUsers succeeded"));
            });

            return ERR_OK_;
          });
        });

    return ERR_OK_;
  });

  return err;
}

AgoraError RteLocalUser::InviteRemoteUserIntoTheScene(UserId user_id) {
  API_LOGGER_MEMBER("user_id: %s", LITE_STR_CONVERT(user_id));

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid user ID");
  }

  return AGORA_OK;
}

AgoraError RteLocalUser::CancelInvitation(UserId user_id) {
  API_LOGGER_MEMBER("user_id: %s", LITE_STR_CONVERT(user_id));

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid user ID");
  }

  return AGORA_OK;
}

AgoraError RteLocalUser::RemoteCreateOrUpdateStream(
    StreamId stream_id, const char* stream_name, UserId user_id, VideoSourceType video_source_type,
    AudioSourceType audio_source_type, MediaStreamType stream_type, const char* operate_id) {
  API_LOGGER_MEMBER(
      "stream_id: %s, stream_name: %s, user_id: %s, video_source_type: %d, audio_source_type: %d, "
      "stream_type: %d, operate_id: %s",
      LITE_STR_CONVERT(stream_id), LITE_STR_CONVERT(stream_name), LITE_STR_CONVERT(user_id),
      video_source_type, audio_source_type, stream_type, LITE_STR_CONVERT(operate_id));

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  if (utils::IsNullOrEmpty(stream_name)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid stream name");
  }

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid user ID");
  }

  // TODO(tomiao): check below similar comments
#if 0
  if (!ValidSourceAndStreamTypes(video_source_type, audio_source_type, stream_type)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid source and stream types");
  }
#else
  if (!ValidSourceTypes(video_source_type, audio_source_type)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid source types");
  }
#endif  // 0

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
    }

    // prepare fetch parameters
  //  auto param = CreateDataParamForTrackCommon(stream_id);

  //  // need to overwrite the user ID
  //  param->AddString(PARAM_USER_UUID, user_id);
  //  param->AddString(PARAM_STREAM_NAME, stream_name);
  //  param->AddInt(PARAM_VIDEO_SOURCE_TYPE, video_source_type);
  //  param->AddInt(PARAM_AUDIO_SOURCE_TYPE, audio_source_type);
  //  param->AddInt(PARAM_VIDEO_STATE, ValidVideoMediaStreamType(stream_type) ? 1 : 0);
  //  param->AddInt(PARAM_AUDIO_STATE, ValidAudioMediaStreamType(stream_type) ? 1 : 0);

  //  std::string operate_id_str(LITE_STR_CAST(operate_id));

  //  agora_refptr<RteLocalUser> shared_this = this;

  //  // fetch
  //  FetchUtility::CallPublishTrack(
  //      param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
  //      [=](bool success, std::shared_ptr<RestfulPublishTrackData> data) {
  //        (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &data]() {
  //          if (!success) {
  //            shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
  //              event_handler->OnRemoteCreateOrUpdateStreamCompleted(
  //                  operate_id_str.c_str(),
  //                  AgoraError(
  //                      ERR_FAILED,
  //                      "RemoteCreateOrUpdateStream() FetchUtility::CallPublishTrack failed"));
  //            });

  //            LOG_ERR_AND_RET_INT(
  //                ERR_FAILED, "RemoteCreateOrUpdateStream() FetchUtility::CallPublishTrack failed");
  //          }

  //          shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
  //            event_handler->OnRemoteCreateOrUpdateStreamCompleted(
  //                operate_id_str.c_str(),
  //                AgoraError(
  //                    ERR_OK,
  //                    "RemoteCreateOrUpdateStream() FetchUtility::CallPublishTrack success"));
  //          });

  //          return ERR_OK_;
  //        });
  //      });

    return ERR_OK_;
  });

  return err;
}

//AgoraError RteLocalUser::PublishLocalMediaTrack(agora_refptr<IAgoraMediaTrack> track,
//                                                const char* operate_id) {
//  API_LOGGER_MEMBER("track: %p, operate_id: %s", track.get(), LITE_STR_CONVERT(operate_id));
//
//  if (!track) {
//    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "nullptr track");
//  }
//
//  AgoraError err;
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
//    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
//      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
//    }
//
//    // get stream ID and name
//    char stream_id[kMaxStreamIdSize] = {0};
//
//    if (track->GetStreamId(stream_id, kMaxStreamIdSize) != ERR_OK) {
//      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "failed to get stream ID", err);
//    }
//
//    if (utils::IsNullOrEmpty(stream_id)) {
//      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "invalid stream ID", err);
//    }
//
//    char stream_name[kMaxStreamIdSize] = {0};
//
//    if (track->GetStreamName(stream_name, kMaxStreamIdSize) != ERR_OK) {
//      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "failed to get stream name", err);
//    }
//
//    // check source types
//    auto video_source_type = track->GetVideoSourceType();
//    auto audio_source_type = track->GetAudioSourceType();
//
//    if (!ValidSourceTypes(video_source_type, audio_source_type)) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "invalid source types", err);
//    }
//
//    // add publish info
//    std::string stream_id_str(stream_id);
//    std::string operate_id_str(LITE_STR_CAST(operate_id));
//
//    auto publish_info_pair = AddPublishInfo(stream_id_str, track);
//    if (!publish_info_pair.first) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to add publish info", err);
//    }
//
//    if (!publish_info_pair.second) {
//      LOG_INFO_SET_AND_RET_INT("publish info already added", err);
//    }
//
//    auto publish_info = publish_info_pair.first;
//
//    // prepare fetch parameters
//    auto param = CreateDataParamForTrackCommon(stream_id);
//
//    param->AddString(PARAM_STREAM_NAME, stream_name);
//
//    if (ValidVideoSourceType(video_source_type)) {
//      param->AddInt(PARAM_VIDEO_SOURCE_TYPE, video_source_type);
//      param->AddInt(PARAM_VIDEO_STATE, 1);
//    } else {
//      param->AddInt(PARAM_AUDIO_SOURCE_TYPE,
//                    MultipleAudioTracks(stream_id) ? TYPE_MIX : audio_source_type);
//      param->AddInt(PARAM_AUDIO_STATE, 1);
//    }
//
//    agora_refptr<RteLocalUser> shared_this = this;
//
//    // fetch
//    FetchUtility::CallPublishTrack(
//        param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
//        [=](bool success, std::shared_ptr<RestfulPublishTrackData> data) {
//          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &data]() {
//            if (!success) {
//              shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
//                event_handler->OnTrackPublishOrUnpublishCompleted(
//                    operate_id_str.c_str(),
//                    AgoraError(ERR_FAILED,
//                               "PublishLocalMediaTrack() FetchUtility::CallPublishTrack failed"));
//              });
//
//              LOG_ERR_AND_RET_INT(ERR_FAILED,
//                                  "PublishLocalMediaTrack() FetchUtility::CallPublishTrack failed");
//            }
//
//            // publish to RTC first
//            if (shared_this->PublishLocalMediaTrackImpl(data->rtc_token,
//                                                        shared_this->scene_->GetSceneUuid(),
//                                                        stream_id_str.c_str(), track)) {
//              publish_info->rtc_published = true;
//            } else {
//              // do not return but continue with below RTE actions
//              LOG_ERR("failed to publish media track: %s", stream_id_str.c_str());
//            }
//
//            // set publish state to be published and announce
//            publish_info->rtc_token = data->rtc_token;
//            publish_info->publish_state = PUBLISH_STATE_PUBLISHED;
//
//            shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
//              event_handler->OnTrackPublishOrUnpublishCompleted(operate_id_str.c_str(),
//                                                                AgoraError());
//            });
//
//            return ERR_OK_;
//          });
//        });
//
//    return ERR_OK_;
//  });
//
//  return err;
//}

//AgoraError RteLocalUser::UnpublishLocalMediaTrack(agora_refptr<IAgoraMediaTrack> track,
//                                                  const char* operate_id) {
//  API_LOGGER_MEMBER("track: %p, operate_id: %s", track.get(), LITE_STR_CONVERT(operate_id));
//
//  if (!track) {
//    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "nullptr track");
//  }
//
//  AgoraError err;
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
//    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
//      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
//    }
//
//    // get stream ID
//    char stream_id[kMaxStreamIdSize] = {0};
//
//    if (track->GetStreamId(stream_id, kMaxStreamIdSize) != ERR_OK) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get stream ID", err);
//    }
//
//    if (utils::IsNullOrEmpty(stream_id)) {
//      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "invalid stream ID", err);
//    }
//
//    // check source types
//    auto video_source_type = track->GetVideoSourceType();
//    auto audio_source_type = track->GetAudioSourceType();
//
//    if (!ValidSourceTypes(video_source_type, audio_source_type)) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "invalid source types", err);
//    }
//
//    // unpublish from RTC first before FetchUtility in case disconnected from RTE server,
//    // and user call this API to do mute action
//    // no need to update 'rtc_published' in publish info here since we will remove the info
//    // below immediately
//    if (!UnpublishLocalMediaTrackImpl(stream_id, track)) {
//      LOG_ERR("failed to unpublish media track: %s", stream_id);
//    }
//
//    // remove publish info
//    std::string stream_id_str(stream_id);
//    std::string operate_id_str(LITE_STR_CAST(operate_id));
//
//    auto ret = RemovePublishInfo(stream_id_str, track);
//
//    if (!ret.first) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to remove publish info", err);
//    }
//
//    if (!ret.second) {
//      LOG_INFO_SET_AND_RET_INT("didn't remove publish info", err);
//    }
//
//    // prepare fetch parameters
//    auto param = CreateDataParamForTrackCommon(stream_id);
//
//    agora_refptr<RteLocalUser> shared_this = this;
//
//    // fetch
//    FetchUtility::CallUnpublishTrack(
//        param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
//        [=](bool success, std::shared_ptr<RestfulResponseBase> data) {
//          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=]() {
//            if (!success) {
//              shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
//                event_handler->OnTrackPublishOrUnpublishCompleted(
//                    operate_id_str.c_str(),
//                    AgoraError(ERR_FAILED, "FetchUtility::CallUnpublishTrack failed"));
//              });
//
//              LOG_ERR_AND_RET_INT(ERR_FAILED, "FetchUtility::CallUnpublishTrack failed");
//            }
//
//            shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
//              event_handler->OnTrackPublishOrUnpublishCompleted(operate_id_str.c_str(),
//                                                                AgoraError());
//            });
//
//            return ERR_OK_;
//          });
//        });
//
//    return ERR_OK_;
//  });
//
//  return err;
//}

AgoraError RteLocalUser::MuteLocalMediaStream(StreamId stream_id, MediaStreamType stream_type) {
  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", LITE_STR_CONVERT(stream_id), stream_type);

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  if (!ValidMediaStreamType(stream_type)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid media stream type");
  }

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
    }

    // get stream publish info to get tracks
    auto iter = stream_publish_infos_map_.find(stream_id);
    if (iter == stream_publish_infos_map_.end()) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to find stream ID in stream publish info map",
                              err);
    }

    // need to mute all the video and audio tracks
    auto mute_video = MediaStreamTypeToMuteVideoBool(stream_type);
    auto mute_audio = MediaStreamTypeToMuteAudioBool(stream_type);

    const auto& stream_publish_info = iter->second;

    if (mute_video && stream_publish_info.video_publish_info) {
      auto video_publish_info = stream_publish_info.video_publish_info;

      /*if (UnpublishLocalMediaTrackImpl(stream_id, video_publish_info->track)) {
        video_publish_info->rtc_published = false;
      } else {
        LOG_ERR("failed to unpublish video track: %s", stream_id);
      }*/
    }

    if (mute_audio) {
      for (const auto& audio_publish_info : stream_publish_info.audio_publish_infos) {
        /*if (UnpublishLocalMediaTrackImpl(stream_id, audio_publish_info->track)) {
          audio_publish_info->rtc_published = false;
        } else {
          LOG_ERR("failed to unpublish audio track: %s", stream_id);
        }*/
      }
    }

    // inform RTE server
	//auto param = CreateDataParamForTrackCommon(stream_id);

	//if (mute_video) {
	//	param->AddInt(PARAM_VIDEO_STATE, 0);
	//}

	//if (mute_audio) {
	//	param->AddInt(PARAM_AUDIO_STATE, 0);
	//}

	//FetchUtility::CallPublishTrack(
	//	param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
	//	[=](bool success, std::shared_ptr<RestfulPublishTrackData> data) {
	//		if (!success) {
	//			LOG_ERR_AND_RET("MuteLocalMediaStream() FetchUtility::CallPublishTrack failed");
	//		}
	//	});

	return ERR_OK_;
	  });

  return err;
}

AgoraError RteLocalUser::UnmuteLocalMediaStream(StreamId stream_id, MediaStreamType stream_type) {
  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", LITE_STR_CONVERT(stream_id), stream_type);

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  if (!ValidMediaStreamType(stream_type)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid media stream type");
  }

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
    }

    // get stream publish info to get tracks
    auto iter = stream_publish_infos_map_.find(stream_id);
    if (iter == stream_publish_infos_map_.end()) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to find stream ID in stream publish info map",
                              err);
    }

    // need to unmute all the video and audio tracks
    auto unmute_video = MediaStreamTypeToMuteVideoBool(stream_type);
    auto unmute_audio = MediaStreamTypeToMuteAudioBool(stream_type);

    const auto& stream_publish_info = iter->second;

    if (unmute_video && stream_publish_info.video_publish_info) {
      const auto& video_publish_info = stream_publish_info.video_publish_info;

      /*if (PublishLocalMediaTrackImpl(video_publish_info->rtc_token, scene_->GetSceneUuid(),
                                     stream_id, video_publish_info->track)) {
        video_publish_info->rtc_published = true;
      } else {
        LOG_ERR("failed to publish video track: %s", stream_id);
      }*/
    }

    if (unmute_audio) {
      for (const auto& audio_publish_info : stream_publish_info.audio_publish_infos) {
        /*if (PublishLocalMediaTrackImpl(audio_publish_info->rtc_token, scene_->GetSceneUuid(),
                                       stream_id, audio_publish_info->track)) {
          audio_publish_info->rtc_published = true;
        } else {
          LOG_ERR("failed to publish audio track: %s", stream_id);
        }*/
      }
    }

    // inform RTE server
   /* auto param = CreateDataParamForTrackCommon(stream_id);

    if (unmute_video) {
      param->AddInt(PARAM_VIDEO_STATE, 1);
    }

    if (unmute_audio) {
      param->AddInt(PARAM_AUDIO_STATE, 1);
    }

    agora_refptr<RteLocalUser> shared_this = this;
    std::string stream_id_str(stream_id);

    FetchUtility::CallPublishTrack(
        param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
        [=](bool success, std::shared_ptr<RestfulPublishTrackData> data) {
          if (!success) {
            LOG_ERR_AND_RET("UnmuteLocalMediaStream() FetchUtility::CallPublishTrack failed");
          }
        });
*/
    return ERR_OK_;
  });

  return err;
}

AgoraError RteLocalUser::SubscribeRemoteStream(StreamId stream_id, MediaStreamType stream_type) {
//  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", LITE_STR_CONVERT(stream_id), stream_type);
//
//  if (utils::IsNullOrEmpty(stream_id)) {
//    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid stream ID");
//  }
//
//  if (!ValidMediaStreamType(stream_type)) {
//    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid media stream type");
//  }
//
//  AgoraError err;
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
//    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
//      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
//    }
//
//    // check whether need to continue to subscribe
//    auto remaining_stream_type = GetRemainingSubscribeStreamType(stream_id, stream_type);
//    if (remaining_stream_type == TYPE_NONE) {
//      LOG_INFO_SET_AND_RET_INT("all stream types already subscribed", err);
//    }
//
//    auto conn = scene_->GetDefaultConnection();
//    if (!conn) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get connection", err);
//    }
//
//    auto rtc_conn = conn->GetRtcConnection();
//    if (!rtc_conn) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get RTC connection", err);
//    }
//
//    auto local_user = rtc_conn->getLocalUser();
//    if (!local_user) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get RTC local user", err);
//    }
//
//#ifdef STRING_UID_WORKAROUND
//    // TODO(tomiao): sometimes may fail here if try to subscribe a remote stream
//    // right after starting the process
//    auto user_id = GetUserIdByStreamId(stream_id);
//    if (user_id.empty()) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get the mapping user ID for stream ID", err);
//    }
//#else
//    auto user_id = stream_id;
//#endif  // STRING_UID_WORKAROUND
//
//    switch (remaining_stream_type) {
//      case TYPE_AUDIO:
//        if (local_user->subscribeAudio(user_id.c_str()) != ERR_OK) {
//          LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to subscribe audio", err);
//        }
//        subscribed_audio_stream_ids_.insert(stream_id);
//        break;
//      case TYPE_VIDEO: {
//        rtc::ILocalUser::VideoSubscriptionOptions subs_options;
//        if (local_user->subscribeVideo(user_id.c_str(), subs_options) != ERR_OK) {
//          LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to subscribe video", err);
//        }
//        subscribed_video_stream_ids_.insert(stream_id);
//      } break;
//      case TYPE_AUDIO_VIDEO: {
//        if (local_user->subscribeAudio(user_id.c_str()) != ERR_OK) {
//          LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to subscribe audio", err);
//        }
//        subscribed_audio_stream_ids_.insert(stream_id);
//
//        rtc::ILocalUser::VideoSubscriptionOptions subs_options;
//        if (local_user->subscribeVideo(user_id.c_str(), subs_options) != ERR_OK) {
//          LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to subscribe video", err);
//        }
//        subscribed_video_stream_ids_.insert(stream_id);
//      } break;
//      default:
//        LOG_ERR_SET_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid remaining stream type", err);
//    }
//
//    std::string stream_id_str(stream_id);
//
//    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
//      event_handler->OnStreamSubscribeStateChanged(stream_id_str.c_str(), stream_type,
//                                                   SUB_STATE_SUBSCRIBED);
//    });
//
//    return ERR_OK_;
//  });
  //return err;
	return static_cast<AgoraError>(ERR_OK);
}

AgoraError RteLocalUser::UnsubscribeRemoteStream(StreamId stream_id, MediaStreamType stream_type) {
//  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", LITE_STR_CONVERT(stream_id), stream_type);
//
//  if (utils::IsNullOrEmpty(stream_id)) {
//    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid stream ID");
//  }
//
//  if (!ValidMediaStreamType(stream_type)) {
//    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid media stream type");
//  }
//
//  AgoraError err;
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
//    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
//      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
//    }
//
//    // check whether need to continue to unsubscribe
//    auto remaining_stream_type = GetRemainingUnsubscribeStreamType(stream_id, stream_type);
//    if (remaining_stream_type == TYPE_NONE) {
//      LOG_INFO_SET_AND_RET_INT("all stream types already unsubscribed", err);
//    }
//
//    auto conn = scene_->GetDefaultConnection();
//    if (!conn) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get connection", err);
//    }
//
//    auto rtc_conn = conn->GetRtcConnection();
//    if (!rtc_conn) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get RTC connection", err);
//    }
//
//    auto local_user = rtc_conn->getLocalUser();
//    if (!local_user) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get RTC local user", err);
//    }
//
//#ifdef STRING_UID_WORKAROUND
//    auto user_id = GetUserIdByStreamId(stream_id);
//    if (user_id.empty()) {
//      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get the mapping user ID for stream ID", err);
//    }
//#else
//    auto user_id = stream_id;
//#endif  // STRING_UID_WORKAROUND
//
//    // now we handle unsubscribe success in onUserAudioTrackStateChanged() and
//    // onUserVideoTrackStateChanged(), so no need to erase from subscribed_video_stream_ids_
//    // or subscribed_audio_stream_ids_ here
//    switch (remaining_stream_type) {
//      case TYPE_AUDIO:
//        if (local_user->unsubscribeAudio(user_id.c_str()) != ERR_OK) {
//          LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to unsubscribe audio", err);
//        }
//        break;
//      case TYPE_VIDEO:
//        if (local_user->unsubscribeVideo(user_id.c_str()) != ERR_OK) {
//          LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to unsubscribe video", err);
//        }
//        break;
//      case TYPE_AUDIO_VIDEO:
//        if (local_user->unsubscribeAudio(user_id.c_str()) != ERR_OK) {
//          LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to unsubscribe audio", err);
//        }
//
//        if (local_user->unsubscribeVideo(user_id.c_str()) != ERR_OK) {
//          LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to unsubscribe video", err);
//        }
//        break;
//      default:
//        LOG_ERR_SET_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid remaining stream type", err);
//    }
//
//    std::string stream_id_str(stream_id);
//
//    event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
//      event_handler->OnStreamSubscribeStateChanged(stream_id_str.c_str(), stream_type,
//                                                   SUB_STATE_NO_SUBSCRIBE);
//    });
//
//    return ERR_OK_;
//  });
//
//  return err;
	return static_cast<AgoraError>(ERR_OK);
}

AgoraError RteLocalUser::SetUserProperties(UserId user_id, const KeyValPairCollection& properties,
                                           bool remove, const char* json_cause,
                                           const char* operate_id) {
  API_LOGGER_MEMBER("user_id: %s, properties: %p, remove: %s, json_cause: %s, operate_id: %s",
                    LITE_STR_CONVERT(user_id), &properties, BOOL_TO_STR(remove),
                    LITE_STR_CONVERT(json_cause), LITE_STR_CONVERT(operate_id));

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid user ID");
  }

  if (0 == properties.count) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid properties");
  }

  auto properties_str = ConstructPropertiesStr(properties, json_cause);

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
    }

    // prepare fetch parameters
    auto param = CreateDataParamForProperties(properties_str, remove);
    param->AddString(PARAM_USER_UUID, user_id);

    agora_refptr<RteLocalUser> shared_this = this;
    std::string operate_id_str(LITE_STR_CAST(operate_id));

    // fetch
    FetchUtility::CallSetUserProperties(
        param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
        [=](bool success, std::shared_ptr<RestfulResponseBase> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=]() {
            if (!success) {
              shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
                event_handler->OnSetUserPropertiesCompleted(
                    operate_id_str.c_str(),
                    AgoraError(ERR_FAILED, "FetchUtility::CallSetUserProperties failed"));
              });

              LOG_ERR_AND_RET_INT(ERR_FAILED, "FetchUtility::CallSetUserProperties failed");
            }

            shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
              event_handler->OnSetUserPropertiesCompleted(
                  operate_id_str.c_str(),
                  AgoraError(ERR_OK, "FetchUtility::CallSetUserProperties succeeded"));
            });

            return ERR_OK_;
          });
        });

    return ERR_OK_;
  });

  return err;
	
}

AgoraError RteLocalUser::SetSceneProperties(const KeyValPairCollection& properties, bool remove,
                                            const char* json_cause, const char* operate_id) {
  API_LOGGER_MEMBER("properties: %p, remove: %s, json_cause: %s, operate_id: %s", &properties,
                    BOOL_TO_STR(remove), LITE_STR_CONVERT(json_cause),
                    LITE_STR_CONVERT(operate_id));

  if (0 == properties.count) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid properties");
  }

  auto properties_str = ConstructPropertiesStr(properties, json_cause);

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    if (scene_->GetConnState() != CONN_STATE_CONNECTED) {
      LOG_ERR_SET_AND_RET_INT(ERR_NOT_READY, "RTE scene not connected", err);
    }

    // prepare fetch parameters
    auto param = CreateDataParamForProperties(properties_str, remove);

    agora_refptr<RteLocalUser> shared_this = this;
    std::string operate_id_str(LITE_STR_CAST(operate_id));

    // fetch
    FetchUtility::CallSetSceneProperties(
        param, data_transfer_method_.data_request_type, scene_->GetDataWorker(),
        [=](bool success, std::shared_ptr<RestfulResponseBase> data) {
          (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=]() {
            if (!success) {
              shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
                event_handler->OnSetScenePropertiesCompleted(
                    operate_id_str.c_str(),
                    AgoraError(ERR_FAILED, "FetchUtility::CallSetSceneProperties failed"));
              });

              LOG_ERR_AND_RET_INT(ERR_FAILED, "FetchUtility::CallSetSceneProperties failed");
            }

            shared_this->event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
              event_handler->OnSetScenePropertiesCompleted(
                  operate_id_str.c_str(),
                  AgoraError(ERR_OK, "FetchUtility::CallSetSceneProperties succeeded"));
            });

            return ERR_OK_;
          });
        });

    return ERR_OK_;
  });

  return err;
}

AgoraError RteLocalUser::SetRemoteStreamView(StreamId stream_id, View view) {
  API_LOGGER_MEMBER("stream_id: %s, view: %p", LITE_STR_CONVERT(stream_id), view);

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    auto uid = GetUidByStreamId(stream_id);
    if (0 == uid) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get the mapping uid for stream ID", err);
    }

    //if (!remote_track_mgr_.setRemoteView({uid, 0 /* track ID */}, view)) {
    //  LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to set remote view", err);
    //}

    return ERR_OK_;
  });

  return err;
}

AgoraError RteLocalUser::SetRemoteStreamRenderMode(StreamId stream_id,
                                                   media::base::RENDER_MODE_TYPE mode) {
  API_LOGGER_MEMBER("stream_id: %s, mode: %d", LITE_STR_CONVERT(stream_id), mode);

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  AgoraError err;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=, &err] {
    auto uid = GetUidByStreamId(stream_id);
    if (0 == uid) {
      LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to get the mapping uid for stream ID", err);
    }

    //if (remote_track_mgr_.setRemoteRenderMode({uid, 0 /* track ID */}, mode) != ERR_OK) {
    //  LOG_ERR_SET_AND_RET_INT(ERR_FAILED, "failed to set remote render mode", err);
    //}

    return ERR_OK_;
  });

  return err;
}

void RteLocalUser::RegisterEventHandler(IAgoraLocalUserEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in RegisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Register(event_handler);
    return ERR_OK;
  });
}

void RteLocalUser::UnregisterEventHandler(IAgoraLocalUserEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in UnregisterEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    event_handlers_->Unregister(event_handler);
    return ERR_OK;
  });
}

void RteLocalUser::RegisterMediaStreamEventHandler(IAgoraMediaStreamEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in RegisterMediaStreamEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    media_stream_event_handlers_->Register(event_handler);
    return ERR_OK;
  });
}

void RteLocalUser::UnregisterMediaStreamEventHandler(IAgoraMediaStreamEventHandler* event_handler) {
  API_LOGGER_MEMBER("event_handler: %p", event_handler);

  if (!event_handler) {
    LOG_ERR_AND_RET("nullptr event_handler in UnregisterMediaStreamEventHandler()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    media_stream_event_handlers_->Unregister(event_handler);
    return ERR_OK;
  });
}

void RteLocalUser::RegisterVideoFrameObserver(IVideoFrameObserver* observer) {
  API_LOGGER_MEMBER("observer: %p", observer);

  if (!observer) {
    LOG_ERR_AND_RET("nullptr observer");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    //rte_video_frame_observer_ = observer;
    return ERR_OK;
  });
}

void RteLocalUser::UnregisterVideoFrameObserver() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    //rte_video_frame_observer_ = nullptr;
    return ERR_OK;
  });
}

void RteLocalUser::OnLocalStreamChanged(const MediaStreamInfoWithOperator& stream_info_with_op,
                                        MediaStreamAction action) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_info_with_op: %p, action: %d", &stream_info_with_op, action);

  OnLocalStreamChangedImpl(stream_info_with_op, action);

  // separate the main logic into OnLocalStreamChangedImpl() to make sure always post the event here
  event_handlers_->Post(LOCATION_HERE, [=](auto event_handler) {
    event_handler->OnLocalStreamChanged(stream_info_with_op, action);
  });
}

void RteLocalUser::OnLocalStreamChangedImpl(const MediaStreamInfoWithOperator& stream_info_with_op,
                                            MediaStreamAction action) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_info_with_op: %p, action: %d", &stream_info_with_op, action);

  // deal with this local stream first
  auto user_id = scene_->GetUserUuid();
  std::string operate_id = stream_info_with_op.operator_user.user_id;

  const auto& stream_info = stream_info_with_op.stream_info;

  auto stream_id = stream_info.stream_id;
  auto video_source_type = stream_info.video_source_type;
  auto audio_source_type = stream_info.audio_source_type;
  auto stream_type = stream_info.stream_type;

  WIN_DBG_OUT("[RTE.DEBUG] OnLocalStreamChanged " << user_id << ": " << stream_info.stream_id
                                                  << " action: " << action
                                                  << " stream_type: " << stream_type);

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET("invalid stream ID in OnLocalStreamChanged()");
  }

  // TODO(tomiao): don't check now, may fail in case TYPE_AUDIO or TYPE_VIDEO since MediaStreamType
  // might mean MediaStreamState here, should enable back later after server side changes
  // accordingly
#if 0
  // check all the types together first to make the logic simpler
  if (!ValidSourceAndStreamTypes(video_source_type, audio_source_type, stream_type)) {
    LOG_ERR_AND_RET("invalid source and stream types");
  }
#else
  if (!ValidSourceTypes(video_source_type, audio_source_type)) {
    LOG_ERR_AND_RET("invalid source types");
  }
#endif  // 0

  // 1. Operate ID is valid.
  // 2. Operate ID is not equal to current user ID.
  // 3. This is an 'update' action.
  // 4. Media stream type is valid (at least one audio or one video is required).
  // Create media track and publish if haven't
  /*if (!operate_id.empty() && operate_id != user_id &&
      (action == STREAM_ADDED || action == STREAM_UPDATED) && ValidMediaStreamType(stream_type) &&
      !CreateAndPublishStreamsIfHavent(stream_id, video_source_type, audio_source_type,
                                       stream_type)) {
    LOG_ERR_AND_RET("failed to create and publish streams if haven't");
  }*/

  // similar logic with MuteLocalMediaStream() / UnmuteLocalMediaStream()
  auto iter = stream_publish_infos_map_.find(stream_id);
  if (iter == stream_publish_infos_map_.end()) {
    // if action is not removed, we should be able to find the stream in the stream publish info map
    if (action == STREAM_REMOVED) {
      // do nothing
      return;
    } else {
      LOG_ERR_AND_RET("failed to find stream ID %s in stream publish info map", stream_id);
    }
  }

  const auto& stream_publish_info = iter->second;

  switch (action) {
    // if action is added or updated, should publish the stream
    case STREAM_ADDED:
    case STREAM_UPDATED: {
      // video
      if (ValidVideoMediaStreamType(stream_type)) {
        // video publish info should exist and should publish
        auto video_publish_info = stream_publish_info.video_publish_info;
        if (!video_publish_info) {
          // do not return but continue with below checks
          LOG_ERR("failed to find video publish info: %s", stream_id);
        } else if (!video_publish_info->rtc_published) {
         /* if (PublishLocalMediaTrackImpl(video_publish_info->rtc_token, scene_->GetSceneUuid(),
                                         stream_id, video_publish_info->track)) {
            video_publish_info->rtc_published = true;
          } else {
            LOG_ERR("failed to publish video track: %s", stream_id);
          }*/
        }
      } else {
        // video publish info may exist and should unpublish
        auto video_publish_info = stream_publish_info.video_publish_info;
        if (video_publish_info && video_publish_info->rtc_published) {
			/* if (UnpublishLocalMediaTrackImpl(stream_id, video_publish_info->track)) {
			   video_publish_info->rtc_published = false;
			 } else {
			   LOG_ERR("failed to unpublish video track: %s", stream_id);
			 }*/
        }
      }

      // audio
      if (ValidAudioMediaStreamType(stream_type)) {
        // audio publish infos shouldn't be empty and should pubsh
        if (stream_publish_info.audio_publish_infos.empty()) {
          // do not return but continue with below checks
          LOG_ERR("failed to find audio publish infos: %s", stream_id);
        } else {
          for (const auto& audio_publish_info : stream_publish_info.audio_publish_infos) {
            if (!audio_publish_info->rtc_published) {
				/*if (PublishLocalMediaTrackImpl(audio_publish_info->rtc_token, scene_->GetSceneUuid(),
											   stream_id, audio_publish_info->track)) {
				  audio_publish_info->rtc_published = true;
				} else {
				  LOG_ERR("failed to publish audio track: %s", stream_id);
				}*/
            }
          }
        }
      } else {
        // audio publish infos may be empty and should unpublish
        for (const auto& audio_publish_info : stream_publish_info.audio_publish_infos) {
          if (audio_publish_info->rtc_published) {
           /* if (UnpublishLocalMediaTrackImpl(stream_id, audio_publish_info->track)) {
              audio_publish_info->rtc_published = false;
            } else {
              LOG_ERR("failed to unpublish audio track: %s", stream_id);
            }*/
          }
        }
      }
    } break;
    case STREAM_REMOVED: {
      // video
      auto video_publish_info = stream_publish_info.video_publish_info;
      // video publish info should exist
      if (ValidVideoMediaStreamType(stream_type) && !video_publish_info) {
        // do not return but continue with below checks
        LOG_ERR("failed to find video publish info: %s", stream_id);
      }

      // unpublish anyway
      if (video_publish_info && video_publish_info->rtc_published) {
        /*if (UnpublishLocalMediaTrackImpl(stream_id, video_publish_info->track)) {
          video_publish_info->rtc_published = false;
        } else {
          LOG_ERR("failed to unpublish video track: %s", stream_id);
        }*/
      }

      // audio
      // audio publish infos shouldn't be empty
      if (ValidAudioMediaStreamType(stream_type) &&
          stream_publish_info.audio_publish_infos.empty()) {
        // do not return but continue with below checks
        LOG_ERR("failed to find audio publish infos: %s", stream_id);
      }

      // unpublish anyway
      for (const auto& audio_publish_info : stream_publish_info.audio_publish_infos) {
        if (audio_publish_info->rtc_published) {
          /*if (UnpublishLocalMediaTrackImpl(stream_id, audio_publish_info->track)) {
            audio_publish_info->rtc_published = false;
          } else {
            LOG_ERR("failed to unpublish audio track: %s", stream_id);
          }*/
        }
      }
    } break;
    default:
      LOG_ERR_AND_RET("invalid stream action");
  }
}

void RteLocalUser::OnLocalUserUpdated(const RtmUserPropertiesChange& changes) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("changes: %p", &changes);

  if (changes.changed_properties.empty()) {
    LOG_ERR_AND_RET("empty changed properties in OnLocalUserUpdated()");
  }

  event_handlers_->Post(LOCATION_HERE, [changes](auto event_handler) {
    std::vector<std::string> changed_keys;
    for (const auto& changed_property : changes.changed_properties) {
      changed_keys.emplace_back(changed_property.first);
    }

    std::vector<const char*> changed_key_addrs;
    for (const auto& changed_key : changed_keys) {
      changed_key_addrs.emplace_back(changed_key.c_str());
    }

    UserInfo user_info;
    CopyRemoteUserData2UserInfo(user_info, changes.from_user);

    // TODO(tomiao): why 'action == 2' means 'properties_remove', need to add a macro for this 2
    // and also replace in rte_scene.cpp
    event_handler->OnLocalUserUpdated(&user_info, &changed_key_addrs[0], changed_key_addrs.size(),
                                      (changes.action == 2), changes.cause.c_str());
  });
}

void RteLocalUser::OnRemoteStreamsRemoved(const std::vector<MediaStreamInfoWithOperator>& streams) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("streams: %p", &streams);

  for (const auto& stream_info_with_operator : streams) {
    const auto& stream_info = stream_info_with_operator.stream_info;

    auto stream_id = stream_info.stream_id;
    auto stream_type = stream_info.stream_type;

    // TODO(tomiao): don't check now, may fail in case TYPE_AUDIO or TYPE_VIDEO since
    // MediaStreamType might mean MediaStreamState here, should enable back later after server side
    // changes accordingly
#if 0
    auto video_source_type = stream_info.video_source_type;
    auto audio_source_type = stream_info.audio_source_type;

    if (!ValidSourceAndStreamTypes(video_source_type, audio_source_type, stream_type)) {
      LOG_ERR_AND_RET_AGORA(ERR_INVALID_ARGUMENT, "invalid source and stream types");
    }
#endif  // 0

    // will check stream type and whether subscribed internally
    if (!UnsubscribeRemoteStream(stream_id, stream_type).ok()) {
      LOG_ERR_AND_RET("failed to unsubscribe remote stream: %s", stream_id);
    }
  }
}

bool RteLocalUser::UpdateStreamIdMaps(StreamId stream_id, rtc::uid_t uid) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_id: %s, uid: %u", LITE_STR_CONVERT(stream_id), uid);

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_BOOL("invalid stream ID in UpdateStreamIdMaps()");
  }

  if (0 == uid) {
    LOG_ERR_AND_RET_BOOL("zero uid in UpdateStreamIdMaps()");
  }

  stream_id_to_uid_map_[stream_id] = uid;
  uid_to_stream_id_map_[uid] = stream_id;

  return true;
}

rtc::uid_t RteLocalUser::GetUidByStreamId(StreamId stream_id) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_id: %s", LITE_STR_CONVERT(stream_id));

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_ZERO("invalid stream ID in GetUidByStreamId()");
  }

  if (stream_id_to_uid_map_.find(stream_id) != stream_id_to_uid_map_.end()) {
    return stream_id_to_uid_map_[stream_id];
  }

  return 0;
}

#ifdef STRING_UID_WORKAROUND
internal_user_id_t RteLocalUser::GetUserIdByStreamId(StreamId stream_id) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_id: %s", LITE_STR_CONVERT(stream_id));

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_STR("invalid stream ID in GetUserIdByStreamId()");
  }

  auto uid = GetUidByStreamId(stream_id);
  if (0 == uid) {
    LOG_ERR_AND_RET_STR("failed to get uid in GetUserIdByStreamId()");
  }

  return rtc::UserIdManagerImpl::convertInternalUid(uid);
}

std::string RteLocalUser::GetStreamIdByUserId(user_id_t user_id) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("user_id: %s", LITE_STR_CONVERT(user_id));

  if (utils::IsNullOrEmpty(user_id)) {
    LOG_ERR_AND_RET_STR("invalid user ID in GetStreamIdByUserId()");
  }

  auto uid = rtc::UserIdManagerImpl::convertUserId(user_id);
  if (0 == uid) {
    LOG_ERR_AND_RET_STR("failed to get uid in GetStreamIdByUserId()");
  }

  if (uid_to_stream_id_map_.find(uid) != uid_to_stream_id_map_.end()) {
    return uid_to_stream_id_map_[uid];
  }

  LOG_ERR_AND_RET_STR("failed to get stream ID in GetStreamIdByUserId()");
}
#endif  // STRING_UID_WORKAROUND

// local audio track
//void RteLocalUser::onAudioTrackPublishSuccess(agora_refptr<rtc::ILocalAudioTrack> audio_track) {
//  API_LOGGER_CALLBACK(onAudioTrackPublishSuccess, "audio_track: %p", audio_track.get());
//
//  if (!audio_track) {
//    LOG_ERR_AND_RET("nullptr audio track in onAudioTrackPublishSuccess()");
//  }
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//    if (published_audio_track_map_.find(audio_track) == published_audio_track_map_.end()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to find audio track in onAudioTrackPublishSuccess()");
//    }
//
//    // currently we do nothing for these RTC callbacks
//#if 0
//    auto stream_id = published_audio_track_map_[audio_track];
//
//    event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
//      event_handler->OnAudioStreamPublishStateChanged(stream_id.c_str(), PUB_STATE_PUBLISHED);
//    });
//#endif  // 0
//
//    return ERR_OK_;
//  });
//}

#if 0
// TODO(tomiao): no one will call this function including Audio Stream Mgr
void RteLocalUser::onAudioTrackPublicationFailure(agora_refptr<rtc::ILocalAudioTrack> audio_track,
                                                  ERROR_CODE_TYPE error) {
  API_LOGGER_CALLBACK(onAudioTrackPublicationFailure, "audio_track: %p, error: %d",
                      audio_track.get(), error);

  if (!audio_track) {
    LOG_ERR_AND_RET("nullptr audio track in onAudioTrackPublicationFailure()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    auto iter = published_audio_track_map_.find(audio_track);
    if (iter == published_audio_track_map_.end()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED,
                          "failed to find audio track in onAudioTrackPublicationFailure()");
    }

    auto stream_id = iter->second;
    published_audio_track_map_.erase(iter);
    published_audio_stream_ids_.erase(stream_id);

    event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
      event_handler->OnAudioStreamPublishStateChanged(stream_id.c_str(), PUB_STATE_NO_PUBLISH);
    });

    return ERR_OK_;
  });
}
#endif  // 0

//void RteLocalUser::onLocalAudioTrackStateChanged(agora_refptr<rtc::ILocalAudioTrack> audio_track,
//                                                 rtc::LOCAL_AUDIO_STREAM_STATE state,
//                                                 rtc::LOCAL_AUDIO_STREAM_ERROR err_code) {
//  API_LOGGER_CALLBACK(onLocalAudioTrackStateChanged, "audio_track: %p, state: %d, err_code: %d",
//                      audio_track.get(), state, err_code);
//
//  if (!audio_track) {
//    LOG_ERR_AND_RET("nullptr audio track in onLocalAudioTrackStateChanged()");
//  }
//
//  if (!ValidLocalAudioStreamState(state)) {
//    LOG_WARN_AND_RET("ignore local audio stream state in onLocalAudioTrackStateChanged()");
//  }
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//    auto iter = published_audio_track_map_.find(audio_track);
//    if (iter == published_audio_track_map_.end()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "failed to find audio track in onLocalAudioTrackStateChanged()");
//    }
//
//    auto stream_id = iter->second;
//
//    // media stream event handler first
//    auto stream_state = ConvertLocalAudioStreamState(state);
//    if (stream_state == STREAM_STATE_UNKNOWN) {
//      LOG_ERR_AND_RET_INT(
//          ERR_FAILED,
//          "failed to convert local audio stream state in onLocalAudioTrackStateChanged()");
//    }
//
//    media_stream_event_handlers_->Post(
//        LOCATION_HERE, [stream_id, stream_state](auto event_handler) {
//          event_handler->OnLocalAudioStreamStateChanged(stream_id.c_str(), stream_state);
//        });
//
//    // when publish succeeded, onAudioTrackPublishSuccess() will be called back,
//    // when publish failed or unpublish succeeded/failed, this function will be called
//    // so we only need to handle publish/unpublish related callbacks when in stopped or
//    // failed state
//    //
//    // TODO(tomiao): need to distinguish whether current user's operation is publish or unpublish,
//    // then call corresponding callbacks to outside observers
//    if (state == rtc::LOCAL_AUDIO_STREAM_STATE_STOPPED ||
//        state == rtc::LOCAL_AUDIO_STREAM_STATE_FAILED) {
//      published_audio_track_map_.erase(iter);
//      published_audio_stream_ids_.erase(stream_id);
//
//      // currently we do nothing for these RTC callbacks
//#if 0
//      event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
//        event_handler->OnAudioStreamPublishStateChanged(stream_id.c_str(), PUB_STATE_NO_PUBLISH);
//      });
//#endif  // 0
//    }
//
//    return ERR_OK_;
//  });
//}

// remote audio track
//void RteLocalUser::onUserAudioTrackSubscribed(user_id_t user_id,
//                                              agora_refptr<rtc::IRemoteAudioTrack> audio_track) {
//  API_LOGGER_CALLBACK(onUserAudioTrackSubscribed, "user_id: %s, audio_track: %p",
//                      LITE_STR_CONVERT(user_id), audio_track.get());
//
//  if (utils::IsNullOrEmpty(user_id)) {
//    LOG_ERR_AND_RET("invalid user ID in onUserAudioTrackSubscribed()");
//  }
//
//  if (!audio_track) {
//    LOG_ERR_AND_RET("nullptr audio track in onUserAudioTrackSubscribed()");
//  }
//
//#if 0
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//#ifdef STRING_UID_WORKAROUND
//    auto stream_id = GetStreamIdByUserId(user_id);
//    if (stream_id.empty()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "failed to get stream ID by user ID in onUserAudioTrackSubscribed()");
//    }
//#else
//    auto stream_id = user_id;
//#endif  // STRING_UID_WORKAROUND
//
//    event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
//      event_handler->OnStreamSubscribeStateChanged(stream_id.c_str(), TYPE_AUDIO,
//                                                   SUB_STATE_SUBSCRIBED);
//    });
//
//    return ERR_OK_;
//  });
//#endif  // 0
//}

//void RteLocalUser::onUserAudioTrackStateChanged(user_id_t user_id,
//                                                agora_refptr<rtc::IRemoteAudioTrack> audio_track,
//                                                rtc::REMOTE_AUDIO_STATE state,
//                                                rtc::REMOTE_AUDIO_STATE_REASON reason,
//                                                int elapsed) {
//  API_LOGGER_CALLBACK(onUserAudioTrackStateChanged,
//                      "user_id: %s, audio_track: %p, state: %d, reason: %d, elapsed: %d",
//                      LITE_STR_CONVERT(user_id), audio_track.get(), state, reason, elapsed);
//
//  if (utils::IsNullOrEmpty(user_id)) {
//    LOG_ERR_AND_RET("invalid user ID in onUserAudioTrackStateChanged()");
//  }
//
//  if (!audio_track) {
//    LOG_ERR_AND_RET("nullptr audio track in onUserAudioTrackStateChanged()");
//  }
//
//  if (!ValidRemoteAudioState(state)) {
//    LOG_WARN_AND_RET("ignore remote audio state in onUserAudioTrackStateChanged()");
//  }
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//#ifdef STRING_UID_WORKAROUND
//    auto stream_id = GetStreamIdByUserId(user_id);
//    if (stream_id.empty()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "failed to get stream ID from user ID in onUserAudioTrackStateChanged()");
//    }
//#else
//    auto stream_id = user_id;
//#endif  // STRING_UID_WORKAROUND
//
//    // only need to check audio
//    if (!RemoteStreamSubscribedAccurately(stream_id.c_str(), TYPE_AUDIO)) {
//      LOG_ERR_AND_RET_INT(
//          ERR_FAILED, "failed to find subscribed audio stream in onUserAudioTrackStateChanged()");
//    }
//
//    // media stream event handler first
//    auto stream_state = ConvertRemoteAudioState(state);
//    if (stream_state == STREAM_STATE_UNKNOWN) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "failed to convert remote audio state in onUserAudioTrackStateChanged()");
//    }
//
//    media_stream_event_handlers_->Post(
//        LOCATION_HERE, [stream_id, stream_state](auto event_handler) {
//          event_handler->OnRemoteAudioStreamStateChanged(stream_id.c_str(), stream_state);
//        });
//
//    // for unsubscribe condition secondly
//    if (state == rtc::REMOTE_AUDIO_STATE_STOPPED || state == rtc::REMOTE_AUDIO_STATE_FAILED) {
//      if (!RemoveSubscribedRemoteStream(stream_id.c_str(), TYPE_AUDIO)) {
//        LOG_ERR_AND_RET_INT(
//            ERR_FAILED,
//            "failed to remove subscribed audio stream in onUserAudioTrackStateChanged()");
//      }
//
//#if 0
//      event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
//        event_handler->OnStreamSubscribeStateChanged(stream_id.c_str(), TYPE_AUDIO,
//                                                     SUB_STATE_NO_SUBSCRIBE);
//      });
//#endif  // 0
//    }
//
//    return ERR_OK_;
//  });
//}

// local video track
//void RteLocalUser::onVideoTrackPublishSuccess(agora_refptr<rtc::ILocalVideoTrack> video_track,
//                                              int elapsed) {
//  API_LOGGER_CALLBACK(onVideoTrackPublishSuccess, "video_track: %p, elapsed: %d", video_track.get(),
//                      elapsed);
//
//  LOG_INFO("onVideoTrackPublishSuccess(): video_track: %p, elapsed: %d", video_track.get(),
//           elapsed);
//
//  if (!video_track) {
//    LOG_ERR_AND_RET("nullptr video track in onVideoTrackPublishSuccess()");
//  }
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//    if (published_video_track_map_.find(video_track) == published_video_track_map_.end()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to find video track in onVideoTrackPublishSuccess()");
//    }
//
//    // currently we do nothing for these RTC callbacks
//#if 0
//    auto stream_id = published_video_track_map_[video_track];
//
//    event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
//      event_handler->OnVideoStreamPublishStateChanged(stream_id.c_str(), PUB_STATE_PUBLISHED);
//    });
//#endif  // 0
//
//    return ERR_OK_;
//  });
//}

#if 0
// TODO(tomiao): no one calls this function including Video Stream Mgr
void RteLocalUser::onVideoTrackPublicationFailure(agora_refptr<rtc::ILocalVideoTrack> video_track,
                                                  ERROR_CODE_TYPE error) {
  API_LOGGER_CALLBACK(onVideoTrackPublicationFailure, "video_track: %p, error: %d",
                      video_track.get(), error);

  LOG_INFO("onVideoTrackPublicationFailure(): video_track:%p, error: %d", video_track.get(), error);

  if (!video_track) {
    LOG_ERR_AND_RET("nullptr video track in onVideoTrackPublicationFailure()");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    auto iter = published_video_track_map_.find(video_track);
    if (iter == published_video_track_map_.end()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED,
                          "failed to find video track in onVideoTrackPublicationFailure()");
    }

    auto stream_id = iter->second;
    published_video_track_map_.erase(iter);
    published_video_stream_ids_.erase(stream_id);

    event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
      event_handler->OnVideoStreamPublishStateChanged(stream_id.c_str(), PUB_STATE_NO_PUBLISH);
    });

    return ERR_OK_;
  });
}
#endif  // 0

//void RteLocalUser::onLocalVideoTrackStateChanged(agora_refptr<rtc::ILocalVideoTrack> video_track,
//                                                 rtc::LOCAL_VIDEO_STREAM_STATE state,
//                                                 rtc::LOCAL_VIDEO_STREAM_ERROR err_code) {
//  API_LOGGER_CALLBACK(onLocalVideoTrackStateChanged, "video_track: %p, state: %d, err_code: %d",
//                      video_track.get(), state, err_code);
//
//  if (!video_track) {
//    LOG_ERR_AND_RET("nullptr video track in onLocalVideoTrackStateChanged()");
//  }
//
//  if (!ValidLocalVideoStreamState(state)) {
//    LOG_WARN_AND_RET("ignore local video stream state in onLocalVideoTrackStateChanged()");
//  }
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//    auto iter = published_video_track_map_.find(video_track);
//    if (iter == published_video_track_map_.end()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "failed to find video track in onLocalVideoTrackStateChanged()");
//    }
//
//    auto stream_id = iter->second;
//
//    // media stream event handler first
//    auto stream_state = ConvertLocalVideoStreamState(state);
//    if (stream_state == STREAM_STATE_UNKNOWN) {
//      LOG_ERR_AND_RET_INT(
//          ERR_FAILED,
//          "failed to convert local video stream state in onLocalAudioTrackStateChanged()");
//    }
//
//    media_stream_event_handlers_->Post(
//        LOCATION_HERE, [stream_id, stream_state](auto event_handler) {
//          event_handler->OnLocalVideoStreamStateChanged(stream_id.c_str(), stream_state);
//        });
//
//    // when publish succeeded, onVideoTrackPublishSuccess() will be called back,
//    // when publish failed or unpublish succeeded/failed, this function will be called
//    // so we only need to handle publish/unpublish related callbacks when in stopped or
//    // failed state
//    //
//    // TODO(tomiao): need to distinguish whether current user's operation is publish or unpublish,
//    // then call corresponding callbacks to outside observers
//    if (state == rtc::LOCAL_VIDEO_STREAM_STATE_STOPPED ||
//        state == rtc::LOCAL_VIDEO_STREAM_STATE_FAILED) {
//      published_video_track_map_.erase(iter);
//      published_video_stream_ids_.erase(stream_id);
//
//      // currently we do nothing for these RTC callbacks
//#if 0
//      event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
//        event_handler->OnVideoStreamPublishStateChanged(stream_id.c_str(), PUB_STATE_NO_PUBLISH);
//      });
//#endif  // 0
//    }
//
//    return ERR_OK_;
//  });
//}

// remote video track
//void RteLocalUser::onUserVideoTrackSubscribed(user_id_t user_id, rtc::VideoTrackInfo track_info,
//                                              agora_refptr<rtc::IRemoteVideoTrack> video_track) {
//  API_LOGGER_CALLBACK(onUserVideoTrackSubscribed,
//                      "user_id: %s, track_info: (ownerUid: %u, trackId: %u, connectionId: %u, "
//                      "streamType: %d, codecType: %d, encodedFrameOnly: %s), video_track: %p",
//                      LITE_STR_CONVERT(user_id), track_info.ownerUid, track_info.trackId,
//                      track_info.connectionId, track_info.streamType, track_info.codecType,
//                      BOOL_TO_STR(track_info.encodedFrameOnly), video_track.get());
//
//  if (utils::IsNullOrEmpty(user_id)) {
//    LOG_ERR_AND_RET("invalid user ID in onUserVideoTrackSubscribed()");
//  }
//
//  if (!video_track) {
//    LOG_ERR_AND_RET("nullptr video track in onUserVideoTrackSubscribed()");
//  }
//
//  auto uid = rtc::UserIdManagerImpl::convertUserId(user_id);
//  if (0 == uid) {
//    LOG_ERR_AND_RET("failed to get uid from user ID in onUserVideoTrackSubscribed()");
//  }
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//    // TODO(tomiao): change to real track ID when we support multiple tracks per user
//    //remote_track_mgr_.setRemoteTrackRenderer(uid, 0 /* track ID */, video_track,
//    //                                         media_node_factory_->createVideoRenderer());
//    //remote_track_mgr_.addRemoteVideoTrack(uid, video_track);
//
//#ifdef STRING_UID_WORKAROUND
//    auto stream_id = GetStreamIdByUserId(user_id);
//    if (stream_id.empty()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "failed to get stream ID from user ID in onUserVideoTrackSubscribed()");
//    }
//#else
//    auto stream_id = user_id;
//#endif  // STRING_UID_WORKAROUND
//
//#if 0
//    event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
//      event_handler->OnStreamSubscribeStateChanged(stream_id.c_str(), TYPE_VIDEO,
//                                                   SUB_STATE_SUBSCRIBED);
//    });
//#endif  // 0
//
//    // handle for video frame observer
//   // if (rte_video_frame_observer_) {
//   //   auto remote_video_frame_observer =
//   //       std::make_unique<RemoteVideoFrameObserver>(stream_id, rte_video_frame_observer_);
//
//	  ///*auto video_sink =
//		 // static_cast<rtc::IMediaNodeFactoryEx*>(media_node_factory_.get())
//			//  ->createObservableVideoSink(remote_video_frame_observer.get(), track_info);*/
//	  ////video_sink = nullptr;
//
//   //   /*if (!video_sink) {
//   //     LOG_ERR_AND_RET_INT(ERR_FAILED,
//   //                         "failed to create video sink in onUserVideoTrackSubscribed()");
//   //   }*/
//
//   //   /*if (!video_track->addRenderer(video_sink)) {
//   //     LOG_ERR_AND_RET_INT(ERR_FAILED,
//   //                         "failed to add video sink as renderer in onUserVideoTrackSubscribed()");
//   //   }*/
//
//   //   //remote_video_frame_observer_map_[stream_id] = std::move(remote_video_frame_observer);
//   //   //video_sink_map_[stream_id] = video_sink;
//   // }
//
//    return ERR_OK_;
//  });
//}

//void RteLocalUser::onUserVideoTrackStateChanged(user_id_t user_id,
//                                                agora_refptr<rtc::IRemoteVideoTrack> video_track,
//                                                rtc::REMOTE_VIDEO_STATE state,
//                                                rtc::REMOTE_VIDEO_STATE_REASON reason,
//                                                int elapsed) {
//  API_LOGGER_CALLBACK(onUserVideoTrackStateChanged,
//                      "user_id: %s, video_track: %p, state: %d, reason: %d, elapsed: %d",
//                      LITE_STR_CONVERT(user_id), video_track.get(), state, reason, elapsed);
//
//  if (utils::IsNullOrEmpty(user_id)) {
//    LOG_ERR_AND_RET("invalid user ID in onUserVideoTrackStateChanged()");
//  }
//
//  if (!video_track) {
//    LOG_ERR_AND_RET("nullptr video track in onUserVideoTrackStateChanged()");
//  }
//
//  auto uid = rtc::UserIdManagerImpl::convertUserId(user_id);
//  if (0 == uid) {
//    LOG_ERR_AND_RET("failed to get uid from user ID in onUserVideoTrackStateChanged()");
//  }
//
//  if (!ValidRemoteVideoState(state)) {
//    LOG_WARN_AND_RET("ignore remote video state in onUserVideoTrackStateChanged()");
//  }
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//    //remote_track_mgr_.removeRemoteVideoTrack(uid, video_track);
//
//#ifdef STRING_UID_WORKAROUND
//    auto stream_id = GetStreamIdByUserId(user_id);
//    if (stream_id.empty()) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "failed to get stream ID from user ID in onUserVideoTrackStateChanged()");
//    }
//#else
//    auto stream_id = user_id;
//#endif  // STRING_UID_WORKAROUND
//
//    // only need to check video
//    if (!RemoteStreamSubscribedAccurately(stream_id.c_str(), TYPE_VIDEO)) {
//      LOG_ERR_AND_RET_INT(
//          ERR_FAILED, "failed to find subscribed video stream in onUserVideoTrackStateChanged()");
//    }
//
//    // media stream event handler first
//    auto stream_state = ConvertRemoteVideoState(state);
//    if (stream_state == STREAM_STATE_UNKNOWN) {
//      LOG_ERR_AND_RET_INT(ERR_FAILED,
//                          "failed to convert remote video state in onUserVideoTrackStateChanged()");
//    }
//
//    media_stream_event_handlers_->Post(
//        LOCATION_HERE, [stream_id, stream_state](auto event_handler) {
//          event_handler->OnRemoteVideoStreamStateChanged(stream_id.c_str(), stream_state);
//        });
//
//    // for unsubscribe condition secondly
//    if (state == rtc::REMOTE_VIDEO_STATE_STOPPED || state == rtc::REMOTE_VIDEO_STATE_FAILED) {
//      if (!RemoveSubscribedRemoteStream(stream_id.c_str(), TYPE_VIDEO)) {
//        LOG_ERR_AND_RET_INT(
//            ERR_FAILED,
//            "failed to remove subscribed video stream in onUserVideoTrackStateChanged()");
//      }
//
//#if 0
//      event_handlers_->Post(LOCATION_HERE, [stream_id](auto event_handler) {
//        event_handler->OnStreamSubscribeStateChanged(stream_id.c_str(), TYPE_VIDEO,
//                                                     SUB_STATE_NO_SUBSCRIBE);
//      });
//#endif  // 0
//
//      // handle for video frame observer, shouldn't check for 'rte_video_frame_observer_' since it
//      // may be set to nullptr after some observers have been registered
//	 /* if (video_sink_map_.find(stream_id) != video_sink_map_.end()) {
//		auto video_sink = video_sink_map_[stream_id];
//
//		if (!video_track->removeRenderer(video_sink)) {
//		  LOG_ERR("failed to remove video sink as renderer in onUserVideoTrackStateChanged()");
//		}
//
//		video_sink_map_.erase(stream_id);
//	  }
//
//	  remote_video_frame_observer_map_.erase(stream_id);*/
//	}
//
//    return ERR_OK_;
//  });
//}

//void RteLocalUser::onAudioVolumeIndication(const rtc::AudioVolumeInfo* speakers,
//                                           unsigned int speaker_num, int total_vol) {
//  API_LOGGER_CALLBACK(onAudioVolumeIndication, "speakers: %p, speaker_num: %u, total_vol: %d",
//                      speakers, speaker_num, total_vol);
//
//  if (!speakers) {
//    // TODO(tomiao): too many logs, directly return temporarily
//    return;
//    // LOG_ERR_AND_RET("nullptr speakers in onAudioVolumeIndication()");
//  }
//
//  // TODO(tomiao): how to get stream ID and volume?
//#if 0
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
//    media_stream_event_handlers_->Post(LOCATION_HERE, [](auto event_handler) {
//      event_handler->OnAudioVolumeIndication(nullptr, 0);
//    });
//
//    return ERR_OK;
//  });
//#endif  // 0
//}

bool RteLocalUser::FindAudioSourceType(const std::vector<PublishInfoType>& audio_publish_infos,
                                       AudioSourceType audio_source_type) {
  for (const auto& audio_publish_info : audio_publish_infos) {
   /* if (audio_publish_info->track->GetAudioSourceType() == audio_source_type) {
      return true;
    }*/
  }

  return false;
}

std::string RteLocalUser::ConstructPropertiesStr(const KeyValPairCollection& properties,
                                                 const char* json_cause) {
  commons::cjson::JsonWrapper dict_cause;
  if (!utils::IsNullOrEmpty(json_cause)) {
    dict_cause.parse(json_cause);
    if (!dict_cause.isObject()) {
      LOG_ERR_AND_RET_STR("failed to parse JSON cause");
    }
  }

  commons::cjson::JsonWrapper json_kv;
  json_kv.setObjectType();
  for (int i = 0; i < properties.count; ++i) {
    const auto& kv = properties.key_vals[i];
    json_kv.setStringValue(kv.key, kv.val);
  }

  commons::cjson::JsonWrapper json;
  json.setObjectType();
  json.setObjectValue("properties", json_kv);

  if (dict_cause.isObject()) {
    json.setObjectValue("cause", dict_cause);
  }

  return json.toString();
}

//bool RteLocalUser::PublishLocalMediaTrackImpl(const std::string& rtc_token,
//                                              const std::string& scene_uuid, StreamId stream_id,
//                                              agora_refptr<IAgoraMediaTrack> track) {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("rtc_token: %s, scene_uuid: %s, stream_id: %s, track: %p", rtc_token.c_str(),
//                    scene_uuid.c_str(), LITE_STR_CONVERT(stream_id), track.get());
//
//  auto conn = scene_->GetConnection(rtc_token, scene_uuid, stream_id);
//  if (!conn) {
//    LOG_ERR_AND_RET_BOOL("failed to get connection");
//  }
//
//  auto local_user = conn->GetRtcLocalUser();
//  if (!local_user) {
//    LOG_ERR_AND_RET_BOOL("failed to get RTC local user");
//  }
//
//  // check source types
//  auto video_source_type = track->GetVideoSourceType();
//  auto audio_source_type = track->GetAudioSourceType();
//
//  if (!ValidSourceTypes(video_source_type, audio_source_type)) {
//    LOG_ERR_AND_RET_BOOL("invalid source types");
//  }
//
//  // try video first (either video or audio, will only enter one switch block)
//  switch (video_source_type) {
//    case TYPE_CAMERA: {
//		/* auto video_track = static_cast<CameraVideoTrack*>(track.get())->GetLocalVideoTrack();
//		 if (!video_track) {
//		   LOG_ERR_AND_RET_BOOL("failed to get local video track");
//		 }
//
//		 if (local_user->publishVideo(video_track) != ERR_OK) {
//		   LOG_ERR_AND_RET_BOOL("failed to publish video");
//		 }
//		 published_video_track_map_[video_track] = stream_id;
//		 published_video_stream_ids_.insert(stream_id);
//	   }*/ break;
//    case TYPE_SCREEN: {
//		/*auto video_track = static_cast<ScreenVideoTrack*>(track.get())->GetLocalVideoTrack();
//		if (!video_track) {
//		  LOG_ERR_AND_RET_BOOL("failed to get local video track");
//		}
//
//		if (local_user->publishVideo(video_track) != ERR_OK) {
//		  LOG_ERR_AND_RET_BOOL("failed to publish video");
//		}
//		published_video_track_map_[video_track] = stream_id;
//		published_video_stream_ids_.insert(stream_id);
//	  }*/ break;
//    default:
//      break;
//  }
//
//  // try audio next
//  switch (audio_source_type) {
//  case TYPE_MIC: /*{
//	auto audio_track = static_cast<MicrophoneAudioTrack*>(track.get())->GetLocalAudioTrack();
//	if (!audio_track) {
//	  LOG_ERR_AND_RET_BOOL("failed to get local audio track");
//	}
//
//	if (local_user->publishAudio(audio_track) != ERR_OK) {
//	  LOG_ERR_AND_RET_BOOL("failed to publish audio");
//	}
//	published_audio_track_map_[audio_track] = stream_id;
//	published_audio_stream_ids_.insert(stream_id);
//  }*/ break;
//    default:
//      break;
//  }
//
//  // Should start media track:
//  // 1. After RTC connection is ready while no need to be 'onConnected'.
//  // 2. After publish, since Video Stream Mgr will start to observer tracks in functions like
//  // doPublishVideo(), if we start tracks first, may miss some callbacks.
//  if (track->Start() != ERR_OK) {
//    LOG_ERR_AND_RET_BOOL("failed to start local media track");
//  }
//
//  return true;
//}
//
//bool RteLocalUser::UnpublishLocalMediaTrackImpl(StreamId stream_id,
//                                                agora_refptr<IAgoraMediaTrack> track) {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("stream_id: %s, track: %p", LITE_STR_CONVERT(stream_id), track.get());
//
//  if (track->Stop() != ERR_OK) {
//    LOG_ERR_AND_RET_BOOL("failed to stop local media track");
//  }
//
//  auto conn = scene_->GetConnection("", "", stream_id);
//  if (!conn) {
//    LOG_ERR_AND_RET_BOOL("failed to get connection");
//  }
//
//  auto local_user = conn->GetRtcLocalUser();
//  if (!local_user) {
//    LOG_ERR_AND_RET_BOOL("failed to get RTC local user");
//  }
//
//  // now we handle unpublish success in onLocalAudioTrackStateChanged() and
//  // onLocalVideoTrackStateChanged(), so no need to erase from published_audio_track_map_ or
//  // published_video_track_map_ here
//
//  // check source types
//  auto video_source_type = track->GetVideoSourceType();
//  auto audio_source_type = track->GetAudioSourceType();
//
//  if (!ValidSourceTypes(video_source_type, audio_source_type)) {
//    LOG_ERR_AND_RET_BOOL("invalid source types");
//  }
//
//  // try video first (either video or audio, will only enter one switch block)
//  switch (video_source_type) {
//    case TYPE_CAMERA: {
//      auto video_track = static_cast<CameraVideoTrack*>(track.get())->GetLocalVideoTrack();
//      if (!video_track) {
//        LOG_ERR_AND_RET_BOOL("failed to get local video track");
//      }
//
//      if (local_user->unpublishVideo(video_track) != ERR_OK) {
//        LOG_ERR_AND_RET_BOOL("failed to unpublish video");
//      }
//    } break;
//    case TYPE_SCREEN: {
//      auto video_track = static_cast<ScreenVideoTrack*>(track.get())->GetLocalVideoTrack();
//      if (!video_track) {
//        LOG_ERR_AND_RET_BOOL("failed to get local video track");
//      }
//
//      if (local_user->unpublishVideo(video_track) != ERR_OK) {
//        LOG_ERR_AND_RET_BOOL("failed to unpublish video");
//      }
//    } break;
//    default:
//      break;
//  }
//
//  // try audio next
//  switch (audio_source_type) {
//    case TYPE_MIC: {
//      auto audio_track = static_cast<MicrophoneAudioTrack*>(track.get())->GetLocalAudioTrack();
//      if (!audio_track) {
//        LOG_ERR_AND_RET_BOOL("failed to get local audio track");
//      }
//
//      if (local_user->unpublishAudio(audio_track) != ERR_OK) {
//        LOG_ERR_AND_RET_BOOL("failed to unpublish audio");
//      }
//    } break;
//    default:
//      break;
//  }
//
//  return true;
//}
////
////// ensured arguments
////bool RteLocalUser::CreateAndPublishStreamsIfHavent(StreamId stream_id,
////                                                   VideoSourceType video_source_type,
////                                                   AudioSourceType audio_source_type,
////                                                   MediaStreamType stream_type) {
////  ASSERT_IS_UI_THREAD();
////
////  API_LOGGER_MEMBER("stream_id: %s, video_source_type: %d, audio_source_type: %d, stream_type: %d",
////                    stream_id, video_source_type, audio_source_type, stream_type);
////
////  bool create_video = false;
////  bool create_audio = false;
////
////  auto iter = stream_publish_infos_map_.find(stream_id);
////  if (iter == stream_publish_infos_map_.end()) {
////    if (ValidVideoMediaStreamType(stream_type)) {
////      create_video = true;
////    }
////
////    if (ValidAudioMediaStreamType(stream_type)) {
////      create_audio = true;
////    }
////  } else {
////    const auto& stream_publish_info = iter->second;
////
////    if (ValidVideoMediaStreamType(stream_type) && !stream_publish_info.video_publish_info) {
////      create_video = true;
////    }
////
////    if (ValidAudioMediaStreamType(stream_type) &&
////        !FindAudioSourceType(stream_publish_info.audio_publish_infos, audio_source_type)) {
////      create_audio = true;
////    }
////  }
////
////  if (create_video) {
////    switch (video_source_type) {
////      case TYPE_CAMERA: {
////        auto camera_track = media_control_->CreateCameraVideoTrack();
////        if (camera_track) {
////          if (!PublishLocalMediaTrack(camera_track, "").ok()) {
////            LOG_ERR("failed to publish camera track");
////          }
////        } else {
////          LOG_ERR("failed to create camera track");
////        }
////      } break;
////      case TYPE_SCREEN: {
////        auto screen_track = media_control_->CreateScreenVideoTrack();
////        if (screen_track) {
////          if (!PublishLocalMediaTrack(screen_track, "").ok()) {
////            LOG_ERR("failed to publish screen track");
////          }
////        } else {
////          LOG_ERR("failed to create screen track");
////        }
////      } break;
////      default:
////        assert(false);
////        return false;
////    }
////  }
////
////  if (create_audio) {
////    switch (audio_source_type) {
////      case TYPE_MIC: {
////        auto microphone_track = media_control_->CreateMicrophoneAudioTrack();
////        if (microphone_track) {
////          if (!PublishLocalMediaTrack(microphone_track, "").ok()) {
////            LOG_ERR("failed to publish microphone track");
////          }
////        } else {
////          LOG_ERR("failed to create microphone track");
////        }
////      } break;
////      default:
////        assert(false);
////        return false;
////    }
////  }
////
////  return true;
////}
//
//#if 0
//// ensured arguments
//bool RteLocalUser::LocalStreamPublished(StreamId stream_id, VideoSourceType video_source_type,
//                                        AudioSourceType audio_source_type) {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("stream_id: %s, video_source_type: %d, audio_source_type: %d", stream_id,
//                    video_source_type, audio_source_type);
//
//  if (ValidVideoSourceType(video_source_type)) {
//    return (published_video_stream_ids_.find(stream_id) != published_video_stream_ids_.end());
//  } else {
//    return (published_audio_stream_ids_.find(stream_id) != published_audio_stream_ids_.end());
//  }
//}
//
//// ensured arguments
//bool RteLocalUser::LocalStreamPublished(StreamId stream_id, MediaStreamType stream_type) {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", stream_id, stream_type);
//
//  switch (stream_type) {
//    case TYPE_AUDIO:
//      return (published_audio_stream_ids_.find(stream_id) != published_audio_stream_ids_.end());
//    case TYPE_VIDEO:
//      return (published_video_stream_ids_.find(stream_id) != published_video_stream_ids_.end());
//    case TYPE_AUDIO_VIDEO:
//      // need to check both audio and video
//      return (published_audio_stream_ids_.find(stream_id) != published_audio_stream_ids_.end() &&
//              published_video_stream_ids_.find(stream_id) != published_video_stream_ids_.end());
//    default:
//      assert(false);
//      return false;
//  }
//}
//#endif  // 0
//
//// ensured arguments
bool RteLocalUser::RemoteStreamSubscribedAccurately(StreamId stream_id,
                                                    MediaStreamType stream_type) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", stream_id, stream_type);

  switch (stream_type) {
    case TYPE_AUDIO:
      return (subscribed_audio_stream_ids_.find(stream_id) != subscribed_audio_stream_ids_.end());
    case TYPE_VIDEO:
      return (subscribed_video_stream_ids_.find(stream_id) != subscribed_video_stream_ids_.end());
    case TYPE_AUDIO_VIDEO:
      // need to check both audio and video
      return (subscribed_audio_stream_ids_.find(stream_id) != subscribed_audio_stream_ids_.end() &&
              subscribed_video_stream_ids_.find(stream_id) != subscribed_video_stream_ids_.end());
    default:
      assert(false);
      return false;
  }
}
//
//#if 0
//// ensured arguments
//bool RteLocalUser::RemoteStreamSubscribedPartially(StreamId stream_id,
//                                                   MediaStreamType stream_type) {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", stream_id, stream_type);
//
//  switch (stream_type) {
//    case TYPE_AUDIO:
//      return (subscribed_audio_stream_ids_.find(stream_id) != subscribed_audio_stream_ids_.end());
//    case TYPE_VIDEO:
//      return (subscribed_video_stream_ids_.find(stream_id) != subscribed_video_stream_ids_.end());
//    case TYPE_AUDIO_VIDEO:
//      // need to check both audio and video
//      return (subscribed_audio_stream_ids_.find(stream_id) != subscribed_audio_stream_ids_.end() ||
//              subscribed_video_stream_ids_.find(stream_id) != subscribed_video_stream_ids_.end());
//    default:
//      assert(false);
//      return false;
//  }
//}
//#endif  // 0
//
//// ensured arguments
MediaStreamType RteLocalUser::GetRemainingSubscribeStreamType(StreamId stream_id,
                                                              MediaStreamType stream_type) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", stream_id, stream_type);

  // accurate match, no more subscription needed
  if (RemoteStreamSubscribedAccurately(stream_id, stream_type)) {
    return TYPE_NONE;
  }

  switch (stream_type) {
    // audio only or video only, since not subscribed yet, just return as it is
    case TYPE_AUDIO:
    case TYPE_VIDEO:
      return stream_type;
    // audio and video, return the missing one
    case TYPE_AUDIO_VIDEO:
      return (subscribed_audio_stream_ids_.find(stream_id) == subscribed_audio_stream_ids_.end()
                  ? TYPE_AUDIO
                  : TYPE_VIDEO);
    default:
      assert(false);
      return TYPE_NONE;
  }
}
//
//// ensured arguments
MediaStreamType RteLocalUser::GetRemainingUnsubscribeStreamType(StreamId stream_id,
                                                                MediaStreamType stream_type) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", stream_id, stream_type);

  // accurate match, all should be unsubscribed
  if (RemoteStreamSubscribedAccurately(stream_id, stream_type)) {
    return stream_type;
  }

  switch (stream_type) {
    // audio only or video only, since not subscribed yet, no need to unsubscribe
    case TYPE_AUDIO:
    case TYPE_VIDEO:
      return TYPE_NONE;
    // audio and video, unsubscribe the subscribed one
    case TYPE_AUDIO_VIDEO:
      return (subscribed_audio_stream_ids_.find(stream_id) != subscribed_audio_stream_ids_.end()
                  ? TYPE_AUDIO
                  : TYPE_VIDEO);
    default:
      assert(false);
      return TYPE_NONE;
  }
}
//
//// ensured arguments
bool RteLocalUser::RemoveSubscribedRemoteStream(StreamId stream_id, MediaStreamType stream_type) {
  ASSERT_IS_UI_THREAD();

  API_LOGGER_MEMBER("stream_id: %s, stream_type: %d", stream_id, stream_type);

  switch (stream_type) {
    case TYPE_AUDIO:
      return (subscribed_audio_stream_ids_.erase(stream_id) > 0);
    case TYPE_VIDEO:
      return (subscribed_video_stream_ids_.erase(stream_id) > 0);
    case TYPE_AUDIO_VIDEO:
      // need to erase from both audio and video
      return (subscribed_audio_stream_ids_.erase(stream_id) > 0 &&
              subscribed_video_stream_ids_.erase(stream_id) > 0);
    default:
      assert(false);
      return false;
  }
}
//
//RteLocalUser::PublishInfoPairType RteLocalUser::AddPublishInfo(
//    const std::string& stream_id, agora_refptr<IAgoraMediaTrack> track) {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("stream_id: %s, track: %p", stream_id.c_str(), track.get());
//
//  // check and add into track to publish info map
//  PublishInfoType publish_info;
//
//  auto iter = track_to_publish_info_map_.find(track);
//
//  if (iter != track_to_publish_info_map_.end()) {
//    publish_info = iter->second;
//
//    if (publish_info->publish_state != PUBLISH_STATE_UNKNOWN) {
//      LOG_INFO("track already published or publishing");
//      return {publish_info, false};
//    }
//  } else {
//    publish_info = std::make_shared<PublishInfo>();
//    publish_info->track = track;
//    track_to_publish_info_map_[track] = publish_info;
//  }
//
//  // get the existing stream publish info or create a new one
//  auto& stream_publish_info = stream_publish_infos_map_[stream_id];
//
//  // either video or audio
//  if (ValidVideoSourceType(track->GetVideoSourceType())) {
//    // for video, no matter existing or new, video publish info should be empty
//    if (stream_publish_info.video_publish_info) {
//      LOG_ERR("one stream can only publish one video track");
//      return {nullptr, false};
//    }
//
//    stream_publish_info.video_publish_info = publish_info;
//  } else {
//    // one stream can publish multiple audio tracks
//    stream_publish_info.audio_publish_infos.emplace_back(publish_info);
//  }
//
//  return {publish_info, true};
//}
//
//std::pair<bool, bool> RteLocalUser::RemovePublishInfo(const std::string& stream_id,
//                                                      agora_refptr<IAgoraMediaTrack> track) {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("stream_id: %s, track: %p", stream_id.c_str(), track.get());
//
//  // check in track to publish info map first
//  auto track_iter = track_to_publish_info_map_.find(track);
//  if (track_iter == track_to_publish_info_map_.end()) {
//    LOG_ERR("failed to find publish info in track to publish info map");
//    return {false, false};
//  }
//
//  auto publish_info = track_iter->second;
//
//  if (publish_info->publish_state != PUBLISH_STATE_PUBLISHED) {
//    LOG_INFO("media track hasn't been published");
//    return {true, false};
//  }
//
//  // remove from stream publish info map first
//  auto id_iter = stream_publish_infos_map_.find(stream_id);
//  if (id_iter == stream_publish_infos_map_.end()) {
//    LOG_ERR("failed to find stream ID %s in stream publish infos map", stream_id.c_str());
//    return {false, false};
//  }
//
//  auto& stream_publish_info = id_iter->second;
//
//  // either video or audio
//  if (ValidVideoSourceType(track->GetVideoSourceType())) {
//    if (stream_publish_info.video_publish_info != publish_info) {
//      LOG_ERR("video publish info mismatch");
//      return {false, false};
//    }
//
//    stream_publish_info.video_publish_info.reset();
//  } else {
//    auto& audio_publish_infos = stream_publish_info.audio_publish_infos;
//
//    audio_publish_infos.erase(std::remove_if(audio_publish_infos.begin(), audio_publish_infos.end(),
//                                             [publish_info](const auto& audio_publish_info) {
//                                               return (audio_publish_info == publish_info);
//                                             }),
//                              audio_publish_infos.end());
//  }
//
//  // if the whole stream publish info has been empty, erase it
//  if (!stream_publish_info.video_publish_info && stream_publish_info.audio_publish_infos.empty()) {
//    stream_publish_infos_map_.erase(id_iter);
//  }
//
//  // remove from track to publish info map
//  track_to_publish_info_map_.erase(track);
//
//  return {true, true};
//}
//
//bool RteLocalUser::MultipleAudioTracks(StreamId stream_id) const {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("stream_id: %s", LITE_STR_CONVERT(stream_id));
//
//  auto iter = stream_publish_infos_map_.find(stream_id);
//  if (iter == stream_publish_infos_map_.end()) {
//    return false;
//  }
//
//  return (iter->second.audio_publish_infos.size() > 1);
//}
//
//agora_refptr<IDataParam> RteLocalUser::CreateDataParamForTrackCommon(const std::string& stream_id) {
//  ASSERT_IS_UI_THREAD();
//
//  API_LOGGER_MEMBER("stream_id: %s", stream_id.c_str());
//
//  auto param = CreateDataParam();
//  param->AddString(PARAM_APP_ID, scene_->GetAppId());
//  param->AddString(PARAM_SCENE_UUID, scene_->GetSceneUuid());
//  param->AddString(PARAM_USER_UUID, scene_->GetUserUuid());
//  param->AddString(PARAM_STREAM_UUID, stream_id);
//  param->AddString(PARAM_HTTP_TOKEN, scene_->GetUserToken());
//
//  return param;
//}
//
agora_refptr<IDataParam> RteLocalUser::CreateDataParamForProperties(
	const std::string& properties_str, bool remove) {
	ASSERT_IS_UI_THREAD();

	API_LOGGER_MEMBER("properties_str: %s, remove: %s", properties_str.c_str(), BOOL_TO_STR(remove));

	auto param = CreateDataParam();
	param->AddString(PARAM_APP_ID, scene_->GetAppId());
	param->AddString(PARAM_SCENE_UUID, scene_->GetSceneUuid());
	param->AddString(PARAM_HTTP_TOKEN, scene_->GetUserToken());
	param->AddString(PARAM_TEXT, properties_str);
	param->AddInt(PARAM_INT1, remove ? 1 : 0);

	return param;
}
//
}  // namespace rte
}  // namespace agora
