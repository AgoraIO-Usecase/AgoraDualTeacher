//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "config_service.h"

#include "base/ap_client.h"
#include "base/base_context.h"
#include "call_engine/ap_protocol.h"
#include "call_engine/call_events.h"
#include "main/core/rtc_globals.h"
#include "utils/storage/storage.h"
#include "utils/thread/thread_pool.h"
#include "utils/tools/crash_handler.h"

namespace agora {
namespace base {
class BaseContext;
}  // namespace base

namespace rtc {

const char MODULE_NAME[] = "[CS]";

static const uint32_t CDS_EXPIRED_MS = 30 * 60 * 1000;  // 30 mins

ConfigService::ConfigService(base::BaseContext& context, uint32_t areaCode) : context_(context) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  storage_ = rtc::RtcGlobals::Instance().Storage();
  assert(storage_);

  std::string path = !context_.getAppId().empty() ? context_.getAppId() : "global";
  path += "/configs";

  cds_cache_ = std::make_unique<SdkCache<std::string>>(path + "/cds", storage_);
  tds_cache_ = std::make_unique<SdkCache<TdsCacheItem>>(path + "/tds", storage_);

  {
    std::string install_id = context_.cache().getInstallIdWithSetMaybe();
    storage_->Save(path + "/general", "install_id", install_id.data(), install_id.length(),
                   CDS_EXPIRED_MS);
  }

  base::APManager::DefaultConfig ap_mgr_cfg;
  ap_mgr_cfg.worker = utils::major_worker();
  if (areaCode != AREA_CODE_GLOB) {
    commons::log(commons::LOG_INFO, "%s: AreaCode set by parameter: %d", MODULE_NAME, areaCode);
    ap_mgr_cfg.mDefaultIpList = context_.getDefaultIps(base::ApIpType::kNormalIp, areaCode);
    ap_mgr_cfg.mDefaultTlsIpList = context_.getDefaultIps(base::ApIpType::kTlsIp, areaCode);
    ap_mgr_cfg.mDomainList.emplace_back(
        context_.getDefaultDomain(base::DomainType::kNormalAp, areaCode));
    ap_mgr_cfg.mTlsDomainList.emplace_back(
        context_.getDefaultDomain(base::DomainType::kTlsAp, areaCode));
    ap_mgr_cfg.mIpv6DomainList.emplace_back(
        context_.getDefaultDomain(base::DomainType::kNormalApIpv6, areaCode));
  } else {
    std::string area_code_str =
        GetTdsValue(CONFIGURABLE_TAG_DEFAULT_IP, AB_TEST::A, CONFIGURABLE_KEY_RTC_IP_AREACODE);
    uint32_t area_code = 0;
    if (!area_code_str.empty() && sscanf(area_code_str.c_str(), "%u", &area_code) == 1) {
      commons::log(commons::LOG_INFO, "%s: AreaCode load from last config: %d", MODULE_NAME,
                   area_code);
      ap_mgr_cfg.mDefaultIpList = context_.getDefaultIps(base::ApIpType::kNormalIp, areaCode);
      ap_mgr_cfg.mDefaultTlsIpList = context_.getDefaultIps(base::ApIpType::kTlsIp, areaCode);
      ap_mgr_cfg.mDomainList.emplace_back(
          context_.getDefaultDomain(base::DomainType::kNormalAp, areaCode));
      ap_mgr_cfg.mTlsDomainList.emplace_back(
          context_.getDefaultDomain(base::DomainType::kTlsAp, areaCode));
      ap_mgr_cfg.mIpv6DomainList.emplace_back(
          context_.getDefaultDomain(base::DomainType::kNormalApIpv6, areaCode));
    }
  }

  ap_mgr_ = std::make_unique<base::APManager>(context_, &ap_mgr_cfg);

  ap_client_.reset(ap_mgr_->createAPClient());
  if (!ap_client_) {
    commons::log(commons::LOG_ERROR, "%s: AP Client not started", MODULE_NAME);
    return;
  }

  ap_client_->ap_event.connect(this,
                               std::bind(&ConfigService::OnAPEvent, this, std::placeholders::_1));

  context.networkMonitor()->network_changed.connect(
      this, std::bind(&ConfigService::OnNetworkChanged, this));

  Config config;
  config.device = context.getDeviceInfo();
  config.system = context.getSystemInfo();
  int numeric_version = 0;
  config.version = getAgoraSdkVersion(&numeric_version);
  config.appid = context_.getAppId();
  config.detail = context.getDeviceId();

  if (!SendRequest(config, protocol::AP_ADDRESS_TYPE_CDS | protocol::AP_ADDRESS_TYPE_TDS)) {
    commons::log(commons::LOG_INFO, "%s: failed to send req in ctor", MODULE_NAME);
  }

  cds_timer_ = std::unique_ptr<commons::timer_base>(utils::major_worker()->createTimer(
      std::bind(&ConfigService::OnCdsExpired, this), CDS_EXPIRED_MS));
}

ConfigService::~ConfigService() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  cds_cache_.reset();
  tds_cache_.reset();
  storage_.reset();
  ap_client_.reset();
  ap_mgr_.reset();
}

bool ConfigService::SendRequest(const Config& config, uint16_t flags) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (config.appid.empty()) {
    return false;
  }

  _FeaturesMap features_map;

  features_map["device"] = config.device;
  features_map["system"] = config.system;
  features_map["version"] = config.version;
  features_map["detail"] = config.detail;
  features_map["vendor"] = config.appid;
  features_map["install_id"] = context_.cache().getInstallIdWithSetMaybe();

  if (sid_.empty()) {
    sid_ = uuid();
    // save sid to BaseContext which will be reused by CallContext
    // FIXME(Ender):
    // I have no idea who and why introduce this brain damaged stupid thing
    // But FIX IT ASAP!
    context_.setSid(sid_);
  }

  features_map["session_id"] = sid_;

  last_issue_config_ = config;

  // send out the request
  ap_client_->requireConfiguration(features_map, flags);

  return true;
}

std::string ConfigService::GetCdsValue(const std::string& key) {
  std::string value;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &key, &value] {
    if (!cds_cache_->Get(key, value)) {
      commons::log(commons::LOG_INFO, "%s: unable to get value from CDS cache for key: %s",
                   MODULE_NAME, key.c_str());
      value = "";
      return -ERR_FAILED;
    }

    return static_cast<int>(ERR_OK);
  });

  return value;
}

std::string ConfigService::GetTdsValue(const std::string& feature, AB_TEST type,
                                       const std::string& key) {
  std::string value;

  (void)utils::major_worker()->sync_call(LOCATION_HERE, [this, &feature, type, &key, &value] {
    TdsCacheItem feature_val;

    if (!tds_cache_->Get(feature, feature_val)) {
      commons::log(commons::LOG_INFO, "%s: found no value for feature %s from TDS cache",
                   MODULE_NAME, feature.c_str());
      auto prefix_feature = "_store " + feature;
      if (!tds_cache_->Get(prefix_feature, feature_val)) {
        commons::log(commons::LOG_INFO, "%s: found no value for feature %s from TDS cache",
                     MODULE_NAME, prefix_feature.c_str());
        return -ERR_FAILED;
      }
    }

    TdsCacheItem::Plan* plan = (type == AB_TEST::A ? &feature_val.PlanA : &feature_val.PlanB);

    if (plan->configs.find(key) == plan->configs.end()) {
      commons::log(commons::LOG_INFO,
                   "%s: failed to find key %s from TDS cache item plan's configs", MODULE_NAME,
                   key.c_str());
      return -ERR_FAILED;
    }

    value = plan->configs[key];

    return static_cast<int>(ERR_OK);
  });

  return value;
}

int64_t ConfigService::RegisterConfigChangeObserver(ConfigChangeObserver&& func) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  int64_t id = 0;

  for (;;) {
    id = commons::getUniformRandomNum(1, INT64_MAX);
    if (observers_.find(id) == observers_.end()) break;
  }

  observers_[id] = std::move(func);

  return id;
}

void ConfigService::UnregisterConfigChangeObserver(int64_t id) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  observers_.erase(id);
}

void ConfigService::OnAPEvent(const signal::APEventData& ap_event_data) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (ap_event_data.err_code != ERR_OK) {
    commons::log(commons::LOG_ERROR, "%s: AP event data error in _onAPEvent()", MODULE_NAME);
    CallEvents::sendApEvent(context_, sid_, ap_event_data);
    return;
  }

  bool changed = false;
  if ((ap_event_data.flag & protocol::AP_ADDRESS_TYPE_CDS) != 0) {
    auto cds = ConfigParser::ParseCds(ap_event_data.config);
    for (const auto& pair : cds) {
      changed |= cds_cache_->Set(pair.first, pair.second, CDS_EXPIRED_MS);
    }
  } else if ((ap_event_data.flag & protocol::AP_ADDRESS_TYPE_TDS) != 0) {
    auto tds = ConfigParser::ParseTds(ap_event_data.config);
    for (const auto& pair : tds) {
      changed |= tds_cache_->Set(pair.first, pair.second);
    }
  }

  if (changed) {
    for (auto& ob : observers_) {
      if (ob.second) ob.second();
    }

    CallEvents::sendApEvent(context_, sid_, ap_event_data);
  }
}

void ConfigService::OnNetworkChanged() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!SendRequest(last_issue_config_, protocol::AP_ADDRESS_TYPE_CDS)) {
    commons::log(commons::LOG_ERROR, "%s: failed to send req in OnNetworkChanged()", MODULE_NAME);
  }
}

void ConfigService::OnCdsExpired() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (!SendRequest(last_issue_config_, protocol::AP_ADDRESS_TYPE_CDS)) {
    commons::log(commons::LOG_ERROR, "%s: failed to send req in OnCdsExpired()", MODULE_NAME);
  }
}

}  // namespace rtc
}  // namespace agora
