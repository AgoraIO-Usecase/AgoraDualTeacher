//
//  rtc_conn_manager.h
//
//  Created by LC on 2020/11/20.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "IAgoraVideoSourceEngine.h"
#include "interface/edu_sdk/IEduTeacherService.h"
#include "interface/edu_sdk/IEduUserService.h"
#include "internal/IAgoraRteTransferProtocol.h"
#include "edu_event.h"

namespace agora {
namespace edu {
struct RtcInfo {
  std::string app_id;
  std::string token;
  std::string channel_name;
  std::string info;
  bool enable_video;
  bool enable_audio;
  rtc::uid_t_ uid;
  rtc::CHANNEL_PROFILE_TYPE_ profile;
};

class RtcConnManager {
  friend class RtcEngineHandler;

 public:
  enum PublishState {
    PUBLISH_STATE_UNKNOWN,
    PUBLISH_STATE_PUBLISHING,
    PUBLISH_STATE_PUBLISHED,
  };

  struct PublishInfo {
    // agora_refptr<IAgoraMediaTrack> track;
    rtc::uid_t_ uid;
    std::string rtc_token;
    PublishState publish_state = PUBLISH_STATE_UNKNOWN;
    bool rtc_published = false;
  };

  using PublishInfoType = std::shared_ptr<PublishInfo>;

  struct StreamPublishInfos {
    PublishInfoType video_publish_info;
    std::vector<PublishInfoType> audio_publish_infos;
  };
  using StreamId = std::string;

 public:
  RtcConnManager() =default;
  ~RtcConnManager() = default;

  int CreateDefaultStream(const RtcInfo& rtc_config);

  bool IsLocalStream(const StreamId& stream);

  int CreateRtcConnnection(const StreamId& stream_id, std::string app_id,
                           std::string token, std::string channel_name,
                           std::string info,bool enable_video,bool enable_auido, rtc::uid_t_ uid = 0);

  int SetVideoConfig(const StreamId& stream_id, const EduVideoConfig& config);

  int CreateLocalStream(const StreamId& stream_id, const EduStream& stream,
                        const RtcInfo rtc_config, const std::string& rtc_token);

  int EnableDualStreamMode(const EduStream& stream,bool enabled);

  int SetRemoteStreamType(const EduStream& stream, agora::rtc::REMOTE_VIDEO_STREAM_TYPE_ type);

  int SwitchCamera(const StreamId& stream_id, const char* device_id = "");

  int StartShareScreen(const EduShareScreenConfig& config, EduStream& stream);

  int StopShareScreen(EduStream& stream);

  int SubscribeStream(const EduStream& stream,
                      const EduSubscribeOptions& options);

  int UnsubscribeStream(const EduStream& stream,
                        const EduSubscribeOptions& options);

  int PublishStream(const EduStream& stream);

  int UnpublishStream(const EduStream& stream);

  int MuteStream(const EduStream& stream,bool mute_video, bool mute_audio);

  int UnmuteStream(const EduStream& stream);

  int UpdateStream(const EduStream& stream);
  int SetCustomRender(bool enabled);
  int EnableHWEncoding(bool enabled);
  int EnableHWDecoding(bool enabled);
  int SetStreamView(EduStream stream, View* view);
  int SetStreamView(EduStream stream, View* view,
                    const EduRenderConfig& config);

  void Destory();

 private:
  bool is_custom_render = false;
  bool is_hwenc_ = false;
  bool is_hwdec_ = false;
  int count_should_publish_audio = 0;
  std::unordered_map<std::string, RtcInfo> map_rtc_info_;
  std::unordered_set<std::string> map_subscribed_audio_stream_;
  std::unordered_set<std::string> map_subscribed_video_stream_;

  std::unordered_set<std::string> map_published_audio_stream_;
  std::unordered_set<std::string> map_published_video_stream_;

  std::unordered_map<rtc::uid_t_, std::string> map_uid_sid_;
  std::unordered_map<std::string, rtc::uid_t_> map_sid_uid_;
  std::unordered_map<std::string, std::string> map_rtc_token_;
  rtc::IAgoraVideoSourceEngine* default_stream_;
  std::unordered_map<std::string, rtc::IAgoraVideoSourceEngine*>
      map_rtc_engines_;
  std::unordered_map<std::string, EduEvent*> map_event_;
  std::unordered_map<std::string, rtc::IVideoSourceEventHandler*>
      map_rtc_event_handler_;
};
}  // namespace edu
}  // namespace agora