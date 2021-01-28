//
//  Agora Media SDK
//
//  Created by Pengfei Han in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "write_to_file_task.h"

#include "base/AgoraBase.h"
#include "facilities/tools/wav_utils.h"
#include "rtc_base/system/file_wrapper.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

static const char* MODULE_NAME = "WTFT";

WriteToFileTask::WriteToFileTask(webrtc::FileWrapper* debug_file, int64_t* num_bytes_left_for_log)
    : debug_file_(debug_file), num_bytes_left_for_log_(num_bytes_left_for_log) {}

WriteToFileTask::~WriteToFileTask() {}

DumpEvent* WriteToFileTask::GetEvent() { return &event_; }

bool WriteToFileTask::IsRoomForNextEvent(size_t event_byte_size) const {
  int64_t next_message_size = event_byte_size + sizeof(int32_t);
  return (*num_bytes_left_for_log_ < 0) || (*num_bytes_left_for_log_ >= next_message_size);
}

void WriteToFileTask::UpdateBytesLeft(size_t event_byte_size) {
  if (!IsRoomForNextEvent(event_byte_size)) {
    return;
  }
  if (*num_bytes_left_for_log_ >= 0) {
    *num_bytes_left_for_log_ -= (sizeof(int32_t) + event_byte_size);
  }
}

bool WriteToFileTask::Run() {
  if (!debug_file_->is_open()) {
    return true;
  }

  auto event_byte_size = event_.event_content.size();
  if (!IsRoomForNextEvent(event_byte_size)) {
    debug_file_->CloseFile();
    return true;
  }

  UpdateBytesLeft(event_byte_size);

  if (event_.type == DumpEvent::Init &&
      (event_.codec == AUDIO_CODEC_PCMA || event_.codec == AUDIO_CODEC_PCMU)) {
    auto wav_header = createWAVHeader(event_.number_of_channels, event_.sample_rate_hz);
    unsigned char wav_header_data[44] = {0};
    makeWAVHeader(wav_header_data, *wav_header);

    if (!debug_file_->Write(wav_header_data, 44)) {
      log(commons::LOG_WARN, "%s: Write to file WAV header failed", MODULE_NAME);
    }
  } else if (!debug_file_->Write(event_.event_content.c_str(), event_byte_size)) {
    log(commons::LOG_WARN, "%s: Write to file failed", MODULE_NAME);
  }

  return true;
}

}  // namespace rtc
}  // namespace agora
