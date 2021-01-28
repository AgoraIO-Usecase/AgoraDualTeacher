//
//  Agora Media SDK
//
//  Created by Pengfei Han in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include "main/core/audio/audio_dump/audio_frame_dump_factory.h"
#include "rtc_base/system/file_wrapper.h"

namespace agora {
namespace rtc {

class WriteToFileTask;

class AudioFrameDumpImpl : public AudioFrameDump {
 public:
  AudioFrameDumpImpl(std::unique_ptr<webrtc::FileWrapper> debug_file, int64_t max_log_size_bytes,
                     ::rtc::TaskQueue* worker_queue);
  ~AudioFrameDumpImpl() override;

  void WriteInitMessage(const AudioStremConfig& audio_format, int64_t time_now_ms) override;

  // Logs Event::Type STREAM message.
  void AddCaptureStreamInput(const webrtc::AudioFrame& frame) override;
  void AddCaptureStreamOutput(const webrtc::AudioFrame& frame) override;

  void AddCaptureStreamInput(const void* audio_data, size_t length) override;
  void AddCaptureStreamOutput(const void* audio_data, size_t length) override;

  void WriteCaptureStreamMessage() override;

 private:
  std::unique_ptr<WriteToFileTask> CreateWriteToFileTask();

 private:
  std::unique_ptr<webrtc::FileWrapper> debug_file_;
  int64_t num_bytes_left_for_log_ = 0;
  ::rtc::TaskQueue* worker_queue_;
  AudioStremConfig audio_format_;
};

}  // namespace rtc
}  // namespace agora
