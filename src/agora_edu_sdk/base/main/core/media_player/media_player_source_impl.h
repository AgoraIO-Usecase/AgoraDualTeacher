//
//  Agora RTC/MEDIA SDK
//
//  Created by Tommy Miao in 2020-03.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <tuple>
#include <unordered_map>

#include "api2/internal/audio_node_i.h"
#include "api2/internal/media_player_source_i.h"
#include "facilities/tools/rtc_callback.h"
#include "utils/thread/thread_control_block.h"  // for agora::utils::worker_type

namespace agora {
namespace base {
class IAgoraService;
}  // namespace base

namespace rtc {

class IAudioPcmDataSender;

class MediaPlayerSourceImpl : public IMediaPlayerSourceEx {
 private:
  enum Action {
    OPEN,
    OPEN_DONE,
    OPEN_FAILED,
    PLAY,
    PLAY_DONE,
    REWIND_FAILED,
    PAUSE,
    PAUSE_DONE,
    STOP,
    STOP_DONE,
    STOP_FAILED,
    RESUME,
    SEEK,
    SEEK_DONE,
    GET_DUR,
    GET_PLAY_POS,
    GET_STREAM_CNT,
    GET_STREAM_INFO,
    GET_DONE,
    UPDATE
  };

  // url, new_pos_ms, get_val, index, info, prev_state
  using action_func_type =
      std::function<int(std::string, int64_t, int64_t*, int64_t, media::base::MediaStreamInfo*,
                        media::base::MEDIA_PLAYER_STATE)>;
  // next state and action function
  using action_pair_type = std::pair<media::base::MEDIA_PLAYER_STATE, action_func_type>;
  using action_tuple_type = std::tuple<Action, media::base::MEDIA_PLAYER_STATE, action_func_type>;
  using state_machine_map_type =
      std::unordered_multimap<media::base::MEDIA_PLAYER_STATE, action_tuple_type>;

 public:
  MediaPlayerSourceImpl(base::IAgoraService* agora_service, utils::worker_type player_worker);

  ~MediaPlayerSourceImpl() override;

  int getSourceId() const final;

  int open(const char* url, int64_t start_pos) final;
  int play() final;

  int pause() final;
  int stop() final;
  int resume() final;
  int seek(int64_t new_pos_ms) final;

  int getDuration(int64_t& dur_ms) final;
  int getPlayPosition(int64_t& curr_pos_ms) final;
  int getStreamCount(int64_t& count) final;
  int getStreamInfo(int64_t index, media::base::MediaStreamInfo* info) final;

  // these operations won't be included in state machine
  int setLoopCount(int64_t loop_count) final;
  int changePlaybackSpeed(media::base::MEDIA_PLAYER_PLAYBACK_SPEED speed) final;
  int selectAudioTrack(int64_t index) final;
  int setPlayerOption(const char* key, int64_t value) final;
  int takeScreenshot(const char* file_name) final;
  int selectInternalSubtitle(int64_t index) final;
  int setExternalSubtitle(const char* url) final;

  media::base::MEDIA_PLAYER_STATE getState() final;

  agora_refptr<rtc::IAudioPcmDataSender> getAudioPcmDataSender() override;
  agora_refptr<rtc::IVideoFrameSender> getVideoFrameSender() override;

  int registerPlayerSourceObserver(IMediaPlayerSourceObserver* observer) override;
  int unregisterPlayerSourceObserver(IMediaPlayerSourceObserver* observer) override;

  int registerAudioFrameObserver(media::base::IAudioFrameObserver* observer) override;
  int unregisterAudioFrameObserver(media::base::IAudioFrameObserver* observer) override;

 protected:
  virtual bool doOpen(const char* url, int64_t start_pos) = 0;
  virtual void doPlay() = 0;

  virtual void doPause() = 0;
  virtual bool doStop() = 0;
  virtual void doResume() = 0;
  virtual void doSeek(int64_t new_pos_ms) = 0;

  virtual void doGetDuration(int64_t& dur_ms) = 0;
  virtual void doGetPlayPosition(int64_t& curr_pos_ms) = 0;
  virtual void doGetStreamCount(int64_t& count) = 0;
  virtual void doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) = 0;

  virtual void doSetLoopCount(int64_t loop_count);

  // these operations won't be used in simple player but will be overriden in FFMPEG player.
  virtual int doChangePlaybackSpeed(media::base::MEDIA_PLAYER_PLAYBACK_SPEED speed) {
    return -ERR_NOT_SUPPORTED;
  }
  virtual int doSelectAudioTrack(int64_t index) { return -ERR_NOT_SUPPORTED; }
  virtual int doSetPlayerOption(const char* key, int64_t value) { return -ERR_NOT_SUPPORTED; }
  virtual int doTakeScreenshot(const char* file_name) { return -ERR_NOT_SUPPORTED; }
  virtual int doSelectInternalSubtitle(int64_t index) { return -ERR_NOT_SUPPORTED; }
  virtual int doSetExternalSubtitle(const char* url) { return -ERR_NOT_SUPPORTED; }

  bool checkStreamFormat();

  void notifyPositionChanged(int curr_pos_secs);
  void notifyCompleted();

  void updateState(media::base::MEDIA_PLAYER_STATE new_state);
  void updateStateAndNotify(media::base::MEDIA_PLAYER_STATE new_state);

 private:
  static bool _IsGetOrSeekAction(Action action) {
    return (action == Action::GET_DUR || action == Action::GET_PLAY_POS ||
            action == Action::GET_STREAM_CNT || action == Action::GET_STREAM_INFO ||
            action == Action::SEEK);
  }

  int _doNothing(std::string, int64_t, int64_t*, int64_t, media::base::MediaStreamInfo*,
                 media::base::MEDIA_PLAYER_STATE) {
    return static_cast<int>(ERR_OK);
  }

  void _revertToPrevState(media::base::MEDIA_PLAYER_STATE prev_state);
  void _notifyStateChanged(media::base::MEDIA_PLAYER_STATE new_state);

  action_pair_type _changeState(Action action);

 protected:
  utils::worker_type player_worker_;

  utils::RtcAsyncCallback<IMediaPlayerSourceObserver>::Type observers_;
  // for FFMPEG player only
  utils::RtcSyncCallback<media::base::IAudioFrameObserver>::Type audio_frame_observers_;

  agora_refptr<IMediaNodeFactory> media_node_factory_;

  agora_refptr<IAudioPcmDataSender> audio_pcm_data_sender_;
  agora_refptr<IVideoFrameSender> video_frame_sender_;

  int64_t loop_count_ = 0;

 private:
  static std::atomic<int32_t> source_ids_;
  const int32_t source_id_;

  base::IAgoraService* agora_service_;

  std::mutex state_sm_lock_;
  std::atomic<media::base::MEDIA_PLAYER_STATE> player_state_;
  state_machine_map_type state_machine_;
};

}  // namespace rtc
}  // namespace agora
