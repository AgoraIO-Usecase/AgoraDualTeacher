
//
//  Agora Media SDK
//
//  Created by panqingyou in 2020/01.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//

#include "media_player_manager.h"

#include "base/base_type.h"
#include "channel_manager.h"
#include "facilities/tools/audio_utils.h"
#include "local_track_manager.h"
#include "ui_thread.h"
#include "utils/lock/locks.h"
#include "utils/log/log.h"

const char MODULE_NAME[] = "[MPM]";

namespace agora {
namespace rtc {

namespace {

using audio_mixing_state_map = std::map<media::base::MEDIA_PLAYER_STATE, AUDIO_MIXING_STATE_TYPE>;
using audio_mixing_error_map = std::map<media::base::MEDIA_PLAYER_ERROR, AUDIO_MIXING_ERROR_TYPE>;

audio_mixing_state_map AUDIO_MIXING_STATE_MAP = {
    {agora::media::base::PLAYER_STATE_IDLE, AUDIO_MIXING_STATE_STOPPED},
    {media::base::PLAYER_STATE_PLAYING, AUDIO_MIXING_STATE_PLAYING},
    {media::base::PLAYER_STATE_PAUSED, AUDIO_MIXING_STATE_PAUSED},
    {media::base::PLAYER_STATE_FAILED, AUDIO_MIXING_STATE_FAILED},
    {media::base::PLAYER_STATE_PLAYBACK_COMPLETED, AUDIO_MIXING_STATE_STOPPED}};

audio_mixing_error_map AUDIO_MIXING_ERROR_MAP = {
    {media::base::PLAYER_ERROR_NONE, AUDIO_MIXING_ERROR_OK},
    {media::base::PLAYER_ERROR_INVALID_ARGUMENTS, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_INTERNAL, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_NO_RESOURCE, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_INVALID_MEDIA_SOURCE, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_UNKNOWN_STREAM_TYPE, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_OBJ_NOT_INITIALIZED, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_CODEC_NOT_SUPPORTED, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_VIDEO_RENDER_FAILED, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_INVALID_STATE, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_URL_NOT_FOUND, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_INVALID_CONNECTION_STATE, AUDIO_MIXING_ERROR_CAN_NOT_OPEN},
    {media::base::PLAYER_ERROR_SRC_BUFFER_UNDERFLOW, AUDIO_MIXING_ERROR_CAN_NOT_OPEN}};

bool convert_media_state_type(media::base::MEDIA_PLAYER_STATE in_state,
                              AUDIO_MIXING_STATE_TYPE& out_state) {
  if (AUDIO_MIXING_STATE_MAP.find(in_state) == AUDIO_MIXING_STATE_MAP.end()) {
    return false;
  }

  out_state = AUDIO_MIXING_STATE_MAP[in_state];
  return true;
}

bool convert_media_error_type(media::base::MEDIA_PLAYER_ERROR in_err,
                              AUDIO_MIXING_ERROR_TYPE& out_err) {
  if (AUDIO_MIXING_ERROR_MAP.find(in_err) == AUDIO_MIXING_ERROR_MAP.end()) {
    return false;
  }

  out_err = AUDIO_MIXING_ERROR_MAP[in_err];
  return true;
}

}  // namespace

class MediaPlayerSourceObserver : public IMediaPlayerSourceObserver {
 public:
  MediaPlayerSourceObserver(int sourceId, MediaPlayerManager* mediaPlayerManager);
  ~MediaPlayerSourceObserver() override = default;

  void onPlayerSourceStateChanged(const media::base::MEDIA_PLAYER_STATE state,
                                  const media::base::MEDIA_PLAYER_ERROR ec) override;

  void onPositionChanged(const int64_t position) override {}

  void onPlayerEvent(const media::base::MEDIA_PLAYER_EVENT event) override {}

  void onMetaData(const void* data, int length) override {}

  void onCompleted() override {}

  int waitForOpenCompleted(media::base::MEDIA_PLAYER_ERROR& err);

 private:
  int source_id_;
  MediaPlayerManager* media_player_manager_;
  utils::AutoResetEvent open_completed_event_;
  volatile media::base::MEDIA_PLAYER_ERROR open_completed_err_;
};

MediaPlayerSourceObserver::MediaPlayerSourceObserver(int sourceId,
                                                     MediaPlayerManager* mediaPlayerManager)
    : source_id_(sourceId),
      media_player_manager_(mediaPlayerManager),
      open_completed_err_(media::base::PLAYER_ERROR_INTERNAL) {}

void MediaPlayerSourceObserver::onPlayerSourceStateChanged(
    const media::base::MEDIA_PLAYER_STATE state, const media::base::MEDIA_PLAYER_ERROR ec) {
  if (media::base::PLAYER_STATE_OPEN_COMPLETED == state) {
    open_completed_err_ = ec;
    open_completed_event_.Set();
  } else if (media::base::PLAYER_STATE_PLAYBACK_COMPLETED == state) {
    ui_thread_sync_call(LOCATION_HERE, [this] {
      media_player_manager_->notifyPlaybackCompleted(source_id_);
      return 0;
    });
  }
}

int MediaPlayerSourceObserver::waitForOpenCompleted(media::base::MEDIA_PLAYER_ERROR& err) {
  open_completed_event_.Wait(2000);
  err = open_completed_err_;
  return 0;
}

MediaPlayerManager::MediaPlayerManager(const agora_refptr<IMediaNodeFactory>& media_node_factory,
                                       LocalTrackManager* local_track_manager,
                                       IRtcEngineEventHandler* event_handler, bool is_ex_handler)
    : media_node_factory_(media_node_factory),
      local_track_manager_(local_track_manager),
      is_ex_handler_(is_ex_handler),
      event_callback_(utils::RtcAsyncCallback<IRtcEngineEventHandler>::Create()),
      audio_mixing_player_source_id_(-1),
      remaining_cycle_(0),
      audio_effect_finished_cb_(nullptr) {
  (void)ui_thread_sync_call(LOCATION_HERE, [=] {
    auto media_player_source = createMediaPlayer(media::base::MEDIA_PLAYER_SOURCE_DEFAULT);
    audio_mixing_player_source_id_ = media_player_source->getSourceId();
    media_player_source->registerPlayerSourceObserver(this);
    event_callback_->Register(event_handler);

    return 0;
  });
}

void MediaPlayerManager::registerEffectFinishedCallBack(AudioEffectFinishedCallBack&& callBack) {
  audio_effect_finished_cb_ = std::move(callBack);
}

void MediaPlayerManager::unregisterEffectFinishedCallBack() { audio_effect_finished_cb_ = nullptr; }

MediaPlayerManager::~MediaPlayerManager() {
  auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
  (void)ui_thread_sync_call(LOCATION_HERE, [this, media_player_source] {
    media_player_source->unregisterPlayerSourceObserver(this);
    event_callback_->Unregister();

    unloadAllEffects();
    while (!media_players_.empty()) {
      destroyMediaPlayer(media_players_.begin()->second);
    }
    media_players_.clear();
    return 0;
  });
}

agora_refptr<IMediaPlayerSource> MediaPlayerManager::createMediaPlayer(
    media::base::MEDIA_PLAYER_SOURCE_TYPE type) {
  agora_refptr<IMediaPlayerSource> media_player;
  (void)ui_thread_sync_call(LOCATION_HERE, [this, type, &media_player] {
    media_player = media_node_factory_->createMediaPlayerSource(type);
    if (media_player) {
      media_players_[media_player->getSourceId()] = media_player;
    }
    return 0;
  });
  return media_player;
}

agora_refptr<IMediaPlayerSource> MediaPlayerManager::getMediaPlayer(int sourceId) {
  agora_refptr<IMediaPlayerSource> media_player;
  (void)ui_thread_sync_call(LOCATION_HERE, [this, sourceId, &media_player] {
    if (media_players_.find(sourceId) != media_players_.end()) {
      media_player = media_players_[sourceId];
    }
    return 0;
  });
  return media_player;
}

int MediaPlayerManager::destroyMediaPlayer(agora_refptr<IMediaPlayerSource> mediaPlayer) {
  return ui_thread_sync_call(LOCATION_HERE, [this, mediaPlayer] {
    for (auto it = media_players_.begin(); it != media_players_.end(); ++it) {
      if (it->second == mediaPlayer) {
        mediaPlayer->stop();
        media_players_.erase(it);
        break;
      }
    }
    return 0;
  });
}

int MediaPlayerManager::startAudioMixing(const char* file_path, bool loop_back, bool replace,
                                         int cycle) {
  commons::log(commons::LOG_INFO,
               "%s: startAudioMixing, file_path: %s, loop_back: %d, replace: %d, cycle: %d",
               MODULE_NAME, file_path, loop_back, replace, cycle);
  if (!file_path || 0 == std::strlen(file_path)) {
    return -ERR_INVALID_ARGUMENT;
  }

  auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
  if (!media_player_source) {
    commons::log(commons::LOG_WARN, "%s: Cannot find media player source for audio mixing",
                 MODULE_NAME);
    return -ERR_INVALID_STATE;
  }
  return ui_thread_sync_call(LOCATION_HERE, [=] {
    if (!local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_)) {
      local_track_manager_->createMediaPlayerAudioTrack(audio_mixing_player_source_id_,
                                                        media_player_source);
    }

    // stop the preview one.
    stopAudioMixing();

    remaining_cycle_ = cycle;
    url_ = std::string(file_path);

    media_player_source->setLoopCount(remaining_cycle_);

    return media_player_source->open(file_path, 0);
  });
}

int MediaPlayerManager::stopAudioMixing() {
  commons::log(commons::LOG_INFO, "%s: stopAudioMixing", MODULE_NAME);

  auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
  return ui_thread_sync_call(LOCATION_HERE, [this, media_player_source] {
    // check state, if idle means no mixing job scheduled.
    if (media_player_source->getState() == media::base::PLAYER_STATE_IDLE ||
        media_player_source->getState() == media::base::PLAYER_STATE_FAILED) {
      return ERR_OK;
    }

    media_player_source->stop();
    return ERR_OK;
  });
}

int MediaPlayerManager::pauseAudioMixing() {
  commons::log(commons::LOG_INFO, "%s: pauseAudioMixing", MODULE_NAME);
  return ui_thread_sync_call(LOCATION_HERE, [this] {
    auto media_audio_track =
        local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_);
    if (media_audio_track) {
      media_audio_track->enableLocalPlayback(false);
    }

    auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
    if (!media_player_source) {
      commons::log(commons::LOG_WARN, "%s: No media player source to pause audio mixing.",
                   MODULE_NAME);
      return -ERR_FAILED;
    }
    return media_player_source->pause();
  });
}

int MediaPlayerManager::resumeAudioMixing() {
  commons::log(commons::LOG_INFO, "%s: resumeAudioMixing", MODULE_NAME);
  return ui_thread_sync_call(LOCATION_HERE, [this] {
    auto media_audio_track =
        local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_);
    if (media_audio_track) {
      media_audio_track->enableLocalPlayback(true);
    }

    auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
    if (!media_player_source) {
      commons::log(commons::LOG_WARN, "%s: No media player source to resume audio mixing.",
                   MODULE_NAME);
      return -ERR_FAILED;
    }
    return media_player_source->resume();
  });
}

int MediaPlayerManager::adjustAudioMixingVolume(int volume) {
  commons::log(commons::LOG_INFO, "%s: adjustAudioMixingVolume volume:%d", MODULE_NAME, volume);
  int retPublish = adjustAudioMixingPublishVolume(volume);
  int retPlayout = adjustAudioMixingPlayoutVolume(volume);
  return (retPublish == ERR_OK && retPlayout == ERR_OK) ? ERR_OK : ERR_FAILED;
}

int MediaPlayerManager::adjustAudioMixingPublishVolume(int volume) {
  commons::log(commons::LOG_INFO, "%s: adjustAudioMixingPublishVolume volume:%d", MODULE_NAME,
               volume);
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    auto media_audio_track =
        local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_);
    if (!media_audio_track) {
      return static_cast<int>(-ERR_NOT_READY);
    }
    return media_audio_track->adjustPublishVolume(volume);
  });
}

int MediaPlayerManager::getAudioMixingPublishVolume() {
  commons::log(commons::LOG_INFO, "%s: getAudioMixingPublishVolume", MODULE_NAME);
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    auto media_audio_track =
        local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_);
    if (!media_audio_track) {
      return static_cast<int>(-ERR_NOT_READY);
    }
    int volume = 0;
    int ret = media_audio_track->getPublishVolume(&volume);
    return (ret == ERR_OK ? volume : ret);
  });
}

int MediaPlayerManager::adjustAudioMixingPlayoutVolume(int volume) {
  commons::log(commons::LOG_INFO, "%s: adjustAudioMixingPlayoutVolume volume:%d", MODULE_NAME,
               volume);
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    auto media_audio_track =
        local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_);
    if (!media_audio_track) {
      return static_cast<int>(-ERR_NOT_READY);
    }
    return media_audio_track->adjustPlayoutVolume(volume);
  });
}

int MediaPlayerManager::getAudioMixingPlayoutVolume() {
  commons::log(commons::LOG_INFO, "%s: getAudioMixingPlayoutVolume", MODULE_NAME);
  return ui_thread_sync_call(LOCATION_HERE, [&] {
    auto media_audio_track =
        local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_);
    if (!media_audio_track) {
      return static_cast<int>(-ERR_NOT_READY);
    }
    int volume = 0;
    int ret = media_audio_track->getPlayoutVolume(&volume);
    return (ret == ERR_OK ? volume : ret);
  });
}

int MediaPlayerManager::getAudioMixingDuration() {
  commons::log(commons::LOG_INFO, "%s: getAudioMixingDuration", MODULE_NAME);
  auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
  return ui_thread_sync_call(LOCATION_HERE, [this, media_player_source] {
    int64_t duration = 0;
    int ret = media_player_source->getDuration(duration);
    return (ret == ERR_OK ? static_cast<int>(duration) : ret);
  });
}

int MediaPlayerManager::getAudioMixingCurrentPosition() {
  commons::log(commons::LOG_INFO, "%s: getAudioMixingCurrentPosition", MODULE_NAME);
  auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
  return ui_thread_sync_call(LOCATION_HERE, [this, media_player_source] {
    int64_t position = 0;
    int ret = media_player_source->getPlayPosition(position);
    return (ret == ERR_OK ? static_cast<int>(position) : ret);
  });
}

int MediaPlayerManager::setAudioMixingPosition(int pos_ms) {
  commons::log(commons::LOG_INFO, "%s: setAudioMixingPosition", MODULE_NAME);
  auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
  return ui_thread_sync_call(LOCATION_HERE, [=] { return media_player_source->seek(pos_ms); });
}

void MediaPlayerManager::onPlayerSourceStateChanged(const media::base::MEDIA_PLAYER_STATE state,
                                                    const media::base::MEDIA_PLAYER_ERROR ec) {
  log(commons::LOG_INFO, "%s: onPlayerSourceStateChanged, state: %d, ec: %d", MODULE_NAME, state,
      ec);

  auto media_player_source = getMediaPlayer(audio_mixing_player_source_id_);
  ui_thread_sync_call(LOCATION_HERE, [&] {
    auto convertState = AUDIO_MIXING_STATE_FAILED;
    auto convertErrorCode = AUDIO_MIXING_ERROR_OK;
    if (convert_media_state_type(state, convertState) &&
        convert_media_error_type(ec, convertErrorCode)) {
      notifyState(convertState, convertErrorCode);
    }

    // operator error, already notified user, just return here.
    if (ec != media::base::PLAYER_ERROR_NONE) {
      return -1;
    }

    // operator success, append action on some state.
    if (media::base::PLAYER_STATE_OPEN_COMPLETED == state) {
      // auto enable local playback.
      auto media_audio_track =
          local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_);
      if (media_audio_track) {
        media_audio_track->enableLocalPlayback(true);
      }

      // it's a async callback, so automate `play`.
      media_player_source->play();
    } else if (media::base::PLAYER_STATE_PLAYBACK_COMPLETED == state) {
      auto media_audio_track =
          local_track_manager_->media_player_audio_track(audio_mixing_player_source_id_);
      if (media_audio_track) {
        media_audio_track->enableLocalPlayback(false);
      }

      if (remaining_cycle_ != -1) {
        remaining_cycle_--;
        if (remaining_cycle_ > 0) {
          log(commons::LOG_INFO, "%s: auto play cycle, remain_cycle_ %d", MODULE_NAME,
              remaining_cycle_);
          media_player_source->open(url_.c_str(), 0);
        }
      }
    }
    return 0;
  });
}

bool MediaPlayerManager::isAudioEffectMediaPlayer(int soundId) const {
  return -1 != getSourceIdBySoundId(soundId);
}

bool MediaPlayerManager::isAudioEffectSourceId(int sourceId) const {
  bool result = false;
  ui_thread_sync_call(LOCATION_HERE, [this, &result, sourceId] {
    for (auto& elem : sound_source_mapping_) {
      if (elem.second.source_id == sourceId) {
        result = true;
        return ERR_OK;
      }
    }
    result = false;
    return ERR_OK;
  });
  return result;
}

int MediaPlayerManager::getAllAudioEffectSoundIds(std::vector<int>& soundIds) const {
  return ui_thread_sync_call(LOCATION_HERE, [this, &soundIds] {
    for (const auto& item : sound_source_mapping_) {
      soundIds.push_back(item.first);
    }
    return ERR_OK;
  });
}

int MediaPlayerManager::preloadEffect(int soundId, const char* filePath) {
  agora_refptr<IMediaPlayerSource> media_player_source;
  std::unique_ptr<MediaPlayerSourceObserver> observer;

  if (isAudioEffectMediaPlayer(soundId)) {
    commons::log(commons::LOG_WARN, "%s: soundId:%d already preload effect", MODULE_NAME, soundId);
    return ERR_OK;
  }

  int ret = openAudioEffectFile(soundId, filePath, media_player_source, observer);
  if (ret == ERR_OK) {
    media::base::MEDIA_PLAYER_ERROR open_completed_err = media::base::PLAYER_ERROR_INTERNAL;
    observer->waitForOpenCompleted(open_completed_err);
    if (open_completed_err != media::base::PLAYER_ERROR_NONE) {
      commons::log(commons::LOG_WARN,
                   "%s: preloadEffect open file %s failed, open_completed_err %d ", MODULE_NAME,
                   filePath, static_cast<int>(open_completed_err));
      ret = -ERR_FAILED;
    }

    if (media_player_source) {
      int sourceId = media_player_source->getSourceId();
      ui_thread_sync_call(LOCATION_HERE, [this, soundId, sourceId, &observer] {
        audio_effect_observers_[sourceId] = std::move(observer);
        return ERR_OK;
      });
    }
  }

  if (ret != ERR_OK && media_player_source) {
    releaseAudioEffect(soundId);
  }
  return ret;
}

int MediaPlayerManager::playEffect(int soundId, int loopCount, double pitch, double pan, int gain) {
  int sourceId = getSourceIdBySoundId(soundId);
  auto media_player_source = getMediaPlayer(sourceId);
  if (!media_player_source) {
    commons::log(commons::LOG_WARN, "%s: playEffect no media player soundId:%d sourceId:%d found",
                 MODULE_NAME, soundId, sourceId);
    return -ERR_FAILED;
  }

  return ui_thread_sync_call(
      LOCATION_HERE, [this, soundId, sourceId, loopCount, media_player_source, pitch, pan, gain] {
        // auto enable local playback.
        auto audio_track = local_track_manager_->media_player_audio_track(sourceId);
        int ret = -ERR_FAILED;
        if (audio_track) {
          audio_track->enableLocalPlayback(true);
          audio_track->adjustPublishVolume(gain);
          audio_track->adjustPlayoutVolume(gain);
          auto media_player_source_state = media_player_source->getState();
          if (media::base::PLAYER_STATE_PLAYING != media_player_source_state) {
            if (media::base::PLAYER_STATE_PLAYBACK_COMPLETED == media_player_source_state ||
                media::base::PLAYER_STATE_IDLE == media_player_source_state) {
              const std::string url = getFilePathBySoundId(soundId);
              media_player_source->open(url.c_str(), 0);
            }
            media_player_source->setLoopCount(loopCount);
            ret = media_player_source->play();
          } else {
            ret = ERR_OK;
          }
        } else {
          commons::log(commons::LOG_WARN, "%s: playEffect no audio track %d found", MODULE_NAME,
                       sourceId);
        }
        return ret;
      });
}

int MediaPlayerManager::playAllEffects(int loopCount, double pitch, double pan, int gain) {
  return ui_thread_sync_call(LOCATION_HERE, [this, loopCount, pitch, pan, gain] {
    for (const auto& item : sound_source_mapping_) {
      playEffect(item.first, loopCount, pitch, pan, gain);
    }
    return ERR_OK;
  });
}

int MediaPlayerManager::pauseEffect(int soundId) {
  int sourceId = getSourceIdBySoundId(soundId);
  return doAction(sourceId, [](agora_refptr<IMediaPlayerSource> media_player_source) {
    return media_player_source->pause();
  });
}

int MediaPlayerManager::pauseAllEffects() {
  return doActionForAll([this](int soundId) { return pauseEffect(soundId); });
}

int MediaPlayerManager::resumeEffect(int soundId) {
  int sourceId = getSourceIdBySoundId(soundId);
  return doAction(sourceId, [](agora_refptr<IMediaPlayerSource> media_player_source) {
    return media_player_source->resume();
  });
}

int MediaPlayerManager::resumeAllEffects() {
  return doActionForAll([this](int soundId) { return resumeEffect(soundId); });
}

int MediaPlayerManager::stopEffect(int soundId) {
  int sourceId = getSourceIdBySoundId(soundId);
  return doAction(sourceId, [](agora_refptr<IMediaPlayerSource> media_player_source) {
    return media_player_source->stop();
  });
}

int MediaPlayerManager::stopAllEffects() {
  return doActionForAll([this](int soundId) { return stopEffect(soundId); });
}

int MediaPlayerManager::unloadEffect(int soundId) { return releaseAudioEffect(soundId); }

int MediaPlayerManager::unloadAllEffects() {
  return ui_thread_sync_call(LOCATION_HERE, [this] {
    while (!sound_source_mapping_.empty()) {
      releaseAudioEffect(sound_source_mapping_.begin()->first);
    }
    return ERR_OK;
  });
}

int MediaPlayerManager::getEffectsVolume() {
  return ui_thread_sync_call(LOCATION_HERE, [this] { return default_effects_volume_; });
}

int MediaPlayerManager::setEffectsVolume(int volume) {
  volume = std::max(volume, 0);
  volume = std::min(volume, 100);

  return ui_thread_sync_call(LOCATION_HERE, [this, volume] {
    default_effects_volume_ = volume;
    for (auto& elem : sound_source_mapping_) {
      setVolumeOfEffect(elem.first, volume);
    }
    return ERR_OK;
  });
}

int MediaPlayerManager::getVolumeOfEffect(int soundId) {
  int sourceId = getSourceIdBySoundId(soundId);
  return getPlayoutVolume(sourceId);
}

int MediaPlayerManager::setVolumeOfEffect(int soundId, int volume) {
  int sourceId = getSourceIdBySoundId(soundId);
  adjustPlayoutVolume(sourceId, volume);
  adjustPublishVolume(sourceId, volume);
  return ERR_OK;
}

int MediaPlayerManager::getSourceIdBySoundId(int soundId) const {
  return ui_thread_sync_call(LOCATION_HERE, [this, soundId] {
    int sourceId = -1;
    const auto search = sound_source_mapping_.find(soundId);
    if (search != sound_source_mapping_.end()) {
      sourceId = search->second.source_id;
    }
    return sourceId;
  });
}

std::string MediaPlayerManager::getFilePathBySoundId(int soundId) const {
  std::string filePath;
  ui_thread_sync_call(LOCATION_HERE, [this, soundId, &filePath] {
    const auto search = sound_source_mapping_.find(soundId);
    if (search != sound_source_mapping_.end()) {
      filePath = search->second.file_path;
    }
    return ERR_OK;
  });
  return filePath;
}

int MediaPlayerManager::getSoundIdBySourceId(int sourceId) const {
  return ui_thread_sync_call(LOCATION_HERE, [this, sourceId] {
    int soundId = -1;
    for (const auto& elem : sound_source_mapping_) {
      if (elem.second.source_id == sourceId) {
        soundId = elem.first;
        break;
      }
    }
    return soundId;
  });
}

int MediaPlayerManager::openAudioEffectFile(int soundId, const char* filePath,
                                            agora_refptr<IMediaPlayerSource>& mediaPlayerSource,
                                            std::unique_ptr<MediaPlayerSourceObserver>& observer) {
  int ret =
      ui_thread_sync_call(LOCATION_HERE, [this, soundId, filePath, &mediaPlayerSource, &observer] {
        int ret = -ERR_FAILED;
        mediaPlayerSource = createMediaPlayer(media::base::MEDIA_PLAYER_SOURCE_DEFAULT);
        if (!mediaPlayerSource) {
          commons::log(commons::LOG_WARN, "%s: create media player source failed", MODULE_NAME);
          return ret;
        }

        auto sourceId = mediaPlayerSource->getSourceId();
        sound_source_mapping_[soundId] = {sourceId, filePath};

        // Create local audio track
        agora_refptr<ILocalAudioTrack> audio_track;
        if (!local_track_manager_->media_player_audio_track(sourceId)) {
          audio_track =
              local_track_manager_->createMediaPlayerAudioTrack(sourceId, mediaPlayerSource);
        }

        if (!audio_track) {
          commons::log(commons::LOG_WARN, "%s: create local audio track failed", MODULE_NAME);
          return ret;
        }

        observer = commons::make_unique<MediaPlayerSourceObserver>(sourceId, this);
        mediaPlayerSource->registerPlayerSourceObserver(observer.get());

        ret = mediaPlayerSource->open(filePath, 0);
        if (ERR_OK != ret) {
          commons::log(commons::LOG_WARN, "%s: open file call %s failed", MODULE_NAME, filePath);
        }
        return ret;
      });
  return ret;
}

int MediaPlayerManager::releaseAudioEffect(int soundId) {
  return ui_thread_sync_call(LOCATION_HERE, [this, soundId] {
    int sourceId = getSourceIdBySoundId(soundId);
    if (-1 == sourceId) {
      return -ERR_NOT_READY;
    }
    sound_source_mapping_.erase(soundId);

    auto media_player_source = getMediaPlayer(sourceId);
    if (media_player_source) {
      media_player_source->stop();

      if (audio_effect_observers_.find(sourceId) != audio_effect_observers_.end()) {
        auto& observer = audio_effect_observers_[sourceId];
        if (observer) {
          media_player_source->unregisterPlayerSourceObserver(observer.get());
        }
        audio_effect_observers_.erase(sourceId);
      }

      auto audio_track = local_track_manager_->media_player_audio_track(sourceId);
      if (audio_track) {
        local_track_manager_->destroyMediaPlayerAudioTrack(sourceId);
      }
      destroyMediaPlayer(media_player_source);
    }
    return static_cast<int>(ERR_OK);
  });
}

int MediaPlayerManager::getPlayoutVolume(int sourceId) {
  int volume = -1;
  (void)ui_thread_sync_call(LOCATION_HERE, [&] {
    int ret = -ERR_NOT_READY;
    auto media_audio_track = local_track_manager_->media_player_audio_track(sourceId);
    if (media_audio_track) {
      ret = media_audio_track->getPlayoutVolume(&volume);
    } else {
      commons::log(commons::LOG_WARN, "%s: No media player audio track %d found", MODULE_NAME,
                   sourceId);
    }

    return ret;
  });
  return volume;
}

int MediaPlayerManager::adjustPlayoutVolume(int sourceId, int volume) {
  return ui_thread_sync_call(LOCATION_HERE, [=] {
    int ret = -ERR_NOT_READY;
    auto media_audio_track = local_track_manager_->media_player_audio_track(sourceId);
    if (media_audio_track) {
      ret = media_audio_track->adjustPlayoutVolume(volume);
    }

    return ret;
  });
}

int MediaPlayerManager::adjustPublishVolume(int sourceId, int volume) {
  return ui_thread_sync_call(LOCATION_HERE, [=] {
    int ret = -ERR_NOT_READY;
    auto media_audio_track = local_track_manager_->media_player_audio_track(sourceId);
    if (media_audio_track) {
      ret = media_audio_track->adjustPublishVolume(volume);
    }

    return ret;
  });
}

int MediaPlayerManager::doAction(int sourceId,
                                 std::function<int(agora_refptr<IMediaPlayerSource>)> action) {
  auto media_player_source = getMediaPlayer(sourceId);
  if (!media_player_source) {
    commons::log(commons::LOG_WARN, "%s: doAction no media player source %d found", MODULE_NAME,
                 sourceId);
    return -ERR_NOT_READY;
  }

  return action(media_player_source);
}

int MediaPlayerManager::doActionForAll(std::function<int(int)> action) {
  return ui_thread_sync_call(LOCATION_HERE, [this, &action] {
    for (const auto& item : sound_source_mapping_) {
      action(item.first);
    }
    return ERR_OK;
  });
}

void MediaPlayerManager::notifyState(const AUDIO_MIXING_STATE_TYPE state,
                                     const AUDIO_MIXING_ERROR_TYPE ec) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  event_callback_->Post(LOCATION_HERE, [=](auto event_handler) {
    protocol::evt::PAudioMixingStateChanged mixing_state_changed;
    mixing_state_changed.state = static_cast<int>(state);
    mixing_state_changed.errorCode = static_cast<int>(ec);

    std::string s;
    serializeEvent(mixing_state_changed, s);

    if (is_ex_handler_ && static_cast<IRtcEngineEventHandlerEx*>(event_handler)
                              ->onEvent(RTC_EVENT::AUDIO_MIXING_STATE_CHANGED, &s)) {
    } else {
      event_handler->onAudioMixingStateChanged(state, ec);
    }
  });
}

void MediaPlayerManager::notifyPlaybackCompleted(int sourceId) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  int soundId = getSoundIdBySourceId(sourceId);
  if (sound_source_mapping_.find(soundId) == sound_source_mapping_.end()) {
    return;
  }
  event_callback_->Post(LOCATION_HERE,
                        [=](auto event_handler) { event_handler->onAudioEffectFinished(soundId); });
  if (audio_effect_finished_cb_) {
    audio_effect_finished_cb_(soundId);
  }
}

}  // namespace rtc
}  // namespace agora
