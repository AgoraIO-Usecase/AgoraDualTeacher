//
//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <jni.h>

#include "sdk/android/native_api/jni/scoped_java_ref.h"
#include "sdk/android/src/jni/jvm.h"
#include "utils/object/object_entry.h"

namespace agora {
namespace utils {

class JObjectStrongRef;

class JObjectWeakRef;

int IdentityJObject(JNIEnv* env, const webrtc::JavaParamRef<jobject>& obj);

class JObjectStrongRef : public ObjectStrongReferenceInterface {
 public:
  static ObjectStrongRef Create(jobject ptr, const void* key);

 public:
  ObjectWeakRef GetWeak() override;

  ~JObjectStrongRef();

  const void* ID() const override { return key_; }

  ObjectType Type() const override { return ObjectType::OBJECT_JOBJECT; }

  webrtc::ScopedJavaGlobalRef<jobject> Raw();

 private:
  explicit JObjectStrongRef(jobject ptr, const void* key);

  jobject shared_ = nullptr;
  const void* key_ = nullptr;
};

class JObjectWeakRef : public ObjectWeakReferenceInterface {
 public:
  static ObjectWeakRef Create(jobject obj, const void* key);

 public:
  ObjectStrongRef Lock() override;

  ~JObjectWeakRef();

  const void* ID() const override { return key_; }

  ObjectType Type() const override { return ObjectType::OBJECT_JOBJECT; }

 private:
  explicit JObjectWeakRef(jobject obj, const void* key);

  jobject weak_;
  const void* key_ = nullptr;
};

}  // namespace utils
}  // namespace agora
