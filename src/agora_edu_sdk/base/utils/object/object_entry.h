//
//  Agora Media SDK
//
//  Created by Ender Zheng in 2020-04.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <memory>

namespace agora {
namespace utils {
class ObjectStrongReferenceInterface;
class ObjectWeakReferenceInterface;

using ObjectStrongRef = std::unique_ptr<ObjectStrongReferenceInterface>;
using ObjectWeakRef = std::unique_ptr<ObjectWeakReferenceInterface>;

enum class ObjectType {
  OBJECT_STD_SHARED,
  OBJECT_REF_COUNTED,
  OBJECT_JOBJECT,
  OBJECT_NSOBJECT,
  OBJECT_UNKNOWN
};

class ObjectTableEntry {
 public:
  virtual ~ObjectTableEntry() = default;

  virtual const void* ID() const = 0;

  virtual ObjectType Type() const = 0;
};

class ObjectStrongReferenceInterface : public ObjectTableEntry {
 public:
  virtual ~ObjectStrongReferenceInterface() = default;

  virtual ObjectWeakRef GetWeak() = 0;
};

class ObjectWeakReferenceInterface : public ObjectTableEntry {
 public:
  virtual ~ObjectWeakReferenceInterface() = default;

  virtual ObjectStrongRef Lock() = 0;
};

}  // namespace utils
}  // namespace agora
