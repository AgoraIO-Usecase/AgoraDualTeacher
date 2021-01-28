//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include <memory>

#include "sdk/objc/Framework/Headers/WebRTC/RTCMacros.h"
#include "utils/object/object_entry.h"

RTC_FWD_DECL_OBJC_CLASS(NSObject);

namespace agora {
namespace utils {

struct WeakWrap;
struct StrongWrap;
class NSObjectStrongRef;
class NSObjectWeakRef;

void* IdentityNSObject(const NSObject* obj);

class NSObjectStrongRef : public ObjectStrongReferenceInterface {
 public:
  static ObjectStrongRef Create(NSObject* ptr, const void* key);

 public:
  ObjectWeakRef GetWeak() override;

  ~NSObjectStrongRef();

  const void* ID() const override { return key_; }

  ObjectType Type() const override { return ObjectType::OBJECT_NSOBJECT; }

  NSObject* Raw();

 private:
  NSObjectStrongRef(NSObject* ptr, const void* key);

  StrongWrap* shared_ = nullptr;
  const void* key_ = nullptr;
};

class NSObjectWeakRef : public ObjectWeakReferenceInterface {
 public:
  static ObjectWeakRef Create(NSObject* obj, const void* key);

 public:
  ObjectStrongRef Lock() override;

  ~NSObjectWeakRef();

  const void* ID() const override { return key_; }

  ObjectType Type() const override { return ObjectType::OBJECT_NSOBJECT; }

 private:
  NSObjectWeakRef(NSObject* ptr, const void* key);

  WeakWrap* weak_ = nullptr;
  const void* key_ = nullptr;
};

}  // namespace utils
}  // namespace agora
