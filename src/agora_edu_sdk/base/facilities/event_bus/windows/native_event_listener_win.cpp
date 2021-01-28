//
//  Agora Media SDK
//
//  Created by Rao Qi in 2019-06.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#include <windows.h>

#include <dbt.h>

#include "facilities/event_bus/event_bus.h"
#include "native_event_listener_win.h"
#include "utils/log/log.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace utils {

using namespace agora::commons;

std::unique_ptr<INativeEventListener> createNativeEventListener() {
  return std::make_unique<NativeEventListenerWin>();
}

void NativeEventListenerWin::initialize() {
  if (hwnd_) {
    log(LOG_INFO, "Message: no need to init twice");
    return;
  }

  if (!CreateWndClass()) {
    uninitialize();
    return;
  }

  MessageLoop();
}

#define WND_CLASS_NAME "NativeEventListenerClass"
#define WND_WINDOW_NAME "NativeEventListenerWnd"
#define WND_WAIT_TIMEOUT (2000)

ATOM NativeEventListenerWin::NativeEventListenerClassAtom = 0;
bool NativeEventListenerWin::CreateWndClass() {
  if (NativeEventListenerClassAtom == 0) {
    WNDCLASSA wc = {};

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = reinterpret_cast<WNDPROC>(NativeEventListenerWin::DeviceChangeCallback);
    wc.lpszClassName = WND_CLASS_NAME;

    NativeEventListenerClassAtom = ::RegisterClassA(&wc);
    if (NativeEventListenerClassAtom == 0) {
      log(LOG_ERROR, "Fail to register window with error: %x", ::GetLastError());
      return false;
    }
  }

  return true;
}

void NativeEventListenerWin::MessageLoop() {
  HANDLE e = ::CreateEvent(NULL, FALSE, FALSE, NULL);

  utils::event_listener_worker()->async_call(LOCATION_HERE, [this, e] {
    char wnd_name[MAX_PATH] = {0};
    snprintf(wnd_name, sizeof(wnd_name), "%s_%d", WND_WINDOW_NAME, GetCurrentProcessId());
    hwnd_ = ::CreateWindowA(WND_CLASS_NAME, wnd_name, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, NULL, NULL,
                            NULL, NULL);
    ::SetEvent(e);

    if (hwnd_ == NULL) {
      log(LOG_ERROR, "Fail to create window with error: %x", ::GetLastError());
      assert(hwnd_);
      return -1;  // ERR_FAILED
    }

    ::SetWindowLongPtr(hwnd_, GWLP_USERDATA, (LONG_PTR)this);

    BOOL bRet;
    MSG msg;
    HWND hwnd = hwnd_;
    while ((bRet = ::GetMessage(&msg, hwnd, 0, 0)) != 0) {
      if (bRet == -1) {
        // handle the error and possibly exit
        log(LOG_ERROR, "Fail to GetMessage with error: %x", ::GetLastError());
        break;
      } else {
        LONG_PTR userData = ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (!userData || msg.message == WM_QUIT || msg.message == WM_DESTROY) {
          ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)NULL);
          break;
        }
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
      }
    }
    return 0;  // ERR_OK
  });

  if (::WaitForSingleObject(e, WND_WAIT_TIMEOUT) == WAIT_TIMEOUT) {
    log(LOG_ERROR, "Timeout to wait EventCenterWnd creation");
  }
  ::CloseHandle(e);
}

void NativeEventListenerWin::uninitialize() {
  if (hwnd_ && ::IsWindow(hwnd_)) {
    ::SetWindowLongPtr(hwnd_, GWLP_USERDATA, (LONG_PTR)NULL);
    ::PostMessage(hwnd_, WM_QUIT, 0, 0);
    hwnd_ = NULL;
  }
}

// https://docs.microsoft.com/en-us/windows-hardware/drivers/install/system-defined-device-setup-classes-available-to-vendors
GUID CameraGuid = {0xca3e7ab9, 0xb4c3, 0x4ae6, 0x82, 0x51, 0x57, 0x9e, 0xf9, 0x33, 0x89, 0x0f};
GUID ImageGuid = {0x6bdd1fc6, 0x810f, 0x11d0, 0xbe, 0xc7, 0x08, 0x00, 0x2b, 0xe2, 0x09, 0x2f};
GUID AudioCaptureGuid = {0x65E8773D, 0x8F56, 0x11D0, 0xA3, 0xB9, 0x00,
                         0xA0,       0xC9,   0x22,   0x31, 0x96};

LRESULT WINAPI NativeEventListenerWin::DeviceChangeCallback(HWND hWnd, UINT message, WPARAM wParam,
                                                            LPARAM lParam) {
  LRESULT lRet = S_FALSE;

  switch (message) {
    case WM_CREATE: {
    } break;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    case WM_DEVICECHANGE: {
      switch (wParam) {
        case DBT_DEVNODES_CHANGED: {
          auto event_bus = rtc::RtcGlobals::Instance().EventBus();
          event_bus->post(VideoDeviceEvent{});
        }
        default:
          log(LOG_NONE,
              "Message: WM_DEVICECHANGE message received unhandled, value wParam :%x lParam: %x",
              wParam, lParam);
          break;
      }
    } break;

    case WM_CLOSE:
      ::DestroyWindow(hWnd);
      break;

    default:
      lRet = DefWindowProc(hWnd, message, wParam, lParam);
      break;
  }

  return lRet;
}

}  // namespace utils
}  // namespace agora
