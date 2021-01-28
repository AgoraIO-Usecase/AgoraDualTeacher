//
//  Agora Media SDK
//
//  Created by Pengfei Han in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include <string>

#include "rtc_base/task_queue.h"

namespace webrtc {
class FileWrapper;
}

namespace agora {
namespace rtc {

struct DumpEvent {
  enum Type {
    Init,
    Stream,
    None,
  };

  Type type = Stream;

  std::string event_content;

  // For init type
  size_t bytes_per_sample{0};
  size_t number_of_channels{0};
  uint32_t sample_rate_hz{0};
  uint16_t codec{0};
};

class WriteToFileTask : public ::rtc::QueuedTask {
 public:
  WriteToFileTask(webrtc::FileWrapper* debug_file, int64_t* num_bytes_left_for_log);

  ~WriteToFileTask() override;
  DumpEvent* GetEvent();

 private:
  bool IsRoomForNextEvent(size_t event_byte_size) const;

  void UpdateBytesLeft(size_t event_byte_size);

  bool Run() override;

 private:
  webrtc::FileWrapper* debug_file_;
  int64_t* num_bytes_left_for_log_;
  DumpEvent event_;
};

}  // namespace rtc
}  // namespace agora
