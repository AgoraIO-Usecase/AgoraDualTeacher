//
//  Agora Media SDK
//
//  Created by Han Pengfei in 2020-09.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#pragma once
#include <string>

namespace agora {
namespace utils {

bool PathExists(const std::string& path);
bool GetFileSize(const std::string& file_path, int64_t* file_size);
bool RemoveFile(const std::string& path);

}  // namespace utils
}  // namespace agora
