// Copyright (c) 2018 Agora.io, Inc.
// Author: jiqi@agora.io (Qi Ji) APR 2018
// encryptor & decryptor of CDS result JSON string

#pragma once  // NOLINT(build/header_guard)

#include <string>

namespace agora {
namespace cds {
namespace cipher {

enum CipherMethod {
  kCipherMethod1 = 1,
};

class Cipher {
 public:
  explicit Cipher(uint16_t method);

  int Encrypt(std::string& text);
  int Decrypt(std::string& text);

 private:
  uint16_t method_;
  std::string key_;

 private:
  Cipher() = delete;

  void DoEncrypt1(std::string& text);
  void DoDecrypt1(std::string& text);
};

inline Cipher::Cipher(uint16_t method) : method_(method) {
  switch (method_) {
  case kCipherMethod1:
    key_ = "Cds@123";
    break;
  default:
    key_ = "";
    break;
  }
}

inline int Cipher::Encrypt(std::string& text) {
  switch (method_) {
  case kCipherMethod1:
    DoEncrypt1(text);
    break;
  default:
    return -1;
  }
  return 0;
}

inline int Cipher::Decrypt(std::string& text) {
  switch (method_) {
  case kCipherMethod1:
    DoDecrypt1(text);
    break;
  default:
    return -1;
  }
  return 0;
}

inline void Cipher::DoEncrypt1(std::string& text) {
  if (key_.empty()) return;

  size_t i = 0;
  for (char &c : text) {
    c ^= key_[i];
    c ^= 1 << 7;
    i = (i == key_.length()-1) ? 0 : i+1;
  }
}

inline void Cipher::DoDecrypt1(std::string& text) {
  if (key_.empty()) return;

  size_t i = 0;
  for (char &c : text) {
    c ^= 1 << 7;
    c ^= key_[i];
    i = (i == key_.length()-1) ? 0 : i+1;
  }
}

}  // namespace cipher
}  // namespace cds
}  // namespace agora
