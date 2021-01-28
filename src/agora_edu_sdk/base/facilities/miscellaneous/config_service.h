//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once

#include <memory>

#include "api2/internal/config_engine_i.h"
#include "base/ap_client.h"
#include "call_engine/rtc_signal_type.h"
#include "facilities/miscellaneous/internal/cache.h"
#include "facilities/miscellaneous/internal/config_parser.h"
#include "facilities/tools/rtc_callback.h"
#include "sigslot.h"

namespace agora {
namespace base {
class BaseContext;
class APClient;
class APManager;
}  // namespace base

namespace rtc {

class ConfigService : public agora::has_slots<> {
 private:
  using _FeaturesMap = std::unordered_map<std::string, std::string>;

 public:
  using ConfigChangeObserver = std::function<void()>;

  enum class AB_TEST { A, B };

  struct Config {
    std::string device;
    std::string system;
    std::string version;
    std::string appid;
    std::string detail;
  };

 public:
  explicit ConfigService(base::BaseContext& context, uint32_t areaCode = AREA_CODE_GLOB);
  ~ConfigService();

  bool SendRequest(const Config& config, uint16_t flags);

  std::string GetCdsValue(const std::string& key);

  std::string GetTdsValue(const std::string& feature, AB_TEST type, const std::string& key);

  int64_t RegisterConfigChangeObserver(ConfigChangeObserver&& func);

  void UnregisterConfigChangeObserver(int64_t id);

 private:
  void OnAPEvent(const signal::APEventData& ap_event_data);
  void OnNetworkChanged();
  void OnCdsExpired();

 private:
  base::BaseContext& context_;
  std::unique_ptr<base::APManager> ap_mgr_;
  std::unique_ptr<base::APClient> ap_client_;
  std::unique_ptr<commons::timer_base> cds_timer_;
  Config last_issue_config_;
  std::shared_ptr<utils::Storage> storage_;
  std::unique_ptr<SdkCache<std::string>> cds_cache_;
  std::unique_ptr<SdkCache<TdsCacheItem>> tds_cache_;
  std::unordered_map<int64_t, ConfigChangeObserver> observers_;

  std::string sid_;
};

}  // namespace rtc
}  // namespace agora
