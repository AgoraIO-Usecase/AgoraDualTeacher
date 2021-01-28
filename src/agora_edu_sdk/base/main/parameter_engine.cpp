//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "parameter_engine.h"
#include <base/base_util.h>
#include <cerrno>
#include <iterator>
#include "utils/log/log.h"
#include "utils/tools/json_util.h"

namespace agora {
namespace base {
using namespace agora::commons;
ParameterEngine::ParameterEngine(agora::base::IParameterEngine* pe) : m_parameterEngine(pe) {}

void ParameterEngine::release() { delete this; }

int ParameterEngine::setParameters(const any_document_t& doc) {
  std::string str = json::to_string(doc);
  return m_parameterEngine ? m_parameterEngine->setParameters(str.c_str()) : -ERR_NOT_INITIALIZED;
}

int ParameterEngine::setBool(const char* key, bool value) {
  if (!is_valid_str(key)) return -ERR_INVALID_ARGUMENT;
  any_document_t doc;
  doc.setObjectType();

  json::insert(doc, key, value);
  return setParameters(doc);
}

int ParameterEngine::setInt(const char* key, int value) {
  if (!is_valid_str(key)) return -ERR_INVALID_ARGUMENT;
  any_document_t doc;
  doc.setObjectType();
  json::insert(doc, key, value);
  return setParameters(doc);
}

int ParameterEngine::setUInt(const char* key, unsigned int value) {
  if (!is_valid_str(key)) return -ERR_INVALID_ARGUMENT;
  any_document_t doc;
  doc.setObjectType();
  json::insert(doc, key, value);
  return setParameters(doc);
}

int ParameterEngine::setNumber(const char* key, double value) {
  if (!is_valid_str(key)) return -ERR_INVALID_ARGUMENT;
  any_document_t doc;
  doc.setObjectType();
  json::insert(doc, key, value);
  return setParameters(doc);
}

int ParameterEngine::setString(const char* key, const char* value) {
  if (!is_valid_str(key) || !value) return -ERR_INVALID_ARGUMENT;
  any_document_t doc;
  doc.setObjectType();
  json::insert(doc, key, value);
  return setParameters(doc);
}

int ParameterEngine::setObject(const char* key, const char* value) {
  if (!is_valid_str(key) || !value) return -ERR_INVALID_ARGUMENT;
  std::string str = "{\"";
  str += key;
  str += "\":";
  str += value;
  str += "}";

  any_document_t doc;
  if (!json::from_string(doc, str)) return -ERR_INVALID_ARGUMENT;
  return setParameters(doc);
}

int ParameterEngine::setArray(const char* key, const char* value) {
  const any_document_t anyValue(value);
  return setArray(key, anyValue);
}

int ParameterEngine::setArray(const char* key, const any_document_t& value) {
  if (!key || !json::is_array(value)) return -ERR_INVALID_ARGUMENT;
  any_document_t doc;
  doc.setObjectType();
  doc.setArrayValue(key, value);
  return setParameters(doc);
}

int ParameterEngine::setParameters(const char* parameters) {
  if (!is_valid_str(parameters)) return -ERR_INVALID_ARGUMENT;
  return m_parameterEngine ? m_parameterEngine->setParameters(parameters) : -ERR_NOT_INITIALIZED;
}

int ParameterEngine::convertPath(const char* filePath, agora::util::AString& value) {
  if (!is_valid_str(filePath)) return -ERR_INVALID_ARGUMENT;
  std::string out;
  std::transform(filePath, filePath + strlen(filePath), std::back_inserter(out),
                 [](char ch) { return ch == '\\' ? '/' : ch; });
  value.reset(new agora::util::StringImpl(std::move(out)));
  return 0;
}

int ParameterEngine::getParameter(const char* key, const char* args, any_document_t& result) {
  if (!m_parameterEngine) return -ERR_NOT_INITIALIZED;
  any_document_t doc;
  doc.setArrayType();
  if (!args || *args == '\0') {
    json::push_back(doc, key);
  } else {
    std::string tmp = key;
    tmp += ':';
    tmp += args;
    json::push_back(doc, tmp);
  }
  std::string str = json::to_string(doc);
  int r = m_parameterEngine->getParameters(str.c_str(), result);
  if (r) return r;
  auto it = result.findNode(key);
  if (it) {
    any_document_t tmp(it);
    result.assign(tmp.duplicate());
    return 0;
  }
  return -ERR_NOT_INITIALIZED;
}

int ParameterEngine::getBool(const char* key, const char* args, bool& value) {
  any_document_t result;
  int r = getParameter(key, args, result);
  if (r) return r;
  if (!json::get_bool(result, value)) return -ERR_INVALID_ARGUMENT;
  return 0;
}

int ParameterEngine::getInt(const char* key, const char* args, int& value) {
  any_document_t result;
  int r = getParameter(key, args, result);
  if (r) return r;
  if (!json::get_int(result, value)) return -ERR_INVALID_ARGUMENT;
  return 0;
}

int ParameterEngine::getUInt(const char* key, const char* args, unsigned int& value) {
  any_document_t result;
  int r = getParameter(key, args, result);
  if (r) return r;
  if (!json::get_uint(result, value)) return -ERR_INVALID_ARGUMENT;
  return 0;
}

int ParameterEngine::getNumber(const char* key, const char* args, double& value) {
  any_document_t result;
  int r = getParameter(key, args, result);
  if (r) return r;
  if (!json::get_double(result, value)) return -ERR_INVALID_ARGUMENT;
  return 0;
}

int ParameterEngine::getString(const char* key, const char* args, agora::util::AString& value) {
  any_document_t result;
  int r = getParameter(key, args, result);
  if (r) return r;
  std::string tmp;
  if (!json::get_string(result, tmp)) return -ERR_INVALID_ARGUMENT;
  value.reset(new agora::util::StringImpl(std::move(tmp)));
  return 0;
}

int ParameterEngine::getObject(const char* key, const char* args, agora::util::AString& value) {
  any_document_t result;
  int r = getParameter(key, args, result);
  if (r) return r;
  if (!json::is_object(result)) return -ERR_INVALID_ARGUMENT;
  std::string str = json::to_string(result);
  value.reset(new agora::util::StringImpl(std::move(str)));
  return 0;
}

int ParameterEngine::getArray(const char* key, const char* args, agora::util::AString& value) {
  any_document_t result;
  int r = getParameter(key, args, result);
  if (r) return r;
  if (!json::is_array(result)) return -ERR_INVALID_ARGUMENT;
  std::string str = json::to_string(result);
  value.reset(new agora::util::StringImpl(std::move(str)));
  return 0;
}
}  // namespace base
}  // namespace agora
