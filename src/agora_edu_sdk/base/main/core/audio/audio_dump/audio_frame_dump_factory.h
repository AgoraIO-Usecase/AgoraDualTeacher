//
//  Agora Media SDK
//
//  Created by Pengfei Han in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

#include "audio_frame_dump.h"
namespace rtc {
class TaskQueue;
}  // namespace rtc

namespace agora {
namespace rtc {

class AudioFrameDumpFactory {
 public:
  // The |worker_queue| may not be null and must outlive the created
  // AudioFrameDump instance. |max_log_size_bytes == -1| means the log size
  // will be unlimited. |handle| may not be null. The AecDump takes
  // responsibility for |handle| and closes it in the destructor. A
  // non-null return value indicates that the file has been
  // sucessfully opened.
  static std::unique_ptr<AudioFrameDump> Create(std::string file_name, int64_t max_log_size_bytes,
                                                ::rtc::TaskQueue* worker_queue);
  static std::unique_ptr<AudioFrameDump> Create(FILE* handle, int64_t max_log_size_bytes,
                                                ::rtc::TaskQueue* worker_queue);
};

}  // namespace rtc
}  // namespace agora
