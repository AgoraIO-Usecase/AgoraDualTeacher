//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#pragma once
#include <list>
#include <string>
#include <unordered_map>

#include "IAgoraRtcEngine.h"
#include "base/AgoraRefPtr.h"
#include "utils/net/ip_type.h"
#include "utils/thread/thread_control_block.h"

namespace agora {

namespace base {
class BaseContext;
enum class ApIpType;
enum class DomainType;
}  // namespace base

namespace rtc {

using base::ApIpType;
using base::DomainType;

class ConfigService;

class PredefineIpList : public RefCountInterface {
 public:
  using AreaIpsMap = std::unordered_map<rtc::AREA_CODE, std::vector<commons::ip_t>>;
  using AreaIpsMapWithType = std::unordered_map<ApIpType, AreaIpsMap>;
  using AreaDomainsMap = std::unordered_map<rtc::AREA_CODE, std::vector<std::string>>;
  using AreaDomainsMapWithType = std::unordered_map<DomainType, AreaDomainsMap>;

 private:
  /**
   * PredefineIpList depend on ConfigService to pull Ip limit policy,
   * but ip list maybe queryed before ConfigService and PredefineIpList created,
   * in this case return default embedded ip list through static function
   */
  friend class base::BaseContext;
  static std::list<commons::ip_t> DefaultPredefineIpList(ApIpType type, uint32_t area_code);
  static std::string DefaultDomain(DomainType type, uint32_t area_code);
  static std::string AreaCodeName();
  static std::size_t AreaCount();

 public:
  PredefineIpList(rtc::ConfigService* config_service, uint32_t area_code);
  ~PredefineIpList();

  std::list<commons::ip_t> GetDefaultIps(ApIpType type) const;
  std::string GetDefaultDomain(DomainType type) const;
  std::string GetAreaCodeName() const;
  std::size_t GetAreaCount() const;

 private:
  static std::list<commons::ip_t> GetAreaEvenlyIpList(const AreaIpsMap& area_ips,
                                                      uint32_t area_code);
  static std::string GetAreaDomainFromList(const AreaDomainsMap& area_domains, uint32_t area_code);

  void initializeDefaultIps();
  void initializeDefaultDomains();
  void onConfigUpdated();
  void updateAreaCode();
  void updateIpListForArea(ApIpType type, rtc::AREA_CODE areaCode);

 private:
  rtc::ConfigService* config_service_;
  uint64_t config_observer_id_;

  uint32_t area_code_;
  std::size_t area_count_;
  std::string area_code_name_;
  AreaIpsMapWithType area_ips_;
  AreaDomainsMapWithType area_domains_;
  std::unordered_map<ApIpType, std::list<commons::ip_t>> default_ips_;
  std::unordered_map<DomainType, std::string> default_domain_;
};

}  // namespace rtc
}  // namespace agora
