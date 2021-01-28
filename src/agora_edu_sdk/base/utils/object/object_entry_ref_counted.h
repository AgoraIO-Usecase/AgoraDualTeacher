//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <mutex>

#include "utils/object/object_entry.h"
#include "utils/refcountedobject.h"

namespace agora {
namespace utils {

template <typename T>
class RefCountedObjectStrongRef;

template <typename T>
class RefCountedObjectWeakRef;

template <typename T>
class RefCountedObjectStrongRef : public ObjectStrongReferenceInterface {
 public:
  static ObjectStrongRef Create(agora_refptr<T>& ptr, const void* key) {
    return std::unique_ptr<ObjectStrongReferenceInterface>(
        static_cast<ObjectStrongReferenceInterface*>(new RefCountedObjectStrongRef(ptr, key)));
  }

 public:
  ObjectWeakRef GetWeak() override { return RefCountedObjectWeakRef<T>::Create(shared_, key_); }

  ~RefCountedObjectStrongRef() { shared_ = nullptr; }

  const void* ID() const override { return key_; }

  ObjectType Type() const override { return ObjectType::OBJECT_REF_COUNTED; }

  agora_refptr<T> Raw() { return shared_; }

 private:
  explicit RefCountedObjectStrongRef(agora_refptr<T>& ptr, const void* key)
      : shared_(ptr), key_(key) {}

  agora_refptr<T> shared_ = nullptr;
  const void* key_ = nullptr;
};

// Note that this is a "fake" weak reference
// still hold lots of memory
template <typename T>
class RefCountedObjectWeakRef : public ObjectWeakReferenceInterface {
 public:
  static ObjectWeakRef Create(agora_refptr<T>& shared, const void* key) {
    return std::unique_ptr<ObjectWeakReferenceInterface>(
        static_cast<ObjectWeakReferenceInterface*>(new RefCountedObjectWeakRef(shared, key)));
  }

 public:
  ObjectStrongRef Lock() override {
    // Think!
    // No lock needed here. Because RefCountedObjectWeakRef class hold at least one reference count
    // so before dtor of RefCountedObjectWeakRef, agora_refptr will never freed.

    if (!shared_) return nullptr;

    if (shared_->HasOneRef()) return nullptr;

    return RefCountedObjectStrongRef<T>::Create(shared_, key_);
  }

  ~RefCountedObjectWeakRef() { shared_ = nullptr; }

  const void* ID() const override { return key_; }

  ObjectType Type() const override { return ObjectType::OBJECT_REF_COUNTED; }

 private:
  explicit RefCountedObjectWeakRef(agora_refptr<T>& shared, const void* key)
      : shared_(shared), key_(key) {}

  agora_refptr<T> shared_ = nullptr;
  const void* key_ = nullptr;
};

}  // namespace utils
}  // namespace agora
