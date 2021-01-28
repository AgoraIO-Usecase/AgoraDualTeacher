//
//  Agora Media SDK
//
//  Created by panqingyou in 2020/01.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include <map>
#include <string>

#include "IAgoraMediaPlayerSource.h"
#include "api2/IAgoraService.h"
#include "api2/NGIAgoraMediaNodeFactory.h"
#include "facilities/tools/rtc_callback.h"
#include "internal/rtc_engine_i.h"
#include "utils/packer/packet.h"

namespace agora {
namespace rtc {
class ChannelManager;
class LocalTrackManager;
class MediaPlayerSourceObserver;

class MediaPlayerManager : public IMediaPlayerSourceObserver {
 protected:
  struct SourceInfo {
    int32_t source_id = -1;
    std::string file_path = "";
  };

 public:
  using AudioEffectFinishedCallBack = std::function<void(int)>;
  MediaPlayerManager(const agora_refptr<IMediaNodeFactory>& media_node_factory,
                     LocalTrackManager* local_track_manager, IRtcEngineEventHandler* event_handler,
                     bool is_ex_handler);

  ~MediaPlayerManager();

  void registerEffectFinishedCallBack(AudioEffectFinishedCallBack&& callBack);
  void unregisterEffectFinishedCallBack();

  agora_refptr<IMediaPlayerSource> createMediaPlayer(media::base::MEDIA_PLAYER_SOURCE_TYPE type);
  agora_refptr<IMediaPlayerSource> getMediaPlayer(int sourceId);
  int destroyMediaPlayer(agora_refptr<IMediaPlayerSource> mediaPlayer);

  // For audio mixing
  int startAudioMixing(const char* filePath, bool loopback, bool replace, int cycle);
  int stopAudioMixing();
  int pauseAudioMixing();
  int resumeAudioMixing();

  int adjustAudioMixingVolume(int volume);
  int adjustAudioMixingPublishVolume(int volume);
  int getAudioMixingPublishVolume();
  int adjustAudioMixingPlayoutVolume(int volume);
  int getAudioMixingPlayoutVolume();
  int getAudioMixingDuration();
  int getAudioMixingCurrentPosition();
  int setAudioMixingPosition(int pos_ms);

  int getAudioMixingPlayerSourceId() { return audio_mixing_player_source_id_; }

  void onPlayerSourceStateChanged(const media::base::MEDIA_PLAYER_STATE state,
                                  const media::base::MEDIA_PLAYER_ERROR ec) override;
  void onPositionChanged(const int64_t position) override {}
  void onPlayerEvent(const media::base::MEDIA_PLAYER_EVENT event) override {}
  void onMetaData(const void* data, int length) override {}
  void onCompleted() override {}

 public:
  // For audio effects
  bool isAudioEffectMediaPlayer(int soundId) const;
  bool isAudioEffectSourceId(int sourceId) const;
  int getAllAudioEffectSoundIds(std::vector<int>& soundIds) const;

  int preloadEffect(int soundId, const char* filePath);
  int playEffect(int soundId, int loopCount, double pitch, double pan, int gain);
  int playAllEffects(int loopCount, double pitch, double pan, int gain);

  int pauseEffect(int soundId);
  int pauseAllEffects();

  int resumeEffect(int soundId);
  int resumeAllEffects();

  int stopEffect(int soundId);
  int stopAllEffects();

  int unloadEffect(int soundId);
  int unloadAllEffects();

  int getEffectsVolume();
  int setEffectsVolume(int volume);

  int getVolumeOfEffect(int soundId);
  int setVolumeOfEffect(int soundId, int volume);

  int getSourceIdBySoundId(int soundId) const;
  std::string getFilePathBySoundId(int soundId) const;
  int getSoundIdBySourceId(int sourceId) const;

 private:
  int openAudioEffectFile(int soundId, const char* filePath,
                          agora_refptr<IMediaPlayerSource>& mediaPlayerSource /* output*/,
                          std::unique_ptr<MediaPlayerSourceObserver>& observer /* output*/);
  int releaseAudioEffect(int soundId);
  int getPlayoutVolume(int sourceId);
  int adjustPlayoutVolume(int sourceId, int volume);
  int adjustPublishVolume(int sourceId, int volume);
  int doAction(int sourceId, std::function<int(agora_refptr<IMediaPlayerSource>)> action);
  int doActionForAll(std::function<int(int)> action);

  template <class T>
  static bool serializeEvent(const T& p, std::string& result) {
    commons::packer pk;
    pk << p;
    pk.pack();
    result = std::string(pk.buffer(), pk.length());
    return true;
  }

  void notifyState(const AUDIO_MIXING_STATE_TYPE state, const AUDIO_MIXING_ERROR_TYPE ec);
  void notifyPlaybackCompleted(int sourceId);

 private:
  friend class MediaPlayerSourceObserver;
  int32_t audio_mixing_player_source_id_ = -1;
  int default_effects_volume_ = 100;

  agora_refptr<IMediaNodeFactory> media_node_factory_;
  LocalTrackManager* local_track_manager_;
  bool is_ex_handler_;

  utils::RtcAsyncCallback<IRtcEngineEventHandler>::Type event_callback_;

  std::map<int32_t, SourceInfo> sound_source_mapping_;
  std::map<int32_t, agora_refptr<IMediaPlayerSource>> media_players_;
  std::map<int32_t, std::unique_ptr<MediaPlayerSourceObserver>> audio_effect_observers_;

  std::string url_;
  int remaining_cycle_;
  AudioEffectFinishedCallBack audio_effect_finished_cb_;
};

}  // namespace rtc
}  // namespace agora
