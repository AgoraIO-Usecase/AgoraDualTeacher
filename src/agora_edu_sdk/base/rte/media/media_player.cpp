//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "media_player.h"

#include "facilities/tools/api_logger.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/strings/string_util.h"

#include "api2/IAgoraService.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "base/IAgoraMediaPlayerSource.h"

#include "core/video/video_track_configurator.h"

static const char* const MODULE_NAME = "[RTE.MP]";

namespace agora {
namespace rte {

MediaPlayer::MediaPlayer(base::IAgoraService* service) {
  if (!service) {
    LOG_ERR_ASSERT_AND_RET("nullptr service in CTOR");
  }

  if (!(media_node_factory_ = service->createMediaNodeFactory())) {
    LOG_ERR_ASSERT_AND_RET("failed to create media node factory in CTOR");
  }

  if (!(media_player_source_ = media_node_factory_->createMediaPlayerSource())) {
    LOG_ERR_ASSERT_AND_RET("failed to create media player source in CTOR");
  }
}

int MediaPlayer::getSourceId() const {
  API_LOGGER_MEMBER(nullptr);

  int source_id = -1;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    source_id = media_player_source_->getSourceId();
    return ERR_OK;
  });

  return source_id;
}

#define DEFINE_MP_FUNC_NO_ARG(func)                                                   \
  int MediaPlayer::func() {                                                           \
    API_LOGGER_MEMBER(nullptr);                                                       \
                                                                                      \
    return rtc::ui_thread_sync_call(LOCATION_HERE,                                    \
                                    [this] { return media_player_source_->func(); }); \
  }

#define DEFINE_MP_FUNC_INT64_T(func, arg)                                             \
  int MediaPlayer::func(int64_t arg) {                                                \
    API_LOGGER_MEMBER(#arg ": %" PRId64, arg);                                        \
                                                                                      \
    return rtc::ui_thread_sync_call(LOCATION_HERE,                                    \
                                    [=] { return media_player_source_->func(arg); }); \
  }

#define DEFINE_MP_FUNC_INT64_T_REF(func, arg)                                         \
  int MediaPlayer::func(int64_t& arg) {                                               \
    API_LOGGER_MEMBER("arg: %p", &arg);                                               \
                                                                                      \
    return rtc::ui_thread_sync_call(LOCATION_HERE,                                    \
                                    [&] { return media_player_source_->func(arg); }); \
  }

#define DEFINE_MP_FUNC_LITE_STR(func, arg)                                            \
  int MediaPlayer::func(const char* arg) {                                            \
    API_LOGGER_MEMBER(#arg ": %s", LITE_STR_CONVERT(arg));                            \
                                                                                      \
    return rtc::ui_thread_sync_call(LOCATION_HERE,                                    \
                                    [=] { return media_player_source_->func(arg); }); \
  }

#define DEFINE_MP_FUNC_LITE_STR_INT64_T(func, arg_1, arg_2)                                    \
  int MediaPlayer::func(const char* arg_1, int64_t arg_2) {                                    \
    API_LOGGER_MEMBER(#arg_1 ": %s, " #arg_2 ": %" PRId64, LITE_STR_CONVERT(arg_1), arg_2);    \
                                                                                               \
    return rtc::ui_thread_sync_call(LOCATION_HERE,                                             \
                                    [=] { return media_player_source_->func(arg_1, arg_2); }); \
  }

DEFINE_MP_FUNC_LITE_STR_INT64_T(open, url, start_pos)
DEFINE_MP_FUNC_NO_ARG(play)

DEFINE_MP_FUNC_NO_ARG(pause)
DEFINE_MP_FUNC_NO_ARG(stop)
DEFINE_MP_FUNC_NO_ARG(resume)
DEFINE_MP_FUNC_INT64_T(seek, new_pos_ms)

DEFINE_MP_FUNC_INT64_T_REF(getDuration, dur_ms)
DEFINE_MP_FUNC_INT64_T_REF(getPlayPosition, curr_pos_ms)
DEFINE_MP_FUNC_INT64_T_REF(getStreamCount, count)

int MediaPlayer::getStreamInfo(int64_t index, media::base::MediaStreamInfo* info) {
  API_LOGGER_MEMBER("index: %" PRId64 ", info: %p", index, info);

  return rtc::ui_thread_sync_call(LOCATION_HERE,
                                  [=] { return media_player_source_->getStreamInfo(index, info); });
}

DEFINE_MP_FUNC_INT64_T(setLoopCount, loop_count)

int MediaPlayer::changePlaybackSpeed(media::base::MEDIA_PLAYER_PLAYBACK_SPEED speed) {
  API_LOGGER_MEMBER("speed: %d", speed);

  return rtc::ui_thread_sync_call(LOCATION_HERE,
                                  [=] { return media_player_source_->changePlaybackSpeed(speed); });
}

DEFINE_MP_FUNC_INT64_T(selectAudioTrack, index)
DEFINE_MP_FUNC_LITE_STR_INT64_T(setPlayerOption, key, value)
DEFINE_MP_FUNC_LITE_STR(takeScreenshot, file_name)
DEFINE_MP_FUNC_INT64_T(selectInternalSubtitle, index)
DEFINE_MP_FUNC_LITE_STR(setExternalSubtitle, url)

media::base::MEDIA_PLAYER_STATE MediaPlayer::getState() {
  API_LOGGER_MEMBER(nullptr);

  media::base::MEDIA_PLAYER_STATE state = media::base::PLAYER_STATE_FAILED;

  (void)rtc::ui_thread_sync_call(LOCATION_HERE, [&] {
    state = media_player_source_->getState();
    return ERR_OK;
  });

  return state;
}

int MediaPlayer::registerPlayerSourceObserver(rtc::IMediaPlayerSourceObserver* observer) {
  return observerCommon(observer, &rtc::IMediaPlayerSource::registerPlayerSourceObserver);
}

int MediaPlayer::unregisterPlayerSourceObserver(rtc::IMediaPlayerSourceObserver* observer) {
  return observerCommon(observer, &rtc::IMediaPlayerSource::unregisterPlayerSourceObserver);
}

int MediaPlayer::registerAudioFrameObserver(media::base::IAudioFrameObserver* observer) {
  return observerCommon(observer, &rtc::IMediaPlayerSource::registerAudioFrameObserver);
}

int MediaPlayer::unregisterAudioFrameObserver(media::base::IAudioFrameObserver* observer) {
  return observerCommon(observer, &rtc::IMediaPlayerSource::unregisterAudioFrameObserver);
}

int MediaPlayer::SetStreamId(StreamId stream_id) {
  API_LOGGER_MEMBER("stream_id: %s", stream_id);

  if (utils::IsNullOrEmpty(stream_id)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid stream ID");
  }

  return rtc::ui_thread_sync_call(LOCATION_HERE, [=] {
    stream_id_.assign(stream_id);
    return ERR_OK;
  });
}

int MediaPlayer::GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const {
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

}  // namespace rte
}  // namespace agora
