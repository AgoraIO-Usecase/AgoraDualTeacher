//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "media/media_control.h"

#include "facilities/tools/api_logger.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/strings/string_util.h"

//#include "api2/NGIAgoraAudioTrack.h"
//#include "api2/NGIAgoraVideoTrack.h"

//#include "rte_engine.h"

//#include "media/camera_video_track.h"
//#include "media/media_player.h"
//#include "media/microphone_audio_track.h"
//#include "media/screen_video_track.h"

static const char* const MODULE_NAME = "[RTE.MC]";

namespace agora {
namespace rte {

bool AgoraMessage::SetMessage(const char* msg) {
  if (utils::IsNullOrEmpty(msg)) {
    LOG_ERR_AND_RET_BOOL("invalid msg");
  }

  msg_.assign(msg);

  return true;
}

MediaControl::MediaControl(base::IAgoraService* service) : service_(service) {
  if (!service_) {
    LOG_ERR_ASSERT_AND_RET("nullptr service in CTOR");
  }
}

agora_refptr<IAgoraMessage> MediaControl::CreateMessage() {
  return new RefCountedObject<AgoraMessage>();
}

//agora_refptr<IAgoraCameraVideoTrack> MediaControl::CreateCameraVideoTrack() {
//  //API_LOGGER_MEMBER(nullptr);
//
//  //agora_refptr<IAgoraCameraVideoTrack> camera_track;
//
//  //(void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
//  //  //camera_track = new RefCountedObject<CameraVideoTrack>(service_, video_frame_observer_);
//  //  return ERR_OK;
//  //});
//
//  //return camera_track;
//	return nullptr;
//}

//agora_refptr<IAgoraScreenVideoTrack> MediaControl::CreateScreenVideoTrack() {
//  API_LOGGER_MEMBER(nullptr);
//
//  //agora_refptr<IAgoraScreenVideoTrack> screen_track;
//
//  //(void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
//  //  //screen_track = new RefCountedObject<ScreenVideoTrack>(service_, video_frame_observer_);
//  //  return ERR_OK;
//  //});
//
//  //return screen_track;
//  return nullptr;
//}
//
//agora_refptr<IAgoraMicrophoneAudioTrack> MediaControl::CreateMicrophoneAudioTrack() {
//  API_LOGGER_MEMBER(nullptr);
//
//  //agora_refptr<IAgoraMicrophoneAudioTrack> microphone_track;
//
//  //(void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
//  //  //microphone_track = new RefCountedObject<MicrophoneAudioTrack>(service_);
//  //  return ERR_OK;
//  //});
//
//  //return microphone_track;
//  return nullptr;
//}

//agora_refptr<IAgoraMediaPlayer> MediaControl::CreateMediaPlayer() {
//  API_LOGGER_MEMBER(nullptr);
//
//  /*agora_refptr<IAgoraMediaPlayer> media_player;
//
//  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
//	media_player = new RefCountedObject<MediaPlayer>(service_);
//	return ERR_OK;
//  });
//
//  return media_player;*/
//  return nullptr;
//}

int MediaControl::EnableLocalAudioFilter(const char* name, const char* vendor, bool enable) {
  API_LOGGER_MEMBER("name: %s, vendor: %s, enable: %s", LITE_STR_CONVERT(name),
                    LITE_STR_CONVERT(vendor), BOOL_TO_STR(enable));

  if (utils::IsNullOrEmpty(name) || utils::IsNullOrEmpty(vendor)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid name or vendor");
  }

  // TODO(tomiao): to implement...
  return -ERR_NOT_SUPPORTED;
}

int MediaControl::EnableLocalVideoFilter(const char* name, const char* vendor, bool enable) {
  API_LOGGER_MEMBER("name: %s, vendor: %s, enable: %s", LITE_STR_CONVERT(name),
                    LITE_STR_CONVERT(vendor), BOOL_TO_STR(enable));

  if (utils::IsNullOrEmpty(name) || utils::IsNullOrEmpty(vendor)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid name or vendor");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
	  return 0;/*(enable ? local_extensions_.CreateVideoFilter(name, vendor)
					 : local_extensions_.DestroyVideoFilter(name, vendor));*/
  });
}

int MediaControl::EnableRemoteVideoFilter(const char* name, const char* vendor, bool enable) {
  API_LOGGER_MEMBER("name: %s, vendor: %s, enable: %s", LITE_STR_CONVERT(name),
                    LITE_STR_CONVERT(vendor), BOOL_TO_STR(enable));

  if (utils::IsNullOrEmpty(name) || utils::IsNullOrEmpty(vendor)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid name or vendor");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
	  return 0;/*(enable ? remote_extensions_.CreateVideoFilter(name, vendor)
					 : remote_extensions_.DestroyVideoFilter(name, vendor))*/
  });
}

int MediaControl::SetExtensionProperty(const char* name, const char* vendor, const char* key,
                                       const void* value, int size) {
  API_LOGGER_MEMBER("name: %s, vendor: %s, key: %s, value: %p, size: %d", LITE_STR_CONVERT(name),
                    LITE_STR_CONVERT(vendor), LITE_STR_CONVERT(key), value, size);

  if (utils::IsNullOrEmpty(name) || utils::IsNullOrEmpty(vendor) || utils::IsNullOrEmpty(key)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid name, vendor or key");
  }

  if (!value) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "nullptr value");
  }

  return 0;
  /*return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    auto video_filter = remote_extensions_.GetVideoFilter(name, vendor);
    if (!video_filter) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to get video filter");
    }

    return video_filter->setProperty(key, value, size);
  });*/
}

int MediaControl::GetExtensionProperty(const char* name, const char* vendor, const char* key,
                                       void* value, int size) {
  API_LOGGER_MEMBER("name: %s, vendor: %s, key: %s, value: %p, size: %d", LITE_STR_CONVERT(name),
                    LITE_STR_CONVERT(vendor), LITE_STR_CONVERT(key), value, size);

  if (utils::IsNullOrEmpty(name) || utils::IsNullOrEmpty(vendor) || utils::IsNullOrEmpty(key)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid name, vendor or key");
  }

  if (!value) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "nullptr value");
  }

  /*return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    auto video_filter = remote_extensions_.GetVideoFilter(name, vendor);
    if (!video_filter) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to get video filter");
    }

    return video_filter->getProperty(key, value, size);
  });*/
  return 0;
}

void MediaControl::RegisterVideoFrameObserver(IVideoFrameObserver* observer) {
  API_LOGGER_MEMBER("observer: %p", observer);

  if (!observer) {
    LOG_ERR_AND_RET("nullptr observer");
  }

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    video_frame_observer_ = observer;
    return ERR_OK;
  });
}

void MediaControl::UnregisterVideoFrameObserver() {
  API_LOGGER_MEMBER(nullptr);

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
    video_frame_observer_ = nullptr;
    return ERR_OK;
  });
}

}  // namespace rte
}  // namespace agora
