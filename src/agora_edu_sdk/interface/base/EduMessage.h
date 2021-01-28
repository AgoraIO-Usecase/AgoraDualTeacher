//
//  EduMessage.h
//
//  Created by WQX on 2020/11/19.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once

#include "AgoraRefPtr.h"

namespace agora {
namespace edu {

class IAgoraEduMessage : public RefCountInterface {
 public:
  virtual bool SetEduMessage(const char* msg) = 0;
  virtual const char* GetEduMessage() const = 0;
  virtual void SetTimestamp(uint64_t ts) = 0;
  virtual uint64_t GetTimestamp() = 0;
};

}  // namespace edu
}  // namespace agora
