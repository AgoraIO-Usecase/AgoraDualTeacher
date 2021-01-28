//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//

#include "utils/files/file.h"

#include "utils/build_config.h"
#include "utils/files/file_path.h"

#if defined(OS_POSIX) || defined(OS_FUCHSIA)
#include <errno.h>
#endif

namespace agora {
namespace utils {

File::Info::Info() : size(0), is_directory(false), is_symbolic_link(false) {}

File::Info::~Info() = default;

File::File() : error_details_(FILE_ERROR_FAILED), created_(false), async_(false) {}

#if !defined(OS_NACL)
File::File(const FilePath& path, uint32_t flags)
    : error_details_(FILE_OK), created_(false), async_(false) {
  Initialize(path, flags);
}
#endif

File::File(PlatformFile platform_file) : File(platform_file, false) {}

File::File(PlatformFile platform_file, bool async)
    : file_(platform_file), error_details_(FILE_OK), created_(false), async_(async) {}

File::File(Error error_details) : error_details_(error_details), created_(false), async_(false) {}

File::File(File&& other)
    : file_(other.TakePlatformFile()),
      error_details_(other.error_details()),
      created_(other.created()),
      async_(other.async_) {}

File::~File() {
  // Go through the AssertIOAllowed logic.
  Close();
}

File& File::operator=(File&& other) {
  Close();
  SetPlatformFile(other.TakePlatformFile());
  error_details_ = other.error_details();
  created_ = other.created();
  async_ = other.async_;
  return *this;
}

#if !defined(OS_NACL)
void File::Initialize(const FilePath& path, uint32_t flags) {
  if (path.ReferencesParent()) {
#if defined(OS_WIN)
    ::SetLastError(ERROR_ACCESS_DENIED);
#elif defined(OS_POSIX) || defined(OS_FUCHSIA)
    errno = EACCES;
#else
#error Unsupported platform
#endif
    error_details_ = FILE_ERROR_ACCESS_DENIED;
    return;
  }
  DoInitialize(path, flags);
}
#endif

std::string File::ErrorToString(Error error) {
  switch (error) {
    case FILE_OK:
      return "FILE_OK";
    case FILE_ERROR_FAILED:
      return "FILE_ERROR_FAILED";
    case FILE_ERROR_IN_USE:
      return "FILE_ERROR_IN_USE";
    case FILE_ERROR_EXISTS:
      return "FILE_ERROR_EXISTS";
    case FILE_ERROR_NOT_FOUND:
      return "FILE_ERROR_NOT_FOUND";
    case FILE_ERROR_ACCESS_DENIED:
      return "FILE_ERROR_ACCESS_DENIED";
    case FILE_ERROR_TOO_MANY_OPENED:
      return "FILE_ERROR_TOO_MANY_OPENED";
    case FILE_ERROR_NO_MEMORY:
      return "FILE_ERROR_NO_MEMORY";
    case FILE_ERROR_NO_SPACE:
      return "FILE_ERROR_NO_SPACE";
    case FILE_ERROR_NOT_A_DIRECTORY:
      return "FILE_ERROR_NOT_A_DIRECTORY";
    case FILE_ERROR_INVALID_OPERATION:
      return "FILE_ERROR_INVALID_OPERATION";
    case FILE_ERROR_SECURITY:
      return "FILE_ERROR_SECURITY";
    case FILE_ERROR_ABORT:
      return "FILE_ERROR_ABORT";
    case FILE_ERROR_NOT_A_FILE:
      return "FILE_ERROR_NOT_A_FILE";
    case FILE_ERROR_NOT_EMPTY:
      return "FILE_ERROR_NOT_EMPTY";
    case FILE_ERROR_INVALID_URL:
      return "FILE_ERROR_INVALID_URL";
    case FILE_ERROR_IO:
      return "FILE_ERROR_IO";
    case FILE_ERROR_MAX:
      break;
  }

  return "";
}

}  // namespace utils
}  // namespace agora
