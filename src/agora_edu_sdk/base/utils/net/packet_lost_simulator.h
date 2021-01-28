//
//  Agora RTC/MEDIA SDK
//
//  Created by Bob Zhang in 2019-12.
//  Copyright (c) 2019 Agora.io. All rights reserved.
//
#pragma once

#include <cstdint>
#include <queue>
#include <string>
#include <utility>

namespace agora {
namespace utils {

class SimulateTxLostRate {
 public:
  explicit SimulateTxLostRate(int percentage);
  ~SimulateTxLostRate();

  static bool IsTxAllowed();
  static bool SetTxLostRate(int percentage);

 private:
  static int percentage_;
};

class SimulateRxLostRate {
 public:
  explicit SimulateRxLostRate(int percentage);
  ~SimulateRxLostRate();

  static bool IsRxAllowed();
  static bool SetRxLostRate(int percentage);

 private:
  static int percentage_;
};

}  // namespace utils
}  // namespace agora
