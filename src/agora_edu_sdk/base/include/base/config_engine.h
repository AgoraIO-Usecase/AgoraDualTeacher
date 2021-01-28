//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <functional>
#include <mutex>
#include <unordered_map>

#include "api2/internal/config_engine_i.h"
#include "utils/tools/json_util.h"
#include "utils/tools/json_wrapper.h"

namespace agora {
namespace base {
class BaseWorker;
class INotification {
 public:
  virtual void suppressApiCallNotification(bool suppressed) = 0;
  virtual bool isApiCallNotificationSuppressed() const = 0;
  virtual void onApiCallExecuted(int err, const char* api, const char* result) = 0;
};

class ParameterBase : public config::IParameter {
 public:
  virtual ~ParameterBase() {}
  void release() override { delete this; }
  bool getValue(config::AnyValue& value) const override { return false; }
  bool setValue(const config::AnyValue& value, bool storeOnly) override { return false; }
  bool getOriginalValue(config::AnyValue& value) const override { return false; }
  bool setOriginalValue(const config::AnyValue& v) override { return false; }
  virtual bool setOriginalValue(const any_value_t& value) { return false; }
  bool connectExternalObserver(config::IObserver* observer, bool triggerOnConnect) override {
    return false;
  }
  bool disconnectExternalObserver() override { return false; }
  bool connectExternalFilter(config::IFilter* filter, bool triggerOnConnect) override {
    return false;
  }
  bool disconnectExternalFilter() override { return false; }
  virtual int onSetParameter(const std::string& key, const any_value_t& value) = 0;
  virtual int onGetParameter(const std::string& key, const char* args, any_document_t& out) = 0;
  virtual void disconnect() {}
};

struct ConfigEngineCallbacks {
  using set_parameters_type = std::function<int(const any_document_t& doc)>;
  using get_parameters_type =
      std::function<int(const any_document_t& doc, any_document_t& results)>;
  using set_value_type = std::function<void(const char*, any_value_t& value)>;
  set_parameters_type on_set_parameters;
  get_parameters_type on_get_parameters;
  set_value_type on_set_value;
  ConfigEngineCallbacks(set_parameters_type&& s = nullptr, get_parameters_type&& g = nullptr,
                        set_value_type&& sv = nullptr)
      : on_set_parameters(std::move(s)),
        on_get_parameters(std::move(g)),
        on_set_value(std::move(sv)) {}
};
class ConfigEngine : public config::IConfigEngine {
 public:
  ConfigEngine();
  ~ConfigEngine();
  // full match
  bool registerHandler(const std::string& key, ParameterBase* handler);
  bool unregisterHandler(const std::string& key);
  // partial match
  bool registerHandler2(const std::string& key, ParameterBase* handler);
  bool unregisterHandler2(const std::string& key);
  bool setDefaultHandler(ParameterBase* handler);
  int onGetParameters(const std::string& parameters, any_document_t& results);
  int onGetParameters(const any_document_t& doc, any_document_t& results);
  int onSetParameters(INotification* notification, const any_document_t& doc, bool cache,
                      bool suppressNotification, bool setOriginalValue = false);
  void queryParameters(const std::string& key, ParameterBase* handler, bool exact);
  config::IParameter* createParameter(const char* key, config::AnyValue::Type valueType,
                                      PARAMETER_TYPE paramType) override;
  config::IParameter* getParameter(const char* key) override;
  int setParameters(const char* parameters) override;

 public:
  void setWorker(BaseWorker* worker, INotification* notification, bool suppressNotification);
  ConfigEngineCallbacks& callbacks() { return m_callbacks; }

 public:
  template <class T>
  int setParameter(INotification* notification, const char* key, const T& value) {
    any_document_t doc;
    doc.setObjectType();

    commons::json::insert(doc, key, value);
    return onSetParameters(notification, doc, false, true);
  }

 private:
  int onParameter2(const char* key, const any_value_t* in, any_document_t* out);
  ParameterBase* findHandler(const char* key);
  ParameterBase* findHandler2(const char* key);
  int setParameters(const char* key, const any_value_t& value);

 private:
  std::unordered_map<std::string, ParameterBase*> m_handlers;   // exact match
  std::unordered_map<std::string, ParameterBase*> m_handlers2;  // partial match
  ParameterBase* m_defHandler;
  any_document_t m_parameters;
  ConfigEngineCallbacks m_callbacks;
};

class ConfigEngineManager {
  struct Item {
    ConfigEngine* engine;
    any_document_t doc;
  };

 public:
  bool registerConfigEngine(const char* key, ConfigEngine* e);
  bool unregisterConfigEngine(const char* key);
  int setParameters(const char* parameters);
  int getParameters(const char* parameters, any_document_t& results);

 private:
  int setParameters(any_document_t& doc);
  int getParameters(any_document_t& doc, any_document_t& results);
  int parseParameters(any_document_t& doc, std::unordered_map<std::string, Item>& out);
  int do_setParameters(ConfigEngine* e, const any_document_t& doc);
  int do_getParameters(ConfigEngine* e, const any_document_t& doc, any_document_t& results);
  void moveResults(any_document_t& from, any_document_t& to);
  ConfigEngine* findConfigEngine(const char* name, std::string& key);

 private:
  std::mutex m_mutex;
  std::unordered_map<std::string, ConfigEngine*> m_engines;
};

class AutoApiCallNotificationSuppression {
  base::INotification* ntf_;
  bool prev_;

 public:
  AutoApiCallNotificationSuppression(base::INotification* ntf, bool suppressNotification)
      : ntf_(ntf) {
    if (!ntf) return;
    prev_ = ntf->isApiCallNotificationSuppressed();
    ntf->suppressApiCallNotification(suppressNotification);
  }
  ~AutoApiCallNotificationSuppression() {
    if (ntf_) ntf_->suppressApiCallNotification(prev_);
  }
};

}  // namespace base
}  // namespace agora
