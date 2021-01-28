//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "base/AgoraBase.h"
#include "utils/object/object_table.h"

namespace agora {
namespace rtc {

//
// ViewToHandle:
//   Translate a agora::view_t into utils::object_handle
//   An object_handle is a "weak reference" to the system resource like view, the Real holder of
//   system resource can free it as wish, and object_handle user like renderer MUST obtain a real
//   object from object_handle using utils::HandleToXXX functions.
//
// @param   view:           a pointer represent system resource
// @ret:    object_handle
//
// Note:
//   * object_handle is a "weak reference", so utils::HandleToXXX functions will return
//     nullptr if real resource is already freed
//   * Windows resource handle is already a "weak reference", don't have to manage it by
//     object table. But you still can call ViewToHandle and HandleToView (and those functions
//     do nothing in Windows)
//
utils::object_handle ViewToHandle(view_t view);

//
// HandleToView:
//  Translate object_handle to system resource type
//
// @param   handle          An object handle from ViewToHandle
// @ret     System resource. nullptr (or empty object) if that resource already freed
//
#if defined(WEBRTC_WIN)
HANDLE HandleToView(utils::object_handle handle);
#elif defined(WEBRTC_ANDROID)
webrtc::ScopedJavaGlobalRef<jobject> HandleToView(utils::object_handle handle);
#elif defined(WEBRTC_MAC) || defined(WEBRTC_IOS)
NSObject* HandleToView(utils::object_handle handle);
#elif defined(WEBRTC_LINUX)
view_t HandleToView(utils::object_handle handle);
#else
void* HandleToView(utils::object_handle handle);
#endif

}  // namespace rtc
}  // namespace agora
