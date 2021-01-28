//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2019-06.
//  Copyright (c) 2019 Agora IO. All rights reserved.
//
#pragma once
#include <string>
#include "facilities/argus/protobuf-c/protobuf_wrapper.h"
#include "utils/packer/packer.h"

namespace agora {
namespace commons {

template <typename T>
class ProtobufPackable {
 public:
  ProtobufPackable(base::ProtobufInterface& instance, const std::string& property)
      : instance_(instance), property_(property), string_ptr_(nullptr), has_content_(false) {
    Reload();
  }

  void Reload() {
    string_ptr_ = nullptr;
    has_content_ = false;
    string_ptr_ = instance_.getString(property_);
    if (string_ptr_ && !string_ptr_->empty()) {
      unpacker up(string_ptr_->data(), string_ptr_->size());
      up.rewind();
      up.pop_uint16();
      uint16_t uri = up.pop_uint16();
      up.rewind();
      if (up.length() > 0 && uri == object_.uri) {
        up >> object_;
        has_content_ = true;
      }
    }
  }

  bool IsValid() const { return string_ptr_ != nullptr; }
  bool IsEmpty() const { return !has_content_; }

  const T& Value() const { return object_; }
  T& Value() { return object_; }

  void Flush() {
    if (IsValid()) {
      packer pk;
      object_.pack(pk);
      string_ptr_->assign(pk.buffer(), pk.length());
    }
  }

 private:
  base::ProtobufInterface& instance_;
  std::string property_;
  T object_;
  std::string* string_ptr_;
  bool has_content_;
};

}  // namespace commons
}  // namespace agora
