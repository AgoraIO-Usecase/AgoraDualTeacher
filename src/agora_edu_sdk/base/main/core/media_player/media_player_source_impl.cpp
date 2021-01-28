//
//  Agora RTC/MEDIA SDK
//
//  Created by Tommy Miao in 2020-03.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "media_player_source_impl.h"

#include <cinttypes>

#include "api2/IAgoraService.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "facilities/tools/api_logger.h"
#include "facilities/tools/audio_utils.h"
#include "main/core/audio/audio_local_track_pcm.h"
#include "main/core/video/video_local_track_yuv.h"
#include "simple_player/media_player_source_dummy.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/strings/string_util.h"
#include "utils/thread/thread_pool.h"

#if defined(ENABLE_MEDIA_PLAYER)
#if defined(ENABLE_FFMPEG_PLAYER)
#include "src/media_player_source_ffmpeg.h"
#endif  // ENABLE_FFMPEG_PLAYER

#if defined(WEBRTC_WIN)
#include "main/core/media_player/simple_player/win/media_player_source_win.h"
#elif defined(WEBRTC_ANDROID)
#include "main/core/media_player/simple_player/android/media_player_source_android.h"
#elif defined(WEBRTC_LINUX)
#include "main/core/media_player/simple_player/linux/media_player_source_linux.h"
#elif defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
#include "main/core/media_player/simple_player/apple/media_player_source_apple.h"
#endif  // WEBRTC_WIN
#endif  // ENABLE_MEDIA_PLAYER

using namespace std::placeholders;

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[MPSI]";

namespace {
agora_refptr<IMediaPlayerSource> create_ffmpeg_media_player_source(
    base::IAgoraService* agora_service, utils::worker_type player_worker) {
#if defined(ENABLE_FFMPEG_PLAYER)
  auto media_player_source = new RefCountedObject<rtc::MediaPlayerSourceFfmpeg>(
      agora_service, utils::GetUtilGlobal()->thread_pool.get(), player_worker);
  return media_player_source;
#else
  return nullptr;
#endif  // ENABLE_FFMPEG_PLAYER
}

static agora_refptr<IMediaPlayerSource> create_simple_media_player_source(
    base::IAgoraService* agora_service, utils::worker_type player_worker) {
  agora_refptr<IMediaPlayerSource> media_player_source;
#if defined(WEBRTC_WIN)
  media_player_source =
      new RefCountedObject<rtc::MediaPlayerSourceWin>(agora_service, player_worker);
#elif defined(WEBRTC_ANDROID)
  media_player_source =
      new RefCountedObject<rtc::MediaPlayerSourceAndroid>(agora_service, player_worker);
#elif defined(WEBRTC_LINUX)
  // media_player_source = new RefCountedObject<rtc::MediaPlayerSourceLinux>(agora_service,
  // player_worker);
#elif defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
  media_player_source = new RefCountedObject<MediaPlayerSourceApple>(agora_service, player_worker);
#endif  // WEBRTC_WIN

  if (!media_player_source) {
    // TODO(hanpfei): Remove the following lines after implementing real media player
    // on all system platform.
    media_player_source =
        new RefCountedObject<rtc::MediaPlayerSourceDummy>(agora_service, player_worker);
  }

  return media_player_source;
}
}  // namespace

agora_refptr<IMediaPlayerSource> IMediaPlayerSourceEx::Create(
    base::IAgoraService* agora_service, utils::worker_type player_worker,
    media::base::MEDIA_PLAYER_SOURCE_TYPE type) {
  assert(agora_service && player_worker);

  agora_refptr<IMediaPlayerSource> media_player_source;

  (void)player_worker->sync_call(
      LOCATION_HERE, [&media_player_source, agora_service, player_worker, type] {
#if defined(ENABLE_MEDIA_PLAYER)
        if (type == media::base::MEDIA_PLAYER_SOURCE_FULL_FEATURED) {
          media_player_source = create_ffmpeg_media_player_source(agora_service, player_worker);
        } else if (type == media::base::MEDIA_PLAYER_SOURCE_SIMPLE) {
          media_player_source = create_simple_media_player_source(agora_service, player_worker);
        } else if (type == media::base::MEDIA_PLAYER_SOURCE_DEFAULT) {
          media_player_source = create_ffmpeg_media_player_source(agora_service, player_worker);
          if (!media_player_source) {
            media_player_source = create_simple_media_player_source(agora_service, player_worker);
          }
        }
#endif  // ENABLE_MEDIA_PLAYER

        if (!media_player_source) {
          LOG_WARN_AND_RET_INT(ERR_NOT_READY, "failed to create media player source type %d", type);
        }

        return ERR_OK_;
      });

  return media_player_source;
}

std::atomic<int32_t> MediaPlayerSourceImpl::source_ids_ = {0};

MediaPlayerSourceImpl::MediaPlayerSourceImpl(base::IAgoraService* agora_service,
                                             utils::worker_type player_worker)
    : source_id_(source_ids_++),
      agora_service_(agora_service),
      player_worker_(player_worker),
      player_state_(media::base::PLAYER_STATE_IDLE),
      observers_(utils::RtcAsyncCallback<IMediaPlayerSourceObserver>::Create()),
      audio_frame_observers_(utils::RtcSyncCallback<media::base::IAudioFrameObserver>::Create()) {
  (void)player_worker_->sync_call(LOCATION_HERE, [this] {
    media_node_factory_ = agora_service_->createMediaNodeFactory();
    audio_pcm_data_sender_ = media_node_factory_->createAudioPcmDataSender();
    video_frame_sender_ = media_node_factory_->createVideoFrameSender();

    //
    // At any time, there will be only one thread (major worker or an external thread) calling
    // _changeState() to update player state because of the lock. When the next state is
    // ***ing, we intend to lock current state and let player worker to do the real job via do***().
    // When player worker finishes its job, it will call back _changeState() to update the
    // player state again, overall, should be thread safe.
    //
    // current state         action             next state         func
    //------------------------------------------------------------------------------
    // idle                  open               opening*           doOpen
    // opening               open_done          open completed*    _doNothing
    // opening               open_failed        FAILED*            _doNothing
    // opening (FFMPEG)      stop               stopping           doStop
    // open completed        open               opening*           doOpen
    // open completed        play               playing*           doPlay
    // open completed        stop               stopping           doStop
    // open completed        seek               seeking            doSeek
    // open completed        get_dur            getting            doGetDuration
    // open completed        get_play_pos       getting            doGetPlayPosition
    // open completed        get_stream_cnt     getting            doGetStreamCount
    // open completed        get_stream_info    getting            doGetStreamInfo
    // playing               pause              pausing            doPause
    // playing               stop               stopping           doStop
    // playing               seek               seeking            doSeek
    // playing               get_dur            getting            doGetDuration
    // playing               get_play_pos       getting            doGetPlayPosition
    // playing               get_stream_cnt     getting            doGetStreamCount
    // playing               get_stream_info    getting            doGetStreamInfo
    // playing               play_done          idle*              _doNothing
    // playing               rewind_failed      FAILED*            _doNothing
    // paused                play               playing*           doPlay
    // paused                stop               stopping           doStop
    // paused                resume             playing*           doResume (doPlay)
    // paused                seek               seeking            doSeek
    // paused                get_dur            getting            doGetDuration
    // paused                get_play_pos       getting            doGetPlayPosition
    // paused                get_stream_cnt     getting            doGetStreamCount
    // paused                get_stream_info    getting            doGetStreamInfo
    // playback completed    open               opening*           doOpen
    // playback completed    stop               stopping           doStop
    // playback completed    seek               seeking            doSeek
    // playback completed    get_dur            getting            doGetDuration
    // playback completed    get_play_pos       getting            doGetPlayPosition
    // playback completed    get_stream_cnt     getting            doGetStreamCount
    // playback completed    get_stream_info    getting            doGetStreamInfo
    // pausing               pause_done         paused*            _doNothing
    // stopping              stop_done          idle*              _doNothing
    // stopping              stop_failed        FAILED*            _doNothing
    // seeking               seek_done          prev_state         _doNothing
    // getting               get_done           prev_state         _doNothing
    // FAILED                open               opening*           doOpen
    // FAILED                stop               stopping           doStop

    //
    // NOTE: Only the next states marked with '*' will notify changed.
    //

    // doOpen() will be the only async_call one
    action_tuple_type open_action_tuple = {
        OPEN, media::base::PLAYER_STATE_OPENING, /* next_state */
        [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,
               media::base::MediaStreamInfo* info, media::base::MEDIA_PLAYER_STATE prev_state) {
          player_worker_->async_call(LOCATION_HERE, [this, url] {
            _notifyStateChanged(media::base::PLAYER_STATE_OPENING);

            uint64_t begin_ms = commons::now_ms();
            bool open_succeeded = doOpen(url.c_str(), 0);
            uint64_t end_ms = commons::now_ms();

#if defined(ENABLE_FFMPEG_PLAYER)
            // for FFMPEG player, doOpen() internally is an async call to an external thread,
            // that thread will update and notify state after done its job
            if (!open_succeeded) {
              updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
            }
#else
            if (open_succeeded) {
              updateStateAndNotify(media::base::PLAYER_STATE_OPEN_COMPLETED);
            } else {
              updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
            }
#endif  // ENABLE_FFMPEG_PLAYER

            LOG_WARN("doOpen() elapsed: %" PRId64 "ms", end_ms - begin_ms);
          });

          return ERR_OK;
        }};

    action_tuple_type play_action_tuple = {
        PLAY, media::base::PLAYER_STATE_PLAYING,
        [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,
               media::base::MediaStreamInfo* info, media::base::MEDIA_PLAYER_STATE prev_state) {
          return player_worker_->sync_call(LOCATION_HERE, [this] {
            doPlay();
            _notifyStateChanged(media::base::PLAYER_STATE_PLAYING);
            return ERR_OK;
          });
        }};

    action_tuple_type pause_action_tuple = {
        PAUSE, media::base::PLAYER_STATE_PAUSING_INTERNAL,
        [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,
               media::base::MediaStreamInfo* info, media::base::MEDIA_PLAYER_STATE prev_state) {
          return player_worker_->sync_call(LOCATION_HERE, [this] {
            doPause();
            updateStateAndNotify(media::base::PLAYER_STATE_PAUSED);
            return ERR_OK;
          });
        }};

    action_tuple_type stop_action_tuple = {
        STOP, media::base::PLAYER_STATE_STOPPING_INTERNAL,
        [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,
               media::base::MediaStreamInfo* info, media::base::MEDIA_PLAYER_STATE prev_state) {
          return player_worker_->sync_call(LOCATION_HERE, [this] {
            if (doStop()) {
              updateStateAndNotify(media::base::PLAYER_STATE_IDLE);
              return ERR_OK_;
            } else {
              updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
              return -ERR_FAILED;
            }
          });
        }};

    action_tuple_type resume_action_tuple = {
        RESUME, media::base::PLAYER_STATE_PLAYING,
        [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,
               media::base::MediaStreamInfo* info, media::base::MEDIA_PLAYER_STATE prev_state) {
          return player_worker_->sync_call(LOCATION_HERE, [this] {
            doResume();
            _notifyStateChanged(media::base::PLAYER_STATE_PLAYING);
            return ERR_OK;
          });
        }};

    action_tuple_type seek_action_tuple = {
        SEEK, media::base::PLAYER_STATE_SEEKING_INTERNAL,
        [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,
               media::base::MediaStreamInfo* info, media::base::MEDIA_PLAYER_STATE prev_state) {
          return player_worker_->sync_call(LOCATION_HERE, [this, new_pos_ms, prev_state] {
            doSeek(new_pos_ms);

            if (prev_state == media::base::PLAYER_STATE_NONE_INTERNAL) {
              LOG_ERR_AND_RET_INT(ERR_FAILED, "prev_state NONE in seek_action_tuple");
            }

            _revertToPrevState(prev_state);

            return ERR_OK_;
          });
        }};

#define DECLARE_NORMAL_GET_ACTION_TUPLE(action, action_tuple, doGetFunc)                       \
  action_tuple_type action_tuple = {                                                           \
      action, media::base::PLAYER_STATE_GETTING_INTERNAL,                                      \
      [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,             \
             media::base::MediaStreamInfo* info, media::base::MEDIA_PLAYER_STATE prev_state) { \
        return player_worker_->sync_call(LOCATION_HERE, [this, get_val, prev_state] {          \
          if (!get_val) {                                                                      \
            LOG_ERR_AND_RET_INT(ERR_FAILED, "get_val nullptr in " #action_tuple);              \
          }                                                                                    \
                                                                                               \
          doGetFunc(*get_val);                                                                 \
                                                                                               \
          if (prev_state == media::base::PLAYER_STATE_NONE_INTERNAL) {                         \
            LOG_ERR_AND_RET_INT(ERR_FAILED, "prev_state NONE in " #action_tuple);              \
          }                                                                                    \
                                                                                               \
          _revertToPrevState(prev_state);                                                      \
                                                                                               \
          return ERR_OK_;                                                                      \
        });                                                                                    \
      }};

    DECLARE_NORMAL_GET_ACTION_TUPLE(GET_DUR, get_dur_action_tuple, doGetDuration)
    DECLARE_NORMAL_GET_ACTION_TUPLE(GET_PLAY_POS, get_play_pos_action_tuple, doGetPlayPosition)
    DECLARE_NORMAL_GET_ACTION_TUPLE(GET_STREAM_CNT, get_stream_cnt_action_tuple, doGetStreamCount)

    action_tuple_type get_stream_info_action_tuple = {
        GET_STREAM_INFO, media::base::PLAYER_STATE_GETTING_INTERNAL,
        [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,
               media::base::MediaStreamInfo* info, media::base::MEDIA_PLAYER_STATE prev_state) {
          return player_worker_->sync_call(LOCATION_HERE, [this, index, info, prev_state] {
            if (!info) {
              LOG_ERR_AND_RET_INT(ERR_FAILED, "info nullptr in get_stream_info_action_tuple");
            }

            int64_t stream_cnt = 0;
            doGetStreamCount(stream_cnt);
            if (0 == stream_cnt) {
              LOG_WARN("zero stream count in get_stream_info_action_tuple");
            } else if (index < 0) {
              LOG_WARN("negative index in get_stream_info_action_tuple");
            } else if (index >= stream_cnt) {
              LOG_WARN("index >= stream count in get_stream_info_action_tuple");
            } else {
              doGetStreamInfo(index, info);
            }

            if (prev_state == media::base::PLAYER_STATE_NONE_INTERNAL) {
              LOG_ERR_AND_RET_INT(ERR_FAILED, "prev_state NONE in get_stream_info_action_tuple");
            }

            _revertToPrevState(prev_state);

            return ERR_OK_;
          });
        }};

#ifdef MP_SRC_SM_FULL_MAP
    auto do_nothing = std::bind(&MediaPlayerSourceImpl::_doNothing, this, _1, _2, _3, _4, _5, _6);

    action_tuple_type open_done_action_tuple = {OPEN_DONE, PLAYER_STATE_OPEN_COMPLETED, do_nothing};
    action_tuple_type open_failed_action_tuple = {OPEN_FAILED, PLAYER_STATE_FAILED, do_nothing};
    action_tuple_type play_done_action_tuple = {PLAY_DONE, PLAYER_STATE_IDLE, do_nothing};
    action_tuple_type rewind_failed_action_tuple = {REWIND_FAILED, PLAYER_STATE_FAILED, do_nothing};
    action_tuple_type pause_done_action_tuple = {PAUSE_DONE, PLAYER_STATE_PAUSED, do_nothing};
    action_tuple_type stop_done_action_tuple = {STOP_DONE, PLAYER_STATE_IDLE, do_nothing};
    action_tuple_type stop_failed_action_tuple = {STOP_FAILED, PLAYER_STATE_FAILED, do_nothing};
    action_tuple_type seek_done_action_tuple = {SEEK_DONE, PLAYER_STATE_NONE_INTERNAL, do_nothing};
    action_tuple_type get_done_action_tuple = {
        GET_DONE, PLAYER_STATE_NONE_INTERNAL,
        [this](std::string url, int64_t new_pos_ms, int64_t* get_val, int64_t index,
               MediaStreamInfo* info, MEDIA_PLAYER_STATE prev_state) {
          return player_worker_->sync_call(LOCATION_HERE, [this, prev_state] {
            if (prev_state == PLAYER_STATE_NONE_INTERNAL) {
              LOG_ERR_AND_RET_INT(ERR_FAILED, "prev_state NONE in get_done_action_tuple");
            }

            _revertToPrevState(prev_state);

            return ERR_OK_;
          });
        }};
#endif  // MP_SRC_SM_FULL_MAP

    state_machine_.insert({media::base::PLAYER_STATE_IDLE, open_action_tuple});

#ifdef MP_SRC_SM_FULL_MAP
    state_machine_.insert({media::base::PLAYER_STATE_OPENING, open_done_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_OPENING, open_failed_action_tuple});
#endif  // MP_SRC_SM_FULL_MAP

    // for FFMPEG player, need to enable stop action while in opening state
#if defined(ENABLE_FFMPEG_PLAYER)
    state_machine_.insert({media::base::PLAYER_STATE_OPENING, stop_action_tuple});
#endif  // ENABLE_FFMPEG_PLAYER

    state_machine_.insert({media::base::PLAYER_STATE_OPEN_COMPLETED, open_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_OPEN_COMPLETED, play_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_OPEN_COMPLETED, stop_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_OPEN_COMPLETED, seek_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_OPEN_COMPLETED, get_dur_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_OPEN_COMPLETED, get_play_pos_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_OPEN_COMPLETED, get_stream_cnt_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_OPEN_COMPLETED, get_stream_info_action_tuple});

    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, pause_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, stop_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, seek_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, get_dur_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, get_play_pos_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, get_stream_cnt_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, get_stream_info_action_tuple});
#ifdef MP_SRC_SM_FULL_MAP
    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, play_done_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYING, rewind_failed_action_tuple});
#endif  // MP_SRC_SM_FULL_MAP

    state_machine_.insert({media::base::PLAYER_STATE_PAUSED, play_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PAUSED, stop_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PAUSED, resume_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PAUSED, seek_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PAUSED, get_dur_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PAUSED, get_play_pos_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PAUSED, get_stream_cnt_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PAUSED, get_stream_info_action_tuple});

    state_machine_.insert({media::base::PLAYER_STATE_PLAYBACK_COMPLETED, open_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYBACK_COMPLETED, stop_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYBACK_COMPLETED, seek_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_PLAYBACK_COMPLETED, get_dur_action_tuple});
    state_machine_.insert(
        {media::base::PLAYER_STATE_PLAYBACK_COMPLETED, get_play_pos_action_tuple});
    state_machine_.insert(
        {media::base::PLAYER_STATE_PLAYBACK_COMPLETED, get_stream_cnt_action_tuple});
    state_machine_.insert(
        {media::base::PLAYER_STATE_PLAYBACK_COMPLETED, get_stream_info_action_tuple});

#ifdef MP_SRC_SM_FULL_MAP
    state_machine_.insert({media::base::PLAYER_STATE_PAUSING_INTERNAL, pause_done_action_tuple});

    state_machine_.insert({media::base::PLAYER_STATE_STOPPING_INTERNAL, stop_done_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_STOPPING_INTERNAL, stop_failed_action_tuple});

    state_machine_.insert({media::base::PLAYER_STATE_SEEKING_INTERNAL, seek_done_action_tuple});

    state_machine_.insert({media::base::PLAYER_STATE_GETTING_INTERNAL, get_done_action_tuple});
#endif  // MP_SRC_SM_FULL_MAP

    state_machine_.insert({media::base::PLAYER_STATE_FAILED, open_action_tuple});
    state_machine_.insert({media::base::PLAYER_STATE_FAILED, stop_action_tuple});

    return ERR_OK;
  });
}

MediaPlayerSourceImpl::~MediaPlayerSourceImpl() {
  (void)player_worker_->sync_call(LOCATION_HERE, [this] {
    video_frame_sender_.reset();
    audio_pcm_data_sender_.reset();
    media_node_factory_.reset();

    return ERR_OK;
  });
}

int MediaPlayerSourceImpl::getSourceId() const {
  API_LOGGER_MEMBER(nullptr);

  return source_id_;
}

int MediaPlayerSourceImpl::open(const char* url, int64_t start_pos) {
  API_LOGGER_MEMBER("url: %s, start_pos: %" PRId64, LITE_STR_CONVERT(url), start_pos);

  if (utils::IsNullOrEmpty(url)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid URL in open()");
  }

  if (start_pos < 0) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "negative start pos in open()");
  }

  // will have lock
  action_pair_type action_pair = _changeState(OPEN);

  if (action_pair.first == media::base::PLAYER_STATE_DO_NOTHING_INTERNAL) {
    return -ERR_FAILED;
  }

  // will sync_call to player worker
  return action_pair.second(url, 0, nullptr, 0, nullptr, action_pair.first);
}

#define DEFINE_NORMAL_ACTION_FUNC(action, ACTION)                             \
  int MediaPlayerSourceImpl::action() {                                       \
    API_LOGGER_MEMBER(nullptr);                                               \
                                                                              \
    action_pair_type action_pair = _changeState(ACTION);                      \
                                                                              \
    if (action_pair.first == media::base::PLAYER_STATE_DO_NOTHING_INTERNAL) { \
      return -ERR_FAILED;                                                     \
    }                                                                         \
                                                                              \
    return action_pair.second("", 0, nullptr, 0, nullptr, action_pair.first); \
  }

DEFINE_NORMAL_ACTION_FUNC(play, PLAY)
DEFINE_NORMAL_ACTION_FUNC(pause, PAUSE)
DEFINE_NORMAL_ACTION_FUNC(stop, STOP)
DEFINE_NORMAL_ACTION_FUNC(resume, RESUME)

int MediaPlayerSourceImpl::seek(int64_t new_pos_ms) {
  API_LOGGER_MEMBER("new_pos_ms: %" PRId64, new_pos_ms);

  if (new_pos_ms < 0) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "negative new pos in seek(): %" PRId64, new_pos_ms);
  }

  action_pair_type action_pair = _changeState(SEEK);

  if (action_pair.first == media::base::PLAYER_STATE_DO_NOTHING_INTERNAL) {
    return -ERR_FAILED;
  }

  return action_pair.second("", new_pos_ms, nullptr, 0, nullptr, action_pair.first);
}

#define DEFINE_NORMAL_GET_FUNC(getFunc, get_ref, ACTION)                       \
  int MediaPlayerSourceImpl::getFunc(int64_t& get_ref) {                       \
    API_LOGGER_MEMBER(nullptr);                                                \
                                                                               \
    action_pair_type action_pair = _changeState(ACTION);                       \
                                                                               \
    if (action_pair.first == media::base::PLAYER_STATE_DO_NOTHING_INTERNAL) {  \
      return -ERR_FAILED;                                                      \
    }                                                                          \
                                                                               \
    return action_pair.second("", 0, &get_ref, 0, nullptr, action_pair.first); \
  }

DEFINE_NORMAL_GET_FUNC(getDuration, dur_ms, GET_DUR)
DEFINE_NORMAL_GET_FUNC(getPlayPosition, curr_pos_ms, GET_PLAY_POS)
DEFINE_NORMAL_GET_FUNC(getStreamCount, count, GET_STREAM_CNT)

int MediaPlayerSourceImpl::getStreamInfo(int64_t index, media::base::MediaStreamInfo* info) {
  API_LOGGER_MEMBER("index: %" PRId64 ", info: %p", index, info);

  if (index < 0) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "negative index in getStreamInfo()");
  }

  if (!info) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "nullptr stream info in getStreamInfo()");
  }

  action_pair_type action_pair = _changeState(GET_STREAM_INFO);

  if (action_pair.first == media::base::PLAYER_STATE_DO_NOTHING_INTERNAL) {
    return -ERR_FAILED;
  }

  return action_pair.second("", 0, nullptr, index, info, action_pair.first);
}

int MediaPlayerSourceImpl::setLoopCount(int64_t loop_count) {
  API_LOGGER_MEMBER("loop_count: %" PRId64, loop_count);

  if (loop_count < -1) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "loop count < -1: %" PRId64, loop_count);
  }

  return player_worker_->sync_call(LOCATION_HERE, [this, loop_count] {
    doSetLoopCount(loop_count);
    return ERR_OK;
  });
}

int MediaPlayerSourceImpl::changePlaybackSpeed(media::base::MEDIA_PLAYER_PLAYBACK_SPEED speed) {
  API_LOGGER_MEMBER("speed: %d", speed);

  return player_worker_->sync_call(LOCATION_HERE,
                                   [this, speed] { return doChangePlaybackSpeed(speed); });
}

int MediaPlayerSourceImpl::selectAudioTrack(int64_t index) {
  API_LOGGER_MEMBER("index: %" PRId64, index);

  if (index < 0) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "negative index in selectAudioTrack()");
  }

  return player_worker_->sync_call(LOCATION_HERE,
                                   [this, index] { return doSelectAudioTrack(index); });
}

int MediaPlayerSourceImpl::setPlayerOption(const char* key, int64_t value) {
  API_LOGGER_MEMBER("key: %s, value: %" PRId64, LITE_STR_CONVERT(key), value);

  if (utils::IsNullOrEmpty(key)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid key in setPlayerOption()");
  }

  return player_worker_->sync_call(LOCATION_HERE,
                                   [this, key, value] { return doSetPlayerOption(key, value); });
}

int MediaPlayerSourceImpl::takeScreenshot(const char* file_name) {
  API_LOGGER_MEMBER("file_name: %s", LITE_STR_CONVERT(file_name));

  if (utils::IsNullOrEmpty(file_name)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid file name in takeScreenshot()");
  }

  return player_worker_->sync_call(LOCATION_HERE,
                                   [this, file_name] { return doTakeScreenshot(file_name); });
}

int MediaPlayerSourceImpl::selectInternalSubtitle(int64_t index) {
  API_LOGGER_MEMBER("index: %" PRId64, index);

  if (index < 0) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "negative index in selectInternalSubtitle()");
  }

  return player_worker_->sync_call(LOCATION_HERE,
                                   [this, index] { return doSelectInternalSubtitle(index); });
}

int MediaPlayerSourceImpl::setExternalSubtitle(const char* url) {
  API_LOGGER_MEMBER("url: %s", LITE_STR_CONVERT(url));

  if (utils::IsNullOrEmpty(url)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid URL in setExternalSubtitle()");
  }

  return player_worker_->sync_call(LOCATION_HERE,
                                   [this, url] { return doSetExternalSubtitle(url); });
}

// when a data member is atomic, we can directly return it without lock or sync_call
media::base::MEDIA_PLAYER_STATE MediaPlayerSourceImpl::getState() {
  API_LOGGER_MEMBER(nullptr);

  return player_state_;
}

agora_refptr<rtc::IAudioPcmDataSender> MediaPlayerSourceImpl::getAudioPcmDataSender() {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<IAudioPcmDataSender> audio_pcm_data_sender;

  (void)player_worker_->sync_call(LOCATION_HERE, [this, &audio_pcm_data_sender] {
    audio_pcm_data_sender = audio_pcm_data_sender_;
    return ERR_OK;
  });

  return audio_pcm_data_sender;
}

agora_refptr<IVideoFrameSender> MediaPlayerSourceImpl::getVideoFrameSender() {
  API_LOGGER_MEMBER(nullptr);

  agora_refptr<IVideoFrameSender> video_frame_sender;

  (void)player_worker_->sync_call(LOCATION_HERE, [this, &video_frame_sender] {
    video_frame_sender = video_frame_sender_;
    return ERR_OK;
  });

  return video_frame_sender;
}

#define DEFINE_REG_UNREG_FUNC(func, IObserver, observers_, Func)       \
  int MediaPlayerSourceImpl::func(IObserver* observer) {               \
    API_LOGGER_MEMBER("observer: %p", observer);                       \
                                                                       \
    return player_worker_->sync_call(LOCATION_HERE, [this, observer] { \
      observers_->Func(observer);                                      \
      return ERR_OK;                                                   \
    });                                                                \
  }

DEFINE_REG_UNREG_FUNC(registerPlayerSourceObserver, IMediaPlayerSourceObserver, observers_,
                      Register)
DEFINE_REG_UNREG_FUNC(unregisterPlayerSourceObserver, IMediaPlayerSourceObserver, observers_,
                      Unregister)
DEFINE_REG_UNREG_FUNC(registerAudioFrameObserver, media::base::IAudioFrameObserver,
                      audio_frame_observers_, Register)
DEFINE_REG_UNREG_FUNC(unregisterAudioFrameObserver, media::base::IAudioFrameObserver,
                      audio_frame_observers_, Unregister)

void MediaPlayerSourceImpl::doSetLoopCount(int64_t loop_count) {
  API_LOGGER_MEMBER("loop_count: %" PRId64, loop_count);

  player_worker_->sync_call(LOCATION_HERE, [this, loop_count] {
    loop_count_ = loop_count;
    return ERR_OK;
  });
}

// will only be called by MediaPlayerSourceWin::doOpen() and MediaPlayerSourceAndroid::doOpen()
// which is on player worker
bool MediaPlayerSourceImpl::checkStreamFormat() {
  API_LOGGER_MEMBER(nullptr);

  ASSERT_THREAD_IS(player_worker_->getThreadId());

#if !defined(ENABLE_FFMPEG_PLAYER)
  int64_t stream_cnt = 0;
  doGetStreamCount(stream_cnt);
  if (0 == stream_cnt) {
    LOG_WARN_AND_RET_BOOL("zero stream count in checkStreamFormat()");
  }

  media::base::MediaStreamInfo stream_info = {0};

  for (int64_t stream_idx = 0; stream_idx < stream_cnt; ++stream_idx) {
    memset(&stream_info, 0, sizeof(stream_info));

    doGetStreamInfo(stream_idx, &stream_info);

    // check stream type
    if (stream_info.streamType != media::base::STREAM_TYPE_AUDIO) {
      LOG_WARN("non-audio stream type in checkStreamFormat(): %d, skip", stream_info.streamType);

      continue;
    }

    // check format
    auto samples_per_sec = stream_info.audioSampleRate;
    auto samples_per_10ms = samples_per_sec / 100;
    auto channels = stream_info.audioChannels;
    // should multiply with 'channels' to align with AudioPcmDataSenderImpl::sendAudioPcmData()
    auto bytes_per_sample = channels * (stream_info.audioBitsPerSample / 8);

    auto err_code =
        audio_format_checker(samples_per_10ms, bytes_per_sample, channels, samples_per_sec);

    if (err_code == AudioFormatErrorCode::ERR_OK) {
      return true;
    }

    LOG_WARN(
        "failed in audio_format_checker() in checkStreamFormat(): err_code: %d, "
        "samples_per_sec: %d, samples_per_10ms: %d, channels: %d, bytes_per_sample: %d",
        err_code, samples_per_sec, samples_per_10ms, channels, bytes_per_sample);
  }

  return false;
#else
  return true;
#endif  // ENABLE_FFMPEG_PLAYER
}

void MediaPlayerSourceImpl::notifyPositionChanged(int curr_pos_secs) {
  API_LOGGER_MEMBER("curr_pos_secs: %d", curr_pos_secs);

  if (curr_pos_secs < 0) {
    LOG_ERR_AND_RET("negative current pos in notifyPositionChanged()");
  }

  (void)player_worker_->sync_call(LOCATION_HERE, [this, curr_pos_secs] {
    observers_->Post(LOCATION_HERE, [curr_pos_secs](auto observer) {
      observer->onPositionChanged(curr_pos_secs);
    });

    return ERR_OK;
  });
}

void MediaPlayerSourceImpl::notifyCompleted() {
  API_LOGGER_MEMBER(nullptr);

// for FFMPEG player, its doStop() needs to wait for the doOpen() related external thread to finish
// its job, and that thread will call this API to notify the state, to get rid of potential DL,
// needs to change to async call here
#if defined(ENABLE_FFMPEG_PLAYER)
  (void)player_worker_->async_call(LOCATION_HERE, [=] {
#else
  (void)player_worker_->sync_call(LOCATION_HERE, [=] {
#endif  // ENABLE_FFMPEG_PLAYER
    observers_->Post(LOCATION_HERE, [](auto observer) { observer->onCompleted(); });

    return ERR_OK;
  });
}  // namespace rtc

void MediaPlayerSourceImpl::updateState(media::base::MEDIA_PLAYER_STATE next_state) {
  API_LOGGER_MEMBER("next_state: %d", next_state);

  std::lock_guard<std::mutex> _(state_sm_lock_);

  player_state_ = next_state;
}

// for Simple Player, will only be called by player worker
// for FFMPEG Player, will be called by player worker, major worker and other external threads
// (TODO(tomiao): after FFMPEG Player is refined, need to assert player worker here and below
// _notifyStateChanged()
void MediaPlayerSourceImpl::updateStateAndNotify(media::base::MEDIA_PLAYER_STATE next_state) {
  API_LOGGER_MEMBER("new_state: %d", next_state);

  updateState(next_state);
  _notifyStateChanged(next_state);
}

void MediaPlayerSourceImpl::_revertToPrevState(media::base::MEDIA_PLAYER_STATE prev_state) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  updateState(prev_state);
}

// although this is just a post, also have to sync call to player worker to ensure thread safety
void MediaPlayerSourceImpl::_notifyStateChanged(media::base::MEDIA_PLAYER_STATE next_state) {
// for FFMPEG player, its doStop() needs to wait for the doOpen() related external thread to finish
// its job, and that thread will call this API to notify the state, to get rid of potential DL,
// needs to change to async call here
#if defined(ENABLE_FFMPEG_PLAYER)
  (void)player_worker_->async_call(LOCATION_HERE, [=] {
#else
  (void)player_worker_->sync_call(LOCATION_HERE, [=] {
#endif  // ENABLE_FFMPEG_PLAYER
    observers_->Post(LOCATION_HERE, [next_state](auto observer) {
      observer->onPlayerSourceStateChanged(
          next_state,
          next_state == media::base::PLAYER_STATE_FAILED
              ? media::base::PLAYER_ERROR_INTERNAL  // TODO(tomiao): to make Media Player Mgr happy,
                                                    // need to refine out later
              : media::base::PLAYER_ERROR_NONE);
    });

    return ERR_OK;
  });
}  // namespace agora

// might be called by player worker, major worker or any external thread
MediaPlayerSourceImpl::action_pair_type MediaPlayerSourceImpl::_changeState(Action action) {
  std::lock_guard<std::mutex> _(state_sm_lock_);

  // go through the SM table
  auto range = state_machine_.equal_range(player_state_);

  for (auto sm_itor = range.first; sm_itor != range.second; ++sm_itor) {
    const auto& action_tuple = sm_itor->second;

    if (std::get<0>(action_tuple) != action) {
      continue;
    }

    // we need to revert to previous state for get and seek actions, so save it now
    auto prev_state =
        _IsGetOrSeekAction(action) ? player_state_.load() : media::base::PLAYER_STATE_NONE_INTERNAL;

    player_state_ = std::get<1>(action_tuple);

    return {prev_state, std::get<2>(action_tuple)};
  }

  // failed to find in the SM table, error path
  auto do_nothing = std::bind(&MediaPlayerSourceImpl::_doNothing, this, _1, _2, _3, _4, _5, _6);
  return {media::base::PLAYER_STATE_DO_NOTHING_INTERNAL, do_nothing};
}

}  // namespace rtc
}  // namespace agora
