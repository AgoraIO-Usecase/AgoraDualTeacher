//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRefPtr.h"

namespace agora {
namespace rte {

class IAgoraMessage : public RefCountInterface {
 public:
  virtual bool SetMessage(const char* msg) = 0;
  virtual const char* GetMessage() const = 0;
  virtual void SetTimestamp(uint64_t ts) = 0;
  virtual uint64_t GetTimestamp() = 0;
};

}  // namespace rte
}  // namespace agora
