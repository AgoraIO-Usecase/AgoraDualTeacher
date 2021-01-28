//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-11.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//

#include "media_player_source_apple.h"
#include "main/core/media_player/simple_player/apple/audio_file_reader_apple_file.h"
#include "main/core/media_player/simple_player/apple/audio_file_reader_apple_url.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[MPSA]";

bool CheckFileProtocol(std::string url, std::string protocol) {
  std::transform(url.begin(), url.end(), url.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  std::transform(protocol.begin(), protocol.end(), protocol.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  std::string::size_type idx = url.find(protocol);
  return (idx == 0);
}

MediaPlayerSourceApple::MediaPlayerSourceApple(base::IAgoraService* agora_service,
                                               utils::worker_type player_worker)
    : MediaPlayerSourceImpl(agora_service, player_worker) {}

MediaPlayerSourceApple::~MediaPlayerSourceApple() {
  player_worker_->sync_call(LOCATION_HERE, [this]() {
    stop();
    if (audio_file_reader_) {
      audio_file_reader_.reset();
    }

    return 0;
  });
}

bool MediaPlayerSourceApple::internalOpen(const char* url, int64_t start_pos, bool if_seek) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  // currently we don't care 'start_pos'
  if (!url || 0 == std::strlen(url)) {
    commons::log(commons::LOG_ERROR, "%s: invalid URL in doOpen()", MODULE_NAME);
    return false;
  }

  is_url_ = CheckFileProtocol(url, "http") || CheckFileProtocol(url, "https");
  if (is_url_) {
    audio_file_reader_ = std::make_unique<AudioFileReaderAppleUrl>(if_seek);
  } else {
    audio_file_reader_ = std::make_unique<AudioFileReaderAppleFile>();
  }

  if (!audio_file_reader_->AudioFileOpen(url)) {
    commons::log(commons::LOG_ERROR, "%s: audio file open failed in doOpen()", MODULE_NAME);
    return false;
  }

  url_ = std::string(url);
  start_position_ = start_pos;
  data_available_ = true;

  return true;
}

bool MediaPlayerSourceApple::doOpen(const char* url, int64_t start_pos) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  return internalOpen(url, start_pos, false);
}

void MediaPlayerSourceApple::doPlay() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  audio_sent_frames_ = 0;
  if (!audio_send_timer_) {
    audio_send_timer_.reset(player_worker_->createTimer(
        std::bind(&MediaPlayerSourceApple::sendAudioData, this), AUDIO_FRAME_SEND_INTERVAL_MS));
  }
}

bool MediaPlayerSourceApple::doStop() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (audio_send_timer_) {
    audio_send_timer_.reset();
  }

  if (audio_file_reader_) {
    bool closed = audio_file_reader_->AudioFileClose();
    if (closed) {
      return true;
    } else {
      commons::log(commons::LOG_ERROR, "%s: audio file close failed", MODULE_NAME);
      return false;
    }
  }

  return true;
}

void MediaPlayerSourceApple::doPause() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (audio_send_timer_) {
    audio_send_timer_.reset();
  }
}

void MediaPlayerSourceApple::doResume() { doPlay(); }

void MediaPlayerSourceApple::doGetDuration(int64_t& dur_ms) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  dur_ms = audio_file_reader_->AudioFileLengthMs();
}

void MediaPlayerSourceApple::doGetPlayPosition(int64_t& curr_pos_ms) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  curr_pos_ms = audio_file_reader_->GetCurrentPositionMillisecond();
}

void MediaPlayerSourceApple::doGetStreamCount(int64_t& count) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  // currently only audio stream
  count = 1;
}

void MediaPlayerSourceApple::doSeek(int64_t new_pos_ms) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  int64_t total = audio_file_reader_->AudioFileLengthMs();
  int64_t position = new_pos_ms;
  position = position > 0 ? position : 0;
  position = position < total ? position : total;

  audio_file_reader_->SeekToPositionMillisecond(position);
  if (is_url_) {
    if (audio_send_timer_) {
      audio_send_timer_.reset();
    }

    if (!url_.empty()) {
      (void)internalOpen(url_.c_str(), start_position_, true);
      doPlay();
    }
  }
}

void MediaPlayerSourceApple::doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) {
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
  info->audioBitsPerSample = audio_file_reader_->AudioFileBytesPerSample() * 8;
  int64_t dur_ms = 0;
  doGetDuration(dur_ms);
  info->duration = dur_ms / 1000;
}

void MediaPlayerSourceApple::sendAudioData() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (!audio_file_reader_->CheckMediaIsAvailable()) {
    (void)audio_file_reader_->AudioFileClose();
    (void)doStop();
    updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
    return;
  }

  if (!data_available_) {
    notifyCompleted();
    if (loop_count_ != 0) {
      if (loop_count_ > 0) {
        --loop_count_;
      }

      if (is_url_) {
        if (audio_send_timer_) {
          audio_send_timer_.reset();
        }

        audio_file_reader_->SeekToPositionMillisecond(0);
        if (!url_.empty()) {
          (void)doOpen(url_.c_str(), start_position_);
          doPlay();
        }
        return;
      } else {
        data_available_ = true;
        if (!audio_file_reader_->AudioFileRewind()) {
          commons::log(commons::LOG_ERROR,
                       "%s: audio file rewind failed in sendAudioData(), going to stop",
                       MODULE_NAME);
          // when looping and failed to rewind, should stop and exit current callback
          // no matter whether stop correctly or not, should update and notify failed
          (void)doStop();
          updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
          return;
        }
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
  } else {
    if (!audio_file_reader_->AudioFileHaveStreamBytes()) {
      return;
    }

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
    uint32_t bytes_per_sample = channels * (audio_file_reader_->AudioFileBytesPerSample());

    int16_t data[8192] = {0};

    while (target_sent_frames > audio_sent_frames_) {
      memset(data, 0, sizeof(data));
      data_available_ = audio_file_reader_->AudioFileRead(data);

      if (!data_available_) {
        commons::log(commons::LOG_ERROR,
                     "%s: audio file read failed in sendAudioData(), going to break send loop",
                     MODULE_NAME);
        break;
      }

      audio_pcm_data_sender_->sendAudioPcmData(reinterpret_cast<void*>(data), 0, samples_per_10ms,
                                               bytes_per_sample, channels, samples_per_sec);
      ++audio_sent_frames_;
    }
  }
}

} /* namespace rtc */
} /* namespace agora */
