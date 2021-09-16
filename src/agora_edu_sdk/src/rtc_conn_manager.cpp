//
//  rtc_conn_manager.cpp
//
//  Created by LC on 2020/11/20.
//  Copyright © 2020 agora. All rights reserved.
//

#include "rtc_conn_manager.h"
#include <algorithm>
#include <condition_variable>
#include "ui_thread.h"
#include "utils/log/log.h"

#pragma once
namespace agora {
namespace edu {
class RtcEngineHandler : public rtc::IVideoSourceEventHandler {
 public:
  virtual void onVideoSourceJoinedChannelSuccess(
      agora::rtc::uid_t_ uid) override {
    if (rtc_conn_manager_) {
      std::stringstream ss;
      ss << "\n"
         << __FUNCTION__ << " stream_uuid:" << uid << " join channel success!"
         << "\n";
      OutputDebugStringA(ss.str().c_str());
      rtc_conn_manager_->map_sid_uid_[id_.c_str()] = uid;
      rtc_conn_manager_->map_event_[id_]->notifyAll();
      rtc_conn_manager_->map_uid_sid_[uid] = id_;
    }
  }
  virtual void onVideoSourceRequestNewToken() {}
  virtual void onVideoSourceLeaveChannel() {}
  virtual void onVideoSourceExit() {}
  virtual void onVideoSourceFirstVideoFrame(int width, int height,
                                            int elapsed) {}
  RtcEngineHandler(RtcConnManager* manager, RtcConnManager::StreamId id) {
    rtc_conn_manager_ = manager;
    id_ = id;
  }
  RtcConnManager* rtc_conn_manager_;
  std::string id_;
};

static const char* MODULE_NAME = "RTC_CONN_MANAGER";

int RtcConnManager::CreateDefaultStream(const RtcInfo& rtc_config) {
  EduStream stream;
  stream.has_audio = false;
  stream.has_video = false;
  stream.source_type = EDU_VIDEO_SOURCE_TYPE_NONE;
  stream.stream_name;
  strcpy(stream.stream_name, "default stream");
  strcpy(stream.stream_uuid, std::to_string(rtc_config.uid).c_str());
  int err = CreateLocalStream(stream.stream_uuid, stream, rtc_config,
                              rtc_config.token);
  return err;
}

bool RtcConnManager::IsLocalStream(const StreamId& stream_uuid) {
  bool is_local_stream_ = false;
  if (map_rtc_engines_.find(stream_uuid) != map_rtc_engines_.end())
    is_local_stream_ = true;
  else
    is_local_stream_ = false;
  return is_local_stream_;
}

int RtcConnManager::CreateRtcConnnection(const StreamId& stream_id,
                                         std::string app_id, std::string token,
                                         std::string channel_name,
                                         std::string info, bool enable_video,
                                         bool enable_auido, rtc::uid_t_ uid) {
  int err = ERR_OK;
  if (stream_id.empty()) return ERR_FAILED;
  auto iter = map_rtc_engines_.find(stream_id);
  if (iter != map_rtc_engines_.end()) return ERR_FAILED;
  auto engine = createAgoraVideoSourceEngine();
  if (!engine) {
    LOG_INFO("create engine faild !");
    return ERR_FAILED;
  }
  LOG_INFO("create engine succsed !");
  map_rtc_engines_[stream_id] = engine;
  if (map_rtc_engines_.size() == 1) default_stream_ = engine;
  auto engine_handler = new RtcEngineHandler(this, stream_id);
  map_rtc_event_handler_[stream_id] = engine_handler;
  err =
      engine->initialize(app_id.c_str(), engine_handler) ? ERR_OK : ERR_FAILED;
  if (err) {
    LOG_INFO("initialize engine faild! err:%d", err);
    return ERR_FAILED;
  }
  engine->setChannelProfile(rtc::CHANNEL_PROFILE_LIVE_BROADCASTING_);
  engine->setCustomRender(is_custom_render);
  engine->enableHardWareVideoDecoding(is_hwdec_);
  engine->enableHardWareVideoEncoding(is_hwenc_);
  LOG_INFO("initialize engine succsed !");
  map_rtc_info_[stream_id] = {app_id,       token,        channel_name, info,
                              enable_video, enable_auido, uid};
  map_event_[stream_id] = new EduEvent(false);
  return err;
}

bool static round(int x, int dest) {
  if (x < dest + 5 && x > dest - 5) return true;
  return false;
}

agora::rtc::VIDEO_PROFILE_TYPE_ static getVideoSize(int width, int height,
                                                    int fps) {
  using namespace agora::rtc;
  static int arr[70][4] = {{160, 120, 15, VIDEO_PROFILE_LANDSCAPE_120P_},
                           {120, 120, 15, VIDEO_PROFILE_LANDSCAPE_120P_3_},
                           {320, 180, 15, VIDEO_PROFILE_LANDSCAPE_180P_},
                           {180, 180, 15, VIDEO_PROFILE_LANDSCAPE_180P_3_},
                           {240, 180, 15, VIDEO_PROFILE_LANDSCAPE_180P_4_},
                           {320, 240, 15, VIDEO_PROFILE_LANDSCAPE_240P_},
                           {240, 240, 15, VIDEO_PROFILE_LANDSCAPE_240P_3_},
                           {424, 240, 15, VIDEO_PROFILE_LANDSCAPE_240P_4_},
                           {640, 360, 15, VIDEO_PROFILE_LANDSCAPE_360P_},
                           {360, 360, 15, VIDEO_PROFILE_LANDSCAPE_360P_3_},
                           {640, 360, 30, VIDEO_PROFILE_LANDSCAPE_360P_4_},
                           {360, 360, 30, VIDEO_PROFILE_LANDSCAPE_360P_6_},
                           {480, 360, 15, VIDEO_PROFILE_LANDSCAPE_360P_7_},
                           {480, 480, 30, VIDEO_PROFILE_LANDSCAPE_360P_8_},
                           {640, 360, 15, VIDEO_PROFILE_LANDSCAPE_360P_9_},
                           {640, 360, 24, VIDEO_PROFILE_LANDSCAPE_360P_10_},
                           {640, 360, 24, VIDEO_PROFILE_LANDSCAPE_360P_11_},
                           {640, 480, 15, VIDEO_PROFILE_LANDSCAPE_480P_},
                           {480, 480, 15, VIDEO_PROFILE_LANDSCAPE_480P_3_},
                           {640, 480, 30, VIDEO_PROFILE_LANDSCAPE_480P_4_},
                           {480, 480, 30, VIDEO_PROFILE_LANDSCAPE_480P_6_},
                           {848, 480, 15, VIDEO_PROFILE_LANDSCAPE_480P_8_},
                           {848, 480, 30, VIDEO_PROFILE_LANDSCAPE_480P_9_},
                           {640, 480, 10, VIDEO_PROFILE_LANDSCAPE_480P_10_},
                           {1280, 720, 15, VIDEO_PROFILE_LANDSCAPE_720P_},
                           {1280, 720, 30, VIDEO_PROFILE_LANDSCAPE_720P_3_},
                           {960, 720, 15, VIDEO_PROFILE_LANDSCAPE_720P_5_},
                           {960, 720, 30, VIDEO_PROFILE_LANDSCAPE_720P_6_},
                           {1920, 1080, 15, VIDEO_PROFILE_LANDSCAPE_1080P_},
                           {1920, 1080, 30, VIDEO_PROFILE_LANDSCAPE_1080P_3_},
                           {1920, 1080, 60, VIDEO_PROFILE_LANDSCAPE_1080P_5_},
                           {2560, 1440, 30, VIDEO_PROFILE_LANDSCAPE_1440P_},
                           {2560, 1440, 60, VIDEO_PROFILE_LANDSCAPE_1440P_2_},
                           {3840, 2160, 30, VIDEO_PROFILE_LANDSCAPE_4K_},
                           {3840, 2160, 60, VIDEO_PROFILE_LANDSCAPE_4K_3_},
                           {9999, 9999, 99, VIDEO_PROFILE_LANDSCAPE_4K_3_}};
  int minIndex = 0;
  int mins = 999999999;
  for (int i = 0; i < 70; i++) {
    int t = abs(width * 100 + height * 100 + fps -
                (arr[i][0] * 100 + arr[i][1] * 100 + arr[i][2]));
    if (mins > t) {
      mins = t;
      minIndex = i;
    }
  }
  return (agora::rtc::VIDEO_PROFILE_TYPE_)arr[minIndex][3];
}

int RtcConnManager::SetVideoConfig(const StreamId& stream_id,
                                   const EduVideoConfig& config) {
  agora::rtc::VideoEncoderConfiguration_ video_encoder_config;
  video_encoder_config.bitrate = config.bitrate;
  video_encoder_config.dimensions.height = config.video_dimension_height;
  video_encoder_config.dimensions.width = config.video_dimension_width;
  video_encoder_config.frameRate = [config]() -> agora::rtc::FRAME_RATE_ {
    using namespace agora::rtc;
    if (config.frame_rate < 4) {
      return FRAME_RATE_FPS_1_;
    } else if (config.frame_rate < 8) {
      return FRAME_RATE_FPS_7_;
    } else if (config.frame_rate < 12) {
      return FRAME_RATE_FPS_10_;
    } else if (config.frame_rate < 17) {
      return FRAME_RATE_FPS_15_;
    } else if (config.frame_rate < 25) {
      return FRAME_RATE_FPS_24_;
    } else if (config.frame_rate < 50) {
      return FRAME_RATE_FPS_30_;
    }
    return FRAME_RATE_FPS_60_;
  }();
  video_encoder_config.degradationPreference =
      static_cast<agora::rtc::DEGRADATION_PREFERENCE_>(
          config.degradation_preference);
  video_encoder_config.orientationMode =
      static_cast<agora::rtc::ORIENTATION_MODE_>(config.orientation_mode);
  LOG_INFO(
      "SetVideoConfig bitrate:%d height:%d width:%d frameRate:%d "
      "degradattionPreference:%d orientationMode:%d",
      config.bitrate, config.video_dimension_height,
      config.video_dimension_width, config.frame_rate);
  int err = ERR_OK;

  auto iter = map_rtc_engines_.find(stream_id);
  if (iter != map_rtc_engines_.end()) {

    err = iter->second->setVideoEncoderConfiguration(video_encoder_config);
    // auto profile = getVideoSize(video_encoder_config.dimensions.width,
    //                  video_encoder_config.dimensions.height,
    //                  video_encoder_config.frameRate);
    //err = iter->second->setVideoProfile(profile, false);
  } else {
    err = ERR_FAILED;
  }
  std::stringstream ss;
  ss << "\n" << __FUNCTION__ << "\n";
  OutputDebugStringA(ss.str().c_str());
  return err;
}

int RtcConnManager::CreateLocalStream(const StreamId& stream_id,
                                      const EduStream& stream,
                                      const RtcInfo rtc_config,
                                      const std::string& rtc_token) {
  int err = ERR_OK;
  auto rtc_iter = map_rtc_engines_.find(stream_id);
  if (rtc_iter == map_rtc_engines_.end()) {
    int err = CreateRtcConnnection(stream_id, rtc_config.app_id, rtc_token,
                                   rtc_config.channel_name, rtc_config.info,
                                   rtc_config.enable_video,
                                   rtc_config.enable_audio, rtc_config.uid);
    if (err) {
      LOG_ERR("RtcConnManager::StartOrUpdateLocalStream failed! err:%d", err);
      return err;
    }
    if (rtc_iter = map_rtc_engines_.find(stream_id);
        rtc_iter == map_rtc_engines_.end()) {
      LOG_ERR("map_rtc_engines_ failed! err:%d", err);
      return err;
    }
  } else {
    err = ERR_FAILED;
    LOG_ERR("local stream is created! err:%d", err);
    return err;
  }
  err |= rtc_iter->second->enableLocalVideo(false);
  err |= rtc_iter->second->enableVideo(true);
  err |= rtc_iter->second->enableAudio(true);
  if (map_rtc_engines_.size() == 1) {
    default_stream_->enableLocalAudio(false);
    default_stream_->setDefaultMuteAllRemoteAudioStreams(true);
  } else {
    rtc_iter->second->enableLocalAudio(rtc_config.enable_audio);

    rtc_iter->second->enableLocalAudio(false);
    rtc_iter->second->muteAllRemoteAudioStreams(true);
    rtc_iter->second->enableDualStreamMode(true);
    default_stream_->muteRemoteAudioStream(atoll(stream_id.c_str()), true);
    default_stream_->muteRemoteVideoStream(atoll(stream_id.c_str()), true);
  }
  if (err != 0) {
    LOG_ERR("enableVideo stream_id:%s failed! err:%d", stream.stream_uuid);
    return err;
  }
  LOG_INFO("enableVideo stream_id:%s succeed!", stream.stream_uuid);
  auto info_iter = map_rtc_info_.find(stream_id);
  err = rtc_iter->second->joinChannel(
      rtc_token.c_str(), info_iter->second.channel_name.c_str(),
      info_iter->second.info.c_str(), info_iter->second.uid);
  // map_event_[stream_id]->WaitFor(1000);
  if (err) {
    err = ERR_FAILED;
    LOG_ERR("joinChannel:%s uid:%d token:%s info:%s failed:code:%d",
            info_iter->second.channel_name.c_str(), info_iter->second.uid,
            rtc_token.c_str(), info_iter->second.info.c_str(), err);
  } else {
    LOG_INFO("joinChannel:%s uid:%d token:%s info:%s succeed",
             info_iter->second.channel_name.c_str(), info_iter->second.uid,
             rtc_token.c_str(), info_iter->second.info.c_str());
  }
  std::stringstream ss;
  ss << "\n"
     << __FUNCTION__ << " stream id:" << stream_id << " rtc_token:" << rtc_token
     << " channel_name:" << info_iter->second.channel_name
     << " uid:" << info_iter->second.uid << "\n";
  OutputDebugStringA(ss.str().c_str());
  return err;
}

int RtcConnManager::EnableDualStreamMode(const EduStream& stream,
                                         bool enabled) {
  int err = ERR_OK;
  auto rtc_iter = map_rtc_engines_.find(stream.stream_uuid);
  if (rtc_iter == map_rtc_engines_.end()) {
    err = ERR_FAILED;
    LOG_ERR("stream id :%s is not exist!", stream.stream_uuid);
    return err;
  }
  err = rtc_iter->second->enableDualStreamMode(enabled);
  if (err) {
    err = ERR_FAILED;
    LOG_ERR("EnableDualStreamMode:stream uid:%d enabled:%d failed:%d",
            stream.stream_uuid, enabled, err);
  }
  return err;
}

int RtcConnManager::SetRemoteStreamType(
    const EduStream& stream, agora::rtc::REMOTE_VIDEO_STREAM_TYPE_ type) {
  int err = ERR_OK;
  auto rtc_iter = map_rtc_engines_.find(stream.stream_uuid);
  if (rtc_iter != map_rtc_engines_.end()) {
    err = ERR_FAILED;
    LOG_ERR("stream id :%s is local stream!", stream.stream_uuid);
    return err;
  }
  err = default_stream_->setRemoteVideoStreamType(atoll(stream.stream_uuid),
                                                  type);
  return err;
}

int RtcConnManager::SwitchCamera(const StreamId& stream_id,
                                 const char* device_id) {
  int err = ERR_OK;
  if (auto iter = map_rtc_engines_.find(stream_id);
      iter != map_rtc_engines_.end()) {
    err = iter->second->startPreview(device_id);
    err = iter->second->enableLocalVideo(true);
    if (err != 0) {
      LOG_ERR("stream uuid:%s SwitchCamera :%s failed! err:%d",
              stream_id.c_str(), device_id, err);
    } else {
      LOG_INFO("stream uuid:%s SwitchCamera :%s succeed!", stream_id.c_str(),
               device_id);
    }
    std::stringstream ss;
    ss << "\n"
       << __FUNCTION__ << "stream id:" << stream_id << " "
       << "\n";
    OutputDebugStringA(ss.str().c_str());
    return err;
  }
  err = ERR_FAILED;
  LOG_ERR("stream uuid:%s is not exist failed! err:%d", stream_id, err);
  return err;
}

int RtcConnManager::StartShareScreen(const EduShareScreenConfig& config,
                                     EduStream& stream) {
  int err = ERR_OK;
  if (auto iter = map_rtc_engines_.find(stream.stream_uuid);
      iter != map_rtc_engines_.end()) {
    RECT rc_tmp = config.rc;
    agora::rtc::Rect_ rc;
    if (!config.enableRect) {
      ::SwitchToThisWindow(config.hwnd, TRUE);
      GetWindowRect(config.hwnd, &rc_tmp);
      rc_tmp.right -= rc_tmp.left;
      rc_tmp.bottom -= rc_tmp.top;
      rc_tmp.left = 0;
      rc_tmp.top = 0;
      // GetWindowRect(GetDesktopWindow(), &rc_tmp);
    }
    rc.left = rc_tmp.left;
    rc.right = rc_tmp.right;
    rc.top = rc_tmp.top;
    rc.bottom = rc_tmp.bottom;
    EduVideoConfig video_config;
    video_config.video_dimension_width = rc.right - rc.left;
    video_config.video_dimension_height = rc.bottom - rc.top;
    err = iter->second->enableLocalVideo(true);
    err = iter->second->startPreview("");
    err = SetVideoConfig(stream.stream_uuid, video_config);
    err = iter->second->startScreenCapture(config.hwnd, config.fps, rc,
                                           config.bitrate);
    if (err != 0) {
      LOG_ERR(
          "stream uuid:%s startScreenCapture: hwnd:%p fps:%d bitrate:%d "
          "failed! "
          "err:%d",
          stream.stream_uuid, config.hwnd, config.fps, config.bitrate, err);
    } else {
      LOG_INFO(
          "stream uuid:%s startScreenCapture: hwnd:%p fps:%d bitrate:%d "
          "succeed!",
          stream.stream_uuid, config.hwnd, config.fps, config.bitrate);
    }
    std::stringstream ss;
    ss << "\n"
       << __FUNCTION__ << "stream id:" << stream.stream_uuid << " "
       << "\n";
    OutputDebugStringA(ss.str().c_str());
    return err;
  }
  err = ERR_FAILED;
  LOG_ERR("StartShareScreen stream uuid:%s is not exist failed! err:%d",
          stream.stream_uuid, err);

  return err;
}

int RtcConnManager::StopShareScreen(EduStream& stream) {
  int err = ERR_OK;
  if (auto iter = map_rtc_engines_.find(stream.stream_uuid);
      iter != map_rtc_engines_.end()) {
    err = iter->second->stopScreenCapture();
    if (err != 0) {
      LOG_ERR(
          "stream uuid:%s stopScreenCapture:  "
          "failed! "
          "err:%d",
          stream.stream_uuid, err);
    } else {
      LOG_INFO("stream uuid:%s stopScreenCapture: succeed!",
               stream.stream_uuid);
    }
    std::stringstream ss;
    ss << "\n"
       << __FUNCTION__ << "stream id:" << stream.stream_uuid << " "
       << "\n";
    OutputDebugStringA(ss.str().c_str());
    return err;
  }
  err = ERR_FAILED;
  LOG_ERR("StopShareScreen stream uuid:%s is not exist failed! err:%d",
          stream.stream_uuid, err);
  return err;
}

int RtcConnManager::SubscribeStream(const EduStream& stream,
                                    const EduSubscribeOptions& options) {
  std::stringstream ss;
  ss << "\n"
     << __FUNCTION__ << "stream id:" << stream.stream_uuid
     << " subscribe audio:" << options.subscribe_audio
     << "subscribe video:" << options.subscribe_video << "\n";
  OutputDebugStringA(ss.str().c_str());
  int err = ERR_OK;
  auto iter = map_rtc_engines_.find(stream.stream_uuid);
  if (iter != map_rtc_engines_.end()) {
    err = ERR_FAILED;
    LOG_ERR("SubscribeStream failed! stream uuid:%s is local stream  err:%d",
            stream.stream_uuid, err);
    return err;
  }
  if (!default_stream_) {
    err = ERR_FAILED;
    LOG_ERR("local stream is not err:%d", stream.stream_uuid, err);
    return err;
  }
  err = default_stream_->muteRemoteAudioStream(atoll(stream.stream_uuid),
                                               !options.subscribe_audio);
  auto iter_audio_subscribed =
      map_subscribed_audio_stream_.find(stream.stream_uuid);
  if (options.subscribe_audio &&
      iter_audio_subscribed != map_subscribed_audio_stream_.end()) {
    err = ERR_FAILED;
    LOG_ERR("MuteStream failed! stream uuid:%s is not exist err:%d",
            stream.stream_uuid, err);
  } else if (!options.subscribe_audio &&
             iter_audio_subscribed == map_subscribed_audio_stream_.end()) {
    err = ERR_FAILED;
    LOG_ERR("MuteStream failed! stream uuid:%s is exist err:%d",
            stream.stream_uuid, err);
  } else {
    if (err) {
      err = ERR_FAILED;
      LOG_ERR("stream_id:%s muteRemoteAudioStream audio faild err:%d",
              stream.stream_uuid, err);
    } else {
      LOG_INFO("stream_id:%s muteRemoteAudioStream audio succeed",
               stream.stream_uuid);
      if (options.subscribe_audio) {
        map_subscribed_audio_stream_.insert(stream.stream_uuid);
      } else {
        map_subscribed_audio_stream_.erase(stream.stream_uuid);
      }
    }
  }
  err = default_stream_->muteRemoteVideoStream(atoll(stream.stream_uuid),
                                               !options.subscribe_video);
  auto iter_video_subscribed =
      map_subscribed_video_stream_.find(stream.stream_uuid);
  if (options.subscribe_video &&
      iter_video_subscribed != map_subscribed_video_stream_.end()) {
    err = ERR_FAILED;
    LOG_ERR("MuteStream failed! stream uuid:%s is not exist err:%d",
            stream.stream_uuid, err);
  } else if (!options.subscribe_video &&
             iter_video_subscribed == map_subscribed_video_stream_.end()) {
    err = ERR_FAILED;
    LOG_ERR("MuteStream failed! stream uuid:%s is exist err:%d",
            stream.stream_uuid, err);
  } else {
    if (err) {
      err = ERR_FAILED;
      LOG_ERR("stream_id:%s muteRemoteVideoStream video faild err:%d",
              stream.stream_uuid, err);
    } else {
      LOG_INFO("stream_id:%s muteRemoteVideoStream video succeed",
               stream.stream_uuid);
    }
    if (options.subscribe_video) {
      map_subscribed_video_stream_.insert(stream.stream_uuid);
    } else {
      map_subscribed_video_stream_.erase(stream.stream_uuid);
    }
  }

  err |= default_stream_->setRemoteVideoStreamType(
      atoll(stream.stream_uuid),
      static_cast<agora::rtc::REMOTE_VIDEO_STREAM_TYPE_>(
          options.video_stream_type));
  if (err) {
    LOG_ERR(
        "subscribeStream stream uuid:%s subscribe_audio:%d subscribe_video:%d "
        "remote_video_type:%d failed! err:%d",
        stream.stream_uuid, options.subscribe_audio, options.subscribe_video,
        options.video_stream_type, err);
  } else {
    LOG_INFO(
        "subscribeStream stream uuid:%s subscribe_audio:%d subscribe_video:%d "
        "remote_video_type:%d succeed!",
        stream.stream_uuid, options.subscribe_audio, options.subscribe_video,
        options.video_stream_type);
  }
  if (!IsLocalStream(stream.stream_uuid)) {
    rtc::REMOTE_VIDEO_STREAM_TYPE_ type = rtc::REMOTE_VIDEO_STREAM_LOW_;
    if (options.video_stream_type == EDU_VIDEO_STREAM_TYPE_HIGH) {
      type = rtc::REMOTE_VIDEO_STREAM_HIGH_;
    }
    err = default_stream_->setRemoteVideoStreamType(atoll(stream.stream_uuid),
                                                    type);
    if (err) {
      err = ERR_FAILED;
      LOG_ERR("stream_id:%s subscribe video type:%d faild err:%d",
              stream.stream_uuid, type, err);
    } else {
      LOG_ERR("stream_id:%s subscribe video type:%d succeed",
              stream.stream_uuid, type);
    }
  }
  return (err);
}

int RtcConnManager::UnsubscribeStream(const EduStream& stream,
                                      const EduSubscribeOptions& options) {
  return SubscribeStream(stream, options);
}

int RtcConnManager::PublishStream(const EduStream& stream) {
  return UnmuteStream(stream);
}

int RtcConnManager::UnpublishStream(const EduStream& stream) {
  return MuteStream(stream, true, true);
}

int RtcConnManager::MuteStream(const EduStream& stream, bool mute_video,
                               bool mute_audio) {
  int err = ERR_OK;
  auto iter = map_rtc_engines_.find(stream.stream_uuid);
  if (iter == map_rtc_engines_.end()) {
    if (!default_stream_) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is not exist err:%d",
              stream.stream_uuid, err);
    }
    std::stringstream ss;
    ss << "\n"
       << __FUNCTION__ << "stream id:" << stream.stream_uuid
       << " mute remote audio:" << mute_audio
       << "mute remote video:" << mute_video << "\n";
    OutputDebugStringA(ss.str().c_str());

    auto iter_audio_subscribed =
        map_subscribed_audio_stream_.find(stream.stream_uuid);
    if (mute_audio &&
        iter_audio_subscribed == map_subscribed_audio_stream_.end()) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is not exist err:%d",
              stream.stream_uuid, err);
    } else if (!mute_audio &&
               iter_audio_subscribed != map_subscribed_audio_stream_.end()) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is exist err:%d",
              stream.stream_uuid, err);
    } else {
      err = default_stream_->muteRemoteAudioStream(atoll(stream.stream_uuid),
                                                   mute_audio);
      if (err) {
        err = ERR_FAILED;
        LOG_ERR("stream_id:%s muteRemoteAudioStream audio faild err:%d",
                stream.stream_uuid, err);
      } else {
        LOG_INFO("stream_id:%s muteRemoteAudioStream audio succeed",
                 stream.stream_uuid);
        if (mute_audio) {
          map_subscribed_audio_stream_.erase(stream.stream_uuid);
        } else {
          map_subscribed_audio_stream_.insert(stream.stream_uuid);
        }
      }
    }

    auto iter_video_subscribed =
        map_subscribed_video_stream_.find(stream.stream_uuid);
    if (mute_video &&
        iter_video_subscribed == map_subscribed_video_stream_.end()) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is not exist err:%d",
              stream.stream_uuid, err);
    } else if (!mute_video &&
               iter_video_subscribed != map_subscribed_video_stream_.end()) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is exist err:%d",
              stream.stream_uuid, err);
    } else {
      err = default_stream_->muteRemoteVideoStream(atoll(stream.stream_uuid),
                                                   mute_video);
      if (err) {
        err = ERR_FAILED;
        LOG_ERR("stream_id:%s muteRemoteVideoStream video faild err:%d",
                stream.stream_uuid, err);
      } else {
        LOG_INFO("stream_id:%s muteRemoteVideoStream video succeed",
                 stream.stream_uuid);
      }
      if (mute_video) {
        map_subscribed_video_stream_.erase(stream.stream_uuid);
      } else {
        map_subscribed_video_stream_.insert(stream.stream_uuid);
      }
    }
    return err;
  } else {
    std::stringstream ss;
    ss << "\n"
       << __FUNCTION__ << "stream id:" << stream.stream_uuid
       << " mute local audio:" << mute_audio
       << "mute local video:" << mute_video << "\n";
    OutputDebugStringA(ss.str().c_str());

    auto iter_publish_video =
        map_published_video_stream_.find(stream.stream_uuid);
    if (mute_video && iter_publish_video == map_published_video_stream_.end()) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is not exist err:%d",
              stream.stream_uuid, err);
    } else if (!mute_video &&
               iter_publish_video != map_published_video_stream_.end()) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is exist err:%d",
              stream.stream_uuid, err);
    } else {
      err = iter->second->muteLocalVideoStream(mute_video);
      if (err) {
        LOG_ERR("stream_id:%s muteLocalStream video faild err:%d",
                stream.stream_uuid, err);
      } else {
        LOG_INFO("stream_id:%s muteLocalStream video succeed",
                 stream.stream_uuid);
        if (!mute_video)
          map_published_video_stream_.insert(stream.stream_uuid);
        else
          map_published_video_stream_.erase(stream.stream_uuid);
      }
    }
    auto iter_publish_audio =
        map_published_audio_stream_.find(stream.stream_uuid);
    if (mute_audio && iter_publish_audio == map_published_audio_stream_.end()) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is not exist err:%d",
              stream.stream_uuid, err);
    } else if (!mute_audio &&
               iter_publish_audio != map_published_audio_stream_.end()) {
      err = ERR_FAILED;
      LOG_ERR("MuteStream failed! stream uuid:%s is exist err:%d",
              stream.stream_uuid, err);
    } else {
      err = iter->second->muteLocalAudioStream(mute_audio);
      if (err) {
        LOG_ERR("stream_id:%s muteLocalStream audio faild err:%d",
                stream.stream_uuid, err);
      } else {
        if (map_rtc_info_[stream.stream_uuid].enable_audio) {
          if (mute_audio)
            count_should_publish_audio--;
          else
            count_should_publish_audio++;
          if (count_should_publish_audio > 0)
            default_stream_->enableLocalAudio(true);
          if (count_should_publish_audio <= 0)
            default_stream_->enableLocalAudio(false);
        }
        LOG_INFO("stream_id:%s muteLocalxStream audio succeed",
                 stream.stream_uuid);
        if (!mute_audio)
          map_published_audio_stream_.insert(stream.stream_uuid);
        else
          map_published_audio_stream_.erase(stream.stream_uuid);
      }
    }
    return err;
  }
}

int RtcConnManager::UnmuteStream(const EduStream& stream) {
  return MuteStream(stream, false, false);
}

int RtcConnManager::UpdateStream(const EduStream& stream) {
  return MuteStream(stream, stream.has_video, stream.has_audio);
}

int RtcConnManager::SetCustomRender(bool enabled) {
  is_custom_render = enabled;

  return ERR_OK;
}

int RtcConnManager::EnableHWEncoding(bool enabled) {
  is_hwenc_ = enabled;
  return ERR_OK;
}

int RtcConnManager::EnableHWDecoding(bool enabled) {
  is_hwdec_ = enabled;
  return ERR_OK;
}

int RtcConnManager::SetStreamView(EduStream stream, View* view) {
  EduRenderConfig config;
  config.render_mode = EDU_RENDER_MODE_FIT;
  return SetStreamView(stream, view, config);
}

int RtcConnManager::SetStreamView(EduStream stream, View* view,
                                  const EduRenderConfig& config) {
  int err = ERR_OK;
  rtc::VideoCanvas_ vc;
  vc.uid = atoll(stream.stream_uuid);
  vc.view = view;
  vc.renderMode = config.render_mode;
  auto func = config.custom_render;
  if (IsLocalStream(stream.stream_uuid)) {
    std::stringstream ss;
    ss << "\n"
       << __FUNCTION__ << "stream id:" << stream.stream_uuid
       << " setuplocalview:" << view << "\n";
    OutputDebugStringA(ss.str().c_str());
    if (func) {
      err = map_rtc_engines_[stream.stream_uuid]->setupLocalVideo(
          vc, [=](agora_video_frame* frame) {
            if (func) func((IEduVideoFrame*)frame);
          });
    } else {
      err = map_rtc_engines_[stream.stream_uuid]->setupLocalVideo(vc, nullptr);
    }
    if (err) {
      LOG_ERR("stream_id:%s setupLocalVideo uid:%d faild err:%d",
              stream.stream_uuid, vc.uid, err);
    } else {
      LOG_INFO("stream_id:%s setupLocalVideo uid:%d succeed",
               stream.stream_uuid, vc.uid);
    }
  } else {
    if (func) {
      std::stringstream ss;
      ss << "\n"
         << __FUNCTION__ << "stream id:" << stream.stream_uuid
         << " setupremoteview:" << view << "\n";
      OutputDebugStringA(ss.str().c_str());
      default_stream_->startPreview("");
      err =
          default_stream_->setupRemoteVideo(vc, [=](agora_video_frame* frame) {
            if (func) func((IEduVideoFrame*)frame);
          });
    } else {
      err = default_stream_->setupRemoteVideo(vc, nullptr);
    }
    if (err) {
      LOG_ERR("stream_id:%s setupRemoteVideo uid:%d faild err:%d",
              stream.stream_uuid, vc.uid, err);
    } else {
      LOG_INFO("stream_id:%s setupRemoteVideo uid:%d succeed",
               stream.stream_uuid, vc.uid);
    }
  }
  return err;
}

void RtcConnManager::Destory() {
  for (auto& pair : map_rtc_engines_) {
    pair.second->leaveChannel();
    pair.second->release();
  }
  for (auto& pair : map_rtc_event_handler_) {
    delete pair.second;
  }
  map_rtc_engines_.clear();
  map_rtc_event_handler_.clear();
  map_rtc_info_.clear();
}

}  // namespace edu
}  // namespace agora
