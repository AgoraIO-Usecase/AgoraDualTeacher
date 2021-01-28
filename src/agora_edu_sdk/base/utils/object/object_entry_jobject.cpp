//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "utils/object/object_entry_jobject.h"

#include "sdk/android/src/jni/jni_generator_helper.h"

const char kClassPath_java_lang_system[] = "java/lang/System";

// Leaking this jclass as we cannot use LazyInstance from some threads.
JNI_REGISTRATION_EXPORT std::atomic<jclass> g_java_lang_system_clazz(nullptr);
#ifndef java_lang_system_clazz_defined
#define java_lang_system_clazz_defined
inline jclass java_lang_system_clazz(JNIEnv* env) {
  return base::android::LazyGetClass(env, kClassPath_java_lang_system, &g_java_lang_system_clazz);
}
#endif

namespace webrtc {
namespace jni {
static std::atomic<jmethodID> g_java_lang_system_identityHashCode(nullptr);
static jint Java_System_identityHashCode(JNIEnv* env, const JavaRef<jobject>& obj) {
  CHECK_CLAZZ(env, java_lang_system_clazz(env), java_lang_system_clazz(env), 0);
  jmethodID method_id = base::android::MethodID::LazyGet<base::android::MethodID::TYPE_STATIC>(
      env, java_lang_system_clazz(env), "identityHashCode", "(Ljava/lang/Object;)I",
      &g_java_lang_system_identityHashCode);

  jint ret = env->CallStaticIntMethod(java_lang_system_clazz(env), method_id, obj.obj());
  jni_generator::CheckException(env);
  return ret;
}
}  // namespace jni
}  // namespace webrtc

namespace agora {
namespace utils {

int IdentityJObject(JNIEnv* env, const webrtc::JavaParamRef<jobject>& obj) {
  return webrtc::jni::Java_System_identityHashCode(env, obj);
}

ObjectStrongRef JObjectStrongRef::Create(jobject ptr, const void* key) {
  return std::unique_ptr<ObjectStrongReferenceInterface>(
      static_cast<ObjectStrongReferenceInterface*>(new JObjectStrongRef(ptr, key)));
}

ObjectWeakRef JObjectStrongRef::GetWeak() {
  if (!shared_) return nullptr;

  return JObjectWeakRef::Create(shared_, key_);
}

webrtc::ScopedJavaGlobalRef<jobject> JObjectStrongRef::Raw() {
  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
  if (!env || !shared_) return webrtc::ScopedJavaGlobalRef<jobject>(nullptr);

  return webrtc::ScopedJavaGlobalRef<jobject>(env, webrtc::JavaParamRef<jobject>(shared_));
}

JObjectStrongRef::JObjectStrongRef(jobject ptr, const void* key) : key_(key) {
  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();

  if (ptr && env) {
    shared_ = env->NewGlobalRef(ptr);
  }
}

JObjectStrongRef::~JObjectStrongRef() {
  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
  if (shared_ && env) {
    env->DeleteGlobalRef(shared_);
  }

  shared_ = nullptr;
}

ObjectWeakRef JObjectWeakRef::Create(jobject obj, const void* key) {
  return std::unique_ptr<ObjectWeakReferenceInterface>(
      static_cast<ObjectWeakReferenceInterface*>(new JObjectWeakRef(obj, key)));
}

ObjectStrongRef JObjectWeakRef::Lock() {
  if (!weak_) return nullptr;
  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
  if (!env) return nullptr;
  if (env->IsSameObject(weak_, nullptr)) return nullptr;
  jobject shared = env->NewGlobalRef(weak_);
  if (!shared) return nullptr;

  auto ret = JObjectStrongRef::Create(shared, key_);
  env->DeleteGlobalRef(shared);
  return ret;
}

JObjectWeakRef::JObjectWeakRef(jobject obj, const void* key) : key_(key) {
  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
  if (obj && env) {
    weak_ = env->NewWeakGlobalRef(obj);
  }
}

JObjectWeakRef::~JObjectWeakRef() {
  JNIEnv* env = webrtc::jni::AttachCurrentThreadIfNeeded();
  if (weak_ && env) {
    env->DeleteWeakGlobalRef(weak_);
  }

  weak_ = nullptr;
}

}  // namespace utils
}  // namespace agora
