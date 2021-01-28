//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#pragma once

#include "api2/NGIAgoraVideoTrack.h"
#include "call/video_receive_stream.h"
#include "engine_adapter/video/video_engine_interface.h"
#include "video_remote_track.h"

namespace agora {
namespace rtc {

class RemoteVideoTrackImageImpl : public RemoteVideoTrackImpl {
 public:
  explicit RemoteVideoTrackImageImpl(const RemoteVideoTrackImpl::RemoteVideoTrackConfig& config);
  ~RemoteVideoTrackImageImpl();

  int registerVideoEncodedImageReceiver(IVideoEncodedImageReceiver* videoReceiver) override;
  int unregisterVideoEncodedImageReceiver(IVideoEncodedImageReceiver* videoReceiver) override;

 protected:
  std::shared_ptr<rtc::VideoNodeDecoder> createVideoRxProcessor(utils::worker_type worker,
                                                                uint8_t payload) override;
};

}  // namespace rtc
}  // namespace agora
