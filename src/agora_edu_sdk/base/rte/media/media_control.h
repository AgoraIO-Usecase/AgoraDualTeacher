//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <unordered_set>

#include "AgoraRefPtr.h"
#include "facilities/tools/rtc_callback.h"
#include "utils/log/log.h"

//#include "media/IAgoraCameraVideoTrack.h"
#include "media/IAgoraMediaControl.h"
//#include "media/IAgoraMediaPlayer.h"
//#include "media/IAgoraMicrophoneAudioTrack.h"
//#include "media/IAgoraScreenVideoTrack.h"

#include "media/IAgoraRealTimeMessage.h"

#include "extension_node_manager.h"

namespace agora {
namespace base {
class IAgoraService;
}  // namespace base

namespace rte {

class AgoraMessage : public IAgoraMessage {
 public:
  bool SetMessage(const char* msg) override;

  const char* GetMessage() const override { return msg_.c_str(); }

  void SetTimestamp(uint64_t ts) override { time_stamp_ = ts; }

  uint64_t GetTimestamp() override { return time_stamp_; }

 private:
  std::string msg_;
  uint64_t time_stamp_ = 0;
};

class MediaControl : public IAgoraMediaControl {
 protected:
  ~MediaControl() = default;

 public:
  explicit MediaControl(base::IAgoraService* service);

  agora_refptr<IAgoraMessage> CreateMessage() override;

  /*agora_refptr<IAgoraCameraVideoTrack> CreateCameraVideoTrack() override;

  agora_refptr<IAgoraScreenVideoTrack> CreateScreenVideoTrack() override;

  agora_refptr<IAgoraMicrophoneAudioTrack> CreateMicrophoneAudioTrack() override;

  agora_refptr<IAgoraMediaPlayer> CreateMediaPlayer() override;*/

  int EnableLocalAudioFilter(const char* name, const char* vendor, bool enable) override;

  int EnableLocalVideoFilter(const char* name, const char* vendor, bool enable) override;

  int EnableRemoteVideoFilter(const char* name, const char* vendor, bool enable) override;

  int SetExtensionProperty(const char* name, const char* vendor, const char* key, const void* value,
                           int size) override;

  int GetExtensionProperty(const char* name, const char* vendor, const char* key, void* value,
                           int size) override;

  void RegisterVideoFrameObserver(IVideoFrameObserver* observer) override;
  void UnregisterVideoFrameObserver() override;

 private:
  base::IAgoraService* service_ = nullptr;

  //rtc::ExtensionNodes local_extensions_;
  //rtc::ExtensionNodes remote_extensions_;

  IVideoFrameObserver* video_frame_observer_ = nullptr;
};

}  // namespace rte
}  // namespace agora
