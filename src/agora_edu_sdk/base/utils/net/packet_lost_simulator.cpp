//  Agora Media SDK
//
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "packet_lost_simulator.h"

#include <ctime>

#include "utils/tools/util.h"

namespace agora {
namespace utils {

int SimulateTxLostRate::percentage_ = 0;

SimulateTxLostRate::SimulateTxLostRate(int percentage) { (void)SetTxLostRate(percentage); }

SimulateTxLostRate::~SimulateTxLostRate() { (void)SetTxLostRate(0); }

bool SimulateTxLostRate::SetTxLostRate(int percentage) {
  if (percentage < 0 || percentage > 100) {
    percentage_ = 0;
    return false;
  }

  percentage_ = percentage;
  return true;
}

#if defined(FEATURE_ENABLE_UT_SUPPORT)
static int generate_random_value() {
  return static_cast<int>(commons::getUniformRandomNum(1, 100));
}
#endif  // FEATURE_ENABLE_UT_SUPPORT

bool SimulateTxLostRate::IsTxAllowed() {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  if (percentage_ <= 0) {
    return true;
  }

  return (generate_random_value() > percentage_);
#else
  return true;
#endif  // FEATURE_ENABLE_UT_SUPPORT
}

int SimulateRxLostRate::percentage_ = 0;

SimulateRxLostRate::SimulateRxLostRate(int percentage) { (void)SetRxLostRate(percentage); }

SimulateRxLostRate::~SimulateRxLostRate() { (void)SetRxLostRate(0); }

bool SimulateRxLostRate::SetRxLostRate(int percentage) {
  if (percentage < 0 || percentage > 100) {
    percentage_ = 0;
    return false;
  }

  percentage_ = percentage;
  return true;
}

bool SimulateRxLostRate::IsRxAllowed() {
#if defined(FEATURE_ENABLE_UT_SUPPORT)
  if (percentage_ <= 0) {
    return true;
  }

  return (generate_random_value() > percentage_);
#else
  return true;
#endif  // FEATURE_ENABLE_UT_SUPPORT
}

}  // namespace utils
}  // namespace agora
