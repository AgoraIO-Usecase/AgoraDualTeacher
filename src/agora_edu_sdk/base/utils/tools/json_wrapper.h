//
//  Agora Media SDK
//
//  Created by Sting Feng in 2018-03.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once

#include <string>

namespace agora {
namespace commons {
namespace cjson {
struct cJSON;

class JsonWrapper {
 public:
  JsonWrapper();
  explicit JsonWrapper(const char* input);
  explicit JsonWrapper(const std::string& input);
  explicit JsonWrapper(cJSON* node, bool owner = false);
  JsonWrapper(const JsonWrapper&);
  JsonWrapper& operator=(const JsonWrapper&);
  JsonWrapper(JsonWrapper&& rhs);
  JsonWrapper& operator=(JsonWrapper&& rhs);

  ~JsonWrapper();
  void parse(const char* input);
  void assign(cJSON* node);
  void setObjectType();
  void setArrayType();
  cJSON* duplicate() const;
  bool fromFile(const std::string& filePath);
  bool toFile(const std::string& filePath) const;
  JsonWrapper* take();
  cJSON* release();
  const char* getName() const;
  int getArraySize() const;
  int getIntValue(const char* name, int defValue) const;
  int getIntValue(int index, int defValue) const;
  unsigned int getUIntValue(const char* name, unsigned int defValue) const;
  unsigned int getUIntValue(int index, unsigned int defValue) const;
  bool tryGetIntValue(const char* name, int& value) const;
  bool tryGetUIntValue(const char* name, unsigned int& value) const;
  bool tryGetStringValue(const char* name, std::string& value) const;
  double getDoubleValue(const char* name, double defValue) const;
  double getDoubleValue(int index, double defValue) const;
  const char* getStringValue(const char* name, const char* defValue) const;
  const char* getStringValue(int index, const char* defValue) const;
  const char* getStringValue(const char* defValue) const;
  bool getBooleanValue(bool defValue) const;
  int getIntValue(int defValue) const;
  unsigned int getUIntValue(unsigned int defValue) const;
  double getDoubleValue(double defValue) const;
  bool getBooleanValue(const char* name, bool defValue) const;
  bool getBooleanValue(int index, bool defValue) const;
  bool tryGetBooleanValue(const char* name, bool& value) const;
  void setIntValue(int value);
  void setIntValue(const char* name, int value);
  void setUIntValue(unsigned int value);
  void setUIntValue(const char* name, unsigned int value);
  void setDoubleValue(double value);
  void setDoubleValue(const char* name, double value);
  void setBooleanValue(bool value);
  void setBooleanValue(const char* name, bool value);
  void setStringValue(const char* value);
  void setStringValue(const char* name, const char* value);
  void setStringValue(const char* name, const std::string& value) {
    return setStringValue(name, value.c_str());
  }
  void setObjectValue(const char* name, const JsonWrapper& value);
  void setObjectValue(const char* name, const cJSON* value);
  void setArrayValue(const char* name, const JsonWrapper& value);
  bool isValid() const;
  bool isNull(const char* name) const;
  bool isNull(int index) const;
  bool isString(const char* name) const;
  bool isString(int index) const;
  bool isNumber(const char* name) const;
  bool isNumber(int index) const;
  bool isBoolean() const;
  bool isBoolean(const char* name) const;
  bool isBoolean(int index) const;
  bool isObject(const char* name) const;
  bool isObject(int index) const;
  bool isArray(const char* name) const;
  bool isArray(int index) const;
  bool isArray() const;
  bool isObject() const;
  bool isString() const;
  bool isInt() const;
  bool isUInt() const;
  bool isDouble() const;
  bool hasNode(const char* name) const;
  std::string toString(bool formatted = false) const;
  JsonWrapper getObject(const char* name) const;
  JsonWrapper getArray(const char* name) const;
  JsonWrapper getArray(int index) const;
  JsonWrapper getChild();
  JsonWrapper getNext();
  JsonWrapper getPrev();
  void merge(const char* profile);
  void merge(JsonWrapper& profile);
  void reverse_merge(const char* profile);
  void reverse_merge(JsonWrapper& profile);
  const cJSON* getRoot() const { return m_root; }
  cJSON* getRoot() { return m_root; }
  cJSON* detachItemFromObject(const char* name);
  bool eraseNode(const char* name);
  bool addStringValueToArray(const char* value);
  bool addIntValueToArray(int value);
  bool addUIntValueToArray(unsigned int value);
  bool addBoolValueToArray(bool value);
  bool addItemToArray(cJSON* item);
  bool addItemToObject(const char* name, cJSON* item);
  bool addItemReferenceToArray(cJSON* item);
  bool addItemReferenceToObject(const char* name, cJSON* item);

  cJSON* findNode(const char* name) const;

 private:
  cJSON* findNode(int index) const;
  cJSON* findNode(const char* name, int type) const;
  void doMerge(cJSON* root1, cJSON* root2);
  std::string doToString(cJSON* root, bool formatted) const;

 private:
  cJSON* m_root;
  bool m_owner;
};
}  // namespace cjson
}  // namespace commons
}  // namespace agora
