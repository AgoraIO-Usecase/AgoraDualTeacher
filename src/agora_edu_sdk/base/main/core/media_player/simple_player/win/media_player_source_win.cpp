//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "media_player_source_win.h"

#include "utils/log/log.h"

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[MPSW]";

MediaPlayerSourceWin::MediaPlayerSourceWin(base::IAgoraService* agora_service,
                                           utils::worker_type player_worker)
    : MediaPlayerSourceImpl(agora_service, player_worker) {
  player_worker_->sync_call(LOCATION_HERE, [this]() {
    audio_file_reader_ = std::make_unique<AudioFileReaderWindows>();
    return 0;
  });
}

MediaPlayerSourceWin::~MediaPlayerSourceWin() {
  player_worker_->sync_call(LOCATION_HERE, [this]() {
    stop();

    if (audio_send_timer_) {
      audio_send_timer_->cancel();
      audio_send_timer_ = nullptr;
    }

    if (audio_file_reader_) {
      audio_file_reader_ = nullptr;
    }

    return 0;
  });
}

bool MediaPlayerSourceWin::doOpen(const char* url, int64_t /*start_pos*/) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (!url || 0 == std::strlen(url)) {
    commons::log(commons::LOG_ERROR, "%s: invalid URL in doOpen()", MODULE_NAME);
    return false;
  }

  if (!audio_file_reader_->AudioFileOpen(url)) {
    commons::log(commons::LOG_ERROR, "%s: audio file open failed in doOpen()", MODULE_NAME);
    return false;
  }

  if (!checkStreamFormat()) {
    commons::log(commons::LOG_ERROR, "%s: check stream format failed in doOpen()", MODULE_NAME);
    return false;
  }

  return true;
}

void MediaPlayerSourceWin::doPlay() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  audio_sent_frames_ = 0;
  audio_send_timer_.reset(player_worker_->createTimer(
      std::bind(&MediaPlayerSourceWin::sendAudioData, this), AUDIO_FRAME_SEND_INTERVAL_MS));
}

void MediaPlayerSourceWin::doPause() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (audio_send_timer_) {
    audio_send_timer_->cancel();
    audio_send_timer_ = nullptr;
  }
}

bool MediaPlayerSourceWin::doStop() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (audio_send_timer_) {
    audio_send_timer_->cancel();
    audio_send_timer_ = nullptr;
  }

  if (!audio_file_reader_->AudioFileClose()) {
    commons::log(commons::LOG_ERROR, "%s: audio file close failed", MODULE_NAME);
    return false;
  }

  return true;
}

void MediaPlayerSourceWin::doResume() { doPlay(); }

void MediaPlayerSourceWin::doSeek(int64_t new_pos_ms) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  audio_file_reader_->SeekToPositionMillisecond(new_pos_ms);
}

void MediaPlayerSourceWin::doGetDuration(int64_t& dur_ms) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  dur_ms = audio_file_reader_->AudioFileLengthMs();
}

void MediaPlayerSourceWin::doGetPlayPosition(int64_t& curr_pos_ms) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  curr_pos_ms = audio_file_reader_->GetCurrentPositionMillisecond();
}

void MediaPlayerSourceWin::doGetStreamCount(int64_t& count) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  // currently only audio stream
  count = 1;
}

void MediaPlayerSourceWin::doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  // currently only audio stream
  if (index != 0) {
    commons::log(commons::LOG_ERROR, "%s: non-zero index in doGetStreamInfo()", MODULE_NAME);
    return;
  }

  if (!info) {
    commons::log(commons::LOG_ERROR, "%s: nullptr stream info in doGetStreamInfo()", MODULE_NAME);
    return;
  }

  info->streamIndex = 0;
  info->streamType = media::base::STREAM_TYPE_AUDIO;
  info->audioSampleRate = audio_file_reader_->AudioFileSampleRate();
  info->audioChannels = audio_file_reader_->AudioFileChannels();
  info->audioBitsPerSample = audio_file_reader_->AudioFileBitsPerSample();
  int64_t dur_ms = 0;
  doGetDuration(dur_ms);
  info->duration = dur_ms / 1000;
}

void MediaPlayerSourceWin::sendAudioData() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  // here are the only two places of calling updateStateAndNotify() outside Media Player Source
  if (audio_file_reader_->IsEndOfFile()) {
    notifyCompleted();
    if (loop_count_ != 0) {
      if (loop_count_ > 0) {
        --loop_count_;
      }

      if (!audio_file_reader_->AudioFileRewind()) {
        commons::log(commons::LOG_ERROR,
                     "%s: audio file rewind failed in sendAudioData(), going to stop", MODULE_NAME);

        // when looping and failed to rewind, should stop and exit current callback
        // no matter whether stop correctly or not, should update and notify failed
        (void)doStop();
        updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
        return;
      }
    } else {
      // when non-looping, should stop and exit current callback
      if (doStop()) {
        // here is the only place to update player state to playback completed, this state
        // will be consumed by MediaPlayerManager::onPlayerSourceStateChanged()
        updateStateAndNotify(media::base::PLAYER_STATE_PLAYBACK_COMPLETED);
      } else {
        updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
      }

      return;
    }
  }

  // 'audio_sent_frames_' will be reset to 0 before each time starting the timer
  if (0 == audio_sent_frames_) {
    audio_send_start_ms_ = commons::now_ms();
    audio_last_report_ms_ = 0;
  }

  uint64_t curr_ms = commons::now_ms();

  // notify position change every second (1000 ms)
  if (curr_ms - audio_last_report_ms_ >= 1000) {
    int64_t curr_pos_ms = 0;
    doGetPlayPosition(curr_pos_ms);
    notifyPositionChanged(static_cast<int>(curr_pos_ms / 1000));

    audio_last_report_ms_ = curr_ms;
  }

  // should send one audio frame every 10 ms
  int64_t target_sent_frames =
      static_cast<int64_t>((curr_ms - audio_send_start_ms_) / AUDIO_FRAME_SEND_INTERVAL_MS) + 1;

  uint32_t samples_per_sec = audio_file_reader_->AudioFileSampleRate();
  uint32_t samples_per_10ms = samples_per_sec / 100;
  uint32_t channels = audio_file_reader_->AudioFileChannels();
  // should multiply with 'channels' to align with AudioPcmDataSenderImpl::sendAudioPcmData()
  uint32_t bytes_per_sample = channels * (audio_file_reader_->AudioFileBitsPerSample() / 8);

  int16_t data[8192] = {0};

  while (target_sent_frames > audio_sent_frames_) {
    memset(data, 0, sizeof(data));

    if (!audio_file_reader_->AudioFileRead(data)) {
      commons::log(commons::LOG_ERROR,
                   "%s: audio file read failed in sendAudioData(), going to break send loop",
                   MODULE_NAME);
      break;
    }

    // the calling arguments are similar to MediaPlayerSourceImpl::checkStreamFormat()
    audio_pcm_data_sender_->sendAudioPcmData(data, 0, samples_per_10ms, bytes_per_sample, channels,
                                             samples_per_sec);
    ++audio_sent_frames_;
  }
}

}  // namespace rtc
}  // namespace agora
