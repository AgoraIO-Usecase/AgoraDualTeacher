#include "utils/tools/json_wrapper.h"

#include <cstdlib>
#include <cstring>
#include <vector>

#include "utils/tools/c_json.h"

namespace agora {
namespace commons {
namespace cjson {
JsonWrapper::JsonWrapper() : m_root(nullptr), m_owner(true) {}
JsonWrapper::JsonWrapper(const char* input) : m_root(nullptr), m_owner(true) { parse(input); }

JsonWrapper::JsonWrapper(const std::string& input) : m_root(nullptr), m_owner(true) {
  parse(input.c_str());
}

JsonWrapper::JsonWrapper(cJSON* node, bool owner) : m_root(node), m_owner(owner) {}

JsonWrapper::JsonWrapper(const JsonWrapper& rhs) : m_root(nullptr), m_owner(true) {
  operator=(rhs);
}

JsonWrapper::JsonWrapper(JsonWrapper&& rhs) : m_root(nullptr), m_owner(true) {
  operator=(std::move(rhs));
}

JsonWrapper& JsonWrapper::operator=(const JsonWrapper& rhs) {
  if (this != &rhs) {
    parse(nullptr);
    m_root = rhs.duplicate();
    m_owner = true;
  }
  return *this;
}

JsonWrapper& JsonWrapper::operator=(JsonWrapper&& rhs) {
  if (this != &rhs) {
    parse(nullptr);
    m_root = rhs.m_root;
    m_owner = rhs.m_owner;
    rhs.m_root = nullptr;
    rhs.m_owner = false;
  }
  return *this;
}

JsonWrapper::~JsonWrapper() { parse(nullptr); }

void JsonWrapper::parse(const char* input) {
  if (m_root && m_owner) cJSON_Delete(m_root);
  m_root = nullptr;
  m_owner = true;
  if (input && *input != '\0') {
    m_root = cJSON_Parse(input);
  }
}

void JsonWrapper::assign(cJSON* node) {
  if (m_root && m_owner) cJSON_Delete(m_root);
  m_root = node;
  m_owner = true;
}

cJSON* JsonWrapper::duplicate() const {
  if (!isValid()) return nullptr;
  return cJSON_Duplicate(m_root, 1);
}

void JsonWrapper::setObjectType() { assign(cJSON_CreateObject()); }

void JsonWrapper::setArrayType() { assign(cJSON_CreateArray()); }

bool JsonWrapper::fromFile(const std::string& filePath) {
  FILE* f = fopen(filePath.c_str(), "rb");
  if (!f) {
    return false;
  }
  std::vector<char> readBuffer;
  readBuffer.resize(65535);
  if (0 == fread(&readBuffer[0], 1, readBuffer.size(), f)) {
    fclose(f);
    return false;
  }

  parse(&readBuffer[0]);
  fclose(f);
  return isValid();
}

bool JsonWrapper::toFile(const std::string& filePath) const {
  if (!isValid() || filePath.empty()) return false;

  FILE* f = fopen(filePath.c_str(), "wb");
  if (!f) {
    return false;
  }
  char* p = cjson::cJSON_Print(m_root);
  if (p) {
    fwrite(p, strlen(p), 1, f);
    fclose(f);
    free(p);
    return true;
  }

  fclose(f);
  return false;
}

JsonWrapper* JsonWrapper::take() {
  JsonWrapper* json = new JsonWrapper;
  if (m_owner) {
    json->m_root = m_root;
    json->m_owner = m_owner;
    m_root = nullptr;
    m_owner = false;
  } else {
    json->m_root = cJSON_Duplicate(m_root, 1);
    json->m_owner = true;
  }
  return json;
}

cJSON* JsonWrapper::release() {
  if (m_owner) {
    cJSON* item = m_root;
    m_root = nullptr;
    m_owner = false;
    return item;
  }
  return nullptr;
}

bool JsonWrapper::isValid() const { return m_root != nullptr; }

JsonWrapper JsonWrapper::getObject(const char* name) const {
  return JsonWrapper(findNode(name, cJSON_Object));
}

JsonWrapper JsonWrapper::getArray(const char* name) const {
  return JsonWrapper(findNode(name, cJSON_Array));
}

JsonWrapper JsonWrapper::getArray(int index) const {
  JsonWrapper r(findNode(index));
  if (r.getRoot()->type == cJSON_Array) return r;
  return JsonWrapper();
}

JsonWrapper JsonWrapper::getChild() { return JsonWrapper(m_root ? m_root->child : nullptr); }

JsonWrapper JsonWrapper::getNext() { return JsonWrapper(m_root ? m_root->next : nullptr); }

JsonWrapper JsonWrapper::getPrev() { return JsonWrapper(m_root ? m_root->prev : nullptr); }

cJSON* JsonWrapper::detachItemFromObject(const char* name) {
  if (isValid()) {
    return cJSON_DetachItemFromObject(m_root, name);
  }
  return nullptr;
}

bool JsonWrapper::eraseNode(const char* name) {
  cJSON* p = detachItemFromObject(name);
  if (p) {
    cJSON_Delete(p);
    return true;
  }
  return false;
}

bool JsonWrapper::addStringValueToArray(const char* value) {
  if (!isValid() || !value) return false;
  cJSON_AddItemToArray(m_root, cJSON_CreateString(value));
  return true;
}

bool JsonWrapper::addIntValueToArray(int value) {
  if (!isValid()) return false;
  cJSON_AddItemToArray(m_root, cJSON_CreateNumber(value));
  return true;
}

bool JsonWrapper::addUIntValueToArray(unsigned int value) {
  if (!isValid()) return false;
  cJSON_AddItemToArray(m_root, cJSON_CreateNumber(value));
  return true;
}

bool JsonWrapper::addBoolValueToArray(bool value) {
  if (!isValid()) return false;
  cJSON_AddItemToArray(m_root, value ? cJSON_CreateTrue() : cJSON_CreateFalse());
  return true;
}

bool JsonWrapper::addItemToArray(cJSON* item) {
  if (!isValid()) return false;
  cJSON_AddItemToArray(m_root, item);
  return true;
}

bool JsonWrapper::addItemToObject(const char* name, cJSON* item) {
  if (!isValid()) return false;
  cJSON_AddItemToObject(m_root, name, item);
  return true;
}

bool JsonWrapper::addItemReferenceToArray(cJSON* item) {
  if (!isValid()) return false;
  cJSON_AddItemReferenceToArray(m_root, item);
  return true;
}

bool JsonWrapper::addItemReferenceToObject(const char* name, cJSON* item) {
  if (!isValid()) return false;
  cJSON_AddItemReferenceToObject(m_root, name, item);
  return true;
}

cJSON* JsonWrapper::findNode(const char* name) const {
  if (m_root && name && *name != '\0') {
    cJSON* node = cJSON_GetObjectItem(m_root, name);
    if (node) return node;
  }
  return nullptr;
}

cJSON* JsonWrapper::findNode(const char* name, int type) const {
  cJSON* node = findNode(name);
  if (node && node->type == type) return node;
  return nullptr;
}

cJSON* JsonWrapper::findNode(int index) const {
  return m_root ? cJSON_GetArrayItem(m_root, index) : nullptr;
}

int JsonWrapper::getArraySize() const { return m_root ? cJSON_GetArraySize(m_root) : 0; }

const char* JsonWrapper::getName() const { return m_root ? m_root->string : nullptr; }

int JsonWrapper::getIntValue(const char* name, int defValue) const {
  cJSON* node = findNode(name, cJSON_Number);
  return node ? node->valueint : defValue;
}

int JsonWrapper::getIntValue(int index, int defValue) const {
  cJSON* node = findNode(index);
  return node ? node->valueint : defValue;
}

unsigned int JsonWrapper::getUIntValue(const char* name, unsigned int defValue) const {
  cJSON* node = findNode(name, cJSON_Number);
  return node ? (unsigned int)node->valuedouble : defValue;
}

unsigned int JsonWrapper::getUIntValue(int index, unsigned int defValue) const {
  cJSON* node = findNode(index);
  return node ? (unsigned int)node->valuedouble : defValue;
}

bool JsonWrapper::tryGetIntValue(const char* name, int& value) const {
  cJSON* node = findNode(name, cJSON_Number);
  if (!node) {
    return false;
  }

  value = node->valueint;
  return true;
}

bool JsonWrapper::tryGetUIntValue(const char* name, unsigned int& value) const {
  cJSON* node = findNode(name, cJSON_Number);
  if (!node) {
    return false;
  }

  value = (unsigned int)node->valuedouble;
  return true;
}

bool JsonWrapper::tryGetStringValue(const char* name, std::string& value) const {
  cJSON* node = findNode(name, cJSON_String);
  if (!node) {
    return false;
  }

  if (node->valuestring) value = node->valuestring;
  return true;
}

double JsonWrapper::getDoubleValue(const char* name, double defValue) const {
  cJSON* node = findNode(name, cJSON_Number);
  return node ? node->valuedouble : defValue;
}

double JsonWrapper::getDoubleValue(int index, double defValue) const {
  cJSON* node = findNode(index);
  return node ? node->valuedouble : defValue;
}

const char* JsonWrapper::getStringValue(const char* name, const char* defValue) const {
  cJSON* node = findNode(name, cJSON_String);
  return node ? node->valuestring : defValue;
}

const char* JsonWrapper::getStringValue(int index, const char* defValue) const {
  cJSON* node = findNode(index);
  return node ? node->valuestring : defValue;
}

int JsonWrapper::getIntValue(int defValue) const {
  if (isInt()) {
    return m_root->valueint;
  }
  return defValue;
}

unsigned int JsonWrapper::getUIntValue(unsigned int defValue) const {
  if (isUInt()) {
    return (unsigned int)m_root->valuedouble;
  }
  return defValue;
}

double JsonWrapper::getDoubleValue(double defValue) const {
  if (isDouble()) {
    return m_root->valuedouble;
  }
  return defValue;
}

bool JsonWrapper::getBooleanValue(bool defValue) const {
  if (m_root) {
    if (m_root->type == cJSON_True)
      return true;
    else if (m_root->type == cJSON_False)
      return false;
  }
  return defValue;
}

const char* JsonWrapper::getStringValue(const char* defValue) const {
  if (isString()) {
    return m_root->valuestring;
  }
  return defValue;
}

bool JsonWrapper::getBooleanValue(const char* name, bool defValue) const {
  cJSON* node = findNode(name);
  if (node) {
    if (node->type == cJSON_True)
      return true;
    else if (node->type == cJSON_False)
      return false;
  }
  return defValue;
}

bool JsonWrapper::getBooleanValue(int index, bool defValue) const {
  cJSON* node = findNode(index);
  if (node) {
    if (node->type == cJSON_True)
      return true;
    else if (node->type == cJSON_False)
      return false;
  }
  return defValue;
}

bool JsonWrapper::tryGetBooleanValue(const char* name, bool& value) const {
  cJSON* node = findNode(name);
  if (!node) return false;
  if (node->type == cJSON_True)
    value = true;
  else if (node->type == cJSON_False)
    value = false;
  return true;
}

void JsonWrapper::setIntValue(int value) {
  if (m_root && m_root->type == cJSON_Number) {
    m_root->valueint = value;
    m_root->valuedouble = value;
  } else
    assign(cJSON_CreateNumber(value));
}

void JsonWrapper::setIntValue(const char* name, int value) {
  if (!m_root || !name) return;

  cJSON* node = findNode(name);
  if (node && node->type == cJSON_Number)
    node->valueint = value;
  else if (node)
    cJSON_ReplaceItemInObject(m_root, name, cJSON_CreateNumber(value));
  else
    cJSON_AddItemToObject(m_root, name, cJSON_CreateNumber(value));
}

void JsonWrapper::setUIntValue(const char* name, unsigned int value) {
  setDoubleValue(name, value);
}

void JsonWrapper::setUIntValue(unsigned int value) { setDoubleValue(value); }

void JsonWrapper::setDoubleValue(double value) {
  if (m_root && m_root->type == cJSON_Number) {
    m_root->valuedouble = value;
  } else
    assign(cJSON_CreateNumber(value));
}

void JsonWrapper::setDoubleValue(const char* name, double value) {
  if (!m_root || !name) return;

  cJSON* node = findNode(name);
  if (node && node->type == cJSON_Number)
    node->valuedouble = value;
  else if (node)
    cJSON_ReplaceItemInObject(m_root, name, cJSON_CreateNumber(value));
  else
    cJSON_AddItemToObject(m_root, name, cJSON_CreateNumber(value));
}

void JsonWrapper::setBooleanValue(bool value) {
  if (m_root && (m_root->type == cJSON_True || m_root->type == cJSON_False)) {
    if (value)
      m_root->type = cJSON_True;
    else
      m_root->type = cJSON_False;
  } else
    assign(cJSON_CreateBool(value));
}

void JsonWrapper::setBooleanValue(const char* name, bool value) {
  if (!m_root || !name) return;

  cJSON* node = findNode(name);
  if (node) {
    if (node->type == cJSON_True && value)
      return;
    else if (node->type == cJSON_False && !value)
      return;
    else
      cJSON_ReplaceItemInObject(m_root, name, cJSON_CreateBool(value));
  } else
    cJSON_AddItemToObject(m_root, name, cJSON_CreateBool(value));
}

void JsonWrapper::setStringValue(const char* value) { assign(cJSON_CreateString(value)); }

void JsonWrapper::setStringValue(const char* name, const char* value) {
  if (!m_root || !name) return;

  cJSON* node = findNode(name);
  if (node) {
    if (!value)
      cJSON_DeleteItemFromObject(m_root, name);
    else
      cJSON_ReplaceItemInObject(m_root, name, cJSON_CreateString(value));
  } else if (value)
    cJSON_AddItemToObject(m_root, name, cJSON_CreateString(value));
}

void JsonWrapper::setObjectValue(const char* name, const JsonWrapper& value) {
  setObjectValue(name, value.getRoot());
}

void JsonWrapper::setObjectValue(const char* name, const cJSON* value) {
  if (!m_root || !name || !value) return;

  cJSON* node = findNode(name);
  if (node)
    cJSON_ReplaceItemInObject(m_root, name, cJSON_Duplicate(const_cast<cJSON*>(value), 1));
  else
    cJSON_AddItemToObject(m_root, name, cJSON_Duplicate(const_cast<cJSON*>(value), 1));
}

void JsonWrapper::setArrayValue(const char* name, const JsonWrapper& value) {
  setObjectValue(name, value);
}

bool JsonWrapper::isNull(const char* name) const {
  cJSON* node = findNode(name, cJSON_NULL);
  return node != nullptr;
}

bool JsonWrapper::isNull(int index) const {
  cJSON* node = findNode(index);
  return node != nullptr && node->type == cJSON_NULL;
}

bool JsonWrapper::isString(const char* name) const {
  cJSON* node = findNode(name, cJSON_String);
  return node != nullptr;
}

bool JsonWrapper::isString(int index) const {
  cJSON* node = findNode(index);
  return node != nullptr && node->type == cJSON_String;
}

bool JsonWrapper::isNumber(const char* name) const {
  cJSON* node = findNode(name, cJSON_Number);
  return node != nullptr;
}

bool JsonWrapper::isNumber(int index) const {
  cJSON* node = findNode(index);
  return node != nullptr && node->type == cJSON_Number;
}

bool JsonWrapper::isBoolean() const {
  if (m_root) {
    if (m_root->type == cJSON_True || m_root->type == cJSON_False) return true;
  }
  return false;
}

bool JsonWrapper::isBoolean(const char* name) const {
  cJSON* node = findNode(name);
  if (node) {
    if (node->type == cJSON_True || node->type == cJSON_False) return true;
  }
  return false;
}

bool JsonWrapper::isBoolean(int index) const {
  cJSON* node = findNode(index);
  if (node) {
    if (node->type == cJSON_True || node->type == cJSON_False) return true;
  }
  return false;
}

bool JsonWrapper::isObject(const char* name) const {
  cJSON* node = findNode(name, cJSON_Object);
  return node != nullptr;
}

bool JsonWrapper::isObject(int index) const {
  cJSON* node = findNode(index);
  return node != nullptr && node->type == cJSON_Object;
}

bool JsonWrapper::isArray(const char* name) const {
  cJSON* node = findNode(name, cJSON_Array);
  return node != nullptr;
}

bool JsonWrapper::isArray(int index) const {
  cJSON* node = findNode(index);
  return node != nullptr && node->type == cJSON_Array;
}

bool JsonWrapper::isObject() const { return isValid() && m_root->type == cJSON_Object; }

bool JsonWrapper::isString() const { return isValid() && m_root->type == cJSON_String; }

bool JsonWrapper::isArray() const { return isValid() && m_root->type == cJSON_Array; }

bool JsonWrapper::isUInt() const { return isInt(); }

bool JsonWrapper::isDouble() const { return isInt(); }

bool JsonWrapper::isInt() const { return isValid() && m_root->type == cJSON_Number; }

bool JsonWrapper::hasNode(const char* name) const { return findNode(name) != 0; }

std::string JsonWrapper::toString(bool formatted) const { return doToString(m_root, formatted); }

std::string JsonWrapper::doToString(cJSON* root, bool formatted) const {
  if (!root) return "";
  if (root->type == cJSON_String) return root->valuestring;

  char* json = formatted ? cJSON_Print(root) : cJSON_PrintUnformatted(root);
  if (!json) return "";
  std::string result(json);
  free(json);
  return result;
}

void JsonWrapper::merge(const char* profile) {
  JsonWrapper p(profile);
  merge(p);
}

void JsonWrapper::merge(JsonWrapper& profile) {
  if (isValid() && profile.isValid()) {
    doMerge(m_root, profile.m_root);
  } else if (profile.isValid()) {
    parse(nullptr);
    m_root = profile.m_root;
  }
  m_owner = true;
  profile.m_owner = false;
}

void JsonWrapper::reverse_merge(const char* profile) {
  JsonWrapper p(profile);
  reverse_merge(p);
}

void JsonWrapper::reverse_merge(JsonWrapper& profile) {
  if (isValid() && profile.isValid()) {
    doMerge(profile.m_root, m_root);
  }
  if (profile.isValid()) {
    parse(nullptr);
    m_root = profile.m_root;
    m_owner = true;
    profile.m_owner = false;
  }
}

void JsonWrapper::doMerge(cJSON* root1, cJSON* root2) {
  if (!root1 || !root2 || root1->type != root2->type) return;

  int which = 0;
  cJSON* item2 = root2->child;
  while (item2) {
    const char* name = item2->string;
    if (!name) {
      // node w/o a name, array item
      item2 = item2->next;
      cJSON_AddItemToArray(root1, cJSON_DetachItemFromArray(root2, which));
    } else {
      cJSON* item1 = cJSON_GetObjectItem(root1, name);
      if (!item1) {
        // the node does not exist, append
        item2 = item2->next;
        cJSON_AddItemToArray(root1, cJSON_DetachItemFromArray(root2, which));
      } else if (item1->child && item2->child && item1->type == item2->type) {
        // the node exists with the same type and both have children, merge it recursively
        doMerge(item1, item2);
        item2 = item2->next;
        which++;
      } else {
        // the node exists but the type is not the same, or either does not have children
        item2 = item2->next;
        cJSON_ReplaceItemInObject(root1, name, cJSON_DetachItemFromArray(root2, which));
      }
    }
  }
}
#if 0
void JsonWrapper::doMerge(cJSON* root1, cJSON* root2)
{
	if (!root1 || !root2 || root1->type != root2->type)
		return;

	for (cJSON* item2 = root2->child; item2 != nullptr; item2 = item2->next)
	{
		if (!item2->string)
			cJSON_AddItemToArray(root1, cJSON_Duplicate(item2, 1));
		else
		{
			cJSON* item1 = cJSON_GetObjectItem(root1, item2->string);
			if (!item1)
				cJSON_AddItemToObject(root1, item2->string, cJSON_Duplicate(item2, 1));
			else if (item1->child && item2->child && item1->type == item2->type)
				doMerge(item1, item2);
			else
				cJSON_ReplaceItemInObject(root1, item2->string, cJSON_Duplicate(item2, 1));
		}
	}
}
#endif
}  // namespace cjson
}  // namespace commons
}  // namespace agora
