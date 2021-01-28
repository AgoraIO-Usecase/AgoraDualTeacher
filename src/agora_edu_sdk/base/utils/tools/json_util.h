//
//  Agora Media SDK
//
//  Created by Ren Jingui in 2020.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <base/base_type.h>
#include <string>
#include <vector>
#include "utils/tools/c_json.h"

namespace agora {
namespace commons {

namespace json {
using Document = agora::any_document_t;
using Value = agora::any_value_t;
inline bool from_string(Document& doc, const std::string& input) {
  doc.parse(input.c_str());
  return doc.isValid();
}
inline std::string to_string(const Document& doc) { return doc.toString(); }
inline bool from_file(Document& doc, const std::string& file_path) {
  return doc.fromFile(file_path);
}
inline bool to_file(const Document& doc, const std::string& file_path) {
  return doc.toFile(file_path);
}
inline void copy(const Value& from, Value& to, Document& container) {
  Document doc(from);
  to = doc.duplicate();
}
inline void copy(const Document& from, Value& to, Document& container) { to = from.duplicate(); }
inline void copy(const Value& from, Document& to) {
  Document doc(from);
  to.assign(doc.duplicate());
}
inline void copy(const Document& from, Document& to) { to = from; }

inline bool is_bool(const Value& val) {
  return val->type == cJSON_True || val->type == cJSON_False;
}
inline bool is_string(const Value& val) { return val->type == cJSON_String; }
inline bool is_string(const Document& val) { return val.isString(); }
inline bool is_double(const Value& val) { return val->type == cJSON_Number; }
inline bool is_int(const Value& val) { return is_double(val); }
inline bool is_uint(const Value& val) { return is_double(val); }
inline bool is_object(const Value& val) { return val->type == cJSON_Object; }
inline bool is_object(const Document& doc) { return doc.isObject(); }
inline bool is_array(const Value& val) { return val->type == cJSON_Array; }
inline bool is_array(const Document& doc) { return doc.isArray(); }
inline size_t get_array_size(const Value& val) { return (size_t)cjson::cJSON_GetArraySize(val); }
inline Value get_array_item(const Value& arr, int index) {
  return cjson::cJSON_GetArrayItem(arr, index);
}
inline bool is_null(const Value& val) { return val->type == cJSON_NULL; }
inline bool get_bool(const Value& val, bool& result) {
  if (!is_bool(val)) return false;
  result = val->type == cJSON_True ? true : false;
  return true;
}
inline bool get_bool(const Document& doc, bool& result) {
  if (!doc.isBoolean()) return false;
  result = doc.getBooleanValue(result);
  return true;
}
inline bool get_string(const Value& val, std::string& result) {
  if (!is_string(val)) return false;
  result = val->valuestring;
  return true;
}
inline bool get_string(const Document& doc, std::string& result) {
  if (!doc.isString()) return false;
  const char* r = doc.getStringValue(nullptr);
  if (!r) return false;
  result = r;
  return true;
}
inline bool get_string(const Document& doc, const std::string& key, std::string& result) {
  if (!doc.isObject()) return false;
  const char* r = doc.getStringValue(key.c_str(), nullptr);
  if (!r) return false;
  result = r;
  return true;
}
inline bool get_int(const Value& val, int& result) {
  if (!is_int(val)) return false;
  result = val->valueint;
  return true;
}
inline bool get_int(const Document& doc, int& result) {
  if (!doc.isInt()) return false;
  result = doc.getIntValue(result);
  return true;
}
inline bool get_uint(const Value& val, unsigned int& result) {
  if (!is_uint(val)) return false;
  result = (unsigned int)val->valuedouble;
  return true;
}
inline bool get_uint(const Document& doc, unsigned int& result) {
  if (!doc.isUInt()) return false;
  result = doc.getUIntValue(result);
  return true;
}
inline bool get_double(const Value& val, double& result) {
  if (!is_double(val)) return false;
  result = val->valuedouble;
  return true;
}
inline bool get_double(const Document& doc, double& result) {
  if (!doc.isDouble()) return false;
  result = doc.getDoubleValue(result);
  return true;
}
inline void insert(Document& doc, const std::string& key, const Value& value) {
  doc.setObjectValue(key.c_str(), value);
}
inline void insert(Document& doc, const std::string& key, const Document& value) {
  doc.setObjectValue(key.c_str(), value);
}
inline void insert(Document& doc, const std::string& key, const char* value) {
  doc.setStringValue(key.c_str(), value);
}
inline void insert(Document& doc, const std::string& key, const std::string& value) {
  insert(doc, key, value.c_str());
}
inline void insert(Document& doc, const std::string& key, bool value) {
  doc.setBooleanValue(key.c_str(), value);
}
inline void insert(Document& doc, const std::string& key, int value) {
  doc.setIntValue(key.c_str(), value);
}
inline void insert(Document& doc, const std::string& key, uint32_t value) {
  doc.setUIntValue(key.c_str(), value);
}
inline void insert(Document& doc, const std::string& key, double value) {
  doc.setDoubleValue(key.c_str(), value);
}
inline void set_bool(Document& doc, bool value) { doc.setBooleanValue(value); }
inline void set_bool(Value& val, bool value) {
  Document doc(val);
  doc.setBooleanValue(value);
  val = doc.release();
}
inline void set_int(Document& doc, int value) { doc.setIntValue(value); }
inline void set_int(Value& val, int value) {
  Document doc(val);
  doc.setIntValue(value);
  val = doc.release();
}
inline void set_double(Document& doc, double value) { doc.setDoubleValue(value); }
inline void set_double(Value& val, double value) {
  Document doc(val);
  doc.setDoubleValue(value);
  val = doc.release();
}
inline void set_uint32(Document& doc, unsigned int value) { doc.setUIntValue(value); }
inline void set_uint32(Value& val, unsigned int value) {
  Document doc(val);
  doc.setUIntValue(value);
  val = doc.release();
}
inline void set_string(Document& doc, const std::string& value) {
  doc.setStringValue(value.c_str());
}
inline void set_string(Document& doc, const std::string& value, Document& container) {
  set_string(doc, value);
}
inline void set_string(Value& val, const std::string& value) {
  Document doc(val);
  doc.setStringValue(value.c_str());
  val = doc.release();
}
inline void set_string(Value& val, const std::string& value, Document& container) {
  Document doc(val);
  set_string(doc, value);
  val = doc.release();
}
inline void push_back(Document& doc, Value&& value) { doc.addItemToArray(value); }
inline void push_back(Document& doc, const char* value) { doc.addStringValueToArray(value); }
inline void push_back(Document& doc, const std::string& value) {
  doc.addStringValueToArray(value.c_str());
}
inline void push_back(Document& doc, int value) { doc.addIntValueToArray(value); }
inline void push_back(Document& doc, bool value) { doc.addBoolValueToArray(value); }
inline void push_back(Document& doc, const std::string& value, Document& container) {
  push_back(doc, value);
}
inline Document clone(const Document& from) {
  Document to;
  from_string(to, to_string(from));
  return to;
}
}  // namespace json

}  // namespace commons
}  // namespace agora
