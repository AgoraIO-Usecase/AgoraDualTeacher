//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "utils/build_config.h"
#include "utils/files/file.h"

namespace agora {
namespace utils {

// Make sure our Whence mappings match the system headers.
static_assert(File::FROM_BEGIN == SEEK_SET && File::FROM_CURRENT == SEEK_CUR &&
                  File::FROM_END == SEEK_END,
              "whence mapping must match the system headers");

namespace {

#if defined(OS_BSD) || defined(OS_MACOSX) || defined(OS_NACL) || defined(OS_FUCHSIA) || \
    (defined(OS_ANDROID) && __ANDROID_API__ < 21)
int CallFstat(int fd, stat_wrapper_t* sb) { return fstat(fd, sb); }
#else
int CallFstat(int fd, stat_wrapper_t* sb) { return fstat64(fd, sb); }
#endif

// NaCl doesn't provide the following system calls, so either simulate them or
// wrap them in order to minimize the number of #ifdef's in this file.
#if !defined(OS_NACL) && !defined(OS_AIX)
bool IsOpenAppend(PlatformFile file) { return (fcntl(file, F_GETFL) & O_APPEND) != 0; }

int CallFtruncate(PlatformFile file, int64_t length) { return ftruncate(file, length); }

#if defined(OS_ANDROID)
static int __futimens(int fd, const struct timespec times[2]) {
  return utimensat(fd, NULL, times, 0);
}
#endif

int CallFutimes(PlatformFile file, const struct timeval times[2]) {
#if defined(__USE_XOPEN2K8) || defined(OS_ANDROID) || defined(OS_LINUX)
  // futimens should be available, but futimes might not be
  // http://pubs.opengroup.org/onlinepubs/9699919799/

  timespec ts_times[2];
  ts_times[0].tv_sec = times[0].tv_sec;
  ts_times[0].tv_nsec = times[0].tv_usec * 1000;
  ts_times[1].tv_sec = times[1].tv_sec;
  ts_times[1].tv_nsec = times[1].tv_usec * 1000;

#if defined(OS_ANDROID)
  return __futimens(file, ts_times);
#else
  return futimens(file, ts_times);
#endif
#else
  return futimes(file, times);
#endif
}

#if !defined(OS_FUCHSIA)
File::Error CallFcntlFlock(PlatformFile file, bool do_lock) {
  struct flock lock;
  lock.l_type = do_lock ? F_WRLCK : F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;  // Lock entire file.
  if (fcntl(file, F_SETLK, &lock) == -1) return File::GetLastFileError();
  return File::FILE_OK;
}
#endif

#else   // defined(OS_NACL) && !defined(OS_AIX)

bool IsOpenAppend(PlatformFile file) {
  // NaCl doesn't implement fcntl. Since NaCl's write conforms to the POSIX
  // standard and always appends if the file is opened with O_APPEND, just
  // return false here.
  return false;
}

int CallFtruncate(PlatformFile file, int64_t length) {
  NOTIMPLEMENTED();  // NaCl doesn't implement ftruncate.
  return 0;
}

int CallFutimes(PlatformFile file, const struct timeval times[2]) {
  NOTIMPLEMENTED();  // NaCl doesn't implement futimes.
  return 0;
}

File::Error CallFcntlFlock(PlatformFile file, bool do_lock) {
  NOTIMPLEMENTED();  // NaCl doesn't implement flock struct.
  return File::FILE_ERROR_INVALID_OPERATION;
}
#endif  // defined(OS_NACL)

}  // namespace

void File::Info::FromStat(const stat_wrapper_t& stat_info) {
  is_directory = S_ISDIR(stat_info.st_mode);
  is_symbolic_link = S_ISLNK(stat_info.st_mode);
  size = stat_info.st_size;

#if defined(OS_LINUX)
  time_t last_modified_sec = stat_info.st_mtim.tv_sec;
  int64_t last_modified_nsec = stat_info.st_mtim.tv_nsec;
  time_t last_accessed_sec = stat_info.st_atim.tv_sec;
  int64_t last_accessed_nsec = stat_info.st_atim.tv_nsec;
  time_t creation_time_sec = stat_info.st_ctim.tv_sec;
  int64_t creation_time_nsec = stat_info.st_ctim.tv_nsec;
#elif defined(OS_ANDROID)
  time_t last_modified_sec = stat_info.st_mtime;
  int64_t last_modified_nsec = stat_info.st_mtime_nsec;
  time_t last_accessed_sec = stat_info.st_atime;
  int64_t last_accessed_nsec = stat_info.st_atime_nsec;
  time_t creation_time_sec = stat_info.st_ctime;
  int64_t creation_time_nsec = stat_info.st_ctime_nsec;
#elif defined(OS_MACOSX) || defined(OS_IOS) || defined(OS_BSD)
  time_t last_modified_sec = stat_info.st_mtimespec.tv_sec;
  int64_t last_modified_nsec = stat_info.st_mtimespec.tv_nsec;
  time_t last_accessed_sec = stat_info.st_atimespec.tv_sec;
  int64_t last_accessed_nsec = stat_info.st_atimespec.tv_nsec;
  time_t creation_time_sec = stat_info.st_ctimespec.tv_sec;
  int64_t creation_time_nsec = stat_info.st_ctimespec.tv_nsec;
#else
  time_t last_modified_sec = stat_info.st_mtime;
  int64_t last_modified_nsec = 0;
  time_t last_accessed_sec = stat_info.st_atime;
  int64_t last_accessed_nsec = 0;
  time_t creation_time_sec = stat_info.st_ctime;
  int64_t creation_time_nsec = 0;
#endif

  last_modified = last_modified_sec * 1000000000 + last_modified_nsec;

  last_accessed = last_accessed_sec * 1000000000 + last_accessed_nsec;

  creation_time = creation_time_sec * 1000000000 + creation_time_nsec;
}

bool File::IsValid() const { return file_ > 0; }

PlatformFile File::GetPlatformFile() const { return file_; }

PlatformFile File::TakePlatformFile() {
  auto ret = file_;
  file_ = -1;
  return ret;
}

void File::Close() {
  if (!IsValid()) return;
  close(file_);
}

int64_t File::Seek(Whence whence, int64_t offset) {
#if defined(OS_ANDROID)
  static_assert(sizeof(int64_t) == sizeof(off64_t), "off64_t must be 64 bits");
  return lseek64(file_, static_cast<off64_t>(offset), static_cast<int>(whence));
#else
  static_assert(sizeof(int64_t) == sizeof(off_t), "off_t must be 64 bits");
  return lseek(file_, static_cast<off_t>(offset), static_cast<int>(whence));
#endif
}

int File::Read(int64_t offset, char* data, int size) {
  if (size < 0) return -1;

  int bytes_read = 0;
  int rv;
  do {
    rv = pread(file_, data + bytes_read, size - bytes_read, offset + bytes_read);
    if (rv <= 0) break;

    bytes_read += rv;
  } while (bytes_read < size);

  return bytes_read ? bytes_read : rv;
}

int File::ReadAtCurrentPos(char* data, int size) {
  if (size < 0) return -1;

  int bytes_read = 0;
  int rv;
  do {
    rv = read(file_, data + bytes_read, size - bytes_read);
    if (rv <= 0) break;

    bytes_read += rv;
  } while (bytes_read < size);

  return bytes_read ? bytes_read : rv;
}

int File::ReadNoBestEffort(int64_t offset, char* data, int size) {
  return pread(file_, data, size, offset);
}

int File::ReadAtCurrentPosNoBestEffort(char* data, int size) {
  if (size < 0) return -1;

  return read(file_, data, size);
}

int File::Write(int64_t offset, const char* data, int size) {
  if (IsOpenAppend(file_)) return WriteAtCurrentPos(data, size);

  if (size < 0) return -1;

  int bytes_written = 0;
  int rv;
  do {
#if defined(OS_ANDROID)
    // In case __USE_FILE_OFFSET64 is not used, we need to call pwrite64()
    // instead of pwrite().
    static_assert(sizeof(int64_t) == sizeof(off64_t), "off64_t must be 64 bits");
    rv = pwrite64(file_, data + bytes_written, size - bytes_written, offset + bytes_written);
#else
    rv = pwrite(file_, data + bytes_written, size - bytes_written, offset + bytes_written);
#endif
    if (rv <= 0) break;

    bytes_written += rv;
  } while (bytes_written < size);

  return bytes_written ? bytes_written : rv;
}

int File::WriteAtCurrentPos(const char* data, int size) {
  if (size < 0) return -1;

  int bytes_written = 0;
  int rv;
  do {
    rv = write(file_, data + bytes_written, size - bytes_written);
    if (rv <= 0) break;

    bytes_written += rv;
  } while (bytes_written < size);

  return bytes_written ? bytes_written : rv;
}

int File::WriteAtCurrentPosNoBestEffort(const char* data, int size) {
  if (size < 0) return -1;

  return write(file_, data, size);
}

int64_t File::GetLength() {
  stat_wrapper_t file_info;
  if (CallFstat(file_, &file_info)) return -1;

  return file_info.st_size;
}

bool File::SetLength(int64_t length) { return !CallFtruncate(file_, length); }

bool File::SetTimes(int64_t last_access_time, int64_t last_modified_time) {
  timeval times[2];
  times[0].tv_sec = last_access_time / 1000000000;
  times[0].tv_usec = last_access_time % 1000000000;
  times[1].tv_sec = last_modified_time / 1000000000;
  times[1].tv_usec = last_modified_time % 1000000000;

  return !CallFutimes(file_, times);
}

bool File::GetInfo(Info* info) {
  stat_wrapper_t file_info;
  if (CallFstat(file_, &file_info)) return false;

  info->FromStat(file_info);
  return true;
}

#if !defined(OS_FUCHSIA)
File::Error File::Lock() { return CallFcntlFlock(file_, true); }

File::Error File::Unlock() { return CallFcntlFlock(file_, false); }
#endif

File File::Duplicate() const {
  if (!IsValid()) return File();

  PlatformFile other_fd = dup(GetPlatformFile());
  if (other_fd == -1) return File(File::GetLastFileError());

  return File(other_fd, async());
}

// Static.
File::Error File::OSErrorToFileError(int saved_errno) {
  switch (saved_errno) {
    case EACCES:
    case EISDIR:
    case EROFS:
    case EPERM:
      return FILE_ERROR_ACCESS_DENIED;
    case EBUSY:
#if !defined(OS_NACL)  // ETXTBSY not defined by NaCl.
    case ETXTBSY:
#endif
      return FILE_ERROR_IN_USE;
    case EEXIST:
      return FILE_ERROR_EXISTS;
    case EIO:
      return FILE_ERROR_IO;
    case ENOENT:
      return FILE_ERROR_NOT_FOUND;
    case ENFILE:  // fallthrough
    case EMFILE:
      return FILE_ERROR_TOO_MANY_OPENED;
    case ENOMEM:
      return FILE_ERROR_NO_MEMORY;
    case ENOSPC:
      return FILE_ERROR_NO_SPACE;
    case ENOTDIR:
      return FILE_ERROR_NOT_A_DIRECTORY;
    default:
      // This function should only be called for errors.
      return FILE_ERROR_FAILED;
  }
}

// NaCl doesn't implement system calls to open files directly.
#if !defined(OS_NACL)
// TODO(erikkay): does it make sense to support FLAG_EXCLUSIVE_* here?
void File::DoInitialize(const FilePath& path, uint32_t flags) {
  int open_flags = 0;
  if (flags & FLAG_CREATE) open_flags = O_CREAT | O_EXCL;

  created_ = false;

  if (flags & FLAG_CREATE_ALWAYS) {
    open_flags = O_CREAT | O_TRUNC;
  }

  if (flags & FLAG_OPEN_TRUNCATED) {
    open_flags = O_TRUNC;
  }

  if (!open_flags && !(flags & FLAG_OPEN) && !(flags & FLAG_OPEN_ALWAYS)) {
    errno = EOPNOTSUPP;
    error_details_ = FILE_ERROR_FAILED;
    return;
  }

  if (flags & FLAG_WRITE && flags & FLAG_READ) {
    open_flags |= O_RDWR;
  } else if (flags & FLAG_WRITE) {
    open_flags |= O_WRONLY;
  } else if (!(flags & FLAG_READ) && !(flags & FLAG_WRITE_ATTRIBUTES) && !(flags & FLAG_APPEND) &&
             !(flags & FLAG_OPEN_ALWAYS)) {
  }

  if (flags & FLAG_TERMINAL_DEVICE) open_flags |= O_NOCTTY | O_NDELAY;

  if (flags & FLAG_APPEND && flags & FLAG_READ)
    open_flags |= O_APPEND | O_RDWR;
  else if (flags & FLAG_APPEND)
    open_flags |= O_APPEND | O_WRONLY;

  static_assert(O_RDONLY == 0, "O_RDONLY must equal zero");

  int mode = S_IRUSR | S_IWUSR;
#if defined(OS_CHROMEOS)
  mode |= S_IRGRP | S_IROTH;
#endif

  int descriptor = open(path.value().c_str(), open_flags, mode);

  if (flags & FLAG_OPEN_ALWAYS) {
    if (descriptor < 0) {
      open_flags |= O_CREAT;
      if (flags & FLAG_EXCLUSIVE_READ || flags & FLAG_EXCLUSIVE_WRITE)
        open_flags |= O_EXCL;  // together with O_CREAT implies O_NOFOLLOW

      descriptor = open(path.value().c_str(), open_flags, mode);
      if (descriptor >= 0) created_ = true;
    }
  }

  if (descriptor < 0) {
    error_details_ = File::GetLastFileError();
    return;
  }

  if (flags & (FLAG_CREATE_ALWAYS | FLAG_CREATE)) created_ = true;

  if (flags & FLAG_DELETE_ON_CLOSE) unlink(path.value().c_str());

  async_ = ((flags & FLAG_ASYNC) == FLAG_ASYNC);
  error_details_ = FILE_OK;
  close(file_);
  file_ = descriptor;
}
#endif  // !defined(OS_NACL)

bool File::Flush() {
#if defined(OS_NACL)
  return true;
#elif defined(OS_LINUX) || defined(OS_ANDROID)
  return !fdatasync(file_);
#else
  return !fsync(file_);
#endif
}

void File::SetPlatformFile(PlatformFile file) {
  if (file_ == file) return;

  close(file_);
  file_ = file;
}

// static
File::Error File::GetLastFileError() { return File::OSErrorToFileError(errno); }

}  // namespace utils
}  // namespace agora
