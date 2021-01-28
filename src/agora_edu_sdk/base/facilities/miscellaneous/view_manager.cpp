//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "view_manager.h"

namespace agora {
namespace rtc {

#if defined(WEBRTC_ANDROID)

utils::object_handle ViewToHandle(view_t view) {
  jobject obj = static_cast<jobject>(view);
  return utils::ObjectToHandle(webrtc::JavaParamRef<jobject>(obj));
}

webrtc::ScopedJavaGlobalRef<jobject> HandleToView(utils::object_handle handle) {
  return utils::HandleToJObject(handle);
}

#elif defined(WEBRTC_LINUX)

// FIXME(Ender):
// Implement later
utils::object_handle ViewToHandle(view_t view) {
  return reinterpret_cast<utils::object_handle>(view);
}

view_t HandleToView(utils::object_handle handle) { return reinterpret_cast<view_t>(handle); }

#elif defined(WEBRTC_WIN)

// No need to use object table in Windows
// Because a "Windows handle" is already a weak reference
utils::object_handle ViewToHandle(view_t view) {
  return reinterpret_cast<utils::object_handle>(view);
}

HANDLE HandleToView(utils::object_handle handle) { return reinterpret_cast<HANDLE>(handle); }

#else

utils::object_handle ViewToHandle(view_t view) { return utils::kInvalidHandle; }

void* HandleToView(utils::object_handle handle) { return nullptr; }

#endif
}  // namespace rtc
}  // namespace agora
