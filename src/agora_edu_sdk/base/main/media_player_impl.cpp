//
//  Agora RTC/MEDIA SDK
//
//  Created by Jixiaomeng in 2019-11.
//  Refined by Tommy Miao in 2020-04.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "media_player_impl.h"

#include "facilities/tools/api_logger.h"
#include "ui_thread.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/strings/string_util.h"
#include "utils/thread/thread_pool.h"

#if defined(__ANDROID__)
#include <sys/android/android_rtc_bridge.h>
#endif  // __ANDROID__

AGORA_PLAYER_API agora::rtc::IMediaPlayer* AGORA_PLAYER_CALL createAgoraMediaPlayer() {
  return new agora::rtc::MediaPlayerImpl;
}

namespace agora {
namespace rtc {

static const char* const MODULE_NAME = "[MPI]";

class VideoFakeRenderer : public IVideoSinkBase {
 public:
  explicit VideoFakeRenderer(MediaPlayerImpl* media_player) : media_player_(media_player) {}

  ~VideoFakeRenderer() override = default;

  // The DTOR of MediaPlayerImpl will call stop() to stop both video and audio pipelines, video
  // pipeline will wait all the onFrame() sync callbacks to finish (via in_flight_count_ in
  // video_node_interface.cpp), so 'media_player_' can be ensured to be thread safe here.
  int onFrame(const media::base::VideoFrame& videoFrame) override {
    API_LOGGER_CALLBACK_TIMES(10, "VideoFakeRenderer::onFrame", nullptr);

    if (!media_player_) {
      LOG_ERR_AND_RET_INT(ERR_NOT_READY, "nullptr Media Player in VideoFakeRenderer");
    }

    media_player_->video_frame_observers_->Call(
        [&videoFrame](auto observer) { observer->onFrame(&videoFrame); });

    return ERR_OK;
  }

 private:
  MediaPlayerImpl* media_player_ = nullptr;
};

MediaPlayerImpl::MediaPlayerImpl()
    : observers_(utils::RtcAsyncCallback<IMediaPlayerObserver>::Create()),
      video_frame_observers_(utils::RtcSyncCallback<media::base::IVideoFrameObserver>::Create()) {}

MediaPlayerImpl::~MediaPlayerImpl() {
  (void)ui_thread_sync_call(LOCATION_HERE, [this] {
    if (media_player_source_) {
      media_player_source_->unregisterPlayerSourceObserver(this);
    }

    stop();

    _unregAndDestroyConn();

    media_player_source_.reset();

    media_node_factory_.reset();

    return ERR_OK;
  });
}

// will run from any thread and should be ensured to be thread safe by user
int MediaPlayerImpl::initialize(const MediaPlayerContext& media_player_ctx) {
  agora_service_ = createAgoraService();
  if (!agora_service_) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_NO_RESOURCE, "failed to create agora service");
  }

  base::AgoraServiceConfiguration agora_svc_cfg;
  agora_svc_cfg.context = media_player_ctx.context;

  if (agora_service_->initialize(agora_svc_cfg) != ERR_OK) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_OBJ_NOT_INITIALIZED,
                         "failed to initialize agora service");
  }

  media_node_factory_ = agora_service_->createMediaNodeFactory();
  media_player_source_ = media_node_factory_->createMediaPlayerSource();
  media_player_source_->registerPlayerSourceObserver(this);

#if defined(__ANDROID__)
  std::string log_dir =
      commons::join_path(jni::RtcAndroidBridge::getConfigDir(), "agoraplayer.log");
  LOG_INFO("Android log: %s", log_dir.c_str());
#elif defined(__APPLE__)
  std::string log_dir = commons::join_path(commons::get_data_dir(), "agoraplayer.log");
  LOG_INFO("Apple log:%s", log_dir.c_str());
#else   // !__ANDROID__ && !__APPLE__
  std::string log_dir = commons::join_path(commons::get_data_dir(), "agoraplayer.log");
  LOG_INFO("Win log:%s", log_dir.c_str());
#endif  // __ANDROID__

  commons::set_log_file(log_dir.c_str(), commons::DEFAULT_LOG_SIZE);

  return media::base::PLAYER_ERROR_NONE;
}

int MediaPlayerImpl::open(const char* url, int64_t start_pos) {
  API_LOGGER_MEMBER("url: %s, start_pos: %" PRId64, LITE_STR_CONVERT(url), start_pos);

  if (utils::IsNullOrEmpty(url)) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS, "invalid URL in open()");
  }

  if (start_pos < 0) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "negative start pos in open(): %" PRId64, start_pos);
  }

  // MediaPlayerSourceImpl::open() will call its doOpen() asyncly, so here could be sync_call
  return _ConvertToPlayerErrInternal(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->open(url, start_pos); }));
}

int MediaPlayerImpl::play() {
  API_LOGGER_MEMBER(nullptr);

  return _ConvertToPlayerErrInternal(
      ui_thread_sync_call(LOCATION_HERE, [this] { return media_player_source_->play(); }));
}

int MediaPlayerImpl::pause() {
  API_LOGGER_MEMBER(nullptr);

  return _ConvertToPlayerErrInternal(
      ui_thread_sync_call(LOCATION_HERE, [this] { return media_player_source_->pause(); }));
}

int MediaPlayerImpl::stop() {
  API_LOGGER_MEMBER(nullptr);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [this] {
    int ret = ERR_OK;

    if (media_player_source_) {
      ret = media_player_source_->stop();

      if (ret != ERR_OK) {
        LOG_ERR("failed to stop media player source in stop(): %d", ret);
        // don't return but continue with below destructions
      }
    }

    // 1. Stop video track.
    // 2. Remove normal video renderer.
    // 3. Remove fake video renderer.
    // 4. Destroy video track.
    // 5. Destroy normal video renderer.
    // 6. Destroy fake video renderer.
    // 7. Stop audio track and destroy.
    // 8. Post state change to observers.
    if (video_track_) {
      video_track_->setEnabled(false);

      if (video_renderer_) {
        video_renderer_->setView(nullptr);
        video_track_->removeRenderer(video_renderer_);
      }

      if (video_fake_renderer_) {
        video_track_->removeRenderer(video_fake_renderer_);
      }

      video_track_.reset();
    }

    video_renderer_.reset();

    video_fake_renderer_.reset();

    if (audio_track_) {
      audio_track_->enableLocalPlayback(false);
      audio_track_->setEnabled(false);
      audio_track_.reset();
    }

    return ret;
  }));
}

int MediaPlayerImpl::seek(int64_t new_pos_ms) {
  API_LOGGER_MEMBER("new_pos_ms: %" PRId64, new_pos_ms);

  if (new_pos_ms < 0) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "negative new pos in seek(): %" PRId64, new_pos_ms);
  }

  return _ConvertToPlayerErrInternal(
      ui_thread_sync_call(LOCATION_HERE, [=] { return media_player_source_->seek(new_pos_ms); }));
}

int MediaPlayerImpl::mute(bool mute) {
  API_LOGGER_MEMBER("mute: %s", BOOL_TO_STR(mute));

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(LOCATION_HERE, [=] {
    if (!_checkStateAndAudioTrack()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to check state and audio track in mute()");
    }

    muted_ = mute;
    audio_track_->enableLocalPlayback(!muted_);

    return ERR_OK_;
  }));
}

int MediaPlayerImpl::getMute(bool& mute) {
  API_LOGGER_MEMBER(nullptr);

  mute = false;

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(LOCATION_HERE, [&] {
    if (!_checkStateAndAudioTrack()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to check state and audio track in getMute()");
    }

    mute = muted_;

    return ERR_OK_;
  }));
}

int MediaPlayerImpl::adjustPlayoutVolume(int volume) {
  API_LOGGER_MEMBER("volume: %d", volume);

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(LOCATION_HERE, [=] {
    if (!_checkStateAndAudioTrack()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED,
                          "failed to check state and audio track in adjustPlayoutVolume()");
    }

    audio_track_->adjustPlayoutVolume(volume);

    return ERR_OK_;
  }));
}

int MediaPlayerImpl::getPlayoutVolume(int& volume) {
  API_LOGGER_MEMBER(nullptr);

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(LOCATION_HERE, [&] {
    if (!_checkStateAndAudioTrack()) {
      LOG_ERR_AND_RET_INT(ERR_FAILED,
                          "failed to check state and audio track in getPlayoutVolume()");
    }

    audio_track_->getPlayoutVolume(&volume);

    return ERR_OK_;
  }));
}

#define DEFINE_NORMAL_GET_FUNC(getFunc, get_val)                                 \
  int MediaPlayerImpl::getFunc(int64_t& get_val) {                               \
    API_LOGGER_MEMBER(nullptr);                                                  \
                                                                                 \
    return _ConvertToPlayerErrInternal(ui_thread_sync_call(                      \
        LOCATION_HERE, [&] { return media_player_source_->getFunc(get_val); })); \
  }

DEFINE_NORMAL_GET_FUNC(getDuration, dur_ms)
DEFINE_NORMAL_GET_FUNC(getPlayPosition, curr_pos_ms)
DEFINE_NORMAL_GET_FUNC(getStreamCount, count)

int MediaPlayerImpl::getStreamInfo(int64_t index, media::base::MediaStreamInfo* info) {
  API_LOGGER_MEMBER("index: %" PRId64 ", info: %p", index, info);

  if (index < 0) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "negative index in getStreamInfo()");
  }

  if (!info) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "nullptr stream info in getStreamInfo()");
  }

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->getStreamInfo(index, info); }));
}

media::base::MEDIA_PLAYER_STATE MediaPlayerImpl::getState() {
  API_LOGGER_MEMBER(nullptr);

  media::base::MEDIA_PLAYER_STATE player_source_state = media::base::PLAYER_STATE_IDLE;

  (void)ui_thread_sync_call(LOCATION_HERE, [&] {
    player_source_state = media_player_source_->getState();
    return ERR_OK;
  });

  return player_source_state;
}

int MediaPlayerImpl::setView(media::base::view_t view) {
  API_LOGGER_MEMBER("view: %p", view);

  if (!view) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS, "nullptr view in setView()");
  }

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [=] {
    video_render_view_ = view;
    return ERR_OK;
  }));
}

int MediaPlayerImpl::setRenderMode(media::base::RENDER_MODE_TYPE render_mode) {
  API_LOGGER_MEMBER("render_mode: %d", render_mode);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [=] {
    video_render_mode_ = render_mode;

    if (video_renderer_) {
      video_renderer_->setRenderMode(video_render_mode_);
    }

    return ERR_OK;
  }));
}

#define DEFINE_REG_UNREG_FUNC(func, IObserver, observers_, Func)                \
  int MediaPlayerImpl::func(IObserver* observer) {                              \
    API_LOGGER_MEMBER("observer: %p", observer);                                \
                                                                                \
    return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [=] { \
      observers_->Func(observer);                                               \
      return ERR_OK;                                                            \
    }));                                                                        \
  }

DEFINE_REG_UNREG_FUNC(registerPlayerObserver, IMediaPlayerObserver, observers_, Register)
DEFINE_REG_UNREG_FUNC(unregisterPlayerObserver, IMediaPlayerObserver, observers_, Unregister)
DEFINE_REG_UNREG_FUNC(registerVideoFrameObserver, media::base::IVideoFrameObserver,
                      video_frame_observers_, Register)
DEFINE_REG_UNREG_FUNC(unregisterVideoFrameObserver, media::base::IVideoFrameObserver,
                      video_frame_observers_, Unregister)

int MediaPlayerImpl::registerAudioFrameObserver(media::base::IAudioFrameObserver* observer) {
  API_LOGGER_MEMBER("observer: %p", observer);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->registerAudioFrameObserver(observer); }));
}

int MediaPlayerImpl::unregisterAudioFrameObserver(media::base::IAudioFrameObserver* observer) {
  API_LOGGER_MEMBER("observer: %p", observer);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->unregisterAudioFrameObserver(observer); }));
}

int MediaPlayerImpl::connect(const char* token, const char* chan_id, user_id_t user_id) {
  API_LOGGER_MEMBER("token: %s, chan_id: %s user_id: %s", LITE_STR_CONVERT(token),
                    LITE_STR_CONVERT(chan_id), LITE_STR_CONVERT(user_id));

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [=] {
    if (rtc_conn_) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "already have RTC connection in connect()");
    }

    if (rtc_conn_state_ != RTC_CONN_STATE_DISCONNECTED) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "RTC connection state is not disconnected in connect()");
    }

    RtcConnectionConfiguration rtc_conn_cfg;
    rtc_conn_cfg.autoSubscribeAudio = false;
    rtc_conn_cfg.autoSubscribeVideo = false;

    rtc_conn_ = agora_service_->createRtcConnection(rtc_conn_cfg);
    if (!rtc_conn_) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to create RTC connection in connect()");
    }

    if (rtc_conn_->registerObserver(this) != ERR_OK) {
      _destroyConn();

      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to register observer to RTC connection in connect()");
    }

    rtc_conn_->getLocalUser()->setUserRole(CLIENT_ROLE_BROADCASTER);

    if (rtc_conn_->connect(token, chan_id, user_id) != ERR_OK) {
      _unregAndDestroyConn();

      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to connect to RTC connection in connect()");
    }

    rtc_conn_state_ = RTC_CONN_STATE_CONNECTING;

    return ERR_OK_;
  }));
}

int MediaPlayerImpl::disconnect() {
  API_LOGGER_MEMBER(nullptr);

  media::base::MEDIA_PLAYER_ERROR ec = media::base::PLAYER_ERROR_NONE;

  (void)ui_thread_sync_call(LOCATION_HERE, [&] {
    publish_video_enabled_ = false;
    publish_audio_enabled_ = false;

    _doPublish();

    if (!rtc_conn_) {
      ec = media::base::PLAYER_ERROR_INVALID_CONNECTION_STATE;

      LOG_ERR_AND_RET_INT(ERR_FAILED, "no RTC connection in disconnect()");
    }

    if (rtc_conn_state_ != RTC_CONN_STATE_CONNECTED) {
      ec = media::base::PLAYER_ERROR_INVALID_CONNECTION_STATE;

      LOG_ERR_AND_RET_INT(ERR_FAILED, "RTC connection state is not connected in disconnect()");
    }

    if (rtc_conn_->disconnect() != ERR_OK) {
      ec = media::base::PLAYER_ERROR_INTERNAL;

      LOG_ERR_AND_RET_INT(ERR_FAILED, "failed to disconnect to RTC connection in disconnect()");
    }

    rtc_conn_state_ = RTC_CONN_STATE_DISCONNECTING;

    return ERR_OK_;
  });

  return ec;
}

int MediaPlayerImpl::publishVideo() {
  API_LOGGER_MEMBER(nullptr);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [this] {
    publish_video_enabled_ = true;

    _doPublish();

    return ERR_OK;
  }));
}

int MediaPlayerImpl::unpublishVideo() {
  API_LOGGER_MEMBER(nullptr);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [this] {
    publish_video_enabled_ = false;

    _doPublish();

    return ERR_OK;
  }));
}

int MediaPlayerImpl::publishAudio() {
  API_LOGGER_MEMBER(nullptr);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [this] {
    publish_audio_enabled_ = true;

    _doPublish();

    return ERR_OK;
  }));
}

int MediaPlayerImpl::unpublishAudio() {
  API_LOGGER_MEMBER(nullptr);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(LOCATION_HERE, [this] {
    publish_audio_enabled_ = false;

    _doPublish();

    return ERR_OK;
  }));
}

int MediaPlayerImpl::adjustPublishSignalVolume(int volume) {
  API_LOGGER_MEMBER("volume: %d", volume);

  if (volume < 0) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "negative volume in adjustPublishSignalVolume()");
  }

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(LOCATION_HERE, [=] {
    if (!audio_track_) {
      LOG_ERR_AND_RET_INT(ERR_FAILED, "nullptr audio track in adjustPublishSignalVolume()");
    }

    audio_track_->adjustPublishVolume(volume);

    return ERR_OK_;
  }));
}

int MediaPlayerImpl::setLogFile(const char* file_path) {
  API_LOGGER_MEMBER("file_path: %s", LITE_STR_CONVERT(file_path));

  if (utils::IsNullOrEmpty(file_path)) {
    LOG_ERR_AND_RET_INT(ERR_INVALID_ARGUMENT, "invalid file path in setLogFile()");
  }

  commons::set_log_file(file_path, commons::DEFAULT_LOG_SIZE);

  return ERR_OK;
}

int MediaPlayerImpl::setLogFilter(unsigned int filter) {
  API_LOGGER_MEMBER("filter: %u", filter);

  commons::set_log_filters(filter);

  return ERR_OK;
}

int MediaPlayerImpl::changePlaybackSpeed(media::base::MEDIA_PLAYER_PLAYBACK_SPEED speed) {
  API_LOGGER_MEMBER("speed: %d", speed);

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->changePlaybackSpeed(speed); }));
}

int MediaPlayerImpl::selectAudioTrack(int index) {
  API_LOGGER_MEMBER("index: %d", index);

  if (index < 0) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "negative index in selectAudioTrack()");
  }

  return _ConvertToPlayerErrInternal(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->selectAudioTrack(index); }));
}

int MediaPlayerImpl::setPlayerOption(const char* key, int value) {
  API_LOGGER_MEMBER("key: %s, value: %d", LITE_STR_CONVERT(key), value);

  if (utils::IsNullOrEmpty(key)) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "invalid key in setPlayerOption()");
  }

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->setPlayerOption(key, value); }));
}

int MediaPlayerImpl::takeScreenshot(const char* file_name) {
  API_LOGGER_MEMBER("file_name: %s", file_name);

  if (utils::IsNullOrEmpty(file_name)) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "invalid file name in takeScreenshot()");
  }

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->takeScreenshot(file_name); }));
}

int MediaPlayerImpl::selectInternalSubtitle(int index) {
  API_LOGGER_MEMBER("index: %d", index);

  if (index < 0) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "negative index in selectInternalSubtitle()");
  }

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->selectInternalSubtitle(index); }));
}

int MediaPlayerImpl::setExternalSubtitle(const char* url) {
  API_LOGGER_MEMBER("url: %s", LITE_STR_CONVERT(url));

  if (utils::IsNullOrEmpty(url)) {
    LOG_ERR_AND_RET_INT_(media::base::PLAYER_ERROR_INVALID_ARGUMENTS,
                         "invalid URL in setExternalSubtitle()");
  }

  return _ConvertToPlayerErrInvalid(ui_thread_sync_call(
      LOCATION_HERE, [=] { return media_player_source_->setExternalSubtitle(url); }));
}

void MediaPlayerImpl::release(bool sync) {
  API_LOGGER_MEMBER("sync: %s", BOOL_TO_STR(sync));

  delete this;
}

void MediaPlayerImpl::onPlayerSourceStateChanged(media::base::MEDIA_PLAYER_STATE state,
                                                 media::base::MEDIA_PLAYER_ERROR ec) {
  // when coming here will be on callback worker, no need to wait for this task to finish, so just
  // async call
  (void)ui_thread_async_call(LOCATION_HERE, [=] {
    if (state == media::base::PLAYER_STATE_OPEN_COMPLETED) {
      _createVideoTrackAndPublish();
      _createAudioTrackAndPublish();

      if (video_track_) {
        video_track_->setEnabled(true);

        if (video_render_view_) {
          video_renderer_ = media_node_factory_->createVideoRenderer();

          if (video_renderer_) {
            video_renderer_->setView(video_render_view_);
            video_renderer_->setRenderMode(video_render_mode_);
            video_track_->addRenderer(video_renderer_);
          } else {
            _onPlayerInternalError(media::base::PLAYER_ERROR_VIDEO_RENDER_FAILED);

            // no need to continue with below audio pipeline's enabling
            return;
          }
        }
      }

      if (audio_track_) {
        audio_track_->setEnabled(true);
        audio_track_->enableLocalPlayback(true);
      }
    }

    observers_->Post(LOCATION_HERE,
                     [=](auto observer) { observer->onPlayerStateChanged(state, ec); });
  });
}

void MediaPlayerImpl::onPositionChanged(int64_t pos) {
  if (pos < 0) {
    LOG_ERR_AND_RET("negative pos in onPositionChanged()");
  }

  (void)ui_thread_sync_call(LOCATION_HERE, [=] {
    observers_->Post(LOCATION_HERE, [=](auto observer) { observer->onPositionChanged(pos); });
    return ERR_OK;
  });
}

void MediaPlayerImpl::onPlayerEvent(media::base::MEDIA_PLAYER_EVENT event) {
  (void)ui_thread_sync_call(LOCATION_HERE, [=] {
    observers_->Post(LOCATION_HERE, [=](auto observer) { observer->onPlayerEvent(event); });
    return ERR_OK;
  });
}

void MediaPlayerImpl::onMetaData(const void* data, int length) {
  API_LOGGER_CALLBACK_TIMES(2, onMetaData, "data: %p, length: %d", data, length);

  if (!data || 0 == length) {
    LOG_ERR_AND_RET("invalid arguments in onMetaData()");
  }

  std::string sei_buf(reinterpret_cast<const char*>(data), length);

  (void)ui_thread_sync_call(LOCATION_HERE, [=] {
    observers_->Post(LOCATION_HERE, [=](auto observer) {
      observer->onMetadata(media::base::PLAYER_METADATA_TYPE_SEI,
                           reinterpret_cast<const uint8_t*>(sei_buf.c_str()), length);
    });

    return ERR_OK;
  });
}

void MediaPlayerImpl::onCompleted() {
  (void)ui_thread_sync_call(LOCATION_HERE, [this] {
    observers_->Post(LOCATION_HERE, [](auto observer) { observer->onCompleted(); });
    return ERR_OK;
  });
}

void MediaPlayerImpl::onConnected(const TConnectionInfo& connectionInfo,
                                  CONNECTION_CHANGED_REASON_TYPE reason) {
  API_LOGGER_CALLBACK(onConnected, "reason: %d", reason);

  (void)ui_thread_sync_call(LOCATION_HERE, [this] {
    rtc_conn_state_ = RTC_CONN_STATE_CONNECTED;
    _doPublish();

    return ERR_OK;
  });
}

void MediaPlayerImpl::onDisconnected(const TConnectionInfo& connectionInfo,
                                     CONNECTION_CHANGED_REASON_TYPE reason) {
  API_LOGGER_CALLBACK(onDisconnected, "reason: %d", reason);

  (void)ui_thread_sync_call(LOCATION_HERE, [this] {
    _unregAndDestroyConn();
    return ERR_OK;
  });
}

void MediaPlayerImpl::onAudioTrackPublishSuccess(agora_refptr<ILocalAudioTrack> audioTrack) {
  API_LOGGER_CALLBACK(onAudioTrackPublishSuccess, nullptr);

  (void)ui_thread_sync_call(LOCATION_HERE, [this] {
    observers_->Post(LOCATION_HERE, [=](auto observer) {
      observer->onPlayerEvent(media::base::PLAYER_EVENT_AUDIO_PUBLISHED);
    });

    return ERR_OK;
  });
}

void MediaPlayerImpl::onVideoTrackPublishSuccess(agora_refptr<ILocalVideoTrack> videoTrack,
                                                 int elapsed) {
  API_LOGGER_CALLBACK(onVideoTrackPublishSuccess, "elapsed: %d", elapsed);

  (void)ui_thread_sync_call(LOCATION_HERE, [this] {
    observers_->Post(LOCATION_HERE, [=](auto observer) {
      observer->onPlayerEvent(media::base::PLAYER_EVENT_VIDEO_PUBLISHED);
    });

    return ERR_OK;
  });
}

bool MediaPlayerImpl::_checkStateAndAudioTrack() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (_NotRunning(media_player_source_->getState())) {
    LOG_ERR_AND_RET_BOOL("media player source not running in _checkStateAndAudioTrack()");
  }

  if (!audio_track_) {
    LOG_ERR_AND_RET_BOOL("no audio track in _checkStateAndAudioTrack()");
  }

  return true;
}

void MediaPlayerImpl::_doPublish() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (video_track_) {
    if (publish_video_enabled_ && !video_published_) {
      if (rtc_conn_) {
        video_track_->setEnabled(true);
        rtc_conn_->getLocalUser()->registerLocalUserObserver(this);
        rtc_conn_->getLocalUser()->publishVideo(video_track_);
        video_published_ = true;
      }
    }

    if (!publish_video_enabled_ && video_published_) {
      if (rtc_conn_) {
        video_track_->setEnabled(false);
        rtc_conn_->getLocalUser()->unpublishVideo(video_track_);
        rtc_conn_->getLocalUser()->unregisterLocalUserObserver(this);
        video_published_ = false;
      }
    }
  }

  if (audio_track_) {
    if (publish_audio_enabled_ && !audio_published_) {
      if (rtc_conn_) {
        audio_track_->setEnabled(true);
        rtc_conn_->getLocalUser()->registerLocalUserObserver(this);
        rtc_conn_->getLocalUser()->publishAudio(audio_track_);
        audio_published_ = true;
      }
    }

    if (!publish_audio_enabled_ && audio_published_) {
      if (rtc_conn_) {
        audio_track_->setEnabled(false);
        rtc_conn_->getLocalUser()->unpublishAudio(audio_track_);
        rtc_conn_->getLocalUser()->unregisterLocalUserObserver(this);
        audio_published_ = false;
      }
    }
  }
}

void MediaPlayerImpl::_createVideoTrackAndPublish() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!video_track_) {
    video_track_ = agora_service_->createMediaPlayerVideoTrack(media_player_source_);
  }

  if (video_track_) {
    video_fake_renderer_ = new RefCountedObject<VideoFakeRenderer>(this);
    video_track_->addRenderer(video_fake_renderer_);
    _doPublish();
  }
}

void MediaPlayerImpl::_createAudioTrackAndPublish() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!audio_track_) {
    audio_track_ = agora_service_->createMediaPlayerAudioTrack(media_player_source_);
  }

  if (audio_track_) {
    _doPublish();
  }
}

void MediaPlayerImpl::_destroyConn() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  rtc_conn_.reset();
  rtc_conn_state_ = RTC_CONN_STATE_DISCONNECTED;
}

void MediaPlayerImpl::_unregAndDestroyConn() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (rtc_conn_) {
    rtc_conn_->unregisterObserver(this);
  }

  _destroyConn();
}

void MediaPlayerImpl::_onPlayerInternalError(media::base::MEDIA_PLAYER_ERROR ec) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  // notify PLAYER_STATE_IDLE first
  stop();

  // then PLAYER_STATE_FAILED secondly
  observers_->Post(LOCATION_HERE, [=](IMediaPlayerObserver* observer) {
    observer->onPlayerStateChanged(media::base::PLAYER_STATE_FAILED, ec);
  });
}

}  // namespace rtc
}  // namespace agora
