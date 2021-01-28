//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>

#include "AgoraRefPtr.h"
#include "utils/build_config.h"
#include "utils/mgnt/util_globals.h"
#include "utils/object/object_entry_ref_counted.h"
#include "utils/object/object_entry_std_shared.h"
#include "utils/thread/thread_control_block.h"
#if defined(OS_ANDROID)
#include "utils/object/object_entry_jobject.h"
#endif
#if defined(OS_IOS) || defined(OS_MAC)
#include "utils/object/object_entry_nsobject.h"
#endif

namespace agora {

namespace commons {
class timer_base;
}

namespace utils {

using object_handle = uintptr_t;
static const object_handle kInvalidHandle = static_cast<object_handle>(0);

class ObjectTable {
 public:
  ObjectTable();
  ~ObjectTable();

 public:
  ObjectStrongRef HandleToObject(object_handle handle);

  object_handle ObjectToHandle(ObjectWeakRef obj);

  void GC();

 private:
  void EraseHandle(object_handle handle);
  void InsertHandle(object_handle handle, const void* key, ObjectWeakRef weak);
  object_handle CalculateHandleValue();
  worker_type gc_worker_;
  std::unique_ptr<commons::timer_base> gc_timer_;

 private:
  std::mutex lock_;
  std::map<object_handle, ObjectWeakRef> handle_to_obj_map_;
  std::map<const void*, object_handle> id_to_handle_map_;
  std::map<object_handle, const void*> handle_to_id_map_;
};

template <typename T>
static std::shared_ptr<T> HandleToStdShared(object_handle handle) {
  if (!GetUtilGlobal() || !GetUtilGlobal()->object_table) return nullptr;

  ObjectStrongRef shared = GetUtilGlobal()->object_table->HandleToObject(handle);
  if (!shared) return nullptr;

  if (shared->Type() != ObjectType::OBJECT_STD_SHARED) return nullptr;

  // dangerous, take your own risk
  auto raw = (static_cast<StdSharedObjectStrongRef<T>*>(shared.get()))->Raw();

  shared.reset();

  return raw;
}

template <typename T>
static object_handle ObjectToHandle(std::shared_ptr<T> obj) {
  if (!GetUtilGlobal() || !GetUtilGlobal()->object_table) return kInvalidHandle;

  auto wrap = StdSharedObjectStrongRef<T>::Create(obj, reinterpret_cast<const void*>(obj.get()));
  if (!wrap) return kInvalidHandle;

  return GetUtilGlobal()->object_table->ObjectToHandle(wrap->GetWeak());
}

template <typename T>
static agora_refptr<T> HandleToAgoraRefPtr(object_handle handle) {
  if (!GetUtilGlobal() || !GetUtilGlobal()->object_table) return nullptr;

  ObjectStrongRef shared = GetUtilGlobal()->object_table->HandleToObject(handle);
  if (!shared) return nullptr;

  if (shared->Type() != ObjectType::OBJECT_REF_COUNTED) return nullptr;

  // dangerous, take your own risk
  auto raw = (static_cast<RefCountedObjectStrongRef<T>*>(shared.get()))->Raw();

  shared.reset();

  return raw;
}

template <typename T>
static object_handle ObjectToHandle(agora_refptr<T> obj) {
  if (!GetUtilGlobal() || !GetUtilGlobal()->object_table) return kInvalidHandle;

  auto wrap = RefCountedObjectStrongRef<T>::Create(obj, reinterpret_cast<const void*>(obj.get()));
  if (!wrap) return kInvalidHandle;

  return GetUtilGlobal()->object_table->ObjectToHandle(wrap->GetWeak());
}

#if defined(OS_ANDROID)

static webrtc::ScopedJavaGlobalRef<jobject> HandleToJObject(JNIEnv* env, object_handle handle) {
  ObjectStrongRef shared = GetUtilGlobal()->object_table->HandleToObject(handle);
  if (!shared) return webrtc::ScopedJavaGlobalRef<jobject>(nullptr);

  if (shared->Type() != ObjectType::OBJECT_JOBJECT)
    return webrtc::ScopedJavaGlobalRef<jobject>(nullptr);

  // not dangerous at all thank you very much
  auto raw = static_cast<JObjectStrongRef*>(shared.get())->Raw();

  shared.reset();

  return raw;
}

__attribute__((unused)) static webrtc::ScopedJavaGlobalRef<jobject> HandleToJObject(object_handle handle) {
  if (!GetUtilGlobal() || !GetUtilGlobal()->object_table)
    return webrtc::ScopedJavaGlobalRef<jobject>(nullptr);

  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
  if (!env) return webrtc::ScopedJavaGlobalRef<jobject>(nullptr);
  return HandleToJObject(env, handle);
}

static object_handle ObjectToHandle(JNIEnv* env, const webrtc::JavaParamRef<jobject>& obj) {
  if (obj.is_null()) return kInvalidHandle;
  auto wrap =
      JObjectStrongRef::Create(obj.obj(), reinterpret_cast<const void*>(IdentityJObject(env, obj)));
  if (!wrap) return kInvalidHandle;

  return GetUtilGlobal()->object_table->ObjectToHandle(wrap->GetWeak());
}

__attribute__((unused)) static object_handle ObjectToHandle(const webrtc::JavaParamRef<jobject>& obj) {
  if (!GetUtilGlobal() || !GetUtilGlobal()->object_table) return kInvalidHandle;

  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
  if (!env) return kInvalidHandle;

  return ObjectToHandle(env, obj);
}
#endif

#if defined(OS_IOS) || defined(OS_MAC)
__attribute__((unused)) static NSObject* HandleToNSObject(object_handle handle) {
  if (!GetUtilGlobal() || !GetUtilGlobal()->object_table) return nullptr;

  ObjectStrongRef shared = GetUtilGlobal()->object_table->HandleToObject(handle);
  if (!shared) return nullptr;

  if (shared->Type() != ObjectType::OBJECT_NSOBJECT) return nullptr;

  // not dangerous at all thank you very much
  auto raw = (static_cast<NSObjectStrongRef*>(shared.get()))->Raw();

  shared.reset();

  return raw;
}

__attribute__((unused)) static object_handle ObjectToHandle(NSObject* obj) {
  if (!GetUtilGlobal() || !GetUtilGlobal()->object_table) return kInvalidHandle;

  auto wrap = NSObjectStrongRef::Create(obj, IdentityNSObject(obj));

  if (!wrap) return kInvalidHandle;

  return GetUtilGlobal()->object_table->ObjectToHandle(wrap->GetWeak());
}

#endif

void ObjectTableGC();

}  // namespace utils
}  // namespace agora
