//
//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2019-12.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#include "media_player_source_android.h"

#include <inttypes.h>
#include "utils/log/log.h"

#include "facilities/tools/api_logger.h"
#include "jni/MediaStreamInfo_jni.h"
#include "jni/SimpleMediaPlayerSource_jni.h"
#include "rtc_base/checks.h"
#include "rtc_base/timeutils.h"
#include "sdk/android/src/jni/jni_helpers.h"
#include "sdk/android/src/jni/jvm.h"

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[MPSA]";

void JavaToNativeMediaStreamInfo(JNIEnv* env, webrtc::ScopedJavaLocalRef<jobject> j_streamInfo,
                                 media::base::MediaStreamInfo* info) {
  info->streamIndex = webrtc::jni::Java_MediaStreamInfo_getStreamIndex(env, j_streamInfo);
  auto j_streamType = webrtc::jni::Java_MediaStreamInfo_getMediaStreamType(env, j_streamInfo);
  info->streamType = static_cast<media::base::MEDIA_STREAM_TYPE>(j_streamType);
  auto j_codecName = webrtc::jni::Java_MediaStreamInfo_getCodecName(env, j_streamInfo);
  if (j_codecName.is_null()) {
    auto codecName = webrtc::JavaToNativeString(env, j_codecName);
    codecName.copy(info->codecName, media::base::kMaxCodecNameLength);
  }
  auto j_language = webrtc::jni::Java_MediaStreamInfo_getLanguage(env, j_streamInfo);
  if (!j_language.is_null()) {
    auto language = webrtc::JavaToNativeString(env, j_language);
    language.copy(info->language, media::base::kMaxCodecNameLength);
  }

  info->videoFrameRate = webrtc::jni::Java_MediaStreamInfo_getVideoFrameRate(env, j_streamInfo);
  info->videoBitRate = webrtc::jni::Java_MediaStreamInfo_getVideoBitRate(env, j_streamInfo);
  info->videoWidth = webrtc::jni::Java_MediaStreamInfo_getVideoWidth(env, j_streamInfo);
  info->videoHeight = webrtc::jni::Java_MediaStreamInfo_getVideoHeight(env, j_streamInfo);
  info->audioSampleRate = webrtc::jni::Java_MediaStreamInfo_getAudioSampleRate(env, j_streamInfo);
  info->audioChannels = webrtc::jni::Java_MediaStreamInfo_getAudioChannels(env, j_streamInfo);
  info->duration = webrtc::jni::Java_MediaStreamInfo_getDuration(env, j_streamInfo);
  if (info->audioChannels) {
    int bytesPerSample =
        webrtc::jni::Java_MediaStreamInfo_getAudioBytesPerSample(env, j_streamInfo);
    info->audioBitsPerSample = bytesPerSample / info->audioChannels * 8;
  }
}

MediaPlayerSourceAndroid::MediaPlayerSourceAndroid(base::IAgoraService* agora_service,
                                                   utils::worker_type player_worker)
    : MediaPlayerSourceImpl(agora_service, player_worker),
      audio_samples_per_channel_(0),
      audio_bytes_per_sample_(0),
      audio_number_of_channels_(0),
      audio_sample_rate_(0),
      send_audio_timer_(nullptr),
      j_media_player_source_(webrtc::jni::Java_SimpleMediaPlayerSource_Constructor(
          webrtc::jni::AttachCurrentThreadIfNeeded(), kAudioFrameSendInterval)) {}

MediaPlayerSourceAndroid::~MediaPlayerSourceAndroid() {
  player_worker_->sync_call(LOCATION_HERE, [this]() {
    stop();
    return 0;
  });
}

bool MediaPlayerSourceAndroid::doOpen(const char* url, int64_t start_pos) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  // currently we don't care 'start_pos'
  if (!url || 0 == std::strlen(url)) {
    commons::log(commons::LOG_ERROR, "%s: invalid URL in doOpen()", MODULE_NAME);
    return false;
  }

  auto env = webrtc::jni::AttachCurrentThreadIfNeeded();
  auto j_url = webrtc::NativeToJavaString(env, url);
  auto result =
      webrtc::jni::Java_SimpleMediaPlayerSource_open(env, j_media_player_source_, j_url, start_pos);
  if (result != ERR_OK) {
    commons::log(commons::LOG_ERROR, "%s: audio file open failed in doOpen()", MODULE_NAME);
    return false;
  }

  if (!checkStreamFormat()) {
    commons::log(commons::LOG_ERROR, "%s: check stream format failed in doOpen()", MODULE_NAME);
    return false;
  }

  audio_number_of_channels_ =
      webrtc::jni::Java_SimpleMediaPlayerSource_getAudioChannels(env, j_media_player_source_);
  audio_sample_rate_ =
      webrtc::jni::Java_SimpleMediaPlayerSource_getAudioSampleRate(env, j_media_player_source_);
  audio_bytes_per_sample_ =
      webrtc::jni::Java_SimpleMediaPlayerSource_getBytesPerSample(env, j_media_player_source_);
  audio_samples_per_channel_ = audio_sample_rate_ / (1000 / kAudioFrameSendInterval);

  return true;
}

void MediaPlayerSourceAndroid::doPlay() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());
  sent_audio_frames_ = 0;
  paused_ = false;
  send_audio_timer_.reset(player_worker_->createTimer(
      std::bind(&MediaPlayerSourceAndroid::onTimer, this), kAudioFrameSendInterval));
}

bool MediaPlayerSourceAndroid::doStop() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());
  send_audio_timer_.reset();
  paused_ = false;

  return true;
}

void MediaPlayerSourceAndroid::doPause() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  paused_ = true;
}

void MediaPlayerSourceAndroid::doResume() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  // TODO(tomiao): consider getting aligned with Win/Mac implementation and abstract sendAudioData()
  // to parent class
  paused_ = false;
  sent_audio_frames_ = 0;
}

void MediaPlayerSourceAndroid::doGetDuration(int64_t& duration) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  auto env = webrtc::jni::AttachCurrentThreadIfNeeded();
  auto j_duration =
      webrtc::jni::Java_SimpleMediaPlayerSource_getDuration(env, j_media_player_source_);
  if (j_duration != -1) {
    duration = j_duration;
  }
}

void MediaPlayerSourceAndroid::doGetPlayPosition(int64_t& currentPosition) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  auto env = webrtc::jni::AttachCurrentThreadIfNeeded();
  jlong j_position =
      webrtc::jni::Java_SimpleMediaPlayerSource_getPlayPosition(env, j_media_player_source_);
  if (j_position != -1) {
    currentPosition = j_position;
  }
}

void MediaPlayerSourceAndroid::doGetStreamCount(int64_t& count) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  auto env = webrtc::jni::AttachCurrentThreadIfNeeded();
  auto j_count =
      webrtc::jni::Java_SimpleMediaPlayerSource_getStreamCount(env, j_media_player_source_);
  if (j_count != -1) {
    count = j_count;
  }
}

void MediaPlayerSourceAndroid::doSeek(int64_t newPos) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  auto env = webrtc::jni::AttachCurrentThreadIfNeeded();
  webrtc::jni::Java_SimpleMediaPlayerSource_seek(env, j_media_player_source_, newPos);
}

void MediaPlayerSourceAndroid::doGetStreamInfo(int64_t index, media::base::MediaStreamInfo* info) {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  auto env = webrtc::jni::AttachCurrentThreadIfNeeded();
  auto j_streamInfo = webrtc::jni::Java_SimpleMediaPlayerSource_getStreamInfo(
      env, j_media_player_source_, static_cast<int>(index));
  if (!j_streamInfo.is_null()) {
    JavaToNativeMediaStreamInfo(env, j_streamInfo, info);
  }
}

void MediaPlayerSourceAndroid::onTimer() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  if (paused_) {
    return;
  }

  if (sent_audio_frames_ == 0) {
    send_start_time_ = agora::commons::now_ms();
  }
  uint64_t now = agora::commons::now_ms();
  uint64_t send_duration = now - send_start_time_;
  uint64_t target_sent_frames = static_cast<uint64_t>(send_duration / kAudioFrameSendInterval) + 1;
  uint64_t remaining = target_sent_frames - sent_audio_frames_;

  while (remaining > 0) {
    if (paused_) {
      break;
    }
    deliverFrame();
    remaining--;
  }
}

void MediaPlayerSourceAndroid::deliverFrame() {
  ASSERT_THREAD_IS(player_worker_->getThreadId());

  auto env = webrtc::jni::AttachCurrentThreadIfNeeded();
  auto j_interval_data =
      webrtc::jni::Java_SimpleMediaPlayerSource_acquireIntervalData(env, j_media_player_source_);
  auto j_error = webrtc::jni::Java_SMPSIntervalData_isError(env, j_interval_data);

  // check error
  if (j_error) {
    commons::log(commons::LOG_ERROR, "%s: notifyPlayerState() loop", MODULE_NAME);
    doStop();
    updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
    return;
  }

  auto j_end_of_stream = webrtc::jni::Java_SMPSIntervalData_isEndOfStream(env, j_interval_data);
  auto j_buffer = webrtc::jni::Java_SMPSIntervalData_getByteBuffer(env, j_interval_data);

  // check end of stream
  if (j_end_of_stream) {
    notifyCompleted();
    if (loop_count_ != 0) {
      if (loop_count_ > 0) {
        --loop_count_;
      }

      commons::log(commons::LOG_INFO, "%s: notifyPlayerState() loop", MODULE_NAME);
      webrtc::jni::Java_SimpleMediaPlayerSource_seek(env, j_media_player_source_, 0);
    } else {
      if (doStop()) {
        updateStateAndNotify(media::base::PLAYER_STATE_PLAYBACK_COMPLETED);
      } else {
        updateStateAndNotify(media::base::PLAYER_STATE_FAILED);
      }
    }
    return;
  }

  // check buffering or decoding
  if (j_buffer.is_null()) {
    return;
  }

  uint64_t curr_ms = commons::now_ms();

  // notify position change every second (1000 ms)
  if (curr_ms - audio_last_report_ms_ >= 1000) {
    int64_t curr_pos_ms = 0;
    doGetPlayPosition(curr_pos_ms);
    notifyPositionChanged(static_cast<int>(curr_pos_ms / 1000));

    audio_last_report_ms_ = curr_ms;
  }

  if (audio_pcm_data_sender_) {
    jlong len = env->GetDirectBufferCapacity(j_buffer.obj());
    if (len < 0) {
      commons::log(commons::LOG_ERROR, "%s: GetDirectBufferCapacity failed! return %" PRId64,
                   MODULE_NAME, len);
      return;
    }

    void* payload = env->GetDirectBufferAddress(j_buffer.obj());
    if (!payload) {
      commons::log(commons::LOG_ERROR, "%s: GetDirectBufferAddress failed!", MODULE_NAME);
      return;
    }

    audio_pcm_data_sender_->sendAudioPcmData(payload, 0, audio_samples_per_channel_,
                                             audio_bytes_per_sample_, audio_number_of_channels_,
                                             audio_sample_rate_);
    sent_audio_frames_++;
  }
}

}  // namespace rtc
}  // namespace agora
