//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <sstream>
#include <string>

namespace agora {
namespace diag {

class Snapshot {
 public:
  static void CaptureThreadInfo(std::stringstream& ss);

  static void CaptureSystemInfo(std::stringstream& ss);

  static void CaptureConnectionInfo(std::stringstream& ss);

 private:
  Snapshot() = delete;
  ~Snapshot() = delete;
};

}  // namespace diag
}  // namespace agora
