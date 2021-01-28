//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-08.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//  This file is based on gbase implementation
//

#include <io.h>
#include <stdint.h>
#include <windows.h>

#include "utils/files/file.h"

namespace agora {
namespace utils {

// Make sure our Whence mappings match the system headers.
static_assert(File::FROM_BEGIN == FILE_BEGIN && File::FROM_CURRENT == FILE_CURRENT &&
                  File::FROM_END == FILE_END,
              "whence mapping must match the system headers");

bool File::IsValid() const { return file_ != INVALID_HANDLE_VALUE && file_ != NULL; }

PlatformFile File::GetPlatformFile() const { return file_; }

PlatformFile File::TakePlatformFile() {
  auto val = file_;
  file_ = NULL;
  return val;
}

void File::Close() {
  if (!IsValid()) return;

  CloseHandle(file_);
  file_ = INVALID_HANDLE_VALUE;
}

int64_t File::Seek(Whence whence, int64_t offset) {
  LARGE_INTEGER distance, res;
  distance.QuadPart = offset;
  DWORD move_method = static_cast<DWORD>(whence);
  if (!SetFilePointerEx(file_, distance, &res, move_method)) return -1;
  return res.QuadPart;
}

int File::Read(int64_t offset, char* data, int size) {
  if (size < 0) return -1;

  LARGE_INTEGER offset_li;
  offset_li.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset_li.LowPart;
  overlapped.OffsetHigh = offset_li.HighPart;

  DWORD bytes_read;
  if (::ReadFile(file_, data, size, &bytes_read, &overlapped)) return bytes_read;
  if (ERROR_HANDLE_EOF == GetLastError()) return 0;

  return -1;
}

int File::ReadAtCurrentPos(char* data, int size) {
  if (size < 0) return -1;

  DWORD bytes_read;
  if (::ReadFile(file_, data, size, &bytes_read, NULL)) return bytes_read;
  if (ERROR_HANDLE_EOF == GetLastError()) return 0;

  return -1;
}

int File::ReadNoBestEffort(int64_t offset, char* data, int size) {
  // TODO(dbeam): trace this separately?
  return Read(offset, data, size);
}

int File::ReadAtCurrentPosNoBestEffort(char* data, int size) {
  // TODO(dbeam): trace this separately?
  return ReadAtCurrentPos(data, size);
}

int File::Write(int64_t offset, const char* data, int size) {
  LARGE_INTEGER offset_li;
  offset_li.QuadPart = offset;

  OVERLAPPED overlapped = {0};
  overlapped.Offset = offset_li.LowPart;
  overlapped.OffsetHigh = offset_li.HighPart;

  DWORD bytes_written;
  if (::WriteFile(file_, data, size, &bytes_written, &overlapped)) return bytes_written;

  return -1;
}

int File::WriteAtCurrentPos(const char* data, int size) {
  if (size < 0) return -1;

  DWORD bytes_written;
  if (::WriteFile(file_, data, size, &bytes_written, NULL)) return bytes_written;

  return -1;
}

int File::WriteAtCurrentPosNoBestEffort(const char* data, int size) {
  return WriteAtCurrentPos(data, size);
}

int64_t File::GetLength() {
  LARGE_INTEGER size;
  if (!::GetFileSizeEx(file_, &size)) return -1;

  return static_cast<int64_t>(size.QuadPart);
}

bool File::SetLength(int64_t length) {
  // Get the current file pointer.
  LARGE_INTEGER file_pointer;
  LARGE_INTEGER zero;
  zero.QuadPart = 0;
  if (!::SetFilePointerEx(file_, zero, &file_pointer, FILE_CURRENT)) return false;

  LARGE_INTEGER length_li;
  length_li.QuadPart = length;
  // If length > file size, SetFilePointerEx() should extend the file
  // with zeroes on all Windows standard file systems (NTFS, FATxx).
  if (!::SetFilePointerEx(file_, length_li, NULL, FILE_BEGIN)) return false;

  // Set the new file length and move the file pointer to its old position.
  // This is consistent with ftruncate()'s behavior, even when the file
  // pointer points to a location beyond the end of the file.
  // TODO(rvargas): Emulating ftruncate details seem suspicious and it is not
  // promised by the interface (nor was promised by PlatformFile). See if this
  // implementation detail can be removed.
  return ((::SetEndOfFile(file_) != FALSE) &&
          (::SetFilePointerEx(file_, file_pointer, NULL, FILE_BEGIN) != FALSE));
}

bool File::SetTimes(int64_t last_access_time, int64_t last_modified_time) {
  FILETIME last_access_filetime;
  FILETIME last_modified_filetime;

  last_access_filetime.dwHighDateTime = last_access_time >> 32;
  last_access_filetime.dwLowDateTime = last_access_time & 0x00000000ffffffff;
  last_modified_filetime.dwHighDateTime = last_modified_time >> 32;
  last_modified_filetime.dwLowDateTime = last_modified_time & 0x00000000ffffffff;
  return (::SetFileTime(file_, NULL, &last_access_filetime, &last_modified_filetime) != FALSE);
}

bool File::GetInfo(Info* info) {
  BY_HANDLE_FILE_INFORMATION file_info;
  if (!GetFileInformationByHandle(file_, &file_info)) return false;

  LARGE_INTEGER size;
  size.HighPart = file_info.nFileSizeHigh;
  size.LowPart = file_info.nFileSizeLow;
  info->size = size.QuadPart;
  info->is_directory = (file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
  info->is_symbolic_link = false;  // Windows doesn't have symbolic links.
  info->last_modified = ((int64_t)file_info.ftLastWriteTime.dwHighDateTime << 32) |
                        file_info.ftLastWriteTime.dwLowDateTime;
  info->last_accessed = ((int64_t)file_info.ftLastAccessTime.dwHighDateTime << 32) |
                        file_info.ftLastAccessTime.dwLowDateTime;
  info->creation_time = ((int64_t)file_info.ftCreationTime.dwHighDateTime << 32) |
                        file_info.ftCreationTime.dwLowDateTime;
  return true;
}

File::Error File::Lock() {
  BOOL result = LockFile(file_, 0, 0, MAXDWORD, MAXDWORD);
  if (!result) return GetLastFileError();
  return FILE_OK;
}

File::Error File::Unlock() {
  BOOL result = UnlockFile(file_, 0, 0, MAXDWORD, MAXDWORD);
  if (!result) return GetLastFileError();
  return FILE_OK;
}

File File::Duplicate() const {
  if (!IsValid()) return File();

  HANDLE other_handle = nullptr;

  if (!::DuplicateHandle(GetCurrentProcess(),  // hSourceProcessHandle
                         GetPlatformFile(),
                         GetCurrentProcess(),  // hTargetProcessHandle
                         &other_handle,
                         0,      // dwDesiredAccess ignored due to SAME_ACCESS
                         FALSE,  // !bInheritHandle
                         DUPLICATE_SAME_ACCESS)) {
    return File(GetLastFileError());
  }

  return File(other_handle, async());
}

bool File::DeleteOnClose(bool delete_on_close) {
  FILE_DISPOSITION_INFO disposition = {delete_on_close ? TRUE : FALSE};
  return ::SetFileInformationByHandle(GetPlatformFile(), FileDispositionInfo, &disposition,
                                      sizeof(disposition)) != 0;
}

// Static.
File::Error File::OSErrorToFileError(DWORD last_error) {
  switch (last_error) {
    case ERROR_SHARING_VIOLATION:
      return FILE_ERROR_IN_USE;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
      return FILE_ERROR_EXISTS;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
      return FILE_ERROR_NOT_FOUND;
    case ERROR_ACCESS_DENIED:
      return FILE_ERROR_ACCESS_DENIED;
    case ERROR_TOO_MANY_OPEN_FILES:
      return FILE_ERROR_TOO_MANY_OPENED;
    case ERROR_OUTOFMEMORY:
    case ERROR_NOT_ENOUGH_MEMORY:
      return FILE_ERROR_NO_MEMORY;
    case ERROR_HANDLE_DISK_FULL:
    case ERROR_DISK_FULL:
    case ERROR_DISK_RESOURCES_EXHAUSTED:
      return FILE_ERROR_NO_SPACE;
    case ERROR_USER_MAPPED_FILE:
      return FILE_ERROR_INVALID_OPERATION;
    case ERROR_NOT_READY:
    case ERROR_SECTOR_NOT_FOUND:
    case ERROR_DEV_NOT_EXIST:
    case ERROR_IO_DEVICE:
    case ERROR_FILE_CORRUPT:
    case ERROR_DISK_CORRUPT:
      return FILE_ERROR_IO;
    default:
      return FILE_ERROR_FAILED;
  }
}

void File::DoInitialize(const FilePath& path, uint32_t flags) {
  DWORD disposition = 0;

  if (flags & FLAG_OPEN) disposition = OPEN_EXISTING;

  if (flags & FLAG_CREATE) {
    disposition = CREATE_NEW;
  }

  if (flags & FLAG_OPEN_ALWAYS) {
    disposition = OPEN_ALWAYS;
  }

  if (flags & FLAG_CREATE_ALWAYS) {
    disposition = CREATE_ALWAYS;
  }

  if (flags & FLAG_OPEN_TRUNCATED) {
    disposition = TRUNCATE_EXISTING;
  }

  if (!disposition) {
    ::SetLastError(ERROR_INVALID_PARAMETER);
    error_details_ = FILE_ERROR_FAILED;
    return;
  }

  DWORD access = 0;
  if (flags & FLAG_WRITE) access = GENERIC_WRITE;
  if (flags & FLAG_APPEND) {
    access = FILE_APPEND_DATA;
  }
  if (flags & FLAG_READ) access |= GENERIC_READ;
  if (flags & FLAG_WRITE_ATTRIBUTES) access |= FILE_WRITE_ATTRIBUTES;
  if (flags & FLAG_EXECUTE) access |= GENERIC_EXECUTE;
  if (flags & FLAG_CAN_DELETE_ON_CLOSE) access |= DELETE;

  DWORD sharing = (flags & FLAG_EXCLUSIVE_READ) ? 0 : FILE_SHARE_READ;
  if (!(flags & FLAG_EXCLUSIVE_WRITE)) sharing |= FILE_SHARE_WRITE;
  if (flags & FLAG_SHARE_DELETE) sharing |= FILE_SHARE_DELETE;

  DWORD create_flags = 0;
  if (flags & FLAG_ASYNC) create_flags |= FILE_FLAG_OVERLAPPED;
  if (flags & FLAG_TEMPORARY) create_flags |= FILE_ATTRIBUTE_TEMPORARY;
  if (flags & FLAG_HIDDEN) create_flags |= FILE_ATTRIBUTE_HIDDEN;
  if (flags & FLAG_DELETE_ON_CLOSE) create_flags |= FILE_FLAG_DELETE_ON_CLOSE;
  if (flags & FLAG_BACKUP_SEMANTICS) create_flags |= FILE_FLAG_BACKUP_SEMANTICS;
  if (flags & FLAG_SEQUENTIAL_SCAN) create_flags |= FILE_FLAG_SEQUENTIAL_SCAN;

  file_ = CreateFileA(path.value().c_str(), access, sharing, NULL, disposition, create_flags, NULL);

  if (IsValid()) {
    error_details_ = FILE_OK;
    async_ = ((flags & FLAG_ASYNC) == FLAG_ASYNC);

    if (flags & (FLAG_OPEN_ALWAYS))
      created_ = (ERROR_ALREADY_EXISTS != GetLastError());
    else if (flags & (FLAG_CREATE_ALWAYS | FLAG_CREATE))
      created_ = true;
  } else {
    error_details_ = GetLastFileError();
  }
}

bool File::Flush() { return ::FlushFileBuffers(file_) != FALSE; }

void File::SetPlatformFile(PlatformFile file) {
  if (IsValid()) CloseHandle(file_);
  file_ = file;
}

// static
File::Error File::GetLastFileError() { return File::OSErrorToFileError(GetLastError()); }

}  // namespace utils
}  // namespace agora
