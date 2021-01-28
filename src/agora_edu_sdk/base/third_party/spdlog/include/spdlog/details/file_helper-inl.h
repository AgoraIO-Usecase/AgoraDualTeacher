// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include <spdlog/details/file_helper.h>
#endif

#include <spdlog/common.h>
#include <spdlog/details/os.h>

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <tuple>

#if defined(_WIN32)
#include <Windows.h>
#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

namespace spdlog {
namespace details {

SPDLOG_INLINE file_helper::~file_helper() { close(); }

SPDLOG_INLINE void file_helper::open(const filename_t &fname, bool truncate) {
  close();
  filename_ = fname;

  for (int tries = 0; tries < open_tries_; ++tries) {
    // create containing folder if not exists already.
    os::create_dir(os::dir_name(fname));
#if defined(_WIN32)
    fd_ = CreateFileA(fname.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL,
                      truncate ? CREATE_ALWAYS : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (fd_ != INVALID_HANDLE_VALUE) {
#else
    auto *mode = truncate ? SPDLOG_FILENAME_T("wb") : SPDLOG_FILENAME_T("ab");

    if (!os::fopen_s(&fd_, fname, mode)) {

#endif
      if (!lock()) {
        close();
      }
      return;
    }

    details::os::sleep_for_millis(open_interval_);
  }

  fd_ = nullptr;
  SPDLOG_THROW(
      spdlog_ex("Failed opening file " + os::filename_to_str(filename_) + " for writing", errno));
}

SPDLOG_INLINE void file_helper::reopen(bool truncate) {
  if (filename_.empty()) {
    SPDLOG_THROW(spdlog_ex("Failed re opening file - was not opened before"));
  }
  this->open(filename_, truncate);
}

SPDLOG_INLINE void file_helper::flush() {
  if (fd_ == nullptr) {
    return;
  }
#if defined(_WIN32)
  FlushFileBuffers(fd_);
#else
  std::fflush(fd_);
#endif
}

SPDLOG_INLINE void file_helper::close() {
  if (fd_ == nullptr) {
    return;
  }

  unlock();

#if defined(_WIN32)
  CloseHandle(fd_);
#else
  std::fclose(fd_);
#endif
  fd_ = nullptr;
}

SPDLOG_INLINE void file_helper::write(const memory_buf_t &buf) {
  if (fd_ == nullptr) {
    return;
  }

  size_t msg_size = buf.size();
  auto data = buf.data();
#if defined(_WIN32)
  DWORD size = 0;
  BOOL ret = ::WriteFile(fd_, data, msg_size, &size, NULL);
  if (!ret || size != msg_size) {
#else
  if (std::fwrite(data, 1, msg_size, fd_) != msg_size) {
#endif
    SPDLOG_THROW(spdlog_ex("Failed writing to file " + os::filename_to_str(filename_), errno));
  }
}

SPDLOG_INLINE size_t file_helper::size() const {
  if (fd_ == nullptr) {
    SPDLOG_THROW_0(spdlog_ex("Cannot use size() on closed file " + os::filename_to_str(filename_)));
  }
#if defined(_WIN32)
  DWORD length_high = 0;
  DWORD length_low = GetFileSize(fd_, &length_high);
  return ((int64_t)length_high << 32) | length_low;
#else
  return os::filesize(fd_);
#endif
}

SPDLOG_INLINE const filename_t &file_helper::filename() const { return filename_; }

//
// return file path and its extension:
//
// "mylog.txt" => ("mylog", ".txt")
// "mylog" => ("mylog", "")
// "mylog." => ("mylog.", "")
// "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
//
// the starting dot in filenames is ignored (hidden files):
//
// ".mylog" => (".mylog". "")
// "my_folder/.mylog" => ("my_folder/.mylog", "")
// "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")
SPDLOG_INLINE std::tuple<filename_t, filename_t> file_helper::split_by_extension(
    const filename_t &fname) {
  auto ext_index = fname.rfind('.');

  // no valid extension found - return whole path and empty string as
  // extension
  if (ext_index == filename_t::npos || ext_index == 0 || ext_index == fname.size() - 1) {
    return std::make_tuple(fname, filename_t());
  }

  // treat cases like "/etc/rc.d/somelogfile or "/abc/.hiddenfile"
  auto folder_index = fname.rfind(details::os::folder_sep);
  if (folder_index != filename_t::npos && folder_index >= ext_index - 1) {
    return std::make_tuple(fname, filename_t());
  }

  // finally - return a valid base and extension tuple
  return std::make_tuple(fname.substr(0, ext_index), fname.substr(ext_index));
}

SPDLOG_INLINE bool file_helper::lock() {
  if (!fd_) return false;

#if defined(_WIN32)
  // Do nothing! because we open file without share write flag
  return true;
#else
  struct flock lock;
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;  // Lock entire file.
  return (fcntl(fileno(fd_), F_SETLK, &lock) != -1);
#endif
}

SPDLOG_INLINE void file_helper::unlock() {
  if (!fd_) return;

#if defined(_WIN32)
    // Do nothing! because we open file without share write flag
#else
  struct flock lock;
  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;  // Lock entire file.
  fcntl(fileno(fd_), F_SETLK, &lock);
#endif
}

}  // namespace details
}  // namespace spdlog
