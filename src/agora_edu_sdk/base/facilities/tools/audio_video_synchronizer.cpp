//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-09.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "audio_video_synchronizer.h"
#include <inttypes.h>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "api2/internal/packet_i.h"
#include "engine_adapter/rtp_protocol.h"
#include "utils/log/log.h"
#include "utils/tools/util.h"

#define MODULE_NAME "Audio Video Synchronizer"

namespace agora {
namespace rtc {

AudioVideoSynchronizer::AudioVideoSynchronizer() {}

AudioVideoSynchronizer::~AudioVideoSynchronizer() {}

class AudioVideoSynchronizerImpl : public AudioVideoSynchronizer,
                                   public std::enable_shared_from_this<AudioVideoSynchronizerImpl> {
 public:
  struct SendTsAndRenderTime {
    SendTsAndRenderTime() : sent_ts_(0), timebase_(0), render_time_(0), timestamp_(0) {}

    SendTsAndRenderTime(const SendTsAndRenderTime& rhs)
        : sent_ts_(rhs.sent_ts_),
          timebase_(rhs.timebase_),
          render_time_(rhs.render_time_),
          timestamp_(rhs.timestamp_) {}

    SendTsAndRenderTime(int64_t packet_sent_ts, uint64_t packet_timebase,
                        int64_t packet_render_time, uint32_t timestamp)
        : sent_ts_(packet_sent_ts),
          timebase_(packet_timebase),
          render_time_(packet_render_time),
          timestamp_(timestamp) {}
    uint64_t sent_ts_{0};
    uint64_t timebase_{0};
    int64_t render_time_{0};
    uint32_t timestamp_{0};
  };

  struct UserTimestampList {
    UserTimestampList()
        : lock_(),
          rendered_audio_frames(0),
          rendered_video_frames(0),
          audio_sendts_and_render_time_list_(),
          video_sendts_and_render_time_list_(),
          video_timestamp_to_sendts_map_() {}
    std::mutex lock_;
    uint32_t rendered_audio_frames;
    uint32_t rendered_video_frames;
    std::map<uint64_t, SendTsAndRenderTime> audio_sendts_and_render_time_list_;
    std::map<uint64_t, SendTsAndRenderTime> video_sendts_and_render_time_list_;
    std::unordered_map<uint32_t, uint64_t> video_timestamp_to_sendts_map_;
  };

 public:
  AudioVideoSynchronizerImpl();
  virtual ~AudioVideoSynchronizerImpl();

  void receiveAudioPacket(uid_t uid, const agora::rtc::audio_packet_t& p) override;
  void receiveVideoPacket(uid_t uid, uint64_t sent_ts, const std::string& real_payload) override;
  void renderAudioFrame(uid_t uid, uint32_t ssrc, int64_t renderTimeMs) override;
  uint64_t getVideoFrameInternalSendTs(uid_t uid, uint32_t ssrc, uint32_t timestamp) override;
  int64_t getVideoRenderTime(uid_t uid, uint32_t ssrc, uint32_t timestamp) override;

  void removeAudioInfo(uid_t uid) override;
  void removeVideoInfo(uid_t uid, uint32_t ssrc) override;

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  bool checkData() override;
#endif
  Stats GetStats() override;

 private:
  uint64_t getTimebase(const SendTsAndRenderTime& last, uint64_t sent_ts);
  bool insertSendTs(std::map<uint64_t, SendTsAndRenderTime>& targetList, uint64_t sent_ts,
                    uint32_t timestamp = 0);

  uint64_t findVideoSendtsByTimestamp(std::shared_ptr<UserTimestampList> user_timestamp_list,
                                      uid_t uid, uint32_t timestamp);
  int64_t getRenderTime(const SendTsAndRenderTime& last, uint64_t timebase, uint64_t sendts);
  int64_t getVideoRenderTimeWithAudioPacket(std::shared_ptr<UserTimestampList> user_timestamp_list,
                                            uid_t uid, uint64_t video_frame_sendts);
  int64_t getVideoRenderTimeWithPrevVideoPacket(
      std::shared_ptr<UserTimestampList> user_timestamp_list, uid_t uid,
      uint64_t video_frame_sendts);

#if defined(FEATURE_ENABLE_UT_SUPPORT)
  bool checkSendtsAndRenderTimeList(const std::map<uint64_t, SendTsAndRenderTime>& list);
  void dumpSendtsAndRenderTimeList(const std::map<uint64_t, SendTsAndRenderTime>& list);
#endif

 private:
  static const int MAX_AUDIO_PACKETS = 500;
  static const int MAX_VIDEO_PACKETS = 500;
  std::mutex lock_;
  std::unordered_map<uid_t, std::shared_ptr<UserTimestampList>> user_timestamp_lists_;
};

AudioVideoSynchronizerImpl::AudioVideoSynchronizerImpl() : lock_(), user_timestamp_lists_() {}

AudioVideoSynchronizerImpl::~AudioVideoSynchronizerImpl() {}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
void AudioVideoSynchronizerImpl::dumpSendtsAndRenderTimeList(
    const std::map<uint64_t, SendTsAndRenderTime>& list) {
  agora::commons::log(agora::commons::LOG_INFO,
                      "%s To dump audio send ts and render time list: ", MODULE_NAME);
  for (auto iter = list.begin(); iter != list.end(); ++iter) {
    auto& cur_item = iter->second;
    agora::commons::log(
        agora::commons::LOG_INFO, "%s Time base %" PRIu64 ", send ts %" PRIu64 ", time %" PRIu64,
        MODULE_NAME, cur_item.timebase_, cur_item.sent_ts_, cur_item.timebase_ + cur_item.sent_ts_);
  }
}

bool AudioVideoSynchronizerImpl::checkSendtsAndRenderTimeList(
    const std::map<uint64_t, SendTsAndRenderTime>& list) {
  bool result = true;
  if (!list.empty() && list.size() > 1) {
    uint64_t prev_time = 0;
    for (auto iter = list.begin(); iter != list.end(); ++iter) {
      uint64_t cur_time = iter->first;
      if (cur_time < prev_time) {
        result = false;
        break;
      }
      if (cur_time != iter->second.timebase_ + iter->second.sent_ts_) {
        result = false;
        break;
      }
      prev_time = cur_time;
    }
  }
  if (!result) {
    dumpSendtsAndRenderTimeList(list);
  }
  return result;
}

bool AudioVideoSynchronizerImpl::checkData() {
  bool result = true;
  for (auto iter = user_timestamp_lists_.begin(); iter != user_timestamp_lists_.end(); ++iter) {
    auto& user_time_list = iter->second;
    result =
        result && checkSendtsAndRenderTimeList(user_time_list->audio_sendts_and_render_time_list_);
    result =
        result && (user_time_list->audio_sendts_and_render_time_list_.size() <= MAX_AUDIO_PACKETS);
    result = result && (user_time_list->audio_sendts_and_render_time_list_.size() >= 0);
    if (!result) {
      log(agora::commons::LOG_WARN, "%s Audio sendts and render time list size %u, uid %d",
          MODULE_NAME, user_time_list->audio_sendts_and_render_time_list_.size(), iter->first);
      break;
    }
    auto& timestamp_to_sendts_map = user_time_list->video_timestamp_to_sendts_map_;

    auto& video_sendts_and_render_time_list = user_time_list->video_sendts_and_render_time_list_;
    result = result && (video_sendts_and_render_time_list.size() == timestamp_to_sendts_map.size());
    if (!result) {
      log(agora::commons::LOG_WARN, "%s timestamp -> sendts size %d, timestamp list size %d",
          MODULE_NAME, video_sendts_and_render_time_list.size(), timestamp_to_sendts_map.size());
      break;
    }
    for (auto& timestamp : video_sendts_and_render_time_list) {
      result = result && (timestamp_to_sendts_map.find(timestamp.second.timestamp_) !=
                          timestamp_to_sendts_map.end());
      if (!result) {
        log(agora::commons::LOG_WARN, "%s Invalid timestamp %u", MODULE_NAME,
            timestamp.second.timestamp_);
      }
    }
    result = result && checkSendtsAndRenderTimeList(video_sendts_and_render_time_list);
  }
  return result;
}
#endif

uint64_t AudioVideoSynchronizerImpl::getTimebase(const SendTsAndRenderTime& last,
                                                 uint64_t sent_ts) {
  uint64_t timebase = last.timebase_;
  if (sent_ts > last.sent_ts_ && (sent_ts - last.sent_ts_) > HALF_OF_MAX_AV_PACKET_SEND_TS) {
    timebase -= MAX_AV_PACKET_SEND_TS;
  } else if (sent_ts < last.sent_ts_ && (last.sent_ts_ - sent_ts) > HALF_OF_MAX_AV_PACKET_SEND_TS) {
    timebase += MAX_AV_PACKET_SEND_TS;
  }
  return timebase;
}

bool AudioVideoSynchronizerImpl::insertSendTs(std::map<uint64_t, SendTsAndRenderTime>& targetList,
                                              uint64_t sent_ts, uint32_t timestamp) {
  SendTsAndRenderTime time(sent_ts, agora::commons::now_ms(), 0, timestamp);
  if (!targetList.empty()) {
    // Calculate time base for new packet
    auto& lastest_packet = targetList.rbegin()->second;
    uint64_t timebase = getTimebase(lastest_packet, sent_ts);

    time.timebase_ = timebase;
  }
  bool ret = false;
  if (targetList.find(time.timebase_ + time.sent_ts_) == targetList.end()) {
    targetList[time.timebase_ + time.sent_ts_] = time;
    ret = true;
  }
  return ret;
}

void AudioVideoSynchronizerImpl::receiveAudioPacket(uid_t uid,
                                                    const agora::rtc::audio_packet_t& p) {
  if (p.sent_ts >= MAX_AV_PACKET_SEND_TS) {
    agora::commons::log(agora::commons::LOG_WARN,
                        "%s: Receive invalid audio packet send ts %" PRIu64, MODULE_NAME,
                        p.sent_ts);
    return;
  }
  std::shared_ptr<UserTimestampList> user_timestamp_list;
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (user_timestamp_lists_.find(uid) == user_timestamp_lists_.end()) {
      user_timestamp_lists_[uid] = std::make_shared<UserTimestampList>();
    }
    user_timestamp_list = user_timestamp_lists_[uid];
  }

  std::lock_guard<std::mutex> lock(user_timestamp_list->lock_);
  insertSendTs(user_timestamp_list->audio_sendts_and_render_time_list_, p.sent_ts);

  while (user_timestamp_list->audio_sendts_and_render_time_list_.size() > MAX_AUDIO_PACKETS) {
    user_timestamp_list->audio_sendts_and_render_time_list_.erase(
        user_timestamp_list->audio_sendts_and_render_time_list_.begin());
  }
}
void AudioVideoSynchronizerImpl::receiveVideoPacket(uid_t uid, uint64_t sent_ts,
                                                    const std::string& real_payload) {
  if (sent_ts >= MAX_AV_PACKET_SEND_TS) {
    return;
  }
  std::shared_ptr<UserTimestampList> user_timestamp_list;
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (user_timestamp_lists_.find(uid) == user_timestamp_lists_.end()) {
      user_timestamp_lists_[uid] = std::make_shared<UserTimestampList>();
    }
    user_timestamp_list = user_timestamp_lists_[uid];
  }

  auto&& packet = agora::rtc::RtpPacket::Parse(
      reinterpret_cast<const uint8_t*>(real_payload.c_str()), real_payload.size());
  auto& packet_header = packet.Header();

  std::lock_guard<std::mutex> lock(user_timestamp_list->lock_);
  auto& timestamp_to_sendts_map = user_timestamp_list->video_timestamp_to_sendts_map_;
  auto& sendts_and_render_time_list = user_timestamp_list->video_sendts_and_render_time_list_;

  if (timestamp_to_sendts_map.find(packet_header.time_stamp) == timestamp_to_sendts_map.end()) {
    if (insertSendTs(sendts_and_render_time_list, sent_ts, packet_header.time_stamp)) {
      timestamp_to_sendts_map[packet_header.time_stamp] = sent_ts;
    }
  }
  while (sendts_and_render_time_list.size() > MAX_VIDEO_PACKETS) {
    uint32_t timestamp = sendts_and_render_time_list.begin()->second.timestamp_;
    sendts_and_render_time_list.erase(sendts_and_render_time_list.begin());
    timestamp_to_sendts_map.erase(timestamp);
  }
}

void AudioVideoSynchronizerImpl::renderAudioFrame(uid_t uid, uint32_t ssrc, int64_t renderTimeMs) {
  std::shared_ptr<UserTimestampList> user_timestamp_list;
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (user_timestamp_lists_.find(uid) == user_timestamp_lists_.end()) {
      return;
    }
    user_timestamp_list = user_timestamp_lists_[uid];
  }

  std::lock_guard<std::mutex> lock(user_timestamp_list->lock_);
  auto& sendts_and_render_time_list = user_timestamp_list->audio_sendts_and_render_time_list_;
  if (sendts_and_render_time_list.empty() ||
      sendts_and_render_time_list.rbegin()->second.render_time_ != 0) {
    return;
  }
  ++user_timestamp_list->rendered_audio_frames;
  if (sendts_and_render_time_list.begin()->second.render_time_ == 0) {
    sendts_and_render_time_list.begin()->second.render_time_ = renderTimeMs;
  } else {
    for (auto iter = sendts_and_render_time_list.rbegin();
         iter != sendts_and_render_time_list.rend(); ++iter) {
      if (iter->second.render_time_ != 0) {
        --iter;
        iter->second.render_time_ = renderTimeMs;
        break;
      }
    }
  }
}

uint64_t AudioVideoSynchronizerImpl::findVideoSendtsByTimestamp(
    std::shared_ptr<UserTimestampList> user_timestamp_list, uid_t uid, uint32_t timestamp) {
  auto& timestamp_to_sendts_map = user_timestamp_list->video_timestamp_to_sendts_map_;
  if (timestamp_to_sendts_map.find(timestamp) == timestamp_to_sendts_map.end()) {
    return 0;
  }
  return timestamp_to_sendts_map[timestamp];
}

uint64_t AudioVideoSynchronizerImpl::getVideoFrameInternalSendTs(uid_t uid, uint32_t ssrc,
                                                                 uint32_t timestamp) {
  std::shared_ptr<UserTimestampList> user_timestamp_list;
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (user_timestamp_lists_.find(uid) != user_timestamp_lists_.end()) {
      user_timestamp_list = user_timestamp_lists_[uid];
    }
  }
  uint64_t sendts = 0;
  if (user_timestamp_list) {
    std::lock_guard<std::mutex> lock(user_timestamp_list->lock_);
    sendts = findVideoSendtsByTimestamp(user_timestamp_list, uid, timestamp);
  }
  return sendts;
}

int64_t AudioVideoSynchronizerImpl::getRenderTime(const SendTsAndRenderTime& base,
                                                  uint64_t timebase, uint64_t sendts) {
  int64_t render_time = 0;
  if (timebase + sendts < base.timebase_ + base.sent_ts_) {
    render_time =
        base.render_time_ - (int64_t)(base.timebase_ + base.sent_ts_ - (timebase + sendts));
  } else {
    render_time =
        base.render_time_ + (int64_t)(timebase + sendts - (base.timebase_ + base.sent_ts_));
  }
  return render_time;
}

int64_t AudioVideoSynchronizerImpl::getVideoRenderTimeWithAudioPacket(
    std::shared_ptr<UserTimestampList> user_timestamp_list, uid_t uid,
    uint64_t video_frame_sendts) {
  auto& audio_sendts_and_render_time_list = user_timestamp_list->audio_sendts_and_render_time_list_;
  if (audio_sendts_and_render_time_list.empty()) {
    return 0;
  }
  int64_t render_time = 0;
  const auto& lastest_audio_packet = audio_sendts_and_render_time_list.rbegin()->second;
  uint64_t timebase = getTimebase(lastest_audio_packet, video_frame_sendts);

  auto iter = audio_sendts_and_render_time_list.lower_bound(timebase + video_frame_sendts);
  if (iter != audio_sendts_and_render_time_list.begin()) {
    --iter;
  }
  while (iter != audio_sendts_and_render_time_list.begin()) {
    if (iter->second.render_time_ != 0) {
      render_time = getRenderTime(iter->second, timebase, video_frame_sendts);
      break;
    }
    --iter;
  }
  if (render_time == 0 && iter->second.render_time_ != 0) {
    render_time = getRenderTime(iter->second, timebase, video_frame_sendts);
  }
  if (iter != audio_sendts_and_render_time_list.begin()) {
    uint64_t time = iter->second.timebase_ + iter->second.sent_ts_;
    uint64_t first_item_time = audio_sendts_and_render_time_list.begin()->second.timebase_ +
                               audio_sendts_and_render_time_list.begin()->second.sent_ts_;
    while (first_item_time < time && audio_sendts_and_render_time_list.size() > 0) {
      audio_sendts_and_render_time_list.erase(audio_sendts_and_render_time_list.begin());
      first_item_time = audio_sendts_and_render_time_list.begin()->second.timebase_ +
                        audio_sendts_and_render_time_list.begin()->second.sent_ts_;
    }
  }
  return render_time;
}

// Suppose the sendts is unique in timestamp list.
int64_t AudioVideoSynchronizerImpl::getVideoRenderTimeWithPrevVideoPacket(
    std::shared_ptr<UserTimestampList> user_timestamp_list, uid_t uid,
    uint64_t video_frame_sendts) {
  int64_t render_time = 0;
  auto& sendts_and_render_time_list = user_timestamp_list->video_sendts_and_render_time_list_;

  const auto& lastest_packet = sendts_and_render_time_list.rbegin()->second;
  uint64_t timebase = getTimebase(lastest_packet, video_frame_sendts);
  uint64_t time = timebase + video_frame_sendts;
  auto iter = sendts_and_render_time_list.find(time);
  if (iter != sendts_and_render_time_list.end()) {
    while (iter != sendts_and_render_time_list.begin()) {
      if (iter->second.render_time_ != 0) {
        render_time = getRenderTime(iter->second, timebase, video_frame_sendts);
        break;
      }
      --iter;
    }

    if (render_time == 0 && iter->second.render_time_ != 0) {
      render_time = getRenderTime(iter->second, timebase, video_frame_sendts);
    }

    if (render_time == 0) {
      render_time = agora::commons::now_ms();
    }
    iter = sendts_and_render_time_list.find(time);
    iter->second.render_time_ = render_time;

    uint64_t time = iter->second.timebase_ + iter->second.sent_ts_;

    uint64_t first_item_time = sendts_and_render_time_list.begin()->second.timebase_ +
                               sendts_and_render_time_list.begin()->second.sent_ts_;
    while (first_item_time < time) {
      user_timestamp_list->video_timestamp_to_sendts_map_.erase(
          sendts_and_render_time_list.begin()->second.timestamp_);
      sendts_and_render_time_list.erase(sendts_and_render_time_list.begin());
      first_item_time = sendts_and_render_time_list.begin()->second.timebase_ +
                        sendts_and_render_time_list.begin()->second.sent_ts_;
    }
  } else {
    render_time = agora::commons::now_ms();
  }

  return render_time;
}

int64_t AudioVideoSynchronizerImpl::getVideoRenderTime(uid_t uid, uint32_t ssrc,
                                                       uint32_t timestamp) {
  int64_t render_time = 0;
  std::shared_ptr<UserTimestampList> user_timestamp_list;
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (user_timestamp_lists_.find(uid) != user_timestamp_lists_.end()) {
      user_timestamp_list = user_timestamp_lists_[uid];
    }
  }

  if (user_timestamp_list) {
    std::lock_guard<std::mutex> lock(user_timestamp_list->lock_);
    ++user_timestamp_list->rendered_video_frames;
    uint64_t sendts = findVideoSendtsByTimestamp(user_timestamp_list, uid, timestamp);
    if (sendts != 0) {
      render_time = getVideoRenderTimeWithAudioPacket(user_timestamp_list, uid, sendts);
      if (render_time == 0) {
        render_time = getVideoRenderTimeWithPrevVideoPacket(user_timestamp_list, uid, sendts);
      }
    }
  }
  if (render_time == 0) {
    render_time = agora::commons::now_ms();
  }
  return render_time;
}

void AudioVideoSynchronizerImpl::removeAudioInfo(uid_t uid) {
  std::shared_ptr<UserTimestampList> user_timestamp_list;
  {
    std::lock_guard<std::mutex> lock(lock_);
    if (user_timestamp_lists_.find(uid) != user_timestamp_lists_.end()) {
      user_timestamp_list = user_timestamp_lists_[uid];
    }
  }
  if (user_timestamp_list) {
    std::lock_guard<std::mutex> lock(user_timestamp_list->lock_);
    user_timestamp_list->audio_sendts_and_render_time_list_.clear();
  }
}

void AudioVideoSynchronizerImpl::removeVideoInfo(uid_t uid, uint32_t ssrc) {
  std::lock_guard<std::mutex> lock(lock_);
  if (user_timestamp_lists_.find(uid) != user_timestamp_lists_.end()) {
    user_timestamp_lists_.erase(uid);
  }
}

AudioVideoSynchronizer::Stats AudioVideoSynchronizerImpl::GetStats() {
  AudioVideoSynchronizer::Stats stat;
  std::lock_guard<std::mutex> lock(lock_);
  stat.number_of_users = user_timestamp_lists_.size();
  for (auto user_info : user_timestamp_lists_) {
    AudioVideoSynchronizer::UserSynchronizeStats userStats;
    std::lock_guard<std::mutex> lock(user_info.second->lock_);
    userStats.audio_packets = user_info.second->audio_sendts_and_render_time_list_.size();
    userStats.video_packets = user_info.second->video_sendts_and_render_time_list_.size();
    userStats.rendered_audio_frames = user_info.second->rendered_audio_frames;
    userStats.rendered_video_frames = user_info.second->rendered_video_frames;
    stat.user_synchronize_stats[user_info.first] = userStats;
  }
  return stat;
}

std::shared_ptr<AudioVideoSynchronizer> AudioVideoSynchronizer::create() {
  return std::make_shared<AudioVideoSynchronizerImpl>();
}
}  // namespace rtc
}  // namespace agora
