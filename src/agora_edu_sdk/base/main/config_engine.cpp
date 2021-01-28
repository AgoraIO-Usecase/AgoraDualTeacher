//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include <base/config_engine.h>
#include <base/parameter_helper.h>
#include <algorithm>
#include "base/base_context.h"
#include "utils/log/log.h"
#include "utils/thread/base_worker.h"
#include "utils/tools/json_util.h"
#include "utils/tools/json_wrapper.h"

namespace agora {
namespace base {
using namespace agora::commons;

config::IConfigEngine* createConfigEngine() { return new ConfigEngine(); }

ConfigEngine::ConfigEngine() : m_defHandler(nullptr) { m_parameters.setObjectType(); }

ConfigEngine::~ConfigEngine() {}

void ConfigEngine::setWorker(BaseWorker* worker, INotification* notification,
                             bool suppressNotification) {
  m_callbacks = ConfigEngineCallbacks{
      [this, worker, notification, suppressNotification](const any_document_t& doc) {
        return worker->sync_call(LOCATION_HERE, [this, doc, notification, suppressNotification]() {
          return onSetParameters(notification, doc, false, suppressNotification);
        });
      },
      [this, worker](const any_document_t& doc, any_document_t& results) {
        return worker->sync_call(
            LOCATION_HERE,
            [this, doc, &results]() {
              onGetParameters(doc, results);
              return 0;
            },
            utils::DEFAULT_SYNC_CALL_TIMEOUT);
      }};
}

bool ConfigEngine::registerHandler(const std::string& key, ParameterBase* handler) {
  if (key.empty() || !handler) return false;
  auto r = m_handlers.emplace(key, handler);
  if (!r.second) return false;

  queryParameters(key, handler, true);

  return true;
}

bool ConfigEngine::unregisterHandler(const std::string& key) {
  if (m_handlers.size() == 0) return false;
  auto it = m_handlers.find(key);
  if (it == m_handlers.end()) return false;

  m_handlers.erase(it);
  return true;
}

bool ConfigEngine::registerHandler2(const std::string& key, ParameterBase* handler) {
  if (key.empty() || !handler) return false;
  auto r = m_handlers2.emplace(key, handler);
  if (!r.second) return false;

  queryParameters(key, handler, false);
  return true;
}

bool ConfigEngine::unregisterHandler2(const std::string& key) {
  auto it = m_handlers2.find(key);
  if (it == m_handlers2.end()) return false;

  m_handlers2.erase(it);
  return true;
}

void ConfigEngine::queryParameters(const std::string& key, ParameterBase* handler, bool exact) {
  if (exact) {
    auto node = m_parameters.findNode(key.c_str());
    if (node) {
      if (!handler->onSetParameter(key, node)) m_parameters.eraseNode(key.c_str());
    }
  } else {
    for (auto it = m_parameters.getChild(); it.isValid();) {
      const char* name = it.getName();
      if (!strncmp(key.c_str(), name, key.length())) {
        auto p = it.getRoot();
        if (!handler->onSetParameter(name, p)) {
          it = it.getNext();
          m_parameters.eraseNode(name);
        } else {
          it = it.getNext();
        }
      } else {
        it = it.getNext();
      }
    }
  }
}

bool ConfigEngine::setDefaultHandler(ParameterBase* handler) {
  m_defHandler = handler;
  return true;
}

int ConfigEngine::onGetParameters(const std::string& parameters, any_document_t& results) {
  any_document_t doc;
  if (!json::from_string(doc, parameters) || !json::is_array(doc)) return -ERR_INVALID_ARGUMENT;

  log(LOG_INFO, "[rp] %s", parameters.c_str());
  return onGetParameters(doc, results);
}

int ConfigEngine::onGetParameters(const any_document_t& doc, any_document_t& results) {
  results.setObjectType();
  for (auto it = const_cast<any_document_t&>(doc).getChild(); it.isValid(); it = it.getNext()) {
    if (!json::is_string(it)) continue;
    const char* key = it.getStringValue(nullptr);
    const char* args = nullptr;
    if (key && *key != '\0') {
      // extract key/args from key
      std::string tmp;
      const char* p = strchr(key, ':');
      if (p) {
        if (p[1] != '\0') args = p + 1;
        tmp.assign(key, p - key);
        key = tmp.c_str();
      }

      int r = -ERR_FAILED;
      any_document_t result;
      ParameterBase* h = findHandler(key);
      if (h) {
        if (!h->onGetParameter(key, args, result)) {
          results.addItemToObject(key, result.release());
        }
        r = 0;
      }
      if (r) {
        h = findHandler2(key);
        if (h) {
          r = h->onGetParameter(key, args, result);
          if (!r) {
            results.addItemToObject(key, result.release());
          }
          r = 0;
        }
      }
      if (r && m_defHandler && !m_defHandler->onGetParameter(key, args, result)) {
        results.addItemToObject(key, result.release());
      }
    }
  }
  return 0;
}

int ConfigEngine::setParameters(const char* parameters) {
  if (!parameters) return -ERR_INVALID_ARGUMENT;
  return onSetParameters(nullptr, commons::cjson::JsonWrapper(parameters), false, true);
}

int ConfigEngine::onSetParameters(INotification* notification, const any_document_t& doc,
                                  bool cache, bool suppressNotification, bool setOriginalValue) {
  if (!json::is_object(doc)) return -ERR_INVALID_ARGUMENT;
  AutoApiCallNotificationSuppression suppression(notification, suppressNotification);
  if (!suppressNotification) {
    std::string s = doc.toString();
    log(LOG_INFO, "[rp] %s", s.c_str());
  }

  int result = -ERR_FAILED;
  for (auto it = const_cast<any_document_t&>(doc).getChild(); it.isValid(); it = it.getNext()) {
    result = -ERR_NOT_SUPPORTED;
    const char* key = it.getName();
    auto value = it.getRoot();
    if (key && *key != '\0') {
      if (m_callbacks.on_set_value) m_callbacks.on_set_value(key, value);
      ParameterBase* h = findHandler(key);
      if (h) {
        result = h->onSetParameter(key, value);
        if (!result && setOriginalValue) h->setOriginalValue(value);
      }
      if (result == -ERR_NOT_SUPPORTED && m_defHandler) {
        result = m_defHandler->onSetParameter(key, value);
        if (!result && setOriginalValue) m_defHandler->setOriginalValue(value);
      }
      if (result == -ERR_NOT_SUPPORTED) {
        h = findHandler2(key);
        if (h) {
          result = h->onSetParameter(key, value);
          if (!result && setOriginalValue) h->setOriginalValue(value);
        }
      }
      if (result == -WARN_PENDING || (cache && result == -ERR_NOT_SUPPORTED)) {
        // default handler
        result = setParameters(key, std::move(value));
      }
    }
    if (notification && result != -WARN_PENDING)
      notification->onApiCallExecuted(result, key, nullptr);
  }
  log(LOG_INFO, "on set parameters doc='%s', ret=%d", doc.toString().c_str(), result);
  return result;
}

config::IParameter* ConfigEngine::getParameter(const char* key) {
  auto p = findHandler(key);
  if (!p) log(LOG_WARN, "cannot find parameter, key='%s'", key);
  return p;
}

ParameterBase* ConfigEngine::findHandler(const char* key) {
  if (key) {
    auto it = m_handlers.find(key);
    if (it != m_handlers.end()) return it->second;
  }
  return nullptr;
}

ParameterBase* ConfigEngine::findHandler2(const char* key) {
  for (auto it = m_handlers2.begin(); it != m_handlers2.end(); ++it) {
    if (!strncmp(key, it->first.c_str(), it->first.length())) {
      return it->second;
    }
  }
  return nullptr;
}

int ConfigEngine::setParameters(const char* key, const any_value_t& value) {
  // valid parameter must contain a dot
  if (strchr(key, '.')) m_parameters.setObjectValue(key, value);
  return -WARN_PENDING;
}

config::IParameter* ConfigEngine::createParameter(const char* key, config::AnyValue::Type valueType,
                                                  PARAMETER_TYPE paramType) {
  auto p = findHandler(key);
  if (p) return nullptr;
  switch (valueType) {
    case config::AnyValue::INTEGER:
      switch (paramType) {
        case agora::config::IConfigEngine::VALUE_ONLY:
          return new ParameterHelper<int>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_FILTER:
          return new ParameterHelperWithExternalFilter<int>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_OBSERVER:
          return new ParameterHelperWithExternalObserver<int>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_TRIGGER:
          return new TriggerParameterHelperWithExternalObserver<int>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_ORIGINAL_VALUE:
          return new ParameterHelperWithOriginalValue<int>(*this, key);
          break;
      }
      break;
    case config::AnyValue::UNSIGNED_INTEGER:
      switch (paramType) {
        case agora::config::IConfigEngine::VALUE_ONLY:
          return new ParameterHelper<unsigned int>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_FILTER:
          return new ParameterHelperWithExternalFilter<unsigned int>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_OBSERVER:
          return new ParameterHelperWithExternalObserver<unsigned int>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_TRIGGER:
          return new TriggerParameterHelperWithExternalObserver<unsigned int>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_ORIGINAL_VALUE:
          return new ParameterHelperWithOriginalValue<unsigned int>(*this, key);
          break;
      }
      break;
    case config::AnyValue::BOOLEAN:
      switch (paramType) {
        case agora::config::IConfigEngine::VALUE_ONLY:
          return new ParameterHelper<bool>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_FILTER:
          return new ParameterHelperWithExternalFilter<bool>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_OBSERVER:
          return new ParameterHelperWithExternalObserver<bool>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_TRIGGER:
          return new TriggerParameterHelperWithExternalObserver<bool>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_ORIGINAL_VALUE:
          return new ParameterHelperWithOriginalValue<bool>(*this, key);
          break;
      }
      break;
    case config::AnyValue::DOUBLE:
      switch (paramType) {
        case agora::config::IConfigEngine::VALUE_ONLY:
          return new ParameterHelper<double>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_FILTER:
          return new ParameterHelperWithExternalFilter<double>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_OBSERVER:
          return new ParameterHelperWithExternalObserver<double>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_TRIGGER:
          return new TriggerParameterHelperWithExternalObserver<double>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_ORIGINAL_VALUE:
          return new ParameterHelperWithOriginalValue<double>(*this, key);
          break;
      }
      break;
    case config::AnyValue::CSTR:
      switch (paramType) {
        case agora::config::IConfigEngine::VALUE_ONLY:
          return new ParameterHelper<std::string>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_FILTER:
          return new ParameterHelperWithExternalFilter<std::string>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_OBSERVER:
          return new ParameterHelperWithExternalObserver<std::string>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_TRIGGER:
          return new TriggerParameterHelperWithExternalObserver<std::string>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_ORIGINAL_VALUE:
          return new ParameterHelperWithOriginalValue<std::string>(*this, key);
          break;
      }
      break;
    case config::AnyValue::JSON:
      switch (paramType) {
        case agora::config::IConfigEngine::VALUE_ONLY:
          return new ParameterHelper<any_document_t>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_FILTER:
          return new ParameterHelperWithExternalFilter<any_document_t>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_OBSERVER:
          return new ParameterHelperWithExternalObserver<any_document_t>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_EXTERNAL_TRIGGER:
          return new TriggerParameterHelperWithExternalObserver<any_document_t>(*this, key);
          break;
        case agora::config::IConfigEngine::HAS_ORIGINAL_VALUE:
          return new ParameterHelperWithOriginalValue<any_document_t>(*this, key);
          break;
      }
      break;
    case config::AnyValue::UNSPEC:
      break;
  }
  return nullptr;
}

int ConfigEngineManager::setParameters(const char* parameters) {
  any_document_t doc;
  if (json::from_string(doc, parameters)) return setParameters(doc);
  return -ERR_INVALID_ARGUMENT;
}

int ConfigEngineManager::getParameters(const char* parameters, any_document_t& results) {
  any_document_t doc;
  if (json::from_string(doc, parameters)) return getParameters(doc, results);
  return -ERR_INVALID_ARGUMENT;
}

int ConfigEngineManager::parseParameters(any_document_t& doc,
                                         std::unordered_map<std::string, Item>& out) {
  bool isObject;
  if (json::is_object(doc))
    isObject = true;
  else if (json::is_array(doc))
    isObject = false;
  else
    return -ERR_INVALID_ARGUMENT;

  for (auto it = doc.getChild(); it.isValid(); it = it.getNext()) {
    const char* name;
    if (isObject)
      name = it.getName();
    else if (json::is_string(it))
      name = it.getStringValue(nullptr);
    else
      continue;
    std::string k;
    ConfigEngine* e = findConfigEngine(name, k);
    if (e) {
      auto i = out.find(k);
      if (i != out.end()) {
        i->second.doc.addItemReferenceToArray(it.getRoot());
      } else {
        auto& v = out[k];
        v.engine = e;
        if (isObject) {
          v.doc.setObjectType();
          v.doc.addItemReferenceToObject(name, it.getRoot());
        } else {
          v.doc.setArrayType();
          v.doc.addItemReferenceToArray(it.getRoot());
        }
      }
    }
  }
  return 0;
}

int ConfigEngineManager::setParameters(any_document_t& doc) {
  std::unordered_map<std::string, Item> items;
  int r = parseParameters(doc, items);
  if (!r) {
    for (auto& p : items) {
      do_setParameters(p.second.engine, p.second.doc);
    }
  }
  return r;
}

int ConfigEngineManager::do_setParameters(ConfigEngine* e, const any_document_t& doc) {
  if (e->callbacks().on_set_parameters)
    return e->callbacks().on_set_parameters(doc);
  else
    return e->onSetParameters(nullptr, doc, false, true);
}

int ConfigEngineManager::getParameters(any_document_t& doc, any_document_t& results) {
  std::unordered_map<std::string, Item> items;
  int r = parseParameters(doc, items);
  if (!r) {
    for (auto& p : items) {
      any_document_t out;
      int rr = do_getParameters(p.second.engine, p.second.doc, out);
      if (!rr) {
        moveResults(out, results);
      }
    }
  }
  return r;
}

void ConfigEngineManager::moveResults(any_document_t& from, any_document_t& to) {
  if (!from.isObject()) return;
  if (!to.isValid()) {
    to = std::move(from);
    return;
  }
  int c = from.getArraySize();
  while (c-- > 0) {
    std::string name = from.getChild().getName();
    auto item = from.detachItemFromObject(name.c_str());
    if (item) to.addItemToObject(name.c_str(), item);
  }
}

int ConfigEngineManager::do_getParameters(ConfigEngine* e, const any_document_t& doc,
                                          any_document_t& results) {
  if (e->callbacks().on_get_parameters)
    return e->callbacks().on_get_parameters(doc, results);
  else
    return e->onGetParameters(doc, results);
}

bool ConfigEngineManager::registerConfigEngine(const char* key, ConfigEngine* e) {
  if (!key || *key == '\0' || !e) return false;
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_engines.emplace(key, e);
  return it.second;
}

bool ConfigEngineManager::unregisterConfigEngine(const char* key) {
  if (!key || *key == '\0') return false;
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_engines.erase(key) > 0;
}

ConfigEngine* ConfigEngineManager::findConfigEngine(const char* name, std::string& key) {
  // e.g., rtc.audio.mute
  if (!name || strlen(name) < 5) return nullptr;

  char k[] = {name[0], name[1], name[2], name[3], '\0'};
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_engines.find(k);
  if (it != m_engines.end()) {
    key = k;
    return it->second;
  }
  return nullptr;
}

}  // namespace base
}  // namespace agora
