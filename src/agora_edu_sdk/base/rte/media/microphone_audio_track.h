//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <string>

#include "AgoraRteBase.h"
#include "IAgoraRteLocalUser.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "media/IAgoraMicrophoneAudioTrack.h"

namespace agora {
namespace base {
class IAgoraService;
}  // namespace base

namespace rtc {
class IMediaNodeFactory;
class ILocalAudioTrack;
}  // namespace rtc

namespace rte {

class MicrophoneAudioTrack : public IAgoraMicrophoneAudioTrack {
 protected:
  ~MicrophoneAudioTrack() = default;

 public:
  explicit MicrophoneAudioTrack(base::IAgoraService* service);

  // IAgoraMediaTrack
  int Start() override;
  int Stop() override;

  int SetStreamId(StreamId stream_id) override;
  int GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const override;

  int SetStreamName(const char* stream_name) override;
  int GetStreamName(char* stream_name_buf, size_t stream_name_buf_size) const override;

  VideoSourceType GetVideoSourceType() const override { return TYPE_VIDEO_NONE; }
  AudioSourceType GetAudioSourceType() const override { return TYPE_MIC; }

  // IAgoraMicrophoneAudioTrack
  int EnableLocalPlayback() override;

  agora_refptr<rtc::ILocalAudioTrack> GetLocalAudioTrack() const;

#ifdef FEATURE_ENABLE_UT_SUPPORT
 private:  // NOLINT
  bool Ready() const;
#endif  // FEATURE_ENABLE_UT_SUPPORT

 private:
  std::string stream_id_;
  std::string stream_name_;

  agora_refptr<rtc::ILocalAudioTrack> microphone_track_;
};

}  // namespace rte
}  // namespace agora
