//
//  Agora Media SDK
//
//  Created by Rao Qi in 2019.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include "AgoraBase.h"
#include "NGIAgoraVideoTrack.h"
#include "internal/rtc_connection_i.h"
#include "internal/track_stat_i.h"
#include "internal/video_config_i.h"
#include "common_defines.h"
#include "facilities/media_config/policy_chain/media_config_policy_chain.h"
#include "facilities/tools/weak_observers.h"

namespace agora {
namespace rtc {

class VideoNodeRtpSink;
class VideoNodeRtpSource;
class VideoTrackConfigurator;

class IVideoTrackObserver : public std::enable_shared_from_this<IVideoTrackObserver> {
 public:
  virtual ~IVideoTrackObserver() = default;
  virtual void onLocalVideoStateChanged(int id,
                                        LOCAL_VIDEO_STREAM_STATE state,
                                        LOCAL_VIDEO_STREAM_ERROR errorCode,
                                        int timestamp_ms) {}
  
  virtual void onRemoteVideoStateChanged(uid_t uid,
                                         REMOTE_VIDEO_STATE state,
                                         REMOTE_VIDEO_STATE_REASON reason,
                                         int timestamp_ms) {}

  virtual void onFirstVideoFrameRendered(uid_t uid, int width, int height, int timestamp_ms) {}

  virtual void onFirstVideoFrameDecoded(uid_t uid, int width, int height, int timestamp_ms) {}

  virtual void onSourceVideoSizeChanged(uid_t uid,
                                        int width, int height,
                                        int rotation, int timestamp_ms) {}
  virtual void onSendSideDelay(int id, int send_delay) {}
  virtual void onRecvSideDelay(uid_t uid, int recv_delay) {}
};

class ILocalVideoTrackEx : public ILocalVideoTrack {
 public:
  enum DetachReason { MANUAL, TRACK_DESTROY, NETWORK_DESTROY };

  struct AttachInfo {
    uint32_t uid;
    uint32_t cid;
    VideoNodeRtpSink* network;
    WeakPipelineBuilder builder;
    uint64_t stats_space;
    CongestionControlType cc_type;
    int32_t rsfec_minimum_level;
    bool enabled_pacer;
    bool enable_two_bytes_extension;
  };

  struct DetachInfo {
    VideoNodeRtpSink* network;
    DetachReason reason;
  };

  ILocalVideoTrackEx() : id_(id_generator_++) {}
  virtual ~ILocalVideoTrackEx() {}
  
  virtual bool hasPublished() = 0;

  virtual int SetVideoConfigEx(const VideoConfigurationEx& configEx, utils::ConfigPriority priority = utils::CONFIG_PRIORITY_USER) = 0;

  virtual int GetConfigExs(std::vector<VideoConfigurationEx>& configs) = 0;

  virtual int prepareNodes() = 0;

  virtual bool attach(const AttachInfo& info) = 0;
  virtual bool detach(const DetachInfo& info) = 0;

  virtual bool registerTrackObserver(std::shared_ptr<IVideoTrackObserver> observer) {
    return false;
  }
  virtual bool unregisterTrackObserver(IVideoTrackObserver* observer) {
    return false;
  }

  virtual int32_t Width() const = 0;
  virtual int32_t Height() const = 0;
  virtual bool Enabled() const = 0;
  
  virtual VideoTrackConfigurator* GetVideoTrackConfigurator() {
    return nullptr;
  }

  int TrackId() const { return id_; }

 public:
  static void resetIdGenerator();

 protected:
  int id_;
  utils::WeakObservers<IVideoTrackObserver> track_observers_;

 private:
  static std::atomic<int> id_generator_;
};

struct RemoteVideoTrackStatsEx : RemoteVideoTrackStats {
  uint64_t firstDecodingTimeTickMs = 0;
  uint64_t firstVideoFrameRendered = 0;
};

class IRemoteVideoTrackEx : public IRemoteVideoTrack {
 public:
  enum DetachReason { MANUAL, TRACK_DESTROY, NETWORK_DESTROY };
  using RemoteVideoEvents = StateEvents<REMOTE_VIDEO_STATE, REMOTE_VIDEO_STATE_REASON>;
  
  struct AttachInfo {
    VideoNodeRtpSource* source;
    VideoNodeRtpSink* rtcp_sender;
    WeakPipelineBuilder builder;
    RECV_TYPE recv_type = RECV_MEDIA_ONLY;
    uint64_t stats_space = 0;
  };

  struct DetachInfo {
    VideoNodeRtpSource* source;
    VideoNodeRtpSink* rtcp_sender;
    DetachReason reason;
  };

  IRemoteVideoTrackEx() = default;

  virtual ~IRemoteVideoTrackEx() {}

  virtual uint32_t getRemoteSsrc() = 0;

  virtual bool attach(const AttachInfo& info, REMOTE_VIDEO_STATE_REASON reason) = 0;
  virtual bool detach(const DetachInfo& info, REMOTE_VIDEO_STATE_REASON reason) = 0;

  virtual bool getStatistics(RemoteVideoTrackStatsEx& statsex) {return false;}

  virtual bool registerTrackObserver(std::shared_ptr<IVideoTrackObserver> observer) {
    return false;
  }
  virtual bool unregisterTrackObserver(IVideoTrackObserver* observer) {
    return false;
  }

 protected:
  utils::WeakObservers<IVideoTrackObserver> track_observers_;
};

}  // namespace rtc
}  // namespace agora
