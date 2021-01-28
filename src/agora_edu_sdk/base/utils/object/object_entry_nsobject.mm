//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#import <Foundation/Foundation.h>

#include "utils/object/object_entry_nsobject.h"

namespace agora {
namespace utils {

struct WeakWrap {
  __weak NSObject* pointer = nullptr;
};

struct StrongWrap {
  __strong NSObject* pointer = nullptr;
};

void* IdentityNSObject(const NSObject* obj) { return (__bridge void*)(obj); }

ObjectStrongRef NSObjectStrongRef::Create(NSObject* ptr, const void* key) {
  return std::unique_ptr<ObjectStrongReferenceInterface>(
      static_cast<ObjectStrongReferenceInterface*>(new NSObjectStrongRef(ptr, key)));
}

ObjectWeakRef NSObjectStrongRef::GetWeak() {
  if (!shared_) return nullptr;

  return NSObjectWeakRef::Create(shared_->pointer, key_);
}

NSObject* NSObjectStrongRef::Raw() {
  if (!shared_) return nullptr;

  return shared_->pointer;
}

NSObjectStrongRef::NSObjectStrongRef(NSObject* ptr, const void* key) : key_(key) {
  shared_ = new StrongWrap();
  shared_->pointer = ptr;
}

NSObjectStrongRef::~NSObjectStrongRef() {
  if (shared_) {
    shared_->pointer = nullptr;
    delete shared_;
  }
  shared_ = nullptr;
}

ObjectWeakRef NSObjectWeakRef::Create(NSObject* obj, const void* key) {
  return std::unique_ptr<ObjectWeakReferenceInterface>(
      static_cast<ObjectWeakReferenceInterface*>(new NSObjectWeakRef(obj, key)));
}

ObjectStrongRef NSObjectWeakRef::Lock() {
  if (!weak_) return nullptr;

  __strong NSObject* strong = weak_->pointer;

  if (!strong) return nullptr;

  auto ret = NSObjectStrongRef::Create(strong, key_);
  strong = nullptr;
  return ret;
}

NSObjectWeakRef::NSObjectWeakRef(NSObject* ptr, const void* key) : key_(key) {
  weak_ = new WeakWrap();
  weak_->pointer = ptr;
}

NSObjectWeakRef::~NSObjectWeakRef() {
  if (weak_) {
    weak_->pointer = nullptr;
    delete weak_;
  }

  weak_ = nullptr;
}

}  // namespace utils
}  // namespace agora
