//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <string>

#include "facilities/tools/api_logger.h"
#include "ui_thread.h"

#include "AgoraRteBase.h"
#include "base/AgoraBase.h"
#include "base/AgoraRefPtr.h"
#include "media/IAgoraMediaPlayer.h"

namespace agora {
namespace base {
class IAgoraService;
}  // namespace base

namespace rtc {
class IMediaNodeFactory;
class IMediaPlayerSourceObserver;
}  // namespace rtc

namespace rte {

class MediaPlayer : public IAgoraMediaPlayer {
 protected:
  ~MediaPlayer() = default;

 public:
  explicit MediaPlayer(base::IAgoraService* service);

  // IMediaPlayerSource
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

  int setLoopCount(int64_t loop_count) final;
  int changePlaybackSpeed(media::base::MEDIA_PLAYER_PLAYBACK_SPEED speed) final;
  int selectAudioTrack(int64_t index) final;
  int setPlayerOption(const char* key, int64_t value) final;
  int takeScreenshot(const char* file_name) final;
  int selectInternalSubtitle(int64_t index) final;
  int setExternalSubtitle(const char* url) final;

  media::base::MEDIA_PLAYER_STATE getState() final;

  template <typename ObserverT, typename FuncT>
  int observerCommon(ObserverT observer, FuncT func) {
    API_LOGGER_MEMBER("observer: %p, func: %p", observer, func);

    return rtc::ui_thread_sync_call(LOCATION_HERE,
                                    [=] { return (*media_player_source_.*func)(observer); });
  }

  int registerPlayerSourceObserver(rtc::IMediaPlayerSourceObserver* observer) override;
  int unregisterPlayerSourceObserver(rtc::IMediaPlayerSourceObserver* observer) override;

  int registerAudioFrameObserver(media::base::IAudioFrameObserver* observer) override;
  int unregisterAudioFrameObserver(media::base::IAudioFrameObserver* observer) override;

  // IAgoraMediaPlayer
  int SetStreamId(StreamId stream_id) override;
  int GetStreamId(char* stream_id_buf, size_t stream_id_buf_size) const override;

 private:
  agora_refptr<rtc::IMediaNodeFactory> media_node_factory_;

  std::string stream_id_;

  agora_refptr<rtc::IMediaPlayerSource> media_player_source_;
};

}  // namespace rte
}  // namespace agora
