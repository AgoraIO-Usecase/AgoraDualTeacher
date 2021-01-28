//
//  Agora Media SDK
//
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "sysstatus.h"

namespace agora {
namespace utils {

namespace {
static SystemStatus current_system_status_;
}

SystemStatus& SystemStatus::GetCurrent() { return current_system_status_; }

}  // namespace utils
}  // namespace agora
