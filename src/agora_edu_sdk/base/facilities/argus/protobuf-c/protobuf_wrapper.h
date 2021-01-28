//
//  Agora Media SDK
//
//  Created by Yao Yuan in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#pragma once

#include "protobuf-c/protobuf-c.h"

#include <cctype>
#include <functional>
#include <map>
#include <string>
#include <vector>

#define PROTOBUF_ADD_PREFIX(NameSpace, MessageName) Io__Agora__Pb__##NameSpace##__##MessageName
#define PROTOBUF_ADD_PREFIX_VOSDK(MessageName) PROTOBUF_ADD_PREFIX(Vosdk, MessageName)
#define PROTOBUF_ADD_PREFIX_COUNTER(MessageName) PROTOBUF_ADD_PREFIX(Counter, MessageName)
#define PROTOBUF_ADD_PREFIX_MESSAGE(MessageName) PROTOBUF_ADD_PREFIX(Message, MessageName)
#define PROTOBUF_ADD_PREFIX_CACHE(MessageName) PROTOBUF_ADD_PREFIX(Cache, MessageName)
#define PROTOBUF_ADD_PREFIX_RTMSDK(MessageName) PROTOBUF_ADD_PREFIX(Rtmsdk, MessageName)

#define PROTOBUF_FUNCTION(NameSpace, MessageName)                     \
  io__agora__pb__##NameSpace##__##MessageName##__init,                \
      io__agora__pb__##NameSpace##__##MessageName##__get_packed_size, \
      io__agora__pb__##NameSpace##__##MessageName##__pack,            \
      io__agora__pb__##NameSpace##__##MessageName##__unpack,          \
      io__agora__pb__##NameSpace##__##MessageName##__free_unpacked

#define PROTOBUF_FUNCTION_VOSDK(MessageName) PROTOBUF_FUNCTION(vosdk, MessageName)
#define PROTOBUF_FUNCTION_COUNTER(MessageName) PROTOBUF_FUNCTION(counter, MessageName)
#define PROTOBUF_FUNCTION_MESSAGE(MessageName) PROTOBUF_FUNCTION(message, MessageName)
#define PROTOBUF_FUNCTION_CACHE(MessageName) PROTOBUF_FUNCTION(cache, MessageName)
#define PROTOBUF_FUNCTION_RTMSDK(MessageName) PROTOBUF_FUNCTION(rtmsdk, MessageName)

namespace agora {

namespace base {

class ProtobufWrapper {
 public:
  ProtobufWrapper(size_t* size_address, char*** string_vector_address);
  explicit ProtobufWrapper(char** string_address);
  explicit ProtobufWrapper(ProtobufCBinaryData* bytes_address);
  ProtobufWrapper(size_t* size_address, ProtobufCBinaryData** bytes_vector_address);
  ~ProtobufWrapper();

  static void TransformStringToCharPtrDeep(char** output, const std::string& input);
  static void TransformStringToBytes(ProtobufCBinaryData* output, const std::string& input);
  static bool isProtobufCBinaryDataEmpty(const ProtobufCBinaryData& binaryData);

  void Pack();

  /* In protobuf, char* member will be init to protobuf_c_empty_string，it's defined:
   * extern const char protobuf_c_empty_string[];
   * So, when want to free a char* member, compare it with protobuf_c_empty_string.If equal, don't
   * free */
  static bool isProtobufEmptyString(const char* string_address) {
    return string_address == reinterpret_cast<const char*>(protobuf_c_empty_string);
  }

  inline std::string* getString() { return &string_; }

  inline const std::string* getString() const { return &string_; }

  inline std::vector<std::string>* getStringList() { return &string_vector_; }

  inline const std::vector<std::string>* getStringList() const { return &string_vector_; }

 private:
  size_t* size_address_;
  char*** string_vector_address_;
  char** string_address_;
  ProtobufCBinaryData* bytes_address_;
  ProtobufCBinaryData** bytes_vector_address_;
  std::vector<std::string> string_vector_;
  std::string string_;
};

class ProtobufInterface {
 public:
  virtual size_t PackInstance(std::string& content) = 0;
  virtual std::string* getString(const std::string& property_name) = 0;
};

template <typename T>
class ProtobufInstance : public ProtobufInterface {
  typedef T value_type;
  typedef T* pointer_type;

  using InitInstanceFunc = std::function<void(pointer_type)>;
  using GetPacketedSizeFunc = std::function<size_t(const pointer_type)>;
  using PackFunc = std::function<size_t(const pointer_type, uint8_t*)>;
  using UnpackFunc = std::function<pointer_type(ProtobufCAllocator*, size_t, const uint8_t*)>;
  using FreeUnpackedFunc = std::function<void(pointer_type, ProtobufCAllocator*)>;

 public:
  ProtobufInstance(InitInstanceFunc initFunc, GetPacketedSizeFunc getSizeFunc, PackFunc packFunc,
                   UnpackFunc unpackFunc, FreeUnpackedFunc freeUnpackedFunc)
      : init_(initFunc),
        get_packed_size_(getSizeFunc),
        pack_(packFunc),
        unpack_(unpackFunc),
        free_unpacked_(freeUnpackedFunc),
        instance_(nullptr) {
    InitInstance();
  }

  ProtobufInstance(const std::string& content, InitInstanceFunc initFunc,
                   GetPacketedSizeFunc getSizeFunc, PackFunc packFunc, UnpackFunc unpackFunc,
                   FreeUnpackedFunc freeUnpackedFunc)
      : init_(initFunc),
        get_packed_size_(getSizeFunc),
        pack_(packFunc),
        unpack_(unpackFunc),
        free_unpacked_(freeUnpackedFunc),
        instance_(nullptr) {
    UnpackInstance(content);
  }

  ProtobufInstance(ProtobufInstance&& rhs) {
    init_ = rhs.init_;
    get_packed_size_ = rhs.get_packed_size_;
    pack_ = rhs.pack_;
    unpack_ = rhs.unpack_;
    free_unpacked_ = rhs.free_unpacked_;
    instance_ = rhs.instance_;
    wrapper_map_ = std::move(rhs.wrapper_map_);
    rhs.instance_ = nullptr;
  }

  ProtobufInstance& operator=(ProtobufInstance&& rhs) {
    if (this != &rhs) {
      init_ = rhs.init_;
      get_packed_size_ = rhs.get_packed_size_;
      pack_ = rhs.pack_;
      unpack_ = rhs.unpack_;
      free_unpacked_ = rhs.free_unpacked_;
      instance_ = rhs.instance_;
      wrapper_map_ = std::move(rhs.wrapper_map_);
      rhs.instance_ = nullptr;
    }
    return *this;
  }

  virtual ~ProtobufInstance() {
    if (instance_) {
      DeinitInstance();
    }
  }

  void reset() {
    InitInstance();
    wrapper_map_.clear();
  }

  pointer_type release() {
    pointer_type tmp = instance_;
    instance_ = nullptr;
    return tmp;
  }

  const pointer_type instance() const { return instance_; }

  pointer_type instance() { return instance_; }

  bool setString(const std::string& property_name, const std::string& value) {
    std::string* str = getString(property_name);
    if (!str) {
      return false;
    }
    *str = value;
    return true;
  }

  std::string* getString(const std::string& property_name) override {
    return const_cast<std::string*>(
        static_cast<const ProtobufInstance&>(*this).getString(property_name));
  }

  const std::string* getString(const std::string& property_name) const {
    if (!instance_ || property_name.empty()) {
      return nullptr;
    }
    const auto descriptor = instance_->base.descriptor;
    for (unsigned int i = 0; i < descriptor->n_fields; ++i) {
      const auto fieldDescriptor = descriptor->fields + i;
      const std::string temp_name = fieldDescriptor->name;
      if (PROTOBUF_C_LABEL_REPEATED == fieldDescriptor->label ||
          !caseInSensStringCompare(temp_name, property_name)) {
        continue;
      }
      switch (fieldDescriptor->type) {
        case PROTOBUF_C_TYPE_STRING: {
          char** member_address = reinterpret_cast<char**>(
              reinterpret_cast<char*>(&instance_->base) + fieldDescriptor->offset);
          return getString(property_name, member_address);
          break;
        }
        case PROTOBUF_C_TYPE_BYTES: {
          ProtobufCBinaryData* member_address = reinterpret_cast<ProtobufCBinaryData*>(
              reinterpret_cast<char*>(&instance_->base) + fieldDescriptor->offset);
          return getString(property_name, member_address);
          break;
        }
        default:
          break;
      }
    }
    return nullptr;
  }

  std::vector<std::string>* getStringList(const std::string& property_name) {
    return const_cast<std::vector<std::string>*>(
        static_cast<const ProtobufInstance&>(*this).getStringList(property_name));
  }

  const std::vector<std::string>* getStringList(const std::string& property_name) const {
    if (!instance_ || property_name.empty()) {
      return nullptr;
    }
    const auto descriptor = instance_->base.descriptor;
    for (int i = 0; i < descriptor->n_fields; i++) {
      const ProtobufCFieldDescriptor* fieldDescriptor = descriptor->fields + i;
      const std::string temp_name = fieldDescriptor->name;
      if (PROTOBUF_C_LABEL_REPEATED != fieldDescriptor->label ||
          !caseInSensStringCompare(temp_name, property_name)) {
        continue;
      }
      switch (fieldDescriptor->type) {
        case PROTOBUF_C_TYPE_STRING: {
          char*** member = reinterpret_cast<char***>(reinterpret_cast<char*>(&instance_->base) +
                                                     fieldDescriptor->offset);
          size_t* qmember = reinterpret_cast<size_t*>(reinterpret_cast<char*>(&instance_->base) +
                                                      fieldDescriptor->quantifier_offset);
          return getStringList(property_name, qmember, member);
          break;
        }
        case PROTOBUF_C_TYPE_BYTES: {
          ProtobufCBinaryData** member = reinterpret_cast<ProtobufCBinaryData**>(
              reinterpret_cast<char*>(&instance_->base) + fieldDescriptor->offset);
          size_t* qmember = reinterpret_cast<size_t*>(reinterpret_cast<char*>(&instance_->base) +
                                                      fieldDescriptor->quantifier_offset);
          return getStringList(property_name, qmember, member);
          break;
        }
        default:
          break;
      }
    }
    return nullptr;
  }

  size_t PackInstance(std::string& content) override {
    PrePack();
    size_t size = get_packed_size_(instance_);
    if (size == 0) {
      return 0;
    }
    content.resize(size);
    pack_(instance_, reinterpret_cast<uint8_t*>(&content[0]));
    return size;
  }

  void UnpackInstance(const std::string& content) {
    wrapper_map_.clear();
    if (content.empty()) {
      InitInstance();
      return;
    } else if (instance_) {
      DeinitInstance();
    }
    const uint8_t* buf = reinterpret_cast<const uint8_t*>(&content[0]);
    instance_ = unpack_(nullptr, content.size(), buf);
  }

  /* update data in wrapper to instance, prepare to pack; */
  void PrePack(const std::string& property_name = std::string()) {
    if (property_name.empty()) {
      for (auto& p : wrapper_map_) {
        p.second.Pack();
      }
    } else {
      ProtobufWrapper* wrapper = getWrapper(property_name);
      if (wrapper) {
        wrapper->Pack();
      }
    }
  }

 private:
  void InitInstance() {
    if (instance_) {
      DeinitInstance();
    }
    instance_ = (pointer_type)malloc(sizeof(value_type));
    init_(instance_);
  }

  void DeinitInstance() {
    free_unpacked_(instance_, nullptr);
    instance_ = nullptr;
  }

  ProtobufWrapper* getWrapper(const std::string& name) {
    auto iter = wrapper_map_.find(name);
    if (iter != wrapper_map_.end()) {
      return &iter->second;
    }
    return nullptr;
  }

  const ProtobufWrapper* getWrapper(const std::string& name) const {
    auto iter = wrapper_map_.find(name);
    if (iter != wrapper_map_.end()) {
      return &iter->second;
    }
    return nullptr;
  }

  void BindString(const std::string& property_name, char** string_address) const {
    if (property_name.empty() || !string_address) {
      return;
    }
    auto iter = wrapper_map_.find(property_name);
    if (iter != wrapper_map_.end()) {
      return;
    }
    // use piecewise_construct to construct in order to avoid copy construct
    wrapper_map_.emplace(std::piecewise_construct, std::make_tuple(property_name),
                         std::make_tuple(string_address));
  }

  void BindString(const std::string& property_name, ProtobufCBinaryData* bytes_address) const {
    if (property_name.empty() || !bytes_address) {
      return;
    }
    auto iter = wrapper_map_.find(property_name);
    if (iter != wrapper_map_.end()) {
      return;
    }
    // use piecewise_construct to construct in order to avoid copy construct
    wrapper_map_.emplace(std::piecewise_construct, std::make_tuple(property_name),
                         std::make_tuple(bytes_address));
  }

  void BindStringList(const std::string& property_name, size_t* size_address,
                      char*** string_vector_address) const {
    if (property_name.empty() || !size_address || !string_vector_address) {
      return;
    }
    auto iter = wrapper_map_.find(property_name);
    if (iter != wrapper_map_.end()) {
      return;
    }
    wrapper_map_.emplace(std::piecewise_construct, std::make_tuple(property_name),
                         std::make_tuple(size_address, string_vector_address));
  }

  void BindStringList(const std::string& property_name, size_t* size_address,
                      ProtobufCBinaryData** bytes_vector_address) const {
    if (property_name.empty() || !size_address || !bytes_vector_address) {
      return;
    }
    auto iter = wrapper_map_.find(property_name);
    if (iter != wrapper_map_.end()) {
      return;
    }
    wrapper_map_.emplace(std::piecewise_construct, std::make_tuple(property_name),
                         std::make_tuple(size_address, bytes_vector_address));
  }

  std::string* getString(const std::string& property_name, char** string_address) {
    return const_cast<std::string*>(
        static_cast<const ProtobufInstance&>(*this).getString(property_name, string_address));
  }

  const std::string* getString(const std::string& property_name, char** string_address) const {
    if (property_name.empty()) {
      return nullptr;
    }
    const ProtobufWrapper* wrapper = getWrapper(property_name);
    if (wrapper) {
      return wrapper->getString();
    } else if (string_address) {
      BindString(property_name, string_address);
      return getWrapper(property_name)->getString();
    }
    return nullptr;
  }

  std::string* getString(const std::string& property_name, ProtobufCBinaryData* bytes_address) {
    return const_cast<std::string*>(
        static_cast<const ProtobufInstance&>(*this).getString(property_name, bytes_address));
  }

  const std::string* getString(const std::string& property_name,
                               ProtobufCBinaryData* bytes_address) const {
    if (property_name.empty()) {
      return nullptr;
    }
    const ProtobufWrapper* wrapper = getWrapper(property_name);
    if (wrapper) {
      return wrapper->getString();
    } else if (bytes_address) {
      BindString(property_name, bytes_address);
      return getWrapper(property_name)->getString();
    }
    return nullptr;
  }

  std::vector<std::string>* getStringList(const std::string& property_name, size_t* size_address,
                                          char*** string_vector_address) {
    return const_cast<std::vector<std::string>*>(
        static_cast<const ProtobufInstance&>(*this).getStringList(property_name, size_address,
                                                                  string_vector_address));
  }

  const std::vector<std::string>* getStringList(const std::string& property_name,
                                                size_t* size_address,
                                                char*** string_vector_address) const {
    if (property_name.empty()) {
      return nullptr;
    }
    const ProtobufWrapper* wrapper = getWrapper(property_name);
    if (wrapper) {
      return wrapper->getStringList();
    } else if (size_address && string_vector_address) {
      BindStringList(property_name, size_address, string_vector_address);
      return getWrapper(property_name)->getStringList();
    }
    return nullptr;
  }

  std::vector<std::string>* getStringList(const std::string& property_name, size_t* size_address,
                                          ProtobufCBinaryData** bytes_vector_address) {
    return const_cast<std::vector<std::string>*>(
        static_cast<const ProtobufInstance&>(*this).getStringList(property_name, size_address,
                                                                  bytes_vector_address));
  }

  const std::vector<std::string>* getStringList(const std::string& property_name,
                                                size_t* size_address,
                                                ProtobufCBinaryData** bytes_vector_address) const {
    if (property_name.empty()) {
      return nullptr;
    }
    const ProtobufWrapper* wrapper = getWrapper(property_name);
    if (wrapper) {
      return wrapper->getStringList();
    } else if (size_address && bytes_vector_address) {
      BindStringList(property_name, size_address, bytes_vector_address);
      return getWrapper(property_name)->getStringList();
    }
    return nullptr;
  }

  static bool compareChar(const char& c1, const char& c2) {
    if (c1 == c2) {
      return true;
    } else if (std::toupper(c1) == std::toupper(c2)) {
      return true;
    }
    return false;
  }

  static bool caseInSensStringCompare(const std::string& str1, const std::string& str2) {
    return ((str1.size() == str2.size()) &&
            std::equal(str1.begin(), str1.end(), str2.begin(), &compareChar));
  }

  InitInstanceFunc init_;
  GetPacketedSizeFunc get_packed_size_;
  PackFunc pack_;
  UnpackFunc unpack_;
  FreeUnpackedFunc free_unpacked_;

  pointer_type instance_;
  mutable std::map<std::string, ProtobufWrapper> wrapper_map_;
};

}  // namespace base
}  // namespace agora
