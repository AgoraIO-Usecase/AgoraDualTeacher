//
//  Agora Media SDK
//
//  Created by Zheng Ender in 2019-11.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <functional>
#include <memory>

namespace agora {
namespace utils {

void InstallPerProcessHandler();

void RestorePerProcessHandler();

void InstallPerThreadHandler();

void RestorePerThreadHandler();

void SetCrashHandlerHook(std::function<void()>&& hook);

void SetXdumpHandlerHook(std::function<void(void*, void*)>&& hook);

}  // namespace utils
}  // namespace agora
