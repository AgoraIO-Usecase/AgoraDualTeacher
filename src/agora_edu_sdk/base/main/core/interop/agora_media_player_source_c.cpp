//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.8
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_observer_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/AgoraRefCountedObject.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "base/IAgoraMediaPlayerSource.h"
#include "base/agora_base.h"
#include "base/agora_media_player_source.h"

namespace {

void copy_media_stream_info(media_stream_info* info,
                            const agora::media::base::MediaStreamInfo& cpp_info) {
  if (!info) {
    return;
  }
  for (int i = 0; i < k_max_codec_name_len; ++i) {
    info->codec_name[i] = cpp_info.codecName[i];
  }
  for (int i = 0; i < k_max_codec_name_len; ++i) {
    info->language[i] = cpp_info.language[i];
  }
  info->audio_bits_per_sample = cpp_info.audioBitsPerSample;
  info->audio_channels = cpp_info.audioChannels;
  info->audio_sample_rate = cpp_info.audioSampleRate;
  info->duration = cpp_info.duration;
  info->stream_index = cpp_info.streamIndex;
  info->video_bit_rate = cpp_info.videoBitRate;
  info->video_frame_rate = cpp_info.videoFrameRate;
  info->video_height = cpp_info.videoHeight;
  info->video_rotation = cpp_info.videoRotation;
  info->video_width = cpp_info.videoWidth;
}
DEFINE_CREATE_AND_DESTROY_STRUCT_FUNCS(media_stream_info, agora::media::base::MediaStreamInfo);

class CMediaPlayerSourceObserver
    : public agora::rtc::IMediaPlayerSourceObserver,
      public agora::interop::CAgoraCallback<agora_media_player_source_observer> {
 public:
  CMediaPlayerSourceObserver() = default;
  ~CMediaPlayerSourceObserver() override = default;

  void onPlayerSourceStateChanged(agora::media::base::MEDIA_PLAYER_STATE state,
                                  agora::media::base::MEDIA_PLAYER_ERROR ec) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_player_source_state_changed) {
        p.second.on_player_source_state_changed(p.first, state, ec);
      }
    }
  }

  void onPositionChanged(int64_t position) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_position_changed) {
        p.second.on_position_changed(p.first, position);
      }
    }
  }

  void onPlayerEvent(agora::media::base::MEDIA_PLAYER_EVENT event) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_player_event) {
        p.second.on_player_event(p.first, event);
      }
    }
  }

  void onMetaData(const void* data, int length) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_meta_data) {
        p.second.on_meta_data(p.first, data, length);
      }
    }
  }

  void onCompleted() override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_completed) {
        p.second.on_completed(p.first);
      }
    }
  }
};

CMediaPlayerSourceObserver g_agora_media_player_source_central_observer;

}  // namespace

AGORA_API_C_INT agora_media_player_source_get_source_id(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->getSourceId();
}

AGORA_API_C_INT agora_media_player_source_open(AGORA_HANDLE agora_media_player_source,
                                               const char* url, int64_t start_pos) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->open(url, start_pos);
}

AGORA_API_C_INT agora_media_player_source_play(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->play();
}

AGORA_API_C_INT agora_media_player_source_pause(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->pause();
}

AGORA_API_C_INT agora_media_player_source_stop(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->stop();
}

AGORA_API_C_INT agora_media_player_source_resume(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->resume();
}

AGORA_API_C_INT agora_media_player_source_seek(AGORA_HANDLE agora_media_player_source,
                                               int64_t new_pos) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->seek(new_pos);
}

AGORA_API_C int64_t AGORA_CALL_C
agora_media_player_source_get_duration(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);
  int64_t duration = 0;
  int ret = agora_media_player_source_holder->Get()->getDuration(duration);
  if (ret != 0) {
    return -1;
  }
  return duration;
}

AGORA_API_C int64_t AGORA_CALL_C
agora_media_player_source_get_play_position(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);
  int64_t position = 0;
  int ret = agora_media_player_source_holder->Get()->getPlayPosition(position);
  if (ret != 0) {
    return -1;
  }
  return position;
}

AGORA_API_C int64_t AGORA_CALL_C
agora_media_player_source_get_stream_count(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);
  int64_t count = 0;
  int ret = agora_media_player_source_holder->Get()->getStreamCount(count);
  if (ret != 0) {
    return -1;
  }
  return count;
}

AGORA_API_C media_stream_info* AGORA_CALL_C
agora_media_player_source_get_stream_info(AGORA_HANDLE agora_media_player_source, int64_t index) {
  if (!agora_media_player_source) {
    return nullptr;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);
  agora::media::base::MediaStreamInfo cpp_info;
  int ret = agora_media_player_source_holder->Get()->getStreamInfo(index, &cpp_info);
  if (ret != 0) {
    return nullptr;
  }
  return create_media_stream_info(cpp_info);
}

AGORA_API_C_VOID agora_media_player_source_destroy_stream_info(
    AGORA_HANDLE agora_media_player_source, media_stream_info* info) {
  destroy_media_stream_info(&info);
}

AGORA_API_C_INT agora_media_player_source_set_loop_count(AGORA_HANDLE agora_media_player_source,
                                                         int loop_count) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->setLoopCount(loop_count);
}

AGORA_API_C_INT agora_media_player_source_change_playback_speed(
    AGORA_HANDLE agora_media_player_source, int speed) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);
  agora::media::base::MEDIA_PLAYER_PLAYBACK_SPEED cpp_speed;
  cpp_speed = agora::media::base::MEDIA_PLAYER_PLAYBACK_SPEED(speed);
  return agora_media_player_source_holder->Get()->changePlaybackSpeed(cpp_speed);
}

AGORA_API_C_INT agora_media_player_source_select_audio_track(AGORA_HANDLE agora_media_player_source,
                                                             int index) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->selectAudioTrack(index);
}

AGORA_API_C_INT agora_media_player_source_set_player_option(AGORA_HANDLE agora_media_player_source,
                                                            const char* key, int value) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->setPlayerOption(key, value);
}

AGORA_API_C_INT agora_media_player_source_take_screenshot(AGORA_HANDLE agora_media_player_source,
                                                          const char* filename) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->takeScreenshot(filename);
}

AGORA_API_C_INT agora_media_player_source_select_internal_subtitle(
    AGORA_HANDLE agora_media_player_source, int index) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->selectInternalSubtitle(index);
}

AGORA_API_C_INT agora_media_player_source_set_external_subtitle(
    AGORA_HANDLE agora_media_player_source, const char* url) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->setExternalSubtitle(url);
}

AGORA_API_C_INT agora_media_player_source_get_state(AGORA_HANDLE agora_media_player_source) {
  if (!agora_media_player_source) {
    return -1;
  }
  REF_PTR_HOLDER_CAST(agora_media_player_source_holder, agora::rtc::IMediaPlayerSource,
                      agora_media_player_source);

  return agora_media_player_source_holder->Get()->getState();
}

AGORA_API_C_INT agora_media_player_source_register_player_source_observer(
    AGORA_HANDLE agora_media_player_source, agora_media_player_source_observer* observer) {
  if (!agora_media_player_source || !observer) {
    return -1;
  }
  g_agora_media_player_source_central_observer.Add(agora_media_player_source, observer);
  return 0;
}

AGORA_API_C_INT agora_media_player_source_unregister_player_source_observer(
    AGORA_HANDLE agora_media_player_source, agora_media_player_source_observer* observer) {
  if (!agora_media_player_source || !observer) {
    return -1;
  }
  g_agora_media_player_source_central_observer.Remove(agora_media_player_source);
  return 0;
}

AGORA_API_C_INT agora_media_player_source_register_audio_frame_observer(
    AGORA_HANDLE agora_media_player_source, audio_frame_observer* observer) {
  if (!agora_media_player_source || !observer) {
    return -1;
  }
  g_audio_frame_central_observer.Add(agora_media_player_source, observer);
  return 0;
}

AGORA_API_C_INT agora_media_player_source_unregister_audio_frame_observer(
    AGORA_HANDLE agora_media_player_source, audio_frame_observer* observer) {
  if (!agora_media_player_source || !observer) {
    return -1;
  }
  g_audio_frame_central_observer.Remove(agora_media_player_source);
  return 0;
}
