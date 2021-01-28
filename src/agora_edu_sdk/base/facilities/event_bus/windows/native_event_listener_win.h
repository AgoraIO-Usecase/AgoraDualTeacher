//
//  Agora Media SDK
//
//  Created by Rao Qi in 2019-06.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once

#include "facilities/event_bus/event_bus.h"
#include "facilities/event_bus/native_event_listener.h"

namespace agora {
namespace utils {

class NativeEventListenerWin : public INativeEventListener {
 public:
  NativeEventListenerWin() = default;
  ~NativeEventListenerWin() = default;

  void initialize() override;
  void uninitialize() override;

 private:
  static ATOM NativeEventListenerClassAtom;
  static LRESULT WINAPI DeviceChangeCallback(HWND, UINT, WPARAM, LPARAM);
  bool CreateWndClass();
  void MessageLoop();

 private:
  HWND hwnd_;
};

}  // namespace utils
}  // namespace agora
