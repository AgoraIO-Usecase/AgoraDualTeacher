//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-09.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <memory>
#include <string>

#include "facilities/tools/crash_info.h"

namespace agora {
namespace base {
class BaseContext;
}

namespace rtc {
class XdumpHandler {
 public:
  static std::unique_ptr<XdumpHandler> Create(base::BaseContext& context);

  static std::string GenerateDump(void* info, void* context);

  ~XdumpHandler();

 private:
  explicit XdumpHandler(base::BaseContext& ctx);

 public:
  void OnJoinChannel(const utils::ChannelContext& channel_ctx);

 private:
  void SaveCrashContext(void* info, void* context);

 private:
  base::BaseContext& context_;
};
}  // namespace rtc
}  // namespace agora
