//
//  Agora Media SDK
//
//  Created by Han Pengfei in 2020-09.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "file_utils.h"

#include <stdio.h>
#include <sys/stat.h>

namespace agora {
namespace utils {

bool PathExists(const std::string& path){
  struct stat file_stat;
  return (stat(path.c_str(), &file_stat) == 0);
}

bool GetFileSize(const std::string& file_path, int64_t* file_size) {
  if (!file_size) {
    return false;
  }
  struct stat file_stat;
  if (stat(file_path.c_str(), &file_stat) != 0) {
    return false;
  }
  *file_size = file_stat.st_size;
  return true;
}

bool RemoveFile(const std::string& path) {
  remove(path.c_str());
  return true;
}

}  // namespace utils
}  // namespace agora
