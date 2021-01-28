//
//  Agora Media SDK
//
//  Created by Pengfei Han in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "audio_frame_dump_impl.h"

#include <fstream>  // std::ifstream

#include "base/AgoraBase.h"
#include "main/core/audio/audio_dump/audio_frame_dump_factory.h"
#include "rtc_base/event.h"
#include "utils/tools/util.h"
#include "write_to_file_task.h"

namespace agora {
namespace rtc {

AudioFrameDumpImpl::AudioFrameDumpImpl(std::unique_ptr<webrtc::FileWrapper> debug_file,
                                       int64_t max_log_size_bytes, ::rtc::TaskQueue* worker_queue)
    : debug_file_(std::move(debug_file)),
      num_bytes_left_for_log_(max_log_size_bytes),
      worker_queue_(worker_queue) {
  debug_file_->SetMaxFileSize(max_log_size_bytes);
}

AudioFrameDumpImpl::~AudioFrameDumpImpl() {
  // Block until all tasks have finished running.
  ::rtc::Event thread_sync_event(false /* manual_reset */, false);
  worker_queue_->PostTask([&thread_sync_event] { thread_sync_event.Set(); });
  // Wait until the event has been signaled with .Set(). By then all
  // pending tasks will have finished.
  thread_sync_event.Wait(::rtc::Event::kForever);

  debug_file_->CloseFile();
}

void AudioFrameDumpImpl::WriteInitMessage(const AudioStremConfig& audio_format,
                                          int64_t time_now_ms) {
  audio_format_ = audio_format;
  auto output_stream = audio_format_.output_stream();
  if (output_stream.codec == static_cast<int>(AUDIO_CODEC_PCMA) ||
      output_stream.codec == static_cast<int>(AUDIO_CODEC_PCMU)) {
    auto task = CreateWriteToFileTask();
    auto event = task->GetEvent();

    event->type = DumpEvent::Type::Init;
    event->bytes_per_sample = output_stream.bytes_per_sample;
    event->number_of_channels = output_stream.number_of_channels;
    event->sample_rate_hz = output_stream.sample_rate_hz;
    event->codec = output_stream.codec;

    worker_queue_->PostTask(std::unique_ptr<::rtc::QueuedTask>(std::move(task)));
  } else if (output_stream.codec == static_cast<int>(AUDIO_CODEC_OPUS)) {
  }
}

// Logs Event::Type STREAM message.
void AudioFrameDumpImpl::AddCaptureStreamInput(const webrtc::AudioFrame& frame) {}

void AudioFrameDumpImpl::AddCaptureStreamOutput(const webrtc::AudioFrame& frame) {}

void AudioFrameDumpImpl::AddCaptureStreamInput(const void* audio_data, size_t length) {
  auto task = CreateWriteToFileTask();
  if (task) {
    auto event = task->GetEvent();
    auto data = static_cast<const char*>(audio_data);
    event->event_content.assign(data, data + length);
    worker_queue_->PostTask(std::unique_ptr<::rtc::QueuedTask>(std::move(task)));
  } else {
    debug_file_->Write(audio_data, length);
  }
}

void AudioFrameDumpImpl::AddCaptureStreamOutput(const void* audio_data, size_t length) {}

void AudioFrameDumpImpl::WriteCaptureStreamMessage() {}

std::unique_ptr<WriteToFileTask> AudioFrameDumpImpl::CreateWriteToFileTask() {
  return commons::make_unique<WriteToFileTask>(debug_file_.get(), &num_bytes_left_for_log_);
}

std::unique_ptr<AudioFrameDump> AudioFrameDumpFactory::Create(std::string file_name,
                                                              int64_t max_log_size_bytes,
                                                              ::rtc::TaskQueue* worker_queue) {
  // check whether the file has already opened
  std::ifstream ifs(file_name);
  if (ifs.is_open()) {
    return nullptr;
  }
  std::unique_ptr<webrtc::FileWrapper> debug_file(webrtc::FileWrapper::Create());
  if (!debug_file->OpenFile(file_name.c_str(), false)) {
    return nullptr;
  }
  return commons::make_unique<AudioFrameDumpImpl>(std::move(debug_file), max_log_size_bytes,
                                                  worker_queue);
}

std::unique_ptr<AudioFrameDump> AudioFrameDumpFactory::AudioFrameDumpFactory::Create(
    FILE* handle, int64_t max_log_size_bytes, ::rtc::TaskQueue* worker_queue) {
  return nullptr;
}

}  // namespace rtc
}  // namespace agora
