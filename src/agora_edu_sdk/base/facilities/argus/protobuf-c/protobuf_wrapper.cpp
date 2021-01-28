//
//  protobuf_wrapper.cpp
//  agora_rtc_sdk
//
//  Created by Yao Yuan on 2019/4/3.
//

#include "protobuf_wrapper.h"
#include "base/AgoraBase.h"

namespace agora {
namespace base {

ProtobufWrapper::ProtobufWrapper(size_t* size_address, char*** string_vector_address)
    : size_address_(size_address),
      string_vector_address_(string_vector_address),
      string_address_(nullptr),
      bytes_address_(nullptr),
      bytes_vector_address_(nullptr) {
  if (size_address && *size_address && string_vector_address && *string_vector_address) {
    size_t size = *size_address;
    for (int i = 0; i < size; ++i) {
      string_vector_.emplace_back((*string_vector_address_)[i]);
    }
  }
}

ProtobufWrapper::ProtobufWrapper(char** string_address)
    : size_address_(nullptr),
      string_address_(string_address),
      string_vector_address_(nullptr),
      bytes_address_(nullptr),
      bytes_vector_address_(nullptr) {
  if (string_address && *string_address) {
    string_.assign(*string_address);
  }
}

ProtobufWrapper::ProtobufWrapper(ProtobufCBinaryData* bytes_address)
    : size_address_(nullptr),
      string_address_(nullptr),
      string_vector_address_(nullptr),
      bytes_address_(bytes_address),
      bytes_vector_address_(nullptr) {
  if (bytes_address && bytes_address->len > 0 && bytes_address->data) {
    string_.assign(reinterpret_cast<char*>(bytes_address->data), bytes_address->len);
  }
}

ProtobufWrapper::ProtobufWrapper(size_t* size_address, ProtobufCBinaryData** bytes_vector_address)
    : size_address_(size_address),
      string_vector_address_(nullptr),
      string_address_(nullptr),
      bytes_address_(nullptr),
      bytes_vector_address_(bytes_vector_address) {
  if (size_address && *size_address && bytes_vector_address_ && *bytes_vector_address_) {
    for (int i = 0; i < *size_address; ++i) {
      string_vector_.emplace_back(reinterpret_cast<char*>((*bytes_vector_address_)[i].data),
                                  (*bytes_vector_address_)[i].len);
    }
  }
}

ProtobufWrapper::~ProtobufWrapper() {}

/* When pack, free the old data and write new data to the instance address */
void ProtobufWrapper::Pack() {
  if (string_vector_address_ && size_address_) {
    int old_size = *size_address_;
    for (int i = 0; i < old_size; ++i) {
      free((*string_vector_address_)[i]);
      (*string_vector_address_)[i] = nullptr;
    }
    free(*string_vector_address_);
    *string_vector_address_ = nullptr;
    *size_address_ = 0;

    int size = string_vector_.size();
    if (size == 0) {
      return;
    }
    *size_address_ = size;
    *string_vector_address_ = (char**)malloc(sizeof(char*) * size);
    for (int i = 0; i < size; ++i) {
      TransformStringToCharPtrDeep(&(*string_vector_address_)[i], string_vector_[i]);
    }
  } else if (string_address_) {
    if (!isProtobufEmptyString(*string_address_)) {
      free(*string_address_);
      *string_address_ = nullptr;
    }
    TransformStringToCharPtrDeep(string_address_, string_);
  } else if (bytes_address_) {
    free(bytes_address_->data);
    bytes_address_->data = nullptr;
    bytes_address_->len = 0;
    TransformStringToBytes(bytes_address_, string_);
  } else if (bytes_vector_address_ && size_address_) {
    for (int i = 0; i < *size_address_; ++i) {
      free((*bytes_vector_address_)[i].data);
      (*bytes_vector_address_)[i].data = nullptr;
      (*bytes_vector_address_)[i].len = 0;
    }
    free(*bytes_vector_address_);
    *bytes_vector_address_ = nullptr;
    *size_address_ = 0;

    int size = string_vector_.size();
    if (size == 0) {
      return;
    }
    *size_address_ = size;
    *bytes_vector_address_ = (ProtobufCBinaryData*)malloc(sizeof(ProtobufCBinaryData) * size);
    for (int i = 0; i < size; ++i) {
      TransformStringToBytes(&(*bytes_vector_address_)[i], string_vector_[i]);
    }
  }
}

void ProtobufWrapper::TransformStringToCharPtrDeep(char** output, const std::string& input) {
  size_t size = input.size();
  *output = (char*)malloc(sizeof(char) * (size + 1));
  input.copy(*output, size);
  (*output)[size] = '\0';
}

void ProtobufWrapper::TransformStringToBytes(ProtobufCBinaryData* output,
                                             const std::string& input) {
  if (input.empty()) {
    return;
  }
  size_t size = input.size();
  output->len = size;
  output->data = (uint8_t*)malloc(sizeof(uint8_t) * size);
  input.copy(reinterpret_cast<char*>(output->data), size);
}

bool ProtobufWrapper::isProtobufCBinaryDataEmpty(const ProtobufCBinaryData& binaryData) {
  return binaryData.len == 0 || !binaryData.data;
}

}  // namespace base
}  // namespace agora
