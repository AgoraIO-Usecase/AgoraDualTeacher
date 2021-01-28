//
//  Agora Media SDK
//
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once
#include <atomic>

#include "IAgoraRtcEngine.h"
#include "api/video/video_frame.h"
#include "api2/internal/video_node_i.h"
#include "facilities/tools/rtc_callback.h"

namespace agora {
namespace rtc {

class VideoMetadataObserverImpl : public IVideoFilterEx {
 public:
  VideoMetadataObserverImpl(
      utils::RtcSteadySyncCallback<agora::rtc::IMetadataObserver>::SharedType send_callback,
      utils::RtcAsyncCallback<agora::rtc::IMetadataObserver>::Type recv_callback,
      VideoTrackInfo trackInfo);
  virtual ~VideoMetadataObserverImpl();

  // IVideoFilterBase
  bool adaptVideoFrame(const media::base::VideoFrame& upStreamFrame,
                       media::base::VideoFrame& downStreamFrame) override {
    return false;  // since we are internal implementation
  }

  // IVideoFilter
  void setEnabled(bool enable) override { enable_ = enable; }
  bool isEnabled() override { return enable_; }
  int setProperty(const char* key, const void* buf, size_t buf_size) override { return 0; }
  int getProperty(const char* key, void* buf, size_t buf_size) override { return 0; }

  // IVideoFilterEx
  bool adaptVideoFrame(const webrtc::VideoFrame& inputFrame,
                       webrtc::VideoFrame& outputFrame) override;
  void onSinkWantsChanged(const ::rtc::VideoSinkWants& wants) override {}

 protected:
  void onReadyToSendMetadata(webrtc::VideoFrame& outputFrame);
  void onMetadataReceived(const webrtc::VideoFrame& inputFrame);

 protected:
  utils::RtcSteadySyncCallback<agora::rtc::IMetadataObserver>::SharedType send_callback_;
  utils::RtcAsyncCallback<agora::rtc::IMetadataObserver>::Type recv_callback_;
  VideoTrackInfo track_info_ = {0};
  std::atomic<bool> enable_ = {false};
  int max_metadata_size_in_byte_ = {IMetadataObserver::INVALID_METADATA_SIZE_IN_BYTE};
  int64_t last_send_timestamp_ms_;
};

}  // namespace rtc
}  // namespace agora
