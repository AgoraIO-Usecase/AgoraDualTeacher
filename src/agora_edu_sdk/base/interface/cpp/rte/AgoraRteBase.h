//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraBase.h"

namespace agora {
namespace rte {

static const int kMaxUserIdSize = 128;
static const int kMaxStreamIdSize = 128;
static const int kMaxSceneIdSize = 128;
static const int kMaxRoleIdSize = 128;
static const int kMaxKeySize = 32;
static const int kMaxValSize = 8 * 1024;

using AppId = const char*;
using UserId = const char*;
using StreamId = const char*;
using SceneId = const char*;
using Context = void*;
using View = void*;
using RequestId = int;
using ClientRole = const char*;

struct KeyValPair {
  char key[kMaxKeySize];
  char val[kMaxValSize];

  KeyValPair() : key{0}, val{0} {}
};

struct KeyValPairCollection {
  size_t count;
  KeyValPair* key_vals;

  KeyValPairCollection() : count(0), key_vals(nullptr) {}
};

// Doesn't contain anything except for a type and message now, but will contain more in the
// future as more errors are implemented.
class AgoraError {
 private:
  AgoraError(const AgoraError&) = delete;
  AgoraError& operator=(const AgoraError&) = delete;

 public:
  AgoraError() {}

  explicit AgoraError(ERROR_CODE_TYPE type) : type_(type) {}

  AgoraError(ERROR_CODE_TYPE type, const char* message) : type_(type), message_(message) {}

  AgoraError(AgoraError&& rhs) : type_(rhs.type_), message_(rhs.message_) {
    rhs.type_ = ERR_NOT_SUPPORTED;
    rhs.message_ = nullptr;
  }

  AgoraError& operator=(AgoraError&& rhs) {
    if (this == &rhs) {
      return *this;
    }

    type_ = rhs.type_;
    message_ = rhs.message_;

    rhs.type_ = ERR_NOT_SUPPORTED;
    rhs.message_ = nullptr;

    return *this;
  }

  ~AgoraError() {}

  // Preferred over the default constructor for code readability.
  static AgoraError OK() { return AgoraError(); }

  ERROR_CODE_TYPE type() const { return type_; }

  operator ERROR_CODE_TYPE() const { return type_; }

  bool ok() const { return (type_ == ERR_OK); }

  void setType(ERROR_CODE_TYPE type) { type_ = type; }

  // Human-readable message describing the error. Shouldn't be used for
  // anything but logging/diagnostics, since messages are not guaranteed to be
  // stable.
  const char* message() const { return message_; }

  void setMessage(const char* message) { message_ = message; }

  void set(ERROR_CODE_TYPE type, const char* message) {
    type_ = type;
    message_ = message;
  }

 private:
  ERROR_CODE_TYPE type_ = ERR_OK;
  const char* message_ = "";
};

}  // namespace rte
}  // namespace agora
