//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include "api2/internal/agora_service_i.h"

namespace agora {
namespace base {

class ParameterEngine : public IAgoraParameter {
 private:
  IParameterEngine* m_parameterEngine = nullptr;
  bool is_valid_str(const char* k) const { return k && *k != '\0'; }

 public:
  explicit ParameterEngine(IParameterEngine* pe);
  void release() override;
  int setBool(const char* key, bool value) override;
  int setInt(const char* key, int value) override;
  int setUInt(const char* key, unsigned int value) override;
  int setNumber(const char* key, double value) override;
  int setString(const char* key, const char* value) override;
  int setObject(const char* key, const char* value) override;
  int setArray(const char* key, const char* value) override;
  int getBool(const char* key, bool& value) override { return getBool(key, nullptr, value); }
  int getInt(const char* key, int& value) override { return getInt(key, nullptr, value); }
  int getUInt(const char* key, unsigned int& value) override {
    return getUInt(key, nullptr, value);
  }
  int getNumber(const char* key, double& value) override { return getNumber(key, nullptr, value); }
  int getString(const char* key, agora::util::AString& value) override {
    return getString(key, nullptr, value);
  }
  int getObject(const char* key, agora::util::AString& value) override {
    return getObject(key, nullptr, value);
  }
  int getArray(const char* key, const char* args, agora::util::AString& value) override;
  int setParameters(const char* parameters) override;
  int convertPath(const char* filePath, agora::util::AString& value) override;

 public:
  int getBool(const char* key, const char* args, bool& value);
  int getInt(const char* key, const char* args, int& value);
  int getUInt(const char* key, const char* args, unsigned int& value);
  int getNumber(const char* key, const char* args, double& value);
  int getString(const char* key, const char* args, agora::util::AString& value);
  int getObject(const char* key, const char* args, agora::util::AString& value);
  int setParameters(const any_document_t& doc);
  int getParameter(const char* key, const char* args, any_document_t& result);
  int setArray(const char* key, const any_document_t& value);
};

}  // namespace base
}  // namespace agora
