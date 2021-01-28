//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include "utils/object/object_entry.h"

namespace agora {
namespace utils {
template <typename T>
class StdSharedObjectStrongRef;

template <typename T>
class StdSharedObjectWeakRef;

template <typename T>
class StdSharedObjectStrongRef : public ObjectStrongReferenceInterface {
 public:
  static ObjectStrongRef Create(std::shared_ptr<T>& ptr, const void* key) {
    return std::unique_ptr<ObjectStrongReferenceInterface>(
        static_cast<ObjectStrongReferenceInterface*>(new StdSharedObjectStrongRef(ptr, key)));
  }

 public:
  ObjectWeakRef GetWeak() override { return StdSharedObjectWeakRef<T>::Create(shared_, key_); }

  ~StdSharedObjectStrongRef() { shared_ = nullptr; }

  const void* ID() const override { return key_; }

  ObjectType Type() const override { return ObjectType::OBJECT_STD_SHARED; }

  std::shared_ptr<T> Raw() { return shared_; }

 private:
  explicit StdSharedObjectStrongRef(std::shared_ptr<T>& ptr, const void* key)
      : shared_(ptr), key_(key) {}

  std::shared_ptr<T> shared_;
  const void* key_ = nullptr;
};

template <typename T>
class StdSharedObjectWeakRef : public ObjectWeakReferenceInterface {
 public:
  static ObjectWeakRef Create(std::shared_ptr<T>& shared, const void* key) {
    return std::unique_ptr<ObjectWeakReferenceInterface>(
        static_cast<ObjectWeakReferenceInterface*>(new StdSharedObjectWeakRef(shared, key)));
  }

 public:
  ObjectStrongRef Lock() override {
    auto shared = weak_.lock();
    if (!shared) return nullptr;

    return StdSharedObjectStrongRef<T>::Create(shared, key_);
  }

  ~StdSharedObjectWeakRef() {}

  const void* ID() const override { return key_; }

  ObjectType Type() const override { return ObjectType::OBJECT_STD_SHARED; }

 private:
  explicit StdSharedObjectWeakRef(std::shared_ptr<T>& shared, const void* key)
      : weak_(shared), key_(key) {}

  std::weak_ptr<T> weak_;
  const void* key_ = nullptr;
};

}  // namespace utils
}  // namespace agora
