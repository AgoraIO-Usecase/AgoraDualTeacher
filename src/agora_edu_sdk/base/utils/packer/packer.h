#pragma once

#include <cstdint>
#include <cstring>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "declare_struct.h"

namespace agora {
namespace commons {

class packer {
  enum { PACKET_BUFFER_SIZE = 1024, PACKET_BUFFER_SIZE_MAX = 64 * 1024 };

 public:
  packer() : buffer_(PACKET_BUFFER_SIZE), length_(0), position_(2) {}
  ~packer() {}

 public:
  packer& pack() {
    length_ = position_;
    position_ = 0;
    *this << length_;
    position_ = length_;
    return *this;
  }

  void reset() {
    length_ = 0;
    position_ = 2;
  }

  void write(uint16_t val, uint16_t position) {
    check_size(sizeof(val), position);
    ::memcpy(&buffer_[0] + position, &val, sizeof(val));
  }

  void write(uint32_t val, uint16_t position) {
    check_size(sizeof(val), position);
    ::memcpy(&buffer_[0] + position, &val, sizeof(val));
  }

  void push(void* val) {
    check_size(sizeof(val), position_);
    ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
    position_ += sizeof(val);
  }

  void push(double val) {
    check_size(sizeof(val), position_);
    ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
    position_ += sizeof(val);
  }

  void push(float val) {
    check_size(sizeof(val), position_);
    ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
    position_ += sizeof(val);
  }

  void push(uint64_t val) {
    check_size(sizeof(val), position_);
    ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
    position_ += sizeof(val);
  }

  void push(uint32_t val) {
    check_size(sizeof(val), position_);
    ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
    position_ += sizeof(val);
  }

  void push(uint16_t val) {
    check_size(sizeof(val), position_);
    ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
    position_ += sizeof(val);
  }

  void push(uint8_t val) {
    check_size(sizeof(val), position_);
    ::memcpy(&buffer_[0] + position_, &val, sizeof(val));
    position_ += sizeof(val);
  }

  void push(const std::string& val) {
    push(static_cast<uint16_t>(val.length()));

    size_t length = val.length();
    check_size(length, position_);
    if (length > 0) {
      ::memcpy(&buffer_[0] + position_, val.data(), length);
      position_ += length;
    }
  }

  packer& push(const void* data, size_t length) {
    check_size(length, position_);
    if (length > 0) {
      ::memcpy(&buffer_[0] + position_, data, length);
      position_ += length;
    }
    return *this;
  }

  const char* buffer() const { return &buffer_[0]; }
  size_t length() const { return length_; }
  std::string body() const { return std::string(&buffer_[0] + 2, length_ - 2); }

  void check_size(size_t more, uint16_t position) {
    if (buffer_.size() - position < more) {
      size_t new_size = buffer_.size() * 4;
      if (new_size - position < more) new_size = more + position;
      if (new_size > PACKET_BUFFER_SIZE_MAX) {
#if AGORARTC_HAS_EXCEPTION
        throw std::overflow_error("packer buffer overflow!");
#endif
      }
      buffer_.resize(new_size);
    }
  }

  packer& operator<<(void* v) {
    push(v);
    return *this;
  }
  packer& operator<<(double v) {
    push(v);
    return *this;
  }
  packer& operator<<(float v) {
    push(v);
    return *this;
  }
  packer& operator<<(uint64_t v) {
    push(v);
    return *this;
  }

  packer& operator<<(uint32_t v) {
    push(v);
    return *this;
  }

  packer& operator<<(uint16_t v) {
    push(v);
    return *this;
  }
  packer& operator<<(uint8_t v) {
    push(v);
    return *this;
  }

  packer& operator<<(bool v) {
    push((uint8_t)v);
    return *this;
  }

  packer& operator<<(int64_t v) {
    push(static_cast<uint64_t>(v));
    return *this;
  }

  packer& operator<<(int32_t v) {
    push(static_cast<uint32_t>(v));
    return *this;
  }

  packer& operator<<(int16_t v) {
    push(static_cast<uint16_t>(v));
    return *this;
  }
  packer& operator<<(int8_t v) {
    push(static_cast<uint8_t>(v));
    return *this;
  }

  packer& operator<<(const std::string& v) {
    push(v);
    return *this;
  }

  template <typename T>
  packer& operator<<(const std::vector<T>& v) {
    uint16_t count = v.size();
    *this << count;
    for (uint16_t i = 0; i < count; i++) {
      *this << v[i];
    }
    return *this;
  }

  template <typename T>
  packer& operator<<(const std::list<T>& v) {
    uint16_t count = v.size();
    *this << count;
    for (auto i = v.cbegin(); i != v.cend(); ++i) {
      *this << *i;
    }
    return *this;
  }

  template <typename T>
  packer& operator<<(const std::set<T>& v) {
    uint16_t count = v.size();
    *this << count;
    for (const T& x : v) {
      *this << x;
    }
    return *this;
  }

  template <typename T>
  packer& operator<<(const std::unique_ptr<T>& v) {
    return *this;
  }

  template <typename K, typename V>
  packer& operator<<(const std::pair<K, V>& p) {
    *this << p.first << p.second;
    return *this;
  }

  template <typename K, typename V>
  packer& operator<<(const std::map<K, V>& v) {
    uint16_t count = v.size();
    *this << count;
    for (const typename std::map<K, V>::value_type& x : v) {
      *this << x;
    }
    return *this;
  }

  template <typename K, typename V>
  packer& operator<<(const std::unordered_map<K, V>& v) {
    uint16_t count = v.size();
    *this << count;
    for (const typename std::unordered_map<K, V>::value_type& x : v) {
      *this << x;
    }
    return *this;
  }

 private:
  std::vector<char> buffer_;
  uint16_t length_;
  uint16_t position_;
};

class unpacker {
 public:
  unpacker(const char* buf, size_t len, bool copy = false)
      : buffer_(NULL), length_(len), position_(0), copy_(copy) {
    if (copy_) {
      buffer_ = new char[len];
      ::memcpy(buffer_, buf, len);
    } else {
      buffer_ = const_cast<char*>(buf);
    }
  }
  ~unpacker() {
    if (copy_) {
      delete[] buffer_;
    }
  }

 public:
  unpacker& rewind() {
    position_ = 2;
    return *this;
  }

  void reset() { position_ = 0; }

  void write(uint16_t val, uint16_t position) {
    check_size(sizeof(val), position);
    ::memcpy(buffer_ + position, &val, sizeof(val));
  }

  void* pop_ptr() {
    void* v = 0;
    check_size(sizeof(v), position_);
    ::memcpy(&v, buffer_ + position_, sizeof(v));
    position_ += sizeof(v);
    return v;
  }
  double pop_double() {
    double v = 0;
    check_size(sizeof(v), position_);
    ::memcpy(&v, buffer_ + position_, sizeof(v));
    position_ += sizeof(v);
    return v;
  }
  float pop_float() {
    float v = 0;
    check_size(sizeof(v), position_);
    ::memcpy(&v, buffer_ + position_, sizeof(v));
    position_ += sizeof(v);
    return v;
  }
  uint64_t pop_uint64() {
    uint64_t v = 0;
    check_size(sizeof(v), position_);
    ::memcpy(&v, buffer_ + position_, sizeof(v));
    position_ += sizeof(v);
    return v;
  }

  uint32_t pop_uint32() {
    uint32_t v = 0;
    check_size(sizeof(v), position_);
    ::memcpy(&v, buffer_ + position_, sizeof(v));
    position_ += sizeof(v);
    return v;
  }

  uint16_t pop_uint16() {
    uint16_t v = 0;
    check_size(sizeof(v), position_);
    ::memcpy(&v, buffer_ + position_, sizeof(v));
    position_ += sizeof(v);
    return v;
  }

  uint8_t pop_uint8() {
    uint8_t v = 0;
    check_size(sizeof(v), position_);
    ::memcpy(&v, buffer_ + position_, sizeof(v));
    position_ += sizeof(v);
    return v;
  }

  std::string pop_string() {
    uint16_t length = pop_uint16();
    check_size(length, position_);
    std::string s = std::string(buffer_ + position_, length);
    position_ += length;

    return s;
  }

  const char* buffer() const { return buffer_; }

  size_t length() const { return length_; }

  void check_size(size_t more, uint16_t position) const {
    if (static_cast<size_t>(length_ - position) < more) {
#if AGORARTC_HAS_EXCEPTION
      throw std::overflow_error("unpacker buffer overflow!");
#endif
    }
  }

  unpacker& operator>>(void*& v) {
    v = pop_ptr();
    return *this;
  }
  unpacker& operator>>(double& v) {
    v = pop_double();
    return *this;
  }
  unpacker& operator>>(float& v) {
    v = pop_float();
    return *this;
  }
  unpacker& operator>>(uint64_t& v) {
    v = pop_uint64();
    return *this;
  }

  unpacker& operator>>(uint32_t& v) {
    v = pop_uint32();
    return *this;
  }
  unpacker& operator>>(uint16_t& v) {
    v = pop_uint16();
    return *this;
  }
  unpacker& operator>>(uint8_t& v) {
    v = pop_uint8();
    return *this;
  }

  unpacker& operator>>(bool& v) {
    v = (pop_uint8() != 0);
    return *this;
  }

  unpacker& operator>>(int64_t& v) {
    v = static_cast<int64_t>(pop_uint64());
    return *this;
  }
  unpacker& operator>>(int32_t& v) {
    v = static_cast<int32_t>(pop_uint32());
    return *this;
  }
  unpacker& operator>>(int16_t& v) {
    v = static_cast<int16_t>(pop_uint16());
    return *this;
  }
  unpacker& operator>>(int8_t& v) {
    v = static_cast<int8_t>(pop_uint8());
    return *this;
  }
  unpacker& operator>>(std::string& v) {
    v = pop_string();
    return *this;
  }

  template <typename T>
  unpacker& operator>>(std::vector<T>& v) {
    uint16_t count = pop_uint16();
    for (uint16_t i = 0; i < count; i++) {
      T t;
      *this >> t;
      v.push_back(std::move(t));
    }
    return *this;
  }

  template <typename T1, typename T2>
  unpacker& operator>>(std::pair<T1, T2>& v) {
    *this >> v.first;
    *this >> v.second;
    return *this;
  }

  template <typename T>
  unpacker& operator>>(std::list<T>& v) {
    uint16_t count = pop_uint16();
    for (uint16_t i = 0; i < count; i++) {
      T t;
      *this >> t;
      v.push_back(std::move(t));
    }
    return *this;
  }

  template <typename T>
  unpacker& operator>>(std::set<T>& v) {
    uint16_t count = pop_uint16();
    for (uint16_t i = 0; i < count; i++) {
      T t;
      *this >> t;
      v.insert(std::move(t));
    }
    return *this;
  }

  template <typename T>
  unpacker& operator>>(std::unique_ptr<T>& v) {
    return *this;
  }

  template <typename K, typename V>
  unpacker& operator>>(std::map<K, V>& x) {
    uint16_t count = pop_uint16();
    for (uint16_t i = 0; i < count; i++) {
      K k;
      V v;
      *this >> k >> v;
      x.insert(std::make_pair(k, v));
    }
    return *this;
  }

  template <typename K, typename V>
  unpacker& operator>>(std::unordered_map<K, V>& x) {
    uint16_t count = pop_uint16();
    for (uint16_t i = 0; i < count; i++) {
      K k;
      V v;
      *this >> k >> v;
      x.insert(std::make_pair(k, v));
    }
    return *this;
  }

 private:
  char* buffer_;
  uint16_t length_;
  uint16_t position_;
  bool copy_;
};

#define DECLARE_PACKABLE_1_START(name, type1, name1)    \
  DECLARE_STRUCT_1_START(name, type1, name1)            \
  friend packer& operator<<(packer& p, const name& x) { \
    p << x.name1;                                       \
    return p;                                           \
  }                                                     \
  friend unpacker& operator>>(unpacker& p, name& x) {   \
    p >> x.name1;                                       \
    return p;                                           \
  }
#define DECLARE_PACKABLE_1(name, type1, name1) \
  DECLARE_PACKABLE_1_START(name, type1, name1) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_2_START(name, type1, name1, type2, name2) \
  DECLARE_STRUCT_2_START(name, type1, name1, type2, name2)         \
  friend packer& operator<<(packer& p, const name& x) {            \
    p << x.name1 << x.name2;                                       \
    return p;                                                      \
  }                                                                \
  friend unpacker& operator>>(unpacker& p, name& x) {              \
    p >> x.name1 >> x.name2;                                       \
    return p;                                                      \
  }
#define DECLARE_PACKABLE_2(name, type1, name1, type2, name2) \
  DECLARE_PACKABLE_2_START(name, type1, name1, type2, name2) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_3_START(name, type1, name1, type2, name2, type3, name3) \
  DECLARE_STRUCT_3_START(name, type1, name1, type2, name2, type3, name3)         \
  friend packer& operator<<(packer& p, const name& x) {                          \
    p << x.name1 << x.name2 << x.name3;                                          \
    return p;                                                                    \
  }                                                                              \
  friend unpacker& operator>>(unpacker& p, name& x) {                            \
    p >> x.name1 >> x.name2 >> x.name3;                                          \
    return p;                                                                    \
  }
#define DECLARE_PACKABLE_3(name, type1, name1, type2, name2, type3, name3) \
  DECLARE_PACKABLE_3_START(name, type1, name1, type2, name2, type3, name3) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_4_START(name, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_STRUCT_4_START(name, type1, name1, type2, name2, type3, name3, type4, name4)         \
  friend packer& operator<<(packer& p, const name& x) {                                        \
    p << x.name1 << x.name2 << x.name3 << x.name4;                                             \
    return p;                                                                                  \
  }                                                                                            \
  friend unpacker& operator>>(unpacker& p, name& x) {                                          \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4;                                             \
    return p;                                                                                  \
  }
#define DECLARE_PACKABLE_4(name, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_PACKABLE_4_START(name, type1, name1, type2, name2, type3, name3, type4, name4) \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                 type5, name5)                                                 \
  DECLARE_STRUCT_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,  \
                         name5)                                                                \
  friend packer& operator<<(packer& p, const name& x) {                                        \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5;                                  \
    return p;                                                                                  \
  }                                                                                            \
  friend unpacker& operator>>(unpacker& p, name& x) {                                          \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5;                                  \
    return p;                                                                                  \
  }
#define DECLARE_PACKABLE_5(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5)                                                               \
  DECLARE_PACKABLE_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5)                                                               \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                 type5, name5)                                                 \
  DECLARE_STRUCT_5_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,  \
                         name5)                                                                \
  friend packer& operator<<(packer& p, const name& x) {                                        \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5;                                  \
    return p;                                                                                  \
  }                                                                                            \
  friend unpacker& operator>>(unpacker& p, name& x) {                                          \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5;                                  \
    return p;                                                                                  \
  }
#define DECLARE_PACKABLE_6_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                 type5, name5, type6, name6)                                   \
  DECLARE_STRUCT_6_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,  \
                         name5, type6, name6)                                                  \
  friend packer& operator<<(packer& p, const name& x) {                                        \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6;                       \
    return p;                                                                                  \
  }                                                                                            \
  friend unpacker& operator>>(unpacker& p, name& x) {                                          \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6;                       \
    return p;                                                                                  \
  }
#define DECLARE_PACKABLE_6(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5, type6, name6)                                                 \
  DECLARE_PACKABLE_6_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5, type6, name6)                                                 \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_7_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                 type5, name5, type6, name6, type7, name7)                     \
  DECLARE_STRUCT_7_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,  \
                         name5, type6, name6, type7, name7)                                    \
  friend packer& operator<<(packer& p, const name& x) {                                        \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7;            \
    return p;                                                                                  \
  }                                                                                            \
  friend unpacker& operator>>(unpacker& p, name& x) {                                          \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7;            \
    return p;                                                                                  \
  }
#define DECLARE_PACKABLE_7(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5, type6, name6, type7, name7)                                   \
  DECLARE_PACKABLE_7_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5, type6, name6, type7, name7)                                   \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_8_START(name, type1, name1, type2, name2, type3, name3, type4, name4, \
                                 type5, name5, type6, name6, type7, name7, type8, name8)       \
  DECLARE_STRUCT_8_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,  \
                         name5, type6, name6, type7, name7, type8, name8)                      \
  friend packer& operator<<(packer& p, const name& x) {                                        \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8; \
    return p;                                                                                  \
  }                                                                                            \
  friend unpacker& operator>>(unpacker& p, name& x) {                                          \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8; \
    return p;                                                                                  \
  }
#define DECLARE_PACKABLE_8(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5, type6, name6, type7, name7, type8, name8)                     \
  DECLARE_PACKABLE_8_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5, type6, name6, type7, name7, type8, name8)                     \
  DECLARE_STRUCT_END
#define DECLARE_PACKABLE_9_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                 type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                 name9)                                                          \
  DECLARE_STRUCT_9_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                         name5, type6, name6, type7, name7, type8, name8, type9, name9)          \
  friend packer& operator<<(packer& p, const name& x) {                                          \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8    \
      << x.name9;                                                                                \
    return p;                                                                                    \
  }                                                                                              \
  friend unpacker& operator>>(unpacker& p, name& x) {                                            \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >> \
        x.name9;                                                                                 \
    return p;                                                                                    \
  }
#define DECLARE_PACKABLE_9(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5, type6, name6, type7, name7, type8, name8, type9, name9)       \
  DECLARE_PACKABLE_9_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5, \
                           name5, type6, name6, type7, name7, type8, name8, type9, name9)       \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_10_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                  name9, type10, name10)                                         \
  DECLARE_STRUCT_10_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10)                                                                \
  friend packer& operator<<(packer& p, const name& x) {                                          \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8    \
      << x.name9 << x.name10;                                                                    \
    return p;                                                                                    \
  }                                                                                              \
  friend unpacker& operator>>(unpacker& p, name& x) {                                            \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >> \
        x.name9 >> x.name10;                                                                     \
    return p;                                                                                    \
  }
#define DECLARE_PACKABLE_10(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10)                                                                \
  DECLARE_PACKABLE_10_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10)                                                                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_11_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                  name9, type10, name10, type11, name11)                         \
  DECLARE_STRUCT_11_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11)                                                \
  friend packer& operator<<(packer& p, const name& x) {                                          \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8    \
      << x.name9 << x.name10 << x.name11;                                                        \
    return p;                                                                                    \
  }                                                                                              \
  friend unpacker& operator>>(unpacker& p, name& x) {                                            \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >> \
        x.name9 >> x.name10 >> x.name11;                                                         \
    return p;                                                                                    \
  }
#define DECLARE_PACKABLE_11(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11)                                                \
  DECLARE_PACKABLE_11_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11)                                                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_12_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                  name9, type10, name10, type11, name11, type12, name12)         \
  DECLARE_STRUCT_12_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11, type12, name12)                                \
  friend packer& operator<<(packer& p, const name& x) {                                          \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8    \
      << x.name9 << x.name10 << x.name11 << x.name12;                                            \
    return p;                                                                                    \
  }                                                                                              \
  friend unpacker& operator>>(unpacker& p, name& x) {                                            \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >> \
        x.name9 >> x.name10 >> x.name11 >> x.name12;                                             \
    return p;                                                                                    \
  }
#define DECLARE_PACKABLE_12(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12)                                \
  DECLARE_PACKABLE_12_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12)                                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_13_START(name, type1, name1, type2, name2, type3, name3, type4, name4,  \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9, \
                                  name9, type10, name10, type11, name11, type12, name12, type13, \
                                  name13)                                                        \
  DECLARE_STRUCT_13_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                          name10, type11, name11, type12, name12, type13, name13)                \
  friend packer& operator<<(packer& p, const name& x) {                                          \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8    \
      << x.name9 << x.name10 << x.name11 << x.name12 << x.name13;                                \
    return p;                                                                                    \
  }                                                                                              \
  friend unpacker& operator>>(unpacker& p, name& x) {                                            \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >> \
        x.name9 >> x.name10 >> x.name11 >> x.name12 >> x.name13;                                 \
    return p;                                                                                    \
  }
#define DECLARE_PACKABLE_13(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13)                \
  DECLARE_PACKABLE_13_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13)                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_14_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                  name9, type10, name10, type11, name11, type12, name12, type13,  \
                                  name13, type14, name14)                                         \
  DECLARE_STRUCT_14_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14) \
  friend packer& operator<<(packer& p, const name& x) {                                           \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8     \
      << x.name9 << x.name10 << x.name11 << x.name12 << x.name13 << x.name14;                     \
    return p;                                                                                     \
  }                                                                                               \
  friend unpacker& operator>>(unpacker& p, name& x) {                                             \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >>  \
        x.name9 >> x.name10 >> x.name11 >> x.name12 >> x.name13 >> x.name14;                      \
    return p;                                                                                     \
  }
#define DECLARE_PACKABLE_14(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14)                                                                \
  DECLARE_PACKABLE_14_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14)                                                                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_15_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                  name9, type10, name10, type11, name11, type12, name12, type13,  \
                                  name13, type14, name14, type15, name15)                         \
  DECLARE_STRUCT_15_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15)                                                         \
  friend packer& operator<<(packer& p, const name& x) {                                           \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8     \
      << x.name9 << x.name10 << x.name11 << x.name12 << x.name13 << x.name14 << x.name15;         \
    return p;                                                                                     \
  }                                                                                               \
  friend unpacker& operator>>(unpacker& p, name& x) {                                             \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >>  \
        x.name9 >> x.name10 >> x.name11 >> x.name12 >> x.name13 >> x.name14 >> x.name15;          \
    return p;                                                                                     \
  }
#define DECLARE_PACKABLE_15(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15)                                                \
  DECLARE_PACKABLE_15_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15)                                                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_16_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                  name9, type10, name10, type11, name11, type12, name12, type13,  \
                                  name13, type14, name14, type15, name15, type16, name16)         \
  DECLARE_STRUCT_16_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16)                                         \
  friend packer& operator<<(packer& p, const name& x) {                                           \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8     \
      << x.name9 << x.name10 << x.name11 << x.name12 << x.name13 << x.name14 << x.name15          \
      << x.name16;                                                                                \
    return p;                                                                                     \
  }                                                                                               \
  friend unpacker& operator>>(unpacker& p, name& x) {                                             \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >>  \
        x.name9 >> x.name10 >> x.name11 >> x.name12 >> x.name13 >> x.name14 >> x.name15 >>        \
        x.name16;                                                                                 \
    return p;                                                                                     \
  }
#define DECLARE_PACKABLE_16(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15, type16, name16)                                \
  DECLARE_PACKABLE_16_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15, type16, name16)                                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_17_START(                                                                \
    name, type1, name1, type2, name2, type3, name3, type4, name4, type5, name5, type6, name6,     \
    type7, name7, type8, name8, type9, name9, type10, name10, type11, name11, type12, name12,     \
    type13, name13, type14, name14, type15, name15, type16, name16, type17, name17)               \
  DECLARE_STRUCT_17_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17)                         \
  friend packer& operator<<(packer& p, const name& x) {                                           \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8     \
      << x.name9 << x.name10 << x.name11 << x.name12 << x.name13 << x.name14 << x.name15          \
      << x.name16 << x.name17;                                                                    \
    return p;                                                                                     \
  }                                                                                               \
  friend unpacker& operator>>(unpacker& p, name& x) {                                             \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >>  \
        x.name9 >> x.name10 >> x.name11 >> x.name12 >> x.name13 >> x.name14 >> x.name15 >>        \
        x.name16 >> x.name17;                                                                     \
    return p;                                                                                     \
  }
#define DECLARE_PACKABLE_17(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15, type16, name16, type17, name17)                \
  DECLARE_PACKABLE_17_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15, type16, name16, type17, name17)                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_18_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                  name9, type10, name10, type11, name11, type12, name12, type13,  \
                                  name13, type14, name14, type15, name15, type16, name16, type17, \
                                  name17, type18, name18)                                         \
  DECLARE_STRUCT_18_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17, type18, name18)         \
  friend packer& operator<<(packer& p, const name& x) {                                           \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8     \
      << x.name9 << x.name10 << x.name11 << x.name12 << x.name13 << x.name14 << x.name15          \
      << x.name16 << x.name17 << x.name18;                                                        \
    return p;                                                                                     \
  }                                                                                               \
  friend unpacker& operator>>(unpacker& p, name& x) {                                             \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >>  \
        x.name9 >> x.name10 >> x.name11 >> x.name12 >> x.name13 >> x.name14 >> x.name15 >>        \
        x.name16 >> x.name17 >> x.name18;                                                         \
    return p;                                                                                     \
  }
#define DECLARE_PACKABLE_18(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15, type16, name16, type17, name17, type18,        \
                            name18)                                                                \
  DECLARE_PACKABLE_18_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15, type16, name16, type17, name17, type18,        \
                            name18)                                                                \
  DECLARE_STRUCT_END

#define DECLARE_PACKABLE_19_START(name, type1, name1, type2, name2, type3, name3, type4, name4,   \
                                  type5, name5, type6, name6, type7, name7, type8, name8, type9,  \
                                  name9, type10, name10, type11, name11, type12, name12, type13,  \
                                  name13, type14, name14, type15, name15, type16, name16, type17, \
                                  name17, type18, name18, type19, name19)                         \
  DECLARE_STRUCT_19_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,    \
                          name5, type6, name6, type7, name7, type8, name8, type9, name9, type10,  \
                          name10, type11, name11, type12, name12, type13, name13, type14, name14, \
                          type15, name15, type16, name16, type17, name17, type18, name18, type19, \
                          name19)                                                                 \
  friend packer& operator<<(packer& p, const name& x) {                                           \
    p << x.name1 << x.name2 << x.name3 << x.name4 << x.name5 << x.name6 << x.name7 << x.name8     \
      << x.name9 << x.name10 << x.name11 << x.name12 << x.name13 << x.name14 << x.name15          \
      << x.name16 << x.name17 << x.name18 << x.name19;                                            \
    return p;                                                                                     \
  }                                                                                               \
  friend unpacker& operator>>(unpacker& p, name& x) {                                             \
    p >> x.name1 >> x.name2 >> x.name3 >> x.name4 >> x.name5 >> x.name6 >> x.name7 >> x.name8 >>  \
        x.name9 >> x.name10 >> x.name11 >> x.name12 >> x.name13 >> x.name14 >> x.name15 >>        \
        x.name16 >> x.name17 >> x.name18 >> x.name19;                                             \
    return p;                                                                                     \
  }
#define DECLARE_PACKABLE_19(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15, type16, name16, type17, name17, type18,        \
                            name18, type19, name19)                                                \
  DECLARE_PACKABLE_19_START(name, type1, name1, type2, name2, type3, name3, type4, name4, type5,   \
                            name5, type6, name6, type7, name7, type8, name8, type9, name9, type10, \
                            name10, type11, name11, type12, name12, type13, name13, type14,        \
                            name14, type15, name15, type16, name16, type17, name17, type18,        \
                            name18, type19, name19)                                                \
  DECLARE_STRUCT_END

}  // namespace commons
}  // namespace agora
