//
//  Agora Media SDK
//
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#include "media_node_factory.h"

#include "IAgoraMediaEngine.h"
#include "agora_service_impl.h"
#include "api2/IAgoraService.h"
#include "api2/internal/media_player_source_i.h"
#include "audio/audio_encoded_frame_sender.h"
#include "audio/audio_pcm_sender.h"
#include "extension_control_impl.h"
#include "facilities/tools/api_logger.h"
#include "media_packet_sender.h"
#include "rtc_globals.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_pool.h"
#include "video/video_camera_source.h"
#include "video/video_frame_adapter.h"
#include "video/video_frame_observer.h"
#include "video/video_frame_sender.h"
#include "video/video_frame_transceiver.h"
#include "video/video_image_sender.h"
#include "video/video_mixer_source.h"
#include "video/video_module_source_camera.h"
#include "video/video_renderer.h"
#include "video/video_screen_source.h"

namespace {
agora::agora_refptr<agora::rtc::IExtensionProvider> getProvider(const char* vendor) {
  auto ex_control = agora::rtc::ExtensionControlImpl::GetInstance();
  if (!ex_control) {
    return nullptr;
  }
  return ex_control->getExtensionProvider(vendor);
}
}  // namespace

// NOTE: All API implementation in the video node should be thread safe.
namespace agora {
namespace rtc {

agora_refptr<ICameraCapturer> MediaNodeFactoryImpl::createCameraCapturer() {
  API_LOGGER_MEMBER(nullptr);

#if !defined(RTC_TRANSMISSION_ONLY) && defined(FEATURE_VIDEO)
  return VideoModuleSourceCamera::Create(video_source_device_worker_);
#else
  return nullptr;
#endif  // !RTC_TRANSMISSION_ONLY && FEATURE_VIDEO
}

agora_refptr<IVideoMixerSource> MediaNodeFactoryImpl::createVideoMixer() {
  API_LOGGER_MEMBER(nullptr);

#if !defined(RTC_TRANSMISSION_ONLY) && defined(FEATURE_VIDEO)
  return VideoMixerSourceImpl::createVideoMixerSource();
#else
  return nullptr;
#endif  // !RTC_TRANSMISSION_ONLY && FEATURE_VIDEO
}

agora_refptr<IVideoFrameTransceiver> MediaNodeFactoryImpl::createVideoFrameTransceiver() {
  API_LOGGER_MEMBER(nullptr);

#if !defined(RTC_TRANSMISSION_ONLY) && defined(FEATURE_VIDEO)
  return VideoFrameTransceiverImpl::createVideoFrameTransceiver();
#else
  return nullptr;
#endif  // !RTC_TRANSMISSION_ONLY && FEATURE_VIDEO
}

agora_refptr<IScreenCapturer> MediaNodeFactoryImpl::createScreenCapturer() {
  API_LOGGER_MEMBER(nullptr);

  // Policy:
  // devices using a separate thread to avoid hang up when calling start
#if !defined(RTC_TRANSMISSION_ONLY)
#ifdef FEATURE_VIDEO
#if defined(__ANDROID__)
  return new RefCountedObject<rtc::VideoScreenSourceAndroid>(video_source_device_worker_);
#else
  return new RefCountedObject<rtc::VideoScreenSourceWrapper>(video_source_device_worker_);
#endif
#else
  return nullptr;
#endif  // FEATURE_VIDEO
#else
  return nullptr;
#endif  // !RTC_TRANSMISSION_ONLY
}

agora_refptr<IVideoFrameSender> MediaNodeFactoryImpl::createVideoFrameSender() {
  API_LOGGER_MEMBER(nullptr);

#if !defined(RTC_TRANSMISSION_ONLY) && defined(FEATURE_VIDEO)
  return new RefCountedObject<rtc::VideoFrameSenderImpl>();
#else
  return nullptr;
#endif  // !RTC_TRANSMISSION_ONLY && FEATURE_VIDEO
}

agora::agora_refptr<agora::rtc::IVideoEncodedImageSender>
MediaNodeFactoryImpl::createVideoEncodedImageSender() {
  API_LOGGER_MEMBER(nullptr);

#ifdef FEATURE_VIDEO
  return new RefCountedObject<rtc::VideoImageSenderImpl>();
#else
  return nullptr;
#endif  // FEATURE_VIDEO
}

agora_refptr<IVideoFrameAdapter> MediaNodeFactoryImpl::createVideoFrameAdapter() {
  API_LOGGER_MEMBER(nullptr);

  // Policy
  // The implementation is thread-safe from applicaiton.
#ifdef FEATURE_VIDEO
  return VideoFrameAdapter::Create();
#else
  return nullptr;
#endif  // FEATURE_VIDEO
}

agora_refptr<IVideoRenderer> MediaNodeFactoryImpl::createVideoRenderer() {
  API_LOGGER_MEMBER(nullptr);

  return renderer_pool_->GetOne();
}

agora_refptr<IAudioFilter> MediaNodeFactoryImpl::createAudioFilter(const char* name,
                                                                   const char* vendor) {
  API_LOGGER_MEMBER("name:\"%s\", vendor:\"%s\"", name, vendor);
  if (!name || !*name) return nullptr;
  if (!vendor || !*vendor) vendor = "io.agora.builtin";
  auto provider = getProvider(vendor);
  if (!provider) return nullptr;
  return provider->createAudioFilter(name);
}

agora_refptr<IVideoFilter> MediaNodeFactoryImpl::createVideoFilter(const char* name,
                                                                   const char* vendor) {
  API_LOGGER_MEMBER("name:\"%s\", vendor:\"%s\"", name, vendor);
  if (!name || !*name) return nullptr;
  if (!vendor || !*vendor) vendor = "io.agora.builtin";
  auto provider = getProvider(vendor);
  if (!provider) return nullptr;
  return provider->createVideoFilter(name);
}

agora_refptr<IVideoSinkBase> MediaNodeFactoryImpl::createVideoSink(const char* name,
                                                                   const char* vendor) {
  API_LOGGER_MEMBER("name:\"%s\", vendor:\"%s\"", name, vendor);
  if (!name || !*name) return nullptr;
  if (!vendor || !*vendor) vendor = "io.agora.builtin";
  auto provider = getProvider(vendor);
  if (!provider) return nullptr;
  return provider->createVideoSink(name);
}

agora_refptr<IVideoSinkBase> MediaNodeFactoryImpl::createObservableVideoSink(
    agora::media::IVideoFrameObserver* observer, VideoTrackInfo trackInfo) {
#ifdef FEATURE_VIDEO
  API_LOGGER_MEMBER(
      "observer:%p, trackInfo:(ownerUid:%u, trackId:%d, connectionId:%d, "
      " streamType:%d, codecType%d, encodedFrameOnly:%d)",
      observer, trackInfo.ownerUid, trackInfo.trackId, trackInfo.connectionId, trackInfo.streamType,
      trackInfo.codecType, trackInfo.encodedFrameOnly);

  return new RefCountedObject<rtc::VideoFrameObserverImpl>(observer, trackInfo);
#else
  return nullptr;
#endif  // FEATURE_VIDEO
}

agora_refptr<IAudioPcmDataSender> MediaNodeFactoryImpl::createAudioPcmDataSender() {
  API_LOGGER_MEMBER(nullptr);

  return new RefCountedObject<rtc::AudioPcmDataSenderImpl>();
}

agora_refptr<IAudioEncodedFrameSender> MediaNodeFactoryImpl::createAudioEncodedFrameSender() {
  API_LOGGER_MEMBER(nullptr);

  return new RefCountedObject<AudioEncodedFrameSenderImpl>();
}

agora_refptr<rtc::IMediaPlayerSource> MediaNodeFactoryImpl::createMediaPlayerSource(
    media::base::MEDIA_PLAYER_SOURCE_TYPE type) {
  API_LOGGER_MEMBER(nullptr);

#ifdef ENABLE_MEDIA_PLAYER
  return IMediaPlayerSourceEx::Create(createAgoraService(), player_worker_, type);
#else
  return nullptr;
#endif  // ENABLE_MEDIA_PLAYER
}

agora_refptr<IMediaPacketSender> MediaNodeFactoryImpl::createMediaPacketSender() {
  API_LOGGER_MEMBER(nullptr);

  return new RefCountedObject<MediaPacketSenderImpl>();
}

MediaNodeFactoryImpl::MediaNodeFactoryImpl() {
#if !defined(RTC_TRANSMISSION_ONLY)
  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this] {
#if defined(FEATURE_VIDEO)
    video_source_device_worker_ = utils::minor_worker(AGORA_VIDEO_SOURCE_DEVICE_WORKER, true);
    video_render_device_worker_ = utils::minor_worker(AGORA_VIDEO_RENDER_DEVICE_WORKER);
#endif  // FEATURE_VIDEO

    player_worker_ = utils::minor_worker(AGORA_VIDEO_PLAYER_WORKER);

    renderer_pool_ = MediaNodePool<IVideoRenderer>::Create([worker = video_render_device_worker_] {
#if !defined(RTC_TRANSMISSION_ONLY) && defined(FEATURE_VIDEO)
      return new RefCountedObject<rtc::VideoRendererWrapper>(worker);
#else
      return nullptr;
#endif  // !RTC_TRANSMISSION_ONLY && FEATURE_VIDEO
    });

    return 0;
  });
#endif  // !RTC_TRANSMISSION_ONLY
}

}  // namespace rtc
}  // namespace agora
