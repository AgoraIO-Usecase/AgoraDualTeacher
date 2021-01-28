//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-12.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "media_player_source_dummy.h"

#include "engine_adapter/audio/audio_node_frame_source.h"
#include "facilities/tools/aac_file_parser.h"
#include "facilities/tools/api_logger.h"
#include "utils/log/log.h"
#include "utils/refcountedobject.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[MPSD]";

class AudioDecodedDataBuffer : public IAudioPcmDataSender {
 public:
  AudioDecodedDataBuffer();
  ~AudioDecodedDataBuffer() override;

 public:  // inherit from IAudioPcmDataSender
  int sendAudioPcmData(
      const void* audio_data, uint32_t capture_timestamp,
      const size_t samples_per_channel,  // for 10ms Data, number_of_samples * 100 = sample_rate
      const size_t bytes_per_sample,     // 2 * number_of_channels
      const size_t number_of_channels, const uint32_t sample_rate) override;

  int reset();

  size_t getNumberOfChannels() const { return number_of_channels_; }

  uint32_t getSampleRate() const { return sample_rate_; }

  int getAudioPcmData(uint8_t* data_buffer, int length);

 private:
  static const int kAudioBufferSize = 10240;

  int bytes_left_;
  std::unique_ptr<uint8_t[]> buffer_left_;
  size_t number_of_channels_;
  uint32_t sample_rate_;
};

AudioDecodedDataBuffer::AudioDecodedDataBuffer()
    : bytes_left_(0),
      buffer_left_(new uint8_t[kAudioBufferSize]),
      number_of_channels_(0),
      sample_rate_(0) {}

AudioDecodedDataBuffer::~AudioDecodedDataBuffer() {}

int AudioDecodedDataBuffer::reset() {
  bytes_left_ = 0;
  number_of_channels_ = 0;
  sample_rate_ = 0;

  return ERR_OK;
}

int AudioDecodedDataBuffer::getAudioPcmData(uint8_t* data_buffer, int length) {
  if (data_buffer == nullptr || length <= 0) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (bytes_left_ < length) {
    return -ERR_NOT_READY;
  }
  memcpy(data_buffer, buffer_left_.get(), length);

  memmove(buffer_left_.get(), buffer_left_.get() + length, bytes_left_ - length);
  bytes_left_ -= length;

  return ERR_OK;
}

int AudioDecodedDataBuffer::sendAudioPcmData(
    const void* audio_data, uint32_t capture_timestamp,
    const size_t samples_per_channel,  // for 10ms Data, number_of_samples * 100 = sample_rate
    const size_t bytes_per_sample,     // sizeof(int16_t) * number_of_channels
    const size_t number_of_channels, const uint32_t sample_rate) {
  if (number_of_channels == 0 || sample_rate == 0) {
    return -ERR_INVALID_ARGUMENT;
  }
  if (bytes_per_sample != sizeof(int16_t) * number_of_channels || samples_per_channel == 0) {
    return -ERR_INVALID_ARGUMENT;
  }

  int bytes = static_cast<int>(samples_per_channel * bytes_per_sample);
  if (bytes > kAudioBufferSize) {
    return -ERR_REFUSED;
  }

  if (number_of_channels_ == 0 || sample_rate_ == 0) {
    number_of_channels_ = number_of_channels;
    sample_rate_ = sample_rate;
  }

  if (bytes_left_ + bytes > kAudioBufferSize) {
    int bytes_to_move = bytes_left_ + bytes - kAudioBufferSize;
    memmove(buffer_left_.get(), buffer_left_.get() + bytes_to_move, bytes_left_ - bytes_to_move);
    bytes_left_ -= bytes_to_move;
  }

  memcpy(buffer_left_.get() + bytes_left_, audio_data, bytes);
  bytes_left_ += bytes;

  return ERR_OK;
}

MediaPlayerSourceDummy::MediaPlayerSourceDummy(base::IAgoraService* agora_service,
                                               agora::utils::worker_type player_worker)
    : MediaPlayerSourceImpl(agora_service, player_worker),
      auido_decoder_data_buffer_(new RefCountedObject<AudioDecodedDataBuffer>) {
  encoded_frame_sender_ = media_node_factory_->createAudioEncodedFrameSender();
}

MediaPlayerSourceDummy::~MediaPlayerSourceDummy() {
  stop();
  encoded_frame_sender_ = nullptr;
}

bool MediaPlayerSourceDummy::doOpen(const char* url, int64_t startPos) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (!url) {
    log(agora::commons::LOG_WARN, "%s: invalid argument url %s", MODULE_NAME, url);
    return false;
  }

  parser_ = std::make_unique<AACFileParser>(url);
  if (!parser_->open()) {
    log(agora::commons::LOG_WARN, "%s: open file %s failed", MODULE_NAME, url);
    parser_ = nullptr;
    return false;
  }

  audio_frame_source_ = commons::make_unique<AudioFrameSource>(encoded_frame_sender_);
  AudioFrameSource* audioFrameSource = static_cast<AudioFrameSource*>(audio_frame_source_.get());
  audioFrameSource->RegisterAudioCallback(auido_decoder_data_buffer_.get());

  return true;
}

void MediaPlayerSourceDummy::doPlay() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (!parser_) {
    log(agora::commons::LOG_WARN, "%s: player has not been opened", MODULE_NAME);
    return;
  }

  audio_sent_frames_ = 0;
  audio_send_timer_.reset(player_worker_->createTimer(
      std::bind(&MediaPlayerSourceDummy::sendAudioData, this), AUDIO_FRAME_SEND_INTERVAL_MS));
}

bool MediaPlayerSourceDummy::doStop() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (audio_send_timer_) {
    audio_send_timer_->cancel();
    audio_send_timer_ = nullptr;
  }

  if (audio_frame_source_) {
    AudioFrameSource* audioFrameSource = static_cast<AudioFrameSource*>(audio_frame_source_.get());
    audioFrameSource->UnregisterAudioCallback(auido_decoder_data_buffer_.get());
    audio_frame_source_ = nullptr;
  }

  return true;
}

void MediaPlayerSourceDummy::doPause() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  audio_sent_frames_ = 0;
  audio_send_timer_.reset(player_worker_->createTimer(
      std::bind(&MediaPlayerSourceDummy::sendAudioData, this), AUDIO_FRAME_SEND_INTERVAL_MS));
}

void MediaPlayerSourceDummy::doResume() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  doPlay();
}

void MediaPlayerSourceDummy::doSeek(int64_t newPos) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());
}

void MediaPlayerSourceDummy::doGetDuration(int64_t& duration) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());
}

void MediaPlayerSourceDummy::doGetPlayPosition(int64_t& position) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());
}

void MediaPlayerSourceDummy::doGetStreamCount(int64_t& count) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  count = 1;
}

void MediaPlayerSourceDummy::doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (index || !info) {  // only audio stream until now
    return;
  }

  info->audioChannels = parser_->getNumberOfChannels();
  info->audioSampleRate = parser_->getSampleRateHz();
  info->audioBitsPerSample = 16;
  info->streamIndex = 0;
  info->streamType = media::base::STREAM_TYPE_AUDIO;
}

bool MediaPlayerSourceDummy::sendEncodedAudioData() {
  agora::rtc::EncodedAudioFrameInfo audioFrameInfo;
  audioFrameInfo.numberOfChannels = parser_->getNumberOfChannels();
  audioFrameInfo.sampleRateHz = parser_->getSampleRateHz();
  audioFrameInfo.codec = parser_->getCodecType();

  if (audioFrameInfo.numberOfChannels == 0 || audioFrameInfo.sampleRateHz == 0) {
    return false;
  }
  static const int kBufferSize = 2048;
  char buffer[kBufferSize] = {0};

  bool ret = false;
  if (parser_->hasNext()) {
    int length = kBufferSize;
    parser_->getNext(buffer, &length);
    if (length > 0) {
      ret = encoded_frame_sender_->sendEncodedAudioFrame((unsigned char*)buffer, length,
                                                         audioFrameInfo);
      if (!ret) {
        log(agora::commons::LOG_WARN, "%s: send encoded audio frame failed", MODULE_NAME);
      }
    }
  }
  return ret;
}

void MediaPlayerSourceDummy::sendAudioData() {
  size_t number_of_channels = auido_decoder_data_buffer_->getNumberOfChannels();
  uint32_t sample_rate = auido_decoder_data_buffer_->getSampleRate();

  if (number_of_channels == 0 || sample_rate == 0) {
    if (sendEncodedAudioData()) {
      number_of_channels = auido_decoder_data_buffer_->getNumberOfChannels();
      sample_rate = auido_decoder_data_buffer_->getSampleRate();
    }

    if (number_of_channels == 0 || sample_rate == 0) {
      (void)doStop();
      updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
      return;
    }
  }

  int get_data_retry_count = GET_AUDIO_DATA_MAX_RETRY_COUNT;

  static const int kBufferSize = 2048;
  uint8_t buffer[kBufferSize] = {0};
  uint32_t samples_per_10ms = sample_rate / 100;
  uint32_t bytes_per_sample = number_of_channels * sizeof(int16_t);
  int frame_length = samples_per_10ms * bytes_per_sample;

  bool got_data = false;
  while (get_data_retry_count > 0 && !(got_data = (auido_decoder_data_buffer_->getAudioPcmData(
                                                       buffer, frame_length) == ERR_OK))) {
    if (!sendEncodedAudioData()) {
      break;
    }

    --get_data_retry_count;
  }

  if (got_data) {
    // the calling arguments are similar to MediaPlayerSourceImpl::checkStreamFormat()
    audio_pcm_data_sender_->sendAudioPcmData(buffer, 0, samples_per_10ms, bytes_per_sample,
                                             number_of_channels, sample_rate);
    ++audio_sent_frames_;
  } else {
    if (!parser_->hasNext()) {
      notifyCompleted();
      if (loop_count_ != 0) {
        loop_count_ = (loop_count_ > 0) ? --loop_count_ : loop_count_;
        parser_->reset();
        auido_decoder_data_buffer_->reset();
      } else {
        if (doStop()) {
          // here is the only place to update player state to playback completed, this state
          // will be consumed by MediaPlayerManager::onPlayerSourceStateChanged()
          updateStateAndNotify(media::base::PLAYER_STATE_PLAYBACK_COMPLETED);
        } else {
          updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
        }
      }
    } else {
      (void)doStop();
      updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
    }
  }
}

} /* namespace rtc */
} /* namespace agora */
