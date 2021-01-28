//
//  Agora Media SDK
//
//  Created by Yaqi Li in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>

namespace agora {
namespace utils {

class INativeEventListener {
 public:
  virtual void initialize() = 0;
  virtual void uninitialize() = 0;
  virtual ~INativeEventListener() = default;
};

std::unique_ptr<INativeEventListener> createNativeEventListener();

}  // namespace utils
}  // namespace agora
