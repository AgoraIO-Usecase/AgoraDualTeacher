//
//  Agora Media SDK
//
//  Created by Ender Zhengin 2018-12.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include "api2/NGIAgoraMediaNodeFactory.h"
#include "api2/internal/media_node_factory_i.h"
#include "facilities/miscellaneous/media_node_pool.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace rtc {

class MediaNodeFactoryImpl : public IMediaNodeFactoryEx {
 public:
  MediaNodeFactoryImpl();

  ~MediaNodeFactoryImpl() override = default;

  agora_refptr<ICameraCapturer> createCameraCapturer() override;

  agora_refptr<IScreenCapturer> createScreenCapturer() override;

  agora_refptr<IVideoMixerSource> createVideoMixer() override;

  agora_refptr<IVideoFrameTransceiver> createVideoFrameTransceiver() override;

  agora_refptr<IVideoFrameSender> createVideoFrameSender() override;

  agora_refptr<IVideoEncodedImageSender> createVideoEncodedImageSender() override;

  agora_refptr<IVideoFrameAdapter> createVideoFrameAdapter() override;

  agora_refptr<IVideoRenderer> createVideoRenderer() override;

  agora_refptr<IAudioFilter> createAudioFilter(const char* name,
                                               const char* vendor = nullptr) override;

  agora_refptr<IVideoFilter> createVideoFilter(const char* name,
                                               const char* vendor = nullptr) override;

  agora_refptr<IVideoSinkBase> createVideoSink(const char* name,
                                               const char* vendor = nullptr) override;

  agora_refptr<IAudioPcmDataSender> createAudioPcmDataSender() override;

  agora_refptr<IAudioEncodedFrameSender> createAudioEncodedFrameSender() override;

  agora_refptr<IVideoSinkBase> createObservableVideoSink(media::IVideoFrameObserver* observer,
                                                         VideoTrackInfo trackInfo) override;

  agora_refptr<rtc::IMediaPlayerSource> createMediaPlayerSource(
      media::base::MEDIA_PLAYER_SOURCE_TYPE type) override;

  agora_refptr<IMediaPacketSender> createMediaPacketSender() override;

 private:
  // in case a device takes un-reasonable long time to start/stop  itself
#if !defined(RTC_TRANSMISSION_ONLY)
  utils::worker_type video_source_device_worker_;
  utils::worker_type video_render_device_worker_;
#endif

  utils::worker_type player_worker_;

  MediaNodePool<IVideoRenderer>::Type renderer_pool_;
};

}  // namespace rtc
}  // namespace agora
