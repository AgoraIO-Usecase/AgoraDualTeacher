

//
//  Agora Media SDK
//
//  Created by Bob Zhang in 2019.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//

#include "local_track_manager.h"

#include "api2/IAgoraService.h"
#include "api2/internal/audio_node_i.h"
#include "api2/internal/video_node_i.h"
#include "api2/internal/video_track_i.h"
#include "core/video/video_frame_metadata_observer.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "video_device_manager.h"

#if defined(HAS_BUILTIN_EXTENSIONS)
#include "agora_extension_provider.h"
#endif

const char MODULE_NAME[] = "[LTM]";

namespace agora {
namespace rtc {

LocalTrackManager::LocalTrackManager(base::IAgoraService* agoraService,
                                     const agora_refptr<IMediaNodeFactoryEx>& nodeFactory)
    : service_refptr_(agoraService),
      media_node_factory_ex_(nodeFactory),
      meta_send_callback_(utils::RtcSteadySyncCallback<IMetadataObserver>::Create()),
      meta_recv_callback_(utils::RtcAsyncCallback<IMetadataObserver>::Create()),
      recording_device_occupied_(false) {}

LocalTrackManager::~LocalTrackManager() = default;

agora_refptr<ILocalAudioTrack> LocalTrackManager::createLocalAudioTrack() {
  commons::log(commons::LOG_INFO, "%s: createLocalAudioTrack", MODULE_NAME);
  local_audio_track_ = service_refptr_->createLocalAudioTrack();
  addAudioFilters();
  return local_audio_track_;
}

void LocalTrackManager::setCaptureFormat(const VideoFormat& format) {
  if (!camera_capturer_) {
    commons::log(commons::LOG_ERROR, "%s: camera capturer not ready", MODULE_NAME);
    return;
  }

  if (format.fps > 0) {
    camera_capturer_->setCaptureFormat(format);
  } else {
    VideoFormat default_video_format;
    camera_capturer_->setCaptureFormat(VideoFormat(
        default_video_format.width, default_video_format.width, default_video_format.fps));
  }
}

agora_refptr<ILocalAudioTrack> LocalTrackManager::createMediaPlayerAudioTrack(
    int id, const agora_refptr<rtc::IMediaPlayerSource>& mediaPlayerSource) {
  commons::log(commons::LOG_INFO, "%s: createMediaAudioTrack", MODULE_NAME);

  media_player_tracks_.insert(
      std::pair<int, MediaPlayerSourceTrackInfo>(id, {nullptr, false, nullptr, false}));

  agora_refptr<ILocalAudioTrack> audio_track;
  if (!media_player_tracks_[id].audio_track) {
    audio_track = service_refptr_->createMediaPlayerAudioTrack(mediaPlayerSource);
    if (audio_track) {
      media_player_tracks_[id].audio_track = audio_track;
    } else {
      commons::log(commons::LOG_WARN,
                   "%s: Create media audio track for media player source %d failed", MODULE_NAME,
                   id);
    }
  } else {
    commons::log(commons::LOG_WARN, "%s: Media audio track for media player source %d has existed",
                 MODULE_NAME, id);
  }
  return audio_track;
}

agora_refptr<ILocalAudioTrack> LocalTrackManager::media_player_audio_track(int id) {
  agora_refptr<ILocalAudioTrack> audio_track;
  if (media_player_tracks_.find(id) != media_player_tracks_.end()) {
    audio_track = media_player_tracks_[id].audio_track;
  }
  return audio_track;
}

bool LocalTrackManager::media_player_audio_track_published(int id) {
  return media_player_tracks_.find(id) != media_player_tracks_.end() &&
         media_player_tracks_[id].audio_track && media_player_tracks_[id].audio_track_has_published;
}

int LocalTrackManager::publishMediaPlayerAudioTrack(int id) {
  auto iter = media_player_tracks_.find(id);
  if (iter != media_player_tracks_.end() && media_player_tracks_[id].audio_track &&
      !media_player_tracks_[id].audio_track_has_published) {
    media_player_tracks_[id].audio_track_has_published = true;
    return ERR_OK;
  } else {
    return -ERR_FAILED;
  }
}

int LocalTrackManager::unpublishMediaPlayerAudioTrack(int id) {
  auto iter = media_player_tracks_.find(id);
  if (iter != media_player_tracks_.end()) {
    media_player_tracks_[id].audio_track_has_published = false;
  }
  return ERR_OK;
}

int LocalTrackManager::destroyMediaPlayerAudioTrack(int id) {
  auto iter = media_player_tracks_.find(id);
  if (iter != media_player_tracks_.end()) {
    media_player_tracks_[id].audio_track.reset();
    media_player_tracks_[id].audio_track_has_published = false;
    if (!media_player_tracks_[id].video_track) {
      media_player_tracks_.erase(iter);
    }
  }
  return ERR_OK;
}

agora_refptr<ILocalVideoTrack> LocalTrackManager::createMediaPlayerVideoTrack(
    int id, const agora_refptr<rtc::IMediaPlayerSource>& mediaPlayerSource) {
  media_player_tracks_.insert(
      std::pair<int, MediaPlayerSourceTrackInfo>(id, {nullptr, false, nullptr, false}));

  agora_refptr<ILocalVideoTrack> video_track;
  if (!media_player_tracks_[id].video_track) {
    video_track = service_refptr_->createMediaPlayerVideoTrack(mediaPlayerSource);
    if (video_track) {
      media_player_tracks_[id].video_track = video_track;
    } else {
      commons::log(commons::LOG_WARN,
                   "%s: Create media video track for media player source %d failed", MODULE_NAME,
                   id);
    }
  } else {
    commons::log(commons::LOG_WARN, "%s: Media video track for media player source %d has existed",
                 MODULE_NAME, id);
  }
  return video_track;
}

agora_refptr<ILocalVideoTrack> LocalTrackManager::media_player_video_track(int id) {
  agora_refptr<ILocalVideoTrack> video_track;
  if (media_player_tracks_.find(id) != media_player_tracks_.end()) {
    video_track = media_player_tracks_[id].video_track;
  }
  return video_track;
}

bool LocalTrackManager::media_player_video_track_published(int id) {
  return media_player_tracks_.find(id) != media_player_tracks_.end() &&
         media_player_tracks_[id].video_track && media_player_tracks_[id].video_track_has_published;
}

int LocalTrackManager::publishMediaPlayerVideoTrack(int id) {
  auto iter = media_player_tracks_.find(id);
  if (iter != media_player_tracks_.end() && media_player_tracks_[id].video_track &&
      !media_player_tracks_[id].video_track_has_published) {
    media_player_tracks_[id].video_track_has_published = true;
    return ERR_OK;
  } else {
    return -ERR_FAILED;
  }
}

int LocalTrackManager::unpublishMediaPlayerVideoTrack(int id) {
  auto iter = media_player_tracks_.find(id);
  if (iter != media_player_tracks_.end()) {
    media_player_tracks_[id].video_track_has_published = false;
  }
  return ERR_OK;
}

int LocalTrackManager::destroyMediaPlayerVideoTrack(int id) {
  auto iter = media_player_tracks_.find(id);
  if (iter != media_player_tracks_.end()) {
    media_player_tracks_[id].video_track.reset();
    media_player_tracks_[id].video_track_has_published = false;
    if (!media_player_tracks_[id].audio_track) {
      media_player_tracks_.erase(iter);
    }
  }
  return ERR_OK;
}

agora_refptr<ILocalVideoTrack> LocalTrackManager::createLocalCameraTrack() {
  commons::log(commons::LOG_INFO, "%s: createLocalCameraTrack", MODULE_NAME);

  if (local_camera_track_) {
    return local_camera_track_;
  }

  camera_capturer_ = media_node_factory_ex_->createCameraCapturer();

#if defined(WEBRTC_ANDROID) || defined(WEBRTC_IOS)

  camera_capturer_->setCameraSource(camera_source_);

#elif defined(WEBRTC_WIN) || (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)) || \
    (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS))

  if (camera_dev_id_.empty()) {
    auto device_info = camera_capturer_->createDeviceInfo();

    if (!device_info) {
      commons::log(commons::LOG_ERROR, "failed to get device info.");
      return agora_refptr<rtc::ILocalVideoTrack>();
    }

    uint32_t dev_cnt = device_info->NumberOfDevices();

    if (0 == dev_cnt) {
      commons::log(commons::LOG_ERROR, "get no camera device.");
      device_info->release();
      return agora_refptr<rtc::ILocalVideoTrack>();
    }

    if (dev_cnt > 1) {
      commons::log(commons::LOG_INFO, "got %u devices, choose the one of index 0.", dev_cnt);
    }

    auto name = std::make_unique<char[]>(260);
    auto id = std::make_unique<char[]>(260);
    auto uuid = std::make_unique<char[]>(260);

    device_info->GetDeviceName(0, name.get(), 260, id.get(), 260, uuid.get(), 260);
    device_info->release();

    camera_dev_id_.assign(id.get());
  }

  commons::log(commons::LOG_INFO, "init camera capturer with device ID %s", camera_dev_id_.c_str());
  camera_capturer_->initWithDeviceId(camera_dev_id_.c_str());

#endif  // WEBRTC_ANDROID || WEBRTC_IOS

  if (!local_camera_track_) {
    local_camera_track_ = service_refptr_->createCameraVideoTrack(camera_capturer_);
    if (video_encode_config_.has_value()) {
      local_camera_track_->setVideoEncoderConfiguration(video_encode_config_.value());
    }
  }

  if (captured_video_frame_observer_) {
    VideoTrackInfo trackInfo = {0};
    // Represents local uid
    trackInfo.ownerUid = 0;
    agora_refptr<IVideoSinkBase> videoFrameObserver =
        media_node_factory_ex_->createObservableVideoSink(captured_video_frame_observer_,
                                                          trackInfo);
    local_camera_track_->addRenderer(videoFrameObserver);
  }

#if defined(FEATURE_VIDEO)
  if (meta_send_callback_->Size() > 0 || meta_recv_callback_->Size() > 0) {
    VideoTrackInfo trackInfo = {0};
    // Represents local uid
    trackInfo.ownerUid = 0;
    agora_refptr<rtc::IVideoFilter> videoFilterMetadataObserver =
        new RefCountedObject<VideoMetadataObserverImpl>(meta_send_callback_, meta_recv_callback_,
                                                        trackInfo);
    local_camera_track_->addVideoFilter(videoFilterMetadataObserver);
    videoFilterMetadataObserver->setEnabled(true);
  }
#endif  // FEATURE_VIDEO

  return local_camera_track_;
}

agora_refptr<ILocalAudioTrack> LocalTrackManager::createCustomAudioTrack(
    const agora_refptr<IAudioPcmDataSender>& audioSource) {
  commons::log(commons::LOG_INFO, "%s: createCustomAudioTrack", MODULE_NAME);
  // Custom Audio Track is kept in channel
  return service_refptr_->createCustomAudioTrack(audioSource);
}

agora_refptr<ILocalVideoTrack> LocalTrackManager::createCustomVideoTrack(
    const agora_refptr<IVideoFrameSender>& videoSource) {
  commons::log(commons::LOG_INFO, "%s: createCustomVideoTrack", MODULE_NAME);
  // Custom Video Track is kept in channel
  return service_refptr_->createCustomVideoTrack(videoSource);
}

agora_refptr<ILocalVideoTrack> LocalTrackManager::createCustomVideoTrack(
    const agora_refptr<IVideoEncodedImageSender>& videoSource) {
  base::SenderOptions options;
  options.ccMode = base::CC_DISABLED;
  return service_refptr_->createCustomVideoTrack(videoSource, options);
}

agora_refptr<ILocalAudioTrack> LocalTrackManager::obtainRecordingDeviceSourceTrack() {
  agora_refptr<ILocalAudioTrack> audio_track;
  if (!recording_device_occupied_) {
    if (!recording_device_source_) {
      // Low frequency call.
      auto audio_device_manager = service_refptr_->createAudioDeviceManager();
      if (audio_device_manager) {
        recording_device_source_ = audio_device_manager->createRecordingDeviceSource(nullptr);
      } else {
        commons::log(commons::LOG_WARN, "%s: create audio device manager failed", MODULE_NAME);
      }
    }
    if (recording_device_source_ && !recording_device_source_track_) {
      recording_device_source_track_ =
          service_refptr_->createRecordingDeviceAudioTrack(recording_device_source_);
    } else {
      commons::log(commons::LOG_WARN, "%s: Create recording device source failed.", MODULE_NAME);
    }
    recording_device_occupied_ = true;

    audio_track = recording_device_source_track_;
  } else {
    commons::log(commons::LOG_WARN, "%s: Recording device source has been occupied.", MODULE_NAME);
  }
  return audio_track;
}

agora_refptr<ILocalAudioTrack> LocalTrackManager::getRecordingDeviceSourceTrack() {
  return recording_device_source_track_;
}

void LocalTrackManager::releaseRecordingDeviceSourceTrack() {
  recording_device_source_track_.reset();
  recording_device_source_.reset();
  recording_device_occupied_ = false;
}

int LocalTrackManager::startRecording() {
  int ret = -ERR_NOT_READY;
  if (recording_device_source_) {
    if ((ret = recording_device_source_->initRecording()) == ERR_OK) {
      ret = recording_device_source_->startRecording();
    }
  }
  return ret;
}

int LocalTrackManager::stopRecording() {
  int ret = -ERR_NOT_READY;
  if (recording_device_source_) {
    ret = recording_device_source_->stopRecording();
  }
  return ret;
}

void LocalTrackManager::createLocalScreenTrack() {
  if (local_screen_track_) {
    return;
  }

  screen_capturer_ = media_node_factory_ex_->createScreenCapturer();
  local_screen_track_ = service_refptr_->createScreenVideoTrack(screen_capturer_);

  if (captured_video_frame_observer_) {
    VideoTrackInfo trackInfo = {0};
    // Represents local uid
    trackInfo.ownerUid = 0;
    agora_refptr<IVideoSinkBase> videoFrameObserver =
        media_node_factory_ex_->createObservableVideoSink(captured_video_frame_observer_,
                                                          trackInfo);
    local_screen_track_->addRenderer(videoFrameObserver);
  }
}

#if defined(_WIN32)
agora_refptr<ILocalVideoTrack> LocalTrackManager::createLocalScreenTrack(
    const Rectangle& screenRect, const Rectangle& regionRect) {
  commons::log(commons::LOG_INFO, "%s: createLocalScreenTrack", MODULE_NAME);
  if (!local_screen_track_) {
    createLocalScreenTrack();
  }

  assert(screen_capturer_);
  screen_capturer_->initWithScreenRect(screenRect, regionRect);
  return local_screen_track_;
}
#endif  // _WIN32

agora_refptr<ILocalVideoTrack> LocalTrackManager::createLocalScreenTrack(
    view_t sourceId, const Rectangle& regionRect) {
  commons::log(commons::LOG_INFO, "%s: createLocalScreenTrack", MODULE_NAME);
  if (!local_screen_track_) {
    createLocalScreenTrack();
  }

  assert(screen_capturer_);

#if TARGET_OS_MAC && !TARGET_OS_IPHONE
  screen_capturer_->initWithDisplayId(sourceId, regionRect);
#elif defined(_WIN32)
  screen_capturer_->initWithWindowId(sourceId, regionRect);
#endif  // TARGET_OS_MAC && !TARGET_OS_IPHONE

  return local_screen_track_;
}

#if defined(__ANDROID__)
agora_refptr<ILocalVideoTrack> LocalTrackManager::createLocalScreenTrack(
    void* mediaProjectionPermissionResultData, const VideoDimensions& dimensions) {
  commons::log(commons::LOG_INFO, "%s: createLocalScreenTrack", MODULE_NAME);
  if (local_screen_track_) {
    // Android media preojection do not support pause/resume.
    // Release the pre and create a new one each time.
    local_screen_track_ = nullptr;
  }

  screen_capturer_ = media_node_factory_ex_->createScreenCapturer();
  screen_capturer_->initWithMediaProjectionPermissionResultData(mediaProjectionPermissionResultData,
                                                                dimensions);
  local_screen_track_ = service_refptr_->createScreenVideoTrack(screen_capturer_);
  return local_screen_track_;
}
#endif  // __ANDROID__

#if defined(WEBRTC_WIN)
void LocalTrackManager::SetScreenCaptureSource(bool allow_magnification_api,
                                               bool allow_directx_capturer) {
  if (!screen_capturer_) {
    return;
  }

  auto screenCapturerEx = static_cast<IScreenCapturerEx*>(screen_capturer_.get());
  screenCapturerEx->SetCaptureSource(allow_magnification_api, allow_directx_capturer);
}
#endif  // WEBRTC_WIN

#if defined(WEBRTC_WIN) || (defined(WEBRTC_LINUX) && !defined(WEBRTC_ANDROID)) || \
    (defined(WEBRTC_MAC) && !defined(WEBRTC_IOS))
int LocalTrackManager::setCameraDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]) {
  if (!VideoDeviceCollection::ValidDeviceStr(dev_id)) {
    commons::log(commons::LOG_ERROR, "%s: invalid device ID in RtcEngine::setCameraDevice()");
    return -ERR_INVALID_ARGUMENT;
  }

  if (camera_dev_id_ == dev_id) {
    return ERR_OK;
  }

  camera_dev_id_.assign(dev_id);

  if (!local_camera_track_) {
    commons::log(commons::LOG_ERROR, "%s: local camera track not ready", MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  if (!camera_capturer_) {
    commons::log(commons::LOG_ERROR, "%s: camera capturer not ready", MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  local_camera_track_->setEnabled(false);
  if (camera_capturer_->initWithDeviceId(camera_dev_id_.c_str()) != ERR_OK) {
    commons::log(commons::LOG_ERROR, "%s: failed to init camera capturer with device id: %s",
                 MODULE_NAME, camera_dev_id_.c_str());
  }
  local_camera_track_->setEnabled(true);

  return ERR_OK;
}
#endif  // WEBRTC_WIN || (WEBRTC_LINUX && !WEBRTC_ANDROID) || (WEBRTC_MAC && !WEBRTC_IOS)

void LocalTrackManager::cleanupLocalMediaTracks() {
  commons::log(commons::LOG_INFO, "%s: cleanupLocalMediaTracks", MODULE_NAME);
  // local_audio_track_ = nullptr;
  camera_capturer_ = nullptr;
  local_camera_track_ = nullptr;
  screen_capturer_ = nullptr;
  local_screen_track_ = nullptr;
  media_player_tracks_.clear();
}

int LocalTrackManager::startPreview() {
  if (!local_camera_track_) return -ERR_INVALID_STATE;

  if (camera_renderer_) {
    (static_cast<IVideoRendererEx*>(camera_renderer_.get()))->setViewEx(camera_view_);
  } else {
    camera_renderer_ = media_node_factory_ex_->createVideoRenderer();
    (static_cast<IVideoRendererEx*>(camera_renderer_.get()))->setViewEx(camera_view_);
    commons::log(commons::LOG_INFO, "%s: start preview, create video renderer", MODULE_NAME);
  }

  if (local_screen_track_) {
    if (screen_renderer_) {
      (static_cast<IVideoRendererEx*>(screen_renderer_.get()))->setViewEx(screen_view_);
    } else {
      screen_renderer_ = media_node_factory_ex_->createVideoRenderer();
      (static_cast<IVideoRendererEx*>(screen_renderer_.get()))->setViewEx(screen_view_);
      commons::log(commons::LOG_INFO, "%s: start preview, create screen renderer", MODULE_NAME);
    }
    local_screen_track_->addRenderer(screen_renderer_);
  }

  setLocalVideoMirrorMode(mirror_mode_);
  local_camera_track_->addRenderer(camera_renderer_);
  local_camera_track_->setEnabled(true);

  // Add file dumper if needed
  if (yuv_dumper_to_file_) {
    agora_refptr<IVideoSinkBase> fileDumper = media_node_factory_ex_->createVideoSink("capturer");
    local_camera_track_->addRenderer(fileDumper);
  }

  return ERR_OK;
}

int LocalTrackManager::stopPreview() {
  if (camera_renderer_) {
    (static_cast<IVideoRendererEx*>(camera_renderer_.get()))->setViewEx(utils::kInvalidHandle);
  }

  if (local_camera_track_) {
    // Only disable local video if not published.
    if (!static_cast<ILocalVideoTrackEx*>(local_camera_track_.get())->hasPublished()) {
      local_camera_track_->setEnabled(false);
    }

    local_camera_track_->removeRenderer(camera_renderer_);
  }

  return ERR_OK;
}

int LocalTrackManager::setLocalVideoMirrorMode(VIDEO_MIRROR_MODE_TYPE mirrorMode) {
  mirror_mode_ = mirrorMode;
  if (!camera_renderer_) {
    camera_renderer_ = media_node_factory_ex_->createVideoRenderer();
    (static_cast<IVideoRendererEx*>(camera_renderer_.get()))->setViewEx(camera_view_);
    commons::log(commons::LOG_INFO, "%s: setLocalVideoMirrorMode, create video renderer",
                 MODULE_NAME);
  }

  switch (mirrorMode) {
    case VIDEO_MIRROR_MODE_AUTO:
#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IPHONE)
      camera_renderer_->setMirror(ICameraCapturer::CAMERA_FRONT == camera_source_);
#else
      camera_renderer_->setMirror(true);
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IPHONE)
      break;
    case VIDEO_MIRROR_MODE_ENABLED:
      camera_renderer_->setMirror(true);
      break;
    case VIDEO_MIRROR_MODE_DISABLED:
      camera_renderer_->setMirror(false);
      break;
    default:
      return -ERR_INVALID_ARGUMENT;
  }
  return ERR_OK;
}

int LocalTrackManager::setCameraRenderMode(media::base::RENDER_MODE_TYPE renderMode) {
  if (!camera_renderer_) {
    camera_renderer_ = media_node_factory_ex_->createVideoRenderer();
    (static_cast<IVideoRendererEx*>(camera_renderer_.get()))->setViewEx(camera_view_);
    commons::log(commons::LOG_INFO, "%s: setCameraRenderMode, create video renderer", MODULE_NAME);
  }
  return camera_renderer_->setRenderMode(renderMode);
}

void LocalTrackManager::setupLocalVideoView(const VideoCanvas& canvas) {
  if (canvas.isScreenView) {
    screen_view_ = ViewToHandle(canvas.view);
    if (screen_renderer_) {
      (static_cast<IVideoRendererEx*>(screen_renderer_.get()))->setViewEx(screen_view_);
    }
  } else {
    camera_view_ = ViewToHandle(canvas.view);
    if (camera_renderer_) {
      (static_cast<IVideoRendererEx*>(camera_renderer_.get()))->setViewEx(camera_view_);
    }
  }
}

int LocalTrackManager::enableLocalAudio(bool enabled) {
  if (local_audio_track_) {
    local_audio_track_->setEnabled(enabled);
  }
  return ERR_OK;
}

int LocalTrackManager::setLocalAudioReverbPreset(AUDIO_REVERB_PRESET reverbPreset) {
  if (reverbPreset != AUDIO_REVERB_OFF) {
    reverb_preset_ = reverbPreset;
  }
  return ERR_OK;
}

int LocalTrackManager::setLocalVoiceChangerPreset(VOICE_CHANGER_PRESET voiceChanger) {
  if (voiceChanger != VOICE_CHANGER_OFF) {
    voice_changer_preset_ = voiceChanger;
  }
  return ERR_OK;
}

int LocalTrackManager::setVideoEncoderConfig(const VideoEncoderConfiguration& config) {
  video_encode_config_ = config;
  if (!local_camera_track_) {
    // This case is not treated as error since we support configuring encoder before
    // camera track is built.
    return ERR_OK;
  }
  return local_camera_track_->setVideoEncoderConfiguration(config);
}

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IOS)
int LocalTrackManager::switchCameraInternal() {
  if (!camera_capturer_) {
    commons::log(commons::LOG_ERROR, "%s: camera capturer not ready", MODULE_NAME);
    return -ERR_INVALID_STATE;
  }

  auto current_source = camera_capturer_->getCameraSource();
  camera_source_ = current_source == ICameraCapturer::CAMERA_FRONT ? ICameraCapturer::CAMERA_BACK
                                                                   : ICameraCapturer::CAMERA_FRONT;
  return camera_capturer_->setCameraSource(camera_source_);
}

int LocalTrackManager::switchCamera() {
  if (!local_camera_track_) {
    return -ERR_INVALID_STATE;
  }

  // For VIDEO_MIRROR_MODE_AUTO, after switch camera, reconfig render's mirror property.
  // Note: before switch, diable local camera track to avoid using new mirror property
  //       render videoFrame captured before switch success.
  local_camera_track_->setEnabled(false);
  auto result = switchCameraInternal();
  if (mirror_mode_ == VIDEO_MIRROR_MODE_AUTO) {
    setLocalVideoMirrorMode(mirror_mode_);
  }
  local_camera_track_->setEnabled(true);
  return result;
}
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IOS)

int LocalTrackManager::addAudioFilters() {
  if (!local_audio_track_ || !media_node_factory_ex_) {
    commons::log(commons::LOG_WARN,
                 "%s: Add filter failed local audio track %p, media node factory %p ", MODULE_NAME,
                 local_audio_track_.get(), media_node_factory_ex_.get());
    return -ERR_FAILED;
  }

#if defined(HAS_BUILTIN_EXTENSIONS)
  auto reverb_filter =
      media_node_factory_ex_->createAudioFilter(BUILTIN_AUDIO_FILTER_REVERB, nullptr);
  if (reverb_filter && local_audio_track_->addAudioFilter(reverb_filter, IAudioTrack::Default)) {
    if (reverb_preset_ != AUDIO_REVERB_OFF) {
      reverb_filter->setProperty("preset", &reverb_preset_, sizeof(reverb_preset_));
    }
    reverb_filter->setEnabled(true);
  } else {
    commons::log(commons::LOG_WARN, "%s: Add reverb filter failed, reverb filter %p", MODULE_NAME,
                 reverb_filter.get());
  }

  auto voice_reshaper_filter =
      media_node_factory_ex_->createAudioFilter(BUILTIN_AUDIO_FILTER_VOICE_RESHAPER, nullptr);
  if (voice_reshaper_filter &&
      local_audio_track_->addAudioFilter(voice_reshaper_filter, IAudioTrack::Default)) {
    if (voice_changer_preset_ != VOICE_CHANGER_OFF) {
      reverb_filter->setProperty("preset", &voice_changer_preset_, sizeof(voice_changer_preset_));
    }
    voice_reshaper_filter->setEnabled(true);
  } else {
    commons::log(commons::LOG_WARN,
                 "%s: Add voice reshaper filter failed, voice reshaper filter %p", MODULE_NAME,
                 voice_reshaper_filter.get());
  }
#endif  // HAS_BUILTIN_EXTENSIONS

  return ERR_OK;
}

}  // namespace rtc
}  // namespace agora
