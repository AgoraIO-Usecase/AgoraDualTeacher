//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "media/microphone_audio_track.h"

#include <cstring>

#include "facilities/tools/api_logger.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"

#include "api2/IAgoraService.h"
#include "api2/NGIAgoraAudioDeviceManager.h"
#include "api2/NGIAgoraAudioTrack.h"
#include "api2/NGIAgoraMediaNodeFactory.h"

static const char* const MODULE_NAME = "[RTE.MAT]";

namespace agora {
namespace rte {

MicrophoneAudioTrack::MicrophoneAudioTrack(base::IAgoraService* service) {
  if (!service) {
    LOG_ERR_ASSERT_AND_RET("nullptr service in CTOR");
  }

  microphone_track_ = service->createLocalAudioTrack();
  if (!microphone_track_) {
    LOG_ERR_ASSERT_AND_RET("failed to create microphone track in CTOR");
  }
}

int MicrophoneAudioTrack::Start() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
#if FEATURE_ENABLE_UT_SUPPORT
    // TODO(tomiao): CI Win VM may failed to create microphone track
    if (!Ready()) {
      return ERR_OK;
    }
#endif  // FEATURE_ENABLE_UT_SUPPORT

    microphone_track_->setEnabled(true);

    return ERR_OK;
  });
}

int MicrophoneAudioTrack::Stop() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
#if FEATURE_ENABLE_UT_SUPPORT
    // TODO(tomiao): CI Win VM may failed to create microphone track
    if (!Ready()) {
      return ERR_OK_;
    }
#endif  // FEATURE_ENABLE_UT_SUPPORT

    if (microphone_track_->enableLocalPlayback(false) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to disable local playback");
    }

    microphone_track_->setEnabled(false);

    return ERR_OK_;
  });
}

int MicrophoneAudioTrack::SetStreamId(StreamId stream_id) {
  API_LOGGER_MEMBER("stream_id: %s", LITE_STR_CONVERT(stream_id));

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    stream_id_.assign(stream_id);
    return ERR_OK;
  });
}

int MicrophoneAudioTrack::GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const {
  API_LOGGER_MEMBER("stream_id_buf: %p, stream_id_buf_size: %zu", stream_id_buf,
                    stream_id_buf_size);

  if (!stream_id_buf) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "nullptr stream ID buf");
  }

  if (stream_id_buf_size <= 1) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "too small stream ID buf size");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    std::strncpy(stream_id_buf, stream_id_.c_str(), stream_id_buf_size - 1);
    stream_id_buf[stream_id_buf_size - 1] = '\0';

    return ERR_OK;
  });
}

int MicrophoneAudioTrack::SetStreamName(const char* stream_name) {
  API_LOGGER_MEMBER("stream_name: %s", LITE_STR_CONVERT(stream_name));

  if (utils::IsNullOrEmpty(stream_name)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid stream name");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    stream_name_.assign(stream_name);
    return ERR_OK;
  });
}

int MicrophoneAudioTrack::GetStreamName(char* stream_name_buf, size_t stream_name_buf_size) const {
  API_LOGGER_MEMBER("stream_name_buf: %p, stream_name_buf_size: %zu", stream_name_buf,
                    stream_name_buf_size);

  if (!stream_name_buf) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "nullptr stream name buf");
  }

  if (stream_name_buf_size <= 1) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "too small stream name buf size");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    std::strncpy(stream_name_buf, stream_name_.c_str(), stream_name_buf_size - 1);
    stream_name_buf[stream_name_buf_size - 1] = '\0';

    return ERR_OK;
  });
}

int MicrophoneAudioTrack::EnableLocalPlayback() {
  API_LOGGER_MEMBER(nullptr);

  return rtc::ui_thread_sync_call(LOCATION_HERE, [this] {
#if FEATURE_ENABLE_UT_SUPPORT
    // TODO(tomiao): CI Win VM may failed to create microphone track
    if (!Ready()) {
      return ERR_OK_;
    }
#endif  // FEATURE_ENABLE_UT_SUPPORT

    if (microphone_track_->enableLocalPlayback(true) != ERR_OK) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to enable local playback");
    }

    return ERR_OK_;
  });
}

agora_refptr<rtc::ILocalAudioTrack> MicrophoneAudioTrack::GetLocalAudioTrack() const {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<rtc::ILocalAudioTrack> audio_track;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    audio_track = microphone_track_;
    return ERR_OK;
  });

  return audio_track;
}

#ifdef FEATURE_ENABLE_UT_SUPPORT
bool MicrophoneAudioTrack::Ready() const {
  ASSERT_IS_UI_THREAD();

  return microphone_track_;
}
#endif  // FEATURE_ENABLE_UT_SUPPORT

}  // namespace rte
}  // namespace agora
