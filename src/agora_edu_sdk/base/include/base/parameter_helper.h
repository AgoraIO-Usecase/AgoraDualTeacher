//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <base/config_engine.h>
#include "utils/tools/json_util.h"
#include "utils/tools/util.h"

#include <functional>
#include <unordered_set>

namespace agora {
namespace config {
template <>
struct ExternalParameterHelperTypeTraits<any_document_t> {
  static const config::AnyValue::Type AnyValueType = config::AnyValue::JSON;
  static const void* getValue(const config::AnyValue& value) { return value.val_cjson; }
};
}  // namespace config
namespace base {
template <class T>
class InternalParameterHelper {
 public:
  explicit InternalParameterHelper(T&& def) : value_(std::move(def)) {}
  const T& value() { return value_; }
  void set_value(const T& v) { value_ = v; }

 private:
  T value_;
};
namespace detail {
template <class T1, class T2>
inline bool ValueConvertor(const T1&, T2&) {
  return false;
}
template <class T1, class T2>
inline bool ValueConvertor(const T1&, T2&, any_document_t&);
template <class T>
inline bool CheckValueType(const any_value_t&);
template <class T>
inline bool IsSameValue(const T& v1, const T& v2) {
  return v1 == v2;
}
template <>
inline bool CheckValueType<any_value_t>(const any_value_t&) {
  return true;
}
template <>
inline bool ValueConvertor<any_value_t, any_value_t>(const any_value_t& from, any_value_t& to,
                                                     any_document_t& container) {
  commons::json::copy(from, to, container);
  return true;
}

// any_document_t
template <>
inline bool CheckValueType<any_document_t>(const any_value_t&) {
  return true;
}
template <>
inline bool IsSameValue<any_document_t>(const any_document_t& v1, const any_document_t& v2) {
  return false;
}
template <>
inline bool ValueConvertor<any_document_t, any_value_t>(const any_document_t& from, any_value_t& to,
                                                        any_document_t& container) {
  commons::json::copy(from, to, container);
  return true;
}
template <>
inline bool ValueConvertor<any_document_t, any_document_t>(const any_document_t& from,
                                                           any_document_t& to) {
  commons::json::copy(from, to);
  return true;
}
template <>
inline bool ValueConvertor<any_value_t, any_document_t>(const any_value_t& from,
                                                        any_document_t& to) {
  commons::json::copy(from, to);
  return true;
}
template <>
inline bool ValueConvertor<any_document_t, const void*>(const any_document_t& from,
                                                        const void*& to) {
  to = from.getRoot();
  return true;
}

// boolean
template <>
inline bool CheckValueType<bool>(const any_value_t& v) {
  return commons::json::is_bool(v);
}
template <>
inline bool ValueConvertor<any_value_t, bool>(const any_value_t& from, bool& to) {
  commons::json::get_bool(from, to);
  return true;
}
template <>
inline bool ValueConvertor<bool, any_document_t>(const bool& from, any_document_t& to) {
  commons::json::set_bool(to, from);
  return true;
}
template <>
inline bool ValueConvertor<bool, any_value_t>(const bool& from, any_value_t& to,
                                              any_document_t& container) {
  commons::json::set_bool(to, from);
  return true;
}

// std::string
template <>
inline bool CheckValueType<std::string>(const any_value_t& v) {
  return commons::json::is_string(v);
}
template <>
inline bool ValueConvertor<any_value_t, std::string>(const any_value_t& from, std::string& to) {
  commons::json::get_string(from, to);
  return true;
}
template <>
inline bool ValueConvertor<std::string, any_document_t>(const std::string& from,
                                                        any_document_t& to) {
  commons::json::set_string(to, from);
  return true;
}
template <>
inline bool ValueConvertor<std::string, any_value_t>(const std::string& from, any_value_t& to,
                                                     any_document_t& container) {
  commons::json::set_string(to, from, container);
  return true;
}

// int
template <>
inline bool CheckValueType<int>(const any_value_t& v) {
  return commons::json::is_int(v);
}
template <>
inline bool ValueConvertor<any_value_t, int>(const any_value_t& from, int& to) {
  commons::json::get_int(from, to);
  return true;
}
template <>
inline bool ValueConvertor<int, any_document_t>(const int& from, any_document_t& to) {
  commons::json::set_int(to, from);
  return true;
}
template <>
inline bool ValueConvertor<int, any_value_t>(const int& from, any_value_t& to,
                                             any_document_t& container) {
  commons::json::set_int(to, from);
  return true;
}

// double
template <>
inline bool CheckValueType<double>(const any_value_t& v) {
  return commons::json::is_double(v);
}
template <>
inline bool ValueConvertor<any_value_t, double>(const any_value_t& from, double& to) {
  commons::json::get_double(from, to);
  return true;
}
template <>
inline bool ValueConvertor<double, any_document_t>(const double& from, any_document_t& to) {
  commons::json::set_double(to, from);
  return true;
}
template <>
inline bool ValueConvertor<double, any_value_t>(const double& from, any_value_t& to,
                                                any_document_t& container) {
  commons::json::set_double(to, from);
  return true;
}

// uint32_t
template <>
inline bool CheckValueType<uint32_t>(const any_value_t& v) {
  return commons::json::is_uint(v);
}
template <>
inline bool ValueConvertor<any_value_t, uint32_t>(const any_value_t& from, uint32_t& to) {
  commons::json::get_uint(from, to);
  return true;
}
template <>
inline bool ValueConvertor<uint32_t, any_document_t>(const uint32_t& from, any_document_t& to) {
  commons::json::set_uint32(to, from);
  return true;
}
template <>
inline bool ValueConvertor<uint32_t, any_value_t>(const uint32_t& from, any_value_t& to,
                                                  any_document_t& container) {
  commons::json::set_uint32(to, from);
  return true;
}
#if 0
// int64_t
template<> inline bool CheckValueType<int64_t>(const any_value_t& v) { return v.IsInt64(); }

template<>
inline void ValueConvertor<any_value_t, int64_t>(const any_value_t& from, int64_t& to) {
  to = from.GetInt64();
}

template<>
inline void ValueConvertor<int64_t, any_document_t>(const int64_t& from, any_document_t& to) {
  to.SetInt64(from);
}

template<>
inline void ValueConvertor<int64_t, any_value_t>(const int64_t& from,
  any_value_t& to, any_document_t& container) {
  to.SetInt64(from);
}

// uint64_t
template<> inline
bool CheckValueType<uint64_t>(const any_value_t& v) {
  return v.IsUint64();
}

template<>
inline void ValueConvertor<any_value_t, uint64_t>(const any_value_t& from, uint64_t& to) {
  to = from.GetUint64();
}

template<>
inline void ValueConvertor<uint64_t, any_document_t>(const uint64_t& from, any_document_t& to) {
  to.SetUint64(from);
}

template<>
inline void ValueConvertor<uint64_t, any_value_t>(const uint64_t& from,
 any_value_t& to, any_document_t& container) {
  to.SetUint64(from);
}
#endif
// uint16_t
template <>
inline bool CheckValueType<uint16_t>(const any_value_t& v) {
  return CheckValueType<int>(v);
}
template <>
inline bool ValueConvertor<uint16_t, uint16_t>(const uint16_t& from, uint16_t& to) {
  to = from;
  return true;
}
template <>
inline bool ValueConvertor<any_value_t, uint16_t>(const any_value_t& from, uint16_t& to) {
  int tmp;
  ValueConvertor(from, tmp);
  to = (uint16_t)tmp;
  return true;
}
template <>
inline bool ValueConvertor<uint16_t, any_document_t>(const uint16_t& from, any_document_t& to) {
  ValueConvertor(static_cast<int>(from), to);
  return true;
}
template <>
inline bool ValueConvertor<uint16_t, any_value_t>(const uint16_t& from, any_value_t& to,
                                                  any_document_t& container) {
  ValueConvertor(static_cast<int>(from), to, container);
  return true;
}

template <>
inline bool ValueConvertor<std::string, agora::commons::ipv4_t>(const std::string& from,
                                                                agora::commons::ipv4_t& to) {
  to = agora::commons::ipv4::from_string(from);
  return true;
}
template <>
inline bool ValueConvertor<agora::commons::ipv4_t, std::string>(const agora::commons::ipv4_t& from,
                                                                std::string& to) {
  to = agora::commons::ipv4::to_string(from);
  return true;
}
template <>
inline bool ValueConvertor<std::string, std::string>(const std::string& from, std::string& to) {
  to = from;
  return true;
}
template <>
inline bool ValueConvertor<std::string, const char*>(const std::string& from, const char*& to) {
  to = from.c_str();
  return true;
}
template <>
inline bool ValueConvertor<int, int>(const int& from, int& to) {
  to = from;
  return true;
}
template <>
inline bool ValueConvertor<unsigned int, unsigned int>(const unsigned int& from, unsigned int& to) {
  to = from;
  return true;
}
template <>
inline bool ValueConvertor<bool, bool>(const bool& from, bool& to) {
  to = from;
  return true;
}
template <>
inline bool ValueConvertor<double, double>(const double& from, double& to) {
  to = from;
  return true;
}

}  // namespace detail

template <class T>
struct ParameterConvertor {
  static bool is_same_type(const any_value_t& v) { return detail::CheckValueType<T>(v); }
  static bool is_same_value(const T& lhs, const T& rhs) { return detail::IsSameValue(lhs, rhs); }
  static bool from_any(const any_value_t& from, T& to) { return detail::ValueConvertor(from, to); }
  static bool to_any(const T& from, any_document_t& to) { return detail::ValueConvertor(from, to); }
  static bool to_any(const T& from, any_value_t& to, any_document_t& container) {
    return detail::ValueConvertor(from, to, container);
  }
  static bool to_any(const T& from, config::AnyValue& to) {
    bool r = false;
    switch (config::ExternalParameterHelperTypeTraits<T>::AnyValueType) {
      case config::AnyValue::INTEGER:
        r = detail::ValueConvertor<T, int>(from, to.val_int);
        break;
      case config::AnyValue::UNSIGNED_INTEGER:
        r = detail::ValueConvertor<T, unsigned int>(from, to.val_uint);
        break;
      case config::AnyValue::BOOLEAN:
        r = detail::ValueConvertor<T, bool>(from, to.val_bool);
        break;
      case config::AnyValue::DOUBLE:
        r = detail::ValueConvertor<T, double>(from, to.val_double);
        break;
      case config::AnyValue::CSTR:
        r = detail::ValueConvertor<T, const char*>(from, to.val_cstr);
        break;
      case config::AnyValue::JSON:
        r = detail::ValueConvertor<T, const void*>(from, to.val_cjson);
        break;
      default:
        break;
    }
    if (r) to.type = config::ExternalParameterHelperTypeTraits<T>::AnyValueType;
    return r;
  }
  static bool from_any(const config::AnyValue& from, T& to) {
    switch (from.type) {
      case config::AnyValue::INTEGER:
        return detail::ValueConvertor<int, T>(from.val_int, to);
      case config::AnyValue::UNSIGNED_INTEGER:
        return detail::ValueConvertor<unsigned int, T>(from.val_uint, to);
      case config::AnyValue::BOOLEAN:
        return detail::ValueConvertor<bool, T>(from.val_bool, to);
      case config::AnyValue::DOUBLE:
        return detail::ValueConvertor<double, T>(from.val_double, to);
      case config::AnyValue::CSTR:
        return detail::ValueConvertor<std::string, T>(from.val_cstr, to);
      case config::AnyValue::JSON:
        return detail::ValueConvertor<any_value_t, T>((any_value_t)from.val_cjson, to);
      case config::AnyValue::UNSPEC:
        return false;
    }
    return false;
  }
};

template <class T>
class ParameterHelper : public ParameterBase {
 public:
  ParameterHelper(ConfigEngine& mp, const char* key) : mp_(mp), key_(key) {
    mp_.registerHandler(key, this);
  }
  ParameterHelper(ConfigEngine& mp, const char* key, T&& def)
      : mp_(mp), key_(key), value_(std::move(def)) {
    mp_.registerHandler(key, this);
  }
  ~ParameterHelper() { mp_.unregisterHandler(key_); }
  const char* key() const { return key_; }
  const T& value() const { return value_; }
  void set_value(const T& v) { value_ = v; }
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    if (parse_value(value, value_)) {
      return 0;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    ParameterConvertor<T>::to_any(value_, out);
    return 0;
  }

 public:
  bool getValue(config::AnyValue& v) const override {
    return ParameterConvertor<T>::to_any(value(), v);
  }
  bool setValue(const config::AnyValue& v, bool storeOnly) override {
    return ParameterConvertor<T>::from_any(v, value_);
  }

 protected:
  bool parse_value(const any_value_t& value, T& out) const {
    if (ParameterConvertor<T>::is_same_type(value)) {
      ParameterConvertor<T>::from_any(value, out);
      return true;
    }
    return false;
  }

 protected:
  ConfigEngine& mp_;
  const char* key_;
  T value_;
};

template <class T>
class ParameterHelperWithOriginalValue : public ParameterHelper<T> {
 public:
  ParameterHelperWithOriginalValue(ConfigEngine& mp, const char* key)
      : ParameterHelper<T>(mp, key) {}
  ParameterHelperWithOriginalValue(ConfigEngine& mp, const char* key, T&& def)
      : ParameterHelper<T>(mp, key, std::move(def)) {}
  const T& original_value() const { return original_value_; }
  void set_orignal_value(const T& v) { original_value_ = v; }

 public:
  bool getOriginalValue(config::AnyValue& v) const override {
    return ParameterConvertor<T>::to_any(original_value(), v);
  }
  bool setOriginalValue(const config::AnyValue& v) override {
    return ParameterConvertor<T>::from_any(v, original_value_);
  }
  bool setOriginalValue(const any_value_t& value) override {
    original_value_ = this->value();
    return true;
  }

 protected:
  T original_value_;
};

template <class T1, class T2>
class ParameterHelper2 : public ParameterBase {
 public:
  ParameterHelper2(ConfigEngine& mp, const char* key, T1&& def1, T2&& def2)
      : mp_(mp), key_(key), value1_(std::move(def1)), value2_(std::move(def2)) {
    mp_.registerHandler(key, this);
  }
  ~ParameterHelper2() { mp_.unregisterHandler(key_); }
  void setValue(const T1& v1, const T2& v2) {
    value1_ = v1;
    value2_ = v2;
  }
  const char* key() const { return key_; }
  const T1& value1() const { return value1_; }
  void set_value1(const T1& v) { value1_ = v; }
  const T2& value2() const { return value2_; }
  void set_value2(const T2& v) { value2_ = v; }
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    if (parse_value(value, value1_, value2_)) {
      return 0;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    any_value_t v1 = nullptr, v2 = nullptr;
    ParameterConvertor<T1>::to_any(value1_, v1, out);
    ParameterConvertor<T2>::to_any(value2_, v2, out);
    commons::json::push_back(out, std::move(v1));
    commons::json::push_back(out, std::move(v2));
    return 0;
  }

 protected:
  bool parse_value(const any_value_t& value, T1& value1, T2& value2) const {
    if (commons::json::is_array(value) && commons::json::get_array_size(value) == 2) {
      const any_value_t v1 = commons::json::get_array_item(value, 0);
      const any_value_t v2 = commons::json::get_array_item(value, 1);
      if (ParameterConvertor<T1>::is_same_type(v1) && ParameterConvertor<T2>::is_same_type(v2)) {
        ParameterConvertor<T1>::from_any(v1, value1);
        ParameterConvertor<T2>::from_any(v2, value2);
        return true;
      }
    }
    return false;
  }

 protected:
  ConfigEngine& mp_;
  const char* key_;
  T1 value1_;
  T2 value2_;
};

template <class T1, class T2, class T3>
class ParameterHelper3 : public ParameterBase {
 public:
  ParameterHelper3(ConfigEngine& mp, const char* key, T1&& def1, T2&& def2, T3&& def3)
      : mp_(mp),
        key_(key),
        value1_(std::move(def1)),
        value2_(std::move(def2)),
        value3_(std::move(def3)) {
    mp_.registerHandler(key, this);
  }
  ~ParameterHelper3() { mp_.unregisterHandler(key_); }
  const char* key() const { return key_; }

  const T1& value1() { return value1_; }
  const T2& value2() { return value2_; }
  const T3& value3() { return value3_; }
  void set_value1(const T1& v) { value1_ = v; }
  void set_value2(const T2& v) { value2_ = v; }
  void set_value3(const T2& v) { value3_ = v; }
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    if (parse_value(value, value1_, value2_, value3_)) {
      return 0;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    any_value_t v1 = nullptr, v2 = nullptr, v3 = nullptr;
    ParameterConvertor<T1>::to_any(value1_, v1, out);
    ParameterConvertor<T2>::to_any(value2_, v2, out);
    ParameterConvertor<T3>::to_any(value3_, v3, out);
    commons::json::push_back(out, std::move(v1));
    commons::json::push_back(out, std::move(v2));
    commons::json::push_back(out, std::move(v3));
    return 0;
  }

 protected:
  bool parse_value(const any_value_t& value, T1& value1, T2& value2, T3& value3) const {
    if (commons::json::is_array(value) && commons::json::get_array_size(value) == 3) {
      const any_value_t v1 = commons::json::get_array_item(value, 0);
      const any_value_t v2 = commons::json::get_array_item(value, 1);
      const any_value_t v3 = commons::json::get_array_item(value, 2);

      if (ParameterConvertor<T1>::is_same_type(v1) && ParameterConvertor<T2>::is_same_type(v2) &&
          ParameterConvertor<T3>::is_same_type(v3)) {
        ParameterConvertor<T1>::from_any(v1, value1);
        ParameterConvertor<T2>::from_any(v2, value2);
        ParameterConvertor<T3>::from_any(v3, value3);
        return true;
      }
    }
    return false;
  }

 protected:
  ConfigEngine& mp_;
  const char* key_;
  T1 value1_;
  T2 value2_;
  T3 value3_;
};

template <class T>
class ListParameterHelper : public ParameterBase {
  typedef typename T::value_type value_type;

 public:
  ListParameterHelper(ConfigEngine& mp, const char* key, T&& def)
      : mp_(mp), key_(key), value_(std::move(def)) {
    mp_.registerHandler(key, this);
  }
  ~ListParameterHelper() { mp_.unregisterHandler(key_); }
  const char* key() const { return key_; }
  const T& value() const { return value_; }
  void set_value(const T& v) { value_ = v; }
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    if (!commons::json::is_array(value)) return -ERR_INVALID_ARGUMENT;
    value_.clear();
    for (auto p = value->child; p != nullptr; p = p->next) {
      value_type to;
      if (ParameterConvertor<value_type>::is_same_type(p)) {
        ParameterConvertor<value_type>::from_any(p, to);
        value_.push_back(std::move(to));
      }
    }
    return 0;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    out.setArrayType();
    for (const value_type& v : this->value_) {
      any_value_t to = nullptr;
      ParameterConvertor<value_type>::to_any(v, to, out);
      commons::json::push_back(out, std::move(to));
    }
    return 0;
  }

 protected:
  ConfigEngine& mp_;
  const char* key_;
  T value_;
};

template <class T, class S>
class ListParameterHelper2 : public ListParameterHelper<T> {
  typedef typename T::value_type value_type;

 public:
  typedef S source_value_type;
  ListParameterHelper2(ConfigEngine& mp, const char* key, T&& def)
      : ListParameterHelper<T>(mp, key, std::move(def)) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    if (commons::json::is_array(value)) {
      this->value_.clear();
      for (auto p = value->child; p != nullptr; p = p->next) {
        if (ParameterConvertor<source_value_type>::is_same_type(p)) {
          source_value_type s;
          ParameterConvertor<source_value_type>::from_any(p, s);
          value_type to;
          detail::ValueConvertor(s, to);
          this->value_.push_back(std::move(to));
        }
      }
      return 0;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    out.setArrayType();
    for (const value_type& v : this->value_) {
      source_value_type s;
      detail::ValueConvertor(v, s);
      any_value_t to = nullptr;
      ParameterConvertor<source_value_type>::to_any(s, to, out);
      commons::json::push_back(out, std::move(to));
    }
    return 0;
  }
};

class ParameterHasSlots {
 public:
  virtual ~ParameterHasSlots() { disconnectAll(); }
  void disconnectAll() {
    for (auto& param : parameters_) {
      param->disconnect();
    }
    parameters_.clear();
  }
  void addParameter(ParameterBase* param) {
    if (param) parameters_.insert(param);
  }

 private:
  std::unordered_set<ParameterBase*> parameters_;
};

template <class T>
class ParameterHelperWithObserver : public ParameterHelper<T> {
  typedef std::function<void(const T&)> observer_type;

 public:
  ParameterHelperWithObserver(ConfigEngine& mp, const char* key, T&& def,
                              bool ignore_same_value = false)
      : ParameterHelper<T>(mp, key, std::move(def)), ignore_same_value_(ignore_same_value) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    int r = -ERR_INVALID_ARGUMENT;
    T tmp;
    if (this->parse_value(value, tmp)) {
      if (ignore_same_value_ && ParameterConvertor<T>::is_same_value(this->value(), tmp)) {
        r = -ERR_CANCELED;
      } else {
        r = 0;
        std::swap(tmp, this->value_);
        if (observer_) observer_(this->value());
      }
    }
    return r;
  }
  void disconnect() override { observer_ = nullptr; }
  void connect(ParameterHasSlots* om, observer_type&& observer, bool triggerOnConnect = false) {
    observer_ = std::move(observer);
    if (om) om->addParameter(this);
    if (triggerOnConnect && observer_) observer_(this->value());
  }

 private:
  observer_type observer_;
  bool ignore_same_value_;
};

template <class T1, class T2>
class ParameterHelperWithObserver2 : public ParameterHelper2<T1, T2> {
  typedef std::function<void(const T1&, const T2&)> observer_type;

 public:
  ParameterHelperWithObserver2(ConfigEngine& mp, const char* key, T1&& def1, T2&& def2)
      : ParameterHelper2<T1, T2>(mp, key, std::move(def1), std::move(def2)) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    int r = -ERR_INVALID_ARGUMENT;
    T1 v1;
    T2 v2;
    if (this->parse_value(value, v1, v2)) {
      r = 0;
      this->value1_ = std::move(v1);
      this->value2_ = std::move(v2);
      if (observer_) {
        observer_(this->value1(), this->value2());
      }
    }
    return r;
  }
  void disconnect() override { observer_ = nullptr; }
  void connect(ParameterHasSlots* om, observer_type&& observer, bool triggerOnConnect = false) {
    observer_ = std::move(observer);
    if (om) {
      om->addParameter(this);
    }
    if (triggerOnConnect && observer_) {
      observer_(this->value1(), this->value2());
    }
  }

 private:
  observer_type observer_;
};

template <class T>
class ParameterHelperWithFilter : public ParameterHelper<T> {
  typedef std::function<int(T&)> filter_type;

 public:
  ParameterHelperWithFilter(ConfigEngine& mp, const char* key, T&& def)
      : ParameterHelper<T>(mp, key, std::move(def)) {}
  ParameterHelperWithFilter(ConfigEngine& mp, const char* key, T&& def, filter_type&& filter)
      : ParameterHelper<T>(mp, key, std::move(def)), filter_(std::move(filter)) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    T v;
    if (this->parse_value(value, v)) {
      int r = 0;
      if (filter_) r = filter_(v);
      if (!r) {
        this->value_ = std::move(v);
      }
      return r;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  void disconnect() override { filter_ = nullptr; }
  void connect(ParameterHasSlots* om, filter_type&& filter) {
    filter_ = std::move(filter);
    if (om) om->addParameter(this);
  }

 private:
  filter_type filter_;
};

template <class T1, class T2>
class ParameterHelperWithFilter2 : public ParameterHelper2<T1, T2> {
  typedef std::function<int(T1&, T2&)> filter_type;

 public:
  ParameterHelperWithFilter2(ConfigEngine& mp, const char* key, T1&& def1, T2&& def2)
      : ParameterHelper2<T1, T2>(mp, key, std::move(def1), std::move(def2)) {}
  ParameterHelperWithFilter2(ConfigEngine& mp, const char* key, T1&& def1, T2&& def2,
                             filter_type&& filter)
      : ParameterHelper2<T1, T2>(mp, key, std::move(def1), std::move(def2)),
        filter_(std::move(filter)) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    T1 v1;
    T2 v2;
    if (this->parse_value(value, v1, v2)) {
      int r = 0;
      if (filter_) r = filter_(v1, v2);
      if (!r) {
        this->value1_ = std::move(v1);
        this->value2_ = std::move(v2);
      }
      return r;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  void disconnect() override { filter_ = nullptr; }
  void connect(ParameterHasSlots* om, filter_type&& filter) {
    filter_ = std::move(filter);
    if (om) om->addParameter(this);
  }

 private:
  filter_type filter_;
};

template <class T1, class T2, class T3>
class ParameterHelperWithFilter3 : public ParameterHelper3<T1, T2, T3> {
  typedef std::function<int(T1&, T2&, T3&)> filter_type;

 public:
  ParameterHelperWithFilter3(ConfigEngine& mp, const char* key, T1&& def1, T2&& def2, T3&& def3)
      : ParameterHelper3<T1, T2, T3>(mp, key, std::move(def1), std::move(def2), std::move(def3)) {}
  ParameterHelperWithFilter3(ConfigEngine& mp, const char* key, T1&& def1, T2&& def2, T3&& def3,
                             filter_type&& filter)
      : ParameterHelper3<T1, T2, T3>(mp, key, std::move(def1), std::move(def2), std::move(def3)),
        filter_(std::move(filter)) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    T1 v1;
    T2 v2;
    T3 v3;
    if (this->parse_value(value, v1, v2, v3)) {
      int r = 0;
      if (filter_) r = filter_(v1, v2, v3);
      if (!r) {
        this->value1_ = std::move(v1);
        this->value2_ = std::move(v2);
        this->value3_ = std::move(v3);
      }
      return r;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  void disconnect() override { filter_ = nullptr; }
  void connect(ParameterHasSlots* om, filter_type&& filter) {
    filter_ = std::move(filter);
    if (om) om->addParameter(this);
  }

 private:
  filter_type filter_;
};

template <class T>
class TriggerParameterHelper : public ParameterBase {
 public:
  TriggerParameterHelper(ConfigEngine& mp, const char* key) : mp_(mp), key_(key) {
    mp_.registerHandler(key, this);
  }
  ~TriggerParameterHelper() { mp_.unregisterHandler(key_); }
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    if (ParameterConvertor<T>::is_same_type(value)) {
      return 0;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    return -ERR_NOT_SUPPORTED;
  }

 protected:
  ConfigEngine& mp_;
  const char* key_;
};

template <class T>
class TriggerParameterHelperWithObserver : public TriggerParameterHelper<T> {
  typedef std::function<void(const T&)> observer_type;

 public:
  TriggerParameterHelperWithObserver(ConfigEngine& mp, const char* key)
      : TriggerParameterHelper<T>(mp, key) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    int r = TriggerParameterHelper<T>::onSetParameter(key, value);
    if (!r && observer_) {
      T val;
      ParameterConvertor<T>::from_any(value, val);
      observer_(val);
    }
    return r;
  }
  void disconnect() override { observer_ = nullptr; }
  void connect(ParameterHasSlots* om, observer_type&& observer) {
    observer_ = std::move(observer);
    if (om) om->addParameter(this);
  }

 private:
  observer_type observer_;
};

// external observer/filter are used for external module, e.g. chat engine
template <class T>
class ParameterHelperWithExternalObserver : public ParameterHelper<T> {
 public:
  ParameterHelperWithExternalObserver(ConfigEngine& mp, const char* key)
      : ParameterHelper<T>(mp, key), observer_(nullptr) {}
  ParameterHelperWithExternalObserver(ConfigEngine& mp, const char* key, T&& def)
      : ParameterHelper<T>(mp, key, std::move(def)), observer_(nullptr) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    int r = ParameterHelper<T>::onSetParameter(key, value);
    if (!r) {
      trigger();
    }
    return r;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    if (observer_) {
      config::AnyValue value;
      if (observer_->onGetValue(value) &&
          ParameterConvertor<any_document_t>::from_any(value, out)) {
        return 0;
      }
    }
    return ParameterHelper<T>::onGetParameter(key, args, out);
  }
  bool setValue(const config::AnyValue& value, bool storeOnly) override {
    if (ParameterHelper<T>::setValue(value, storeOnly)) {
      if (!storeOnly && observer_) observer_->onSetValue(value);
      return true;
    }
    return false;
  }
  bool connectExternalObserver(config::IObserver* observer, bool triggerOnConnect) override {
    observer_ = observer;
    if (triggerOnConnect) trigger();
    return true;
  }
  bool disconnectExternalObserver() override {
    observer_ = nullptr;
    return true;
  }

 private:
  void trigger() {
    if (observer_) {
      config::AnyValue v;
      if (this->getValue(v)) observer_->onSetValue(v);
    }
  }

 private:
  config::IObserver* observer_;
};

template <class T>
class ParameterHelperWithExternalFilter : public ParameterHelper<T> {
 public:
  ParameterHelperWithExternalFilter(ConfigEngine& mp, const char* key)
      : ParameterHelper<T>(mp, key), filter_(nullptr) {}
  ParameterHelperWithExternalFilter(ConfigEngine& mp, const char* key, T&& def)
      : ParameterHelper<T>(mp, key, std::move(def)), filter_(nullptr) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    T v;
    if (this->parse_value(value, v)) {
      int r = trigger();
      if (!r) {
        this->value_ = std::move(v);
      }
      return r;
    }
    return -ERR_INVALID_ARGUMENT;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    if (filter_) {
      config::AnyValue value;
      if (filter_->onGetValue(value) && ParameterConvertor<any_document_t>::from_any(value, out)) {
        return 0;
      }
    }
    return ParameterHelper<T>::onGetParameter(key, args, out);
  }
  bool setValue(const config::AnyValue& value, bool storeOnly) override {
    if (storeOnly || (filter_ && filter_->onSetValue(value)))
      return ParameterHelper<T>::setValue(value, storeOnly);
    return false;
  }

  bool connectExternalFilter(config::IFilter* filter, bool triggerOnConnect) override {
    filter_ = filter;
    if (triggerOnConnect) trigger();
    return true;
  }
  bool disconnectExternalFilter() override {
    filter_ = nullptr;
    return true;
  }

 private:
  int trigger() {
    int r = 0;
    if (filter_) {
      config::AnyValue av;
      if (this->getValue(av)) r = filter_->onSetValue(av) ? 0 : -ERR_INVALID_ARGUMENT;
    }
    return 0;
  }

 private:
  config::IFilter* filter_;
};

template <class T>
class TriggerParameterHelperWithExternalObserver : public ParameterHelper<T> {
 public:
  TriggerParameterHelperWithExternalObserver(ConfigEngine& mp, const char* key)
      : ParameterHelper<T>(mp, key), observer_(nullptr), dirty_(false) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    int r = ParameterHelper<T>::onSetParameter(key, value);
    if (!r) {
      dirty_ = true;
      trigger();
    }
    return r;
  }
  int onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    if (observer_) {
      config::AnyValue value;
      if (observer_->onGetValue(value) &&
          ParameterConvertor<any_document_t>::from_any(value, out)) {
        return 0;
      }
    }
    return ParameterHelper<T>::onGetParameter(key, args, out);
  }
  bool connectExternalObserver(config::IObserver* observer, bool triggerOnConnect) override {
    observer_ = observer;
    if (triggerOnConnect) trigger();
    return true;
  }
  bool disconnectExternalObserver() override {
    observer_ = nullptr;
    return true;
  }

 private:
  void trigger() {
    if (dirty_ && observer_) {
      config::AnyValue v;
      if (this->getValue(v)) observer_->onSetValue(v);
      dirty_ = false;
    }
  }

 private:
  config::IObserver* observer_;
  bool dirty_;
};

template <class T, class S>
class ListParameterHelperWithObserver2 : public ListParameterHelper2<T, S> {
  using observer_type = std::function<void(const T&)>;

 public:
  ListParameterHelperWithObserver2(ConfigEngine& mp, const char* key, T&& def)
      : ListParameterHelper2<T, S>(mp, key, std::move(def)) {}
  int onSetParameter(const std::string& key, const any_value_t& value) override {
    int r = ListParameterHelper2<T, S>::onSetParameter(key, value);
    if (!r && observer_) {
      observer_(this->value());
    }
    return r;
  }
  void disconnect() override { observer_ = nullptr; }
  void connect(ParameterHasSlots* om, observer_type&& observer) {
    observer_ = std::move(observer);
    if (om) {
      om->addParameter(this);
    }
  }

 private:
  observer_type observer_;
};

}  // namespace base
}  // namespace agora
