//
//  Agora SDK
//
//  Created by Albert Zhang in 2018-09.
//  Copyright (c) 2018 Agora.io. All rights reserved.
//
#pragma once

namespace agora {
namespace base {

class IAgoraRefObject {
 public:
  virtual void addRef() = 0;
  virtual void delRef() = 0;

 protected:
  virtual ~IAgoraRefObject() {}
};

}  // namespace base
}  // namespace agora
