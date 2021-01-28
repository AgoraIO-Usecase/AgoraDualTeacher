//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraBase.h"
#include "AgoraRefPtr.h"
#include "AgoraRteBase.h"

namespace agora {
namespace rte {

/**
 * The IAudioPcmDataSender class.
 *
 * In scenarios involving custom audio source, you can use the IAudioPcmDataSender class
 * to push PCM audio data directly to the audio track. If the audio track is disabled,
 * the pushed audio data will be automatically discarded.
 */
class IAudioPcmDataSender : public RefCountInterface {
 public:
  /**
   * Sends the PCM audio data to the local audio track.
   *
   * @param audio_data The PCM audio data to be sent.
   * @param capture_timestamp The timestamp for capturing the audio data.
   * @param samples_per_channel The number of audio samples in 10 ms for each channel.
   * @param bytes_per_sample The number of bytes for each sample.
   * @param number_of_channels The number of channels.
   * @param sample_rate The sample rate (Hz). The minimum value is 8000.
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual AgoraError SendAudioPcmData(
      const void* audio_data, uint32_t capture_timestamp,
      const size_t samples_per_channel,  // for 10ms Data, number_of_samples * 100 = sample_rate
      const size_t bytes_per_sample,     // 2 * number_of_channels
      const size_t number_of_channels,
      const uint32_t sample_rate) = 0;  // sample_rate > 8000

 protected:
  ~IAudioPcmDataSender() {}
};

/**
 * The IAudioFrameObserver class.
 */
class IAudioFrameObserver {
 public:
  /**
   * The audio frame type.
   */
  enum AudioFrameType {
    /**
     * 0: The frame type is PCM.
     */
    FRAME_TYPE_PCM16 = 0,  // PCM 16bit little endian
  };

  /** 
   * The definition of AudioFrame.
   */
  struct AudioFrame {
    AudioFrameType type;
    /** 
     * The number of samples per channel in this frame.
     */
    int samplesPerChannel;
    /** 
     * The number of bytes per sample: Two for PCM 16.
     */
    int bytesPerSample;  
    /** 
     * The number of channels (data is interleaved, if stereo).
     */
    int channels; 
    /** 
     * The Sample rate.
     */
    int samplesPerSec;
    /** 
     * The pointer to the data buffer.
     */
    void* buffer;  
    /** 
     * The timestamp to render the audio data. Use this member to synchronize the audio renderer while 
     * rendering the audio streams.
     *
     * @note
     * This timestamp is for audio stream rendering. Set it as 0.
    */
    int64_t renderTimeMs;
    int avsync_type;

    AudioFrame()
      : type(AudioFrameType::FRAME_TYPE_PCM16),
        samplesPerChannel(0),
        bytesPerSample(0),
        channels(0),
        samplesPerSec(0),
        buffer(nullptr),
        renderTimeMs(0),
        avsync_type(0) {}
  };

  struct AudioFrameObserverConfig {
    bool enable;
    int sampleRate;
    int channelNumber;
    int samplesPerCall;

    AudioFrameObserverConfig()
      : enable(true),
        sampleRate(16000),
        channelNumber(1),
        samplesPerCall(160) {}
  };

 public:
  virtual ~IAudioFrameObserver() = default;

  /** 
   * Occurs when the recorded audio frame is received.
   * @param audioFrame The reference to the audio frame: AudioFrame.
   * @return
   * - true: The recorded audio frame is valid and is encoded and sent.
   * - false: The recorded audio frame is invalid and is not encoded or sent.
   */
  virtual bool OnRecordAudioFrame(AudioFrame& audio_frame) = 0;
  /** 
   * Occurs when the playback audio frame is received.
   * @param audioFrame The reference to the audio frame: AudioFrame.
   * @return
   * - true: The playback audio frame is valid and is encoded and sent.
   * - false: The playback audio frame is invalid and is not encoded or sent.
   */
  virtual bool OnPlaybackAudioFrame(AudioFrame& audio_frame) = 0;
  /** 
   * Occurs when the mixed audio data is received.
   * @param audioFrame The reference to the audio frame: AudioFrame.
   * @return
   * - true: The mixed audio data is valid and is encoded and sent.
   * - false: The mixed audio data is invalid and is not encoded or sent.
   */
  virtual bool OnMixedAudioFrame(AudioFrame& audio_frame) = 0;
  /** 
   * Occurs when the playback audio frame before mixing is received.
   * @param uid ID of the remote user.
   * @param audioFrame The reference to the audio frame: AudioFrame.
   * @return
   * - true: The playback audio frame before mixing is valid and is encoded and sent.
   * - false: The playback audio frame before mixing is invalid and is not encoded or sent.
   */
  virtual bool OnPlaybackAudioFrameBeforeMixing(StreamId stream_id, AudioFrame& audio_frame) = 0;

  virtual AudioFrameObserverConfig WantsRecordingAudioFrame() { 
    AudioFrameObserverConfig config;
    return config; 
  }

  virtual AudioFrameObserverConfig WantsPlaybackAudioFrame() { 
    AudioFrameObserverConfig config;
    return config; 
  }

  virtual AudioFrameObserverConfig WantsMixedAudioFrame() { 
    AudioFrameObserverConfig config;
    return config; 
  }

  virtual AudioFrameObserverConfig WantsPlaybackAudioFrameBeforeMixing() { 
    AudioFrameObserverConfig config;
    return config; 
  }
};

/**
 * The IVideoFrameSender class.
 *
 * In scenarios involving custom video source, you can use this class to push the video
 * data directly to the video track. If the video track is disabled, the pushed data will
 * be automatically discarded.
 */
class IVideoFrameSender : public RefCountInterface {
 public:
  /**
   * Sends the video frame to the video track.
   *
   * @param videoFrame The reference to the video frame to be sent: \ref
   * media::ExternalVideoFrame "ExternalVideoFrame".
   *
   * @return
   * - 0: Success.
   * - < 0: Failure.
   */
  virtual AgoraError SendVideoFrame(const media::base::ExternalVideoFrame& videoFrame) = 0;

  virtual AgoraError StartPreview(View view);
  virtual AgoraError StopPreview();

 protected:
  ~IVideoFrameSender() {}
};

/**
 * The IVideoFrameObserver class.
 */
class IVideoFrameObserver {
 public:
  virtual ~IVideoFrameObserver() = default;

  virtual void OnFrame(StreamId stream_id, media::base::VideoFrame& video_frame) = 0;
};

}  // namespace rte
}  // namespace agora
