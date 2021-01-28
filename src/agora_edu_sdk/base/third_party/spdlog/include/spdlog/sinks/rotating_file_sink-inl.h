// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/sinks/rotating_file_sink.h>
#endif

#include <spdlog/common.h>

#include <spdlog/details/file_helper.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/fmt/fmt.h>

#include <cerrno>
#include <chrono>
#include <ctime>
#include <mutex>
#include <string>
#include <tuple>
#include <vector>

#include "utils/crypto/btea.h"

namespace spdlog {
namespace sinks {
static const char base62_str[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

static int to_base62(uint32_t *input, int len, char *output) {
  unsigned int offset;
  unsigned int tmp;
  int i;
  int k;
  int l;

  k = 0;
  l = 0;
  for (i = 0; i < len; ++i) {
    ((unsigned char *)&tmp)[0] = ((unsigned char *)&input[i])[3];
    ((unsigned char *)&tmp)[1] = ((unsigned char *)&input[i])[2];
    ((unsigned char *)&tmp)[2] = ((unsigned char *)&input[i])[1];
    ((unsigned char *)&tmp)[3] = ((unsigned char *)&input[i])[0];
    int j = 5;
    while (tmp) {
      offset = tmp % 62;
      tmp /= 62;
      output[k + (j--)] = base62_str[offset];
      l++;
    }
    while (j >= 0) {
      output[k + (j--)] = base62_str[0];
      l++;
    }

    k += 6;
  }
  return l;
}

static void encrypt_buf(const memory_buf_t &source, memory_buf_t &dest) {
  if (source.size() == 0) {
    dest.clear();
    return;
  }
  std::vector<char> encrypted_buf;
  // padding
  size_t padding_size = (sizeof(uint32_t) - (source.size() % sizeof(uint32_t))) % sizeof(uint32_t);
  encrypted_buf.resize(source.size() + padding_size);
  memset(reinterpret_cast<void *>(&encrypted_buf[0]), 0, encrypted_buf.size());
  memcpy(reinterpret_cast<void *>(&encrypted_buf[0]), reinterpret_cast<const void *>(source.data()),
         source.size());

  // encrypt
  uint32_t key[4] = {0x8e6ce538, 0x247f1a7c, 0xd4534635, 0x08698fe8};
  int val_count = encrypted_buf.size() / sizeof(uint32_t);
  btea(reinterpret_cast<uint32_t *>(&encrypted_buf[0]), val_count, key);

  // format to text
  dest.reserve(encrypted_buf.size() * 2);
  memset(reinterpret_cast<void *>(dest.data()), 0, encrypted_buf.size() * 2);
  int base62_size =
      to_base62(reinterpret_cast<uint32_t *>(&encrypted_buf[0]), val_count, dest.data());
  dest.resize(base62_size + 1);
  dest[base62_size] = '\n';
}

template <typename Mutex>
SPDLOG_INLINE rotating_file_sink<Mutex>::rotating_file_sink(filename_t base_filename,
                                                            std::size_t max_size,
                                                            std::size_t max_files,
                                                            bool rotate_on_open, bool encrypt)
    : base_filename_(std::move(base_filename)),
      max_size_(max_size),
      max_files_(max_files),
      encrypt_(encrypt) {
  file_helper_.open(calc_filename(base_filename_, 0));
  current_size_ = file_helper_.size();  // expensive. called only once
  if (rotate_on_open && current_size_ > 0) {
    rotate_();
  }
}

// calc filename according to index and file extension if exists.
// e.g. calc_filename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
template <typename Mutex>
SPDLOG_INLINE filename_t rotating_file_sink<Mutex>::calc_filename(const filename_t &filename,
                                                                  std::size_t index) {
  if (index == 0u) {
    return filename;
  }

  filename_t basename, ext;
  std::tie(basename, ext) = details::file_helper::split_by_extension(filename);
  return fmt::format(SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
}

template <typename Mutex>
SPDLOG_INLINE const filename_t &rotating_file_sink<Mutex>::filename() const {
  return file_helper_.filename();
}

template <typename Mutex>
SPDLOG_INLINE void rotating_file_sink<Mutex>::sink_it_(const details::log_msg &msg) {
  memory_buf_t formatted;
  memory_buf_t encrypted;
  base_sink<Mutex>::formatter_->format(msg, formatted);
  if (encrypt_) {
    encrypt_buf(formatted, encrypted);
    formatted = std::move(encrypted);
  }
  current_size_ += formatted.size();
  if (current_size_ > max_size_) {
    rotate_();
    current_size_ = formatted.size();
  }
  file_helper_.write(formatted);
}

template <typename Mutex>
SPDLOG_INLINE void rotating_file_sink<Mutex>::flush_() {
  file_helper_.flush();
}

// Rotate files:
// log.txt -> log.1.txt
// log.1.txt -> log.2.txt
// log.2.txt -> log.3.txt
// log.3.txt -> delete
template <typename Mutex>
SPDLOG_INLINE void rotating_file_sink<Mutex>::rotate_() {
  using details::os::filename_to_str;
  using details::os::path_exists;
  file_helper_.close();
  for (auto i = max_files_; i > 0; --i) {
    filename_t src = calc_filename(base_filename_, i - 1);
    if (!path_exists(src)) {
      continue;
    }
    filename_t target = calc_filename(base_filename_, i);

    if (!rename_file(src, target)) {
      // if failed try again after a small delay.
      // this is a workaround to a windows issue, where very high rotation
      // rates can cause the rename to fail with permission denied (because of antivirus?).
      details::os::sleep_for_millis(100);
      if (!rename_file(src, target)) {
        file_helper_.reopen(
            true);  // truncate the log file anyway to prevent it to grow beyond its limit!
        current_size_ = 0;
        SPDLOG_THROW(spdlog_ex("rotating_file_sink: failed renaming " + filename_to_str(src) +
                                   " to " + filename_to_str(target),
                               errno));
      }
    }
  }
  file_helper_.reopen(true);
}

// delete the target if exists, and rename the src file  to target
// return true on success, false otherwise.
template <typename Mutex>
SPDLOG_INLINE bool rotating_file_sink<Mutex>::rename_file(const filename_t &src_filename,
                                                          const filename_t &target_filename) {
  // try to delete the target file in case it already exists.
  (void)details::os::remove(target_filename);
  return details::os::rename(src_filename, target_filename) == 0;
}

}  // namespace sinks
}  // namespace spdlog
