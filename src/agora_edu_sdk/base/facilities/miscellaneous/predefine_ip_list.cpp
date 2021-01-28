//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-05.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "predefine_ip_list.h"

#include <algorithm>
#include <bitset>
#include <map>

#include "base/base_context.h"
#include "facilities/miscellaneous/config_service.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

using base::ApIpType;
using base::DomainType;

const char MODULE_NAME[] = "[PIP]";

static const std::vector<rtc::AREA_CODE> supported_area_codes = {
    rtc::AREA_CODE_CN, rtc::AREA_CODE_AS, rtc::AREA_CODE_NA, rtc::AREA_CODE_EU};

static const std::vector<ApIpType> predefine_ip_types = {ApIpType::kNormalIp, ApIpType::kTlsIp,
                                                         ApIpType::kProxyIp, ApIpType::kProxyTlsIp};

static const std::vector<DomainType> predefine_domain_types = {
    DomainType::kNormalAp, DomainType::kNormalApIpv6, DomainType::kTlsAp, DomainType::kProxyAp,
    DomainType::kProxyTlsAp};

static std::unordered_map<ApIpType, std::unordered_map<rtc::AREA_CODE, std::string>>
    area_code_to_config_param_map = {
        {ApIpType::kNormalIp,
         {{rtc::AREA_CODE_CN, CONFIGURABLE_KEY_RTC_IP_AREACODE_CN},
          {rtc::AREA_CODE_NA, CONFIGURABLE_KEY_RTC_IP_AREACODE_NA},
          {rtc::AREA_CODE_EU, CONFIGURABLE_KEY_RTC_IP_AREACODE_EUR},
          {rtc::AREA_CODE_AS, CONFIGURABLE_KEY_RTC_IP_AREACODE_AS}}},
        {ApIpType::kTlsIp,
         {{rtc::AREA_CODE_CN, CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_CN},
          {rtc::AREA_CODE_NA, CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_NA},
          {rtc::AREA_CODE_EU, CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_EUR},
          {rtc::AREA_CODE_AS, CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_AS}}},
        {ApIpType::kProxyIp,
         {{rtc::AREA_CODE_CN, CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_CN},
          {rtc::AREA_CODE_NA, CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_NA},
          {rtc::AREA_CODE_EU, CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_EUR},
          {rtc::AREA_CODE_AS, CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_AS}}},
        {ApIpType::kProxyTlsIp,
         {{rtc::AREA_CODE_CN, CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_CN},
          {rtc::AREA_CODE_NA, CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_NA},
          {rtc::AREA_CODE_EU, CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_EUR},
          {rtc::AREA_CODE_AS, CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_AS}}}};

static const PredefineIpList::AreaIpsMapWithType default_predefine_ip_list = {
    {ApIpType::kNormalIp,
     {{rtc::AREA_CODE_CN, {"106.14.12.130", "47.107.39.93", "118.190.148.38", "112.126.96.46"}},
      {rtc::AREA_CODE_EU, {"52.58.56.244", "35.178.208.187"}},
      {rtc::AREA_CODE_NA, {"52.52.84.170", "50.17.126.121"}},
      {rtc::AREA_CODE_AS, {"3.0.163.78", "52.194.158.59"}}}},
    {ApIpType::kTlsIp,
     {{rtc::AREA_CODE_CN, {"123.56.235.221", "101.132.108.165"}},
      {rtc::AREA_CODE_EU, {"52.28.239.238", "3.9.120.239"}},
      {rtc::AREA_CODE_NA, {"52.54.85.111", "184.72.18.217"}},
      {rtc::AREA_CODE_AS, {"13.250.89.184", "18.176.162.64"}}}},
    {ApIpType::kProxyIp,
     {{rtc::AREA_CODE_CN, {"140.210.77.68", "125.88.159.163"}},
      {rtc::AREA_CODE_EU, {"128.1.77.34", "128.1.78.146"}},
      {rtc::AREA_CODE_NA, {"69.28.51.142", "107.155.14.132"}},
      {rtc::AREA_CODE_AS, {"128.1.87.146", "54.178.26.110"}}}},
    {ApIpType::kProxyTlsIp,
     {{rtc::AREA_CODE_CN, {"123.56.235.221", "101.132.108.165"}},
      {rtc::AREA_CODE_EU, {"52.28.239.238", "3.9.120.239"}},
      {rtc::AREA_CODE_NA, {"52.54.85.111", "184.72.18.217"}},
      {rtc::AREA_CODE_AS, {"13.250.89.184", "18.176.162.64"}}}}};

static const PredefineIpList::AreaDomainsMapWithType default_predefine_domain_list = {
    {DomainType::kNormalAp,
     {{rtc::AREA_CODE_CN, {"ap.agoraio.cn"}},
      {rtc::AREA_CODE_EU,
       {"ap-europe.agora.io", "ap-europe2.agora.io", "ap-europe3.agora.io", "ap-europe4.agora.io"}},
      {rtc::AREA_CODE_NA,
       {"ap-america.agora.io", "ap-america2.agora.io", "ap-america3.agora.io",
        "ap-america4.agora.io"}},
      {rtc::AREA_CODE_AS,
       {"ap-asia.agora.io", "ap-asia2.agora.io", "ap-asia3.agora.io", "ap-asia4.agora.io"}},
      {rtc::AREA_CODE_GLOB,
       {"ap1.agora.io", "ap2.agora.io", "ap3.agora.io", "ap4.agora.io", "ap5.agora.io"}}}},
    {DomainType::kNormalApIpv6,
     {{rtc::AREA_CODE_CN, {"ap-ipv6.agoraio.cn"}},
      {rtc::AREA_CODE_EU, {"ap1-europe-ipv6.agora.io", "ap2-europe-ipv6.agora.io"}},
      {rtc::AREA_CODE_NA, {"ap1-america-ipv6.agora.io", "ap2-america-ipv6.agora.io"}},
      {rtc::AREA_CODE_AS, {"ap1-asia-ipv6.agora.io", "ap2-asia-ipv6.agora.io"}},
      {rtc::AREA_CODE_GLOB, {"ap1-ipv6.agora.io"}}}},
    {DomainType::kTlsAp,
     {{rtc::AREA_CODE_CN, {"ap-tls.agoraio.cn"}},
      {rtc::AREA_CODE_EU, {"ap-europe-tls.agora.io", "ap-europe2-tls.agora.io"}},
      {rtc::AREA_CODE_NA, {"ap-america-tls.agora.io", "ap-america2-tls.agora.io"}},
      {rtc::AREA_CODE_AS, {"ap-asia-tls.agora.io", "ap-asia2-tls.agora.io"}},
      {rtc::AREA_CODE_GLOB,
       {"ap1-tls.agora.io", "ap2-tls.agora.io", "ap3-tls.agora.io", "ap4-tls.agora.io",
        "ap5-tls.agora.io"}}}},
    {DomainType::kProxyAp,
     {{rtc::AREA_CODE_CN, {"ap-cloud-proxy.agoraio.cn"}},
      {rtc::AREA_CODE_EU, {"ap-cloud-proxy-europe.agora.io"}},
      {rtc::AREA_CODE_NA, {"ap-cloud-proxy-america.agora.io"}},
      {rtc::AREA_CODE_AS, {"ap-cloud-proxy-asia.agora.io"}},
      {rtc::AREA_CODE_GLOB, {"ap-cloud-proxy.agora.io"}}}},
    {DomainType::kProxyTlsAp,
     {{rtc::AREA_CODE_CN, {"ap-tls-proxy.agoraio.cn"}},
      {rtc::AREA_CODE_EU, {"ap-tls-proxy-europe.agora.io"}},
      {rtc::AREA_CODE_NA, {"ap-tls-proxy-america.agora.io"}},
      {rtc::AREA_CODE_AS, {"ap-tls-proxy-asia.agora.io"}},
      {rtc::AREA_CODE_GLOB, {"ap-tls-proxy.agora.io"}}}}};

static const std::vector<commons::ip_t> default_predefine_ipv6_list = {
    ip::from_string("2600:1f18:64ea:9401:50:17:126:121"),
    ip::from_string("2406:da14:97f:4701:52:194:158:59")};

static void AppendCodeName(std::string* name, uint32_t code, rtc::AREA_CODE area_flag,
                           const char* area_tag) {
  if (code & area_flag) {
    if (!name->empty()) {
      name->append(",");
    }
    name->append(area_tag);
  }
}

static std::string ToAreaCodeName(uint32_t area_code) {
  if (area_code == rtc::AREA_CODE_GLOB) {
    return "GLOBAL";
  } else {
    std::string name;
    AppendCodeName(&name, area_code, rtc::AREA_CODE_CN, "CN");
    AppendCodeName(&name, area_code, rtc::AREA_CODE_NA, "US");
    AppendCodeName(&name, area_code, rtc::AREA_CODE_EU, "EU");
    AppendCodeName(&name, area_code, rtc::AREA_CODE_AS, "AS");
    return name;
  }
}

static std::size_t ParseAreaCount(uint32_t area_code) {
  std::bitset<32> area_bits(area_code);
  return area_bits.count();
}

PredefineIpList::PredefineIpList(rtc::ConfigService* config_service, uint32_t area_code)
    : config_service_(config_service), config_observer_id_(0), area_code_(area_code) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  area_code_name_ = ToAreaCodeName(area_code_);
  area_count_ = ParseAreaCount(area_code_);
  initializeDefaultIps();
  initializeDefaultDomains();

  if (!config_service_) {
    commons::log(commons::LOG_ERROR, "%s: Config Servcie not created yet", MODULE_NAME);
    return;
  }

  onConfigUpdated();
  config_observer_id_ =
      config_service_->RegisterConfigChangeObserver([this] { onConfigUpdated(); });
}

PredefineIpList::~PredefineIpList() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  if (config_service_ && config_observer_id_ > 0) {
    config_service_->UnregisterConfigChangeObserver(config_observer_id_);
  }
}

void shuffle_area_ips(PredefineIpList::AreaIpsMap& area_ips) {
  for (auto& ips : area_ips) {
    std::shuffle(ips.second.begin(), ips.second.end(), getRndGenerator());
  }
}

std::list<commons::ip_t> PredefineIpList::DefaultPredefineIpList(ApIpType type,
                                                                 uint32_t area_code) {
  auto it = default_predefine_ip_list.find(type);
  if (it == default_predefine_ip_list.end()) {
    return std::list<commons::ip_t>();
  }
  auto default_area_ips = it->second;

  shuffle_area_ips(default_area_ips);

  return GetAreaEvenlyIpList(default_area_ips, area_code);
}

std::string PredefineIpList::DefaultDomain(DomainType type, uint32_t area_code) {
  auto it = default_predefine_domain_list.find(type);
  if (it == default_predefine_domain_list.end()) {
    return std::string();
  }
  auto domains = it->second;
  return GetAreaDomainFromList(domains, area_code);
}

std::string PredefineIpList::AreaCodeName() {
  return ToAreaCodeName(static_cast<uint32_t>(rtc::AREA_CODE_GLOB));
}

std::size_t PredefineIpList::AreaCount() {
  return ParseAreaCount(static_cast<uint32_t>(rtc::AREA_CODE_GLOB));
}

std::list<commons::ip_t> PredefineIpList::GetAreaEvenlyIpList(const AreaIpsMap& area_ips,
                                                              uint32_t area_code) {
  // Make ip list in multiple areas evenly in the list,
  // and to make sure top N ips include ip from different areas

  // TODO(xwang): we place China and Asia area codes before other area to make sure top N inlcudes
  // China IP to speed up connection from China, we need to make it truely shuffled in future

  std::map<std::size_t, commons::ip_t> ordered_ips;
  size_t index = 0;
  for (auto code : supported_area_codes) {
    auto pos = index++;

    if ((code & area_code) == 0) {
      continue;
    }

    if (area_ips.find(code) == area_ips.end()) {
      continue;
    }

    const auto& area = area_ips.at(code);
    for (const auto& ip : area) {
      ordered_ips.emplace(pos, ip);
      pos += supported_area_codes.size();
    }
  }

  std::list<commons::ip_t> ip_list;
  for (const auto& ip_pair : ordered_ips) {
    ip_list.emplace_back(ip_pair.second);
  }

  std::copy(default_predefine_ipv6_list.begin(), default_predefine_ipv6_list.end(),
            std::back_inserter(ip_list));

  return ip_list;
}

std::string PredefineIpList::GetAreaDomainFromList(const AreaDomainsMap& area_domains,
                                                   uint32_t area_code) {
  std::bitset<32> area_bits(area_code);
  auto area_count = area_bits.count();
  std::vector<std::string> domains;
  if (area_count == 0) {
    return std::string();
  } else if (area_count == 1) {
    auto it = area_domains.find(static_cast<rtc::AREA_CODE>(area_code));
    if (it == area_domains.end()) {
      return std::string();
    }
    domains = it->second;
  } else {
    // Always use global area domain if area code > 1.
    auto it = area_domains.find(rtc::AREA_CODE_GLOB);
    if (it == area_domains.end()) {
      return std::string();
    }
    domains = it->second;
  }
  if (domains.empty()) {
    return std::string();
  } else {
    std::shuffle(domains.begin(), domains.end(), getRndGenerator());
    return domains[0];
  }
}

void PredefineIpList::initializeDefaultIps() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  area_ips_ = default_predefine_ip_list;

  for (auto type : predefine_ip_types) {
    shuffle_area_ips(area_ips_[type]);

    default_ips_[type] = PredefineIpList::GetAreaEvenlyIpList(area_ips_[type], area_code_);
  }
}

void PredefineIpList::initializeDefaultDomains() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  area_domains_ = default_predefine_domain_list;
  for (auto type : predefine_domain_types) {
    default_domain_[type] = PredefineIpList::GetAreaDomainFromList(area_domains_[type], area_code_);
  }
}

void PredefineIpList::onConfigUpdated() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  updateAreaCode();

  for (auto type : predefine_ip_types) {
    for (auto code : supported_area_codes) {
      updateIpListForArea(type, code);
    }
    default_ips_[type] = PredefineIpList::GetAreaEvenlyIpList(area_ips_[type], area_code_);
  }
  for (auto type : predefine_domain_types) {
    default_domain_[type] = PredefineIpList::GetAreaDomainFromList(area_domains_[type], area_code_);
  }
}

void PredefineIpList::updateAreaCode() {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  std::string area_code_str =
      config_service_->GetTdsValue(CONFIGURABLE_TAG_DEFAULT_IP, rtc::ConfigService::AB_TEST::A,
                                   CONFIGURABLE_KEY_RTC_IP_AREACODE);
  if (area_code_str.empty()) {
    return;
  }

  uint32_t area_code = 0;
  if (sscanf(area_code_str.c_str(), "%u", &area_code) == 1 && area_code > 0) {
    area_code_ = area_code;
    area_code_name_ = ToAreaCodeName(area_code_);
    area_count_ = ParseAreaCount(area_code_);
    commons::log(commons::LOG_INFO, "%s: area code update to:%u", MODULE_NAME, area_code);
  } else {
    commons::log(commons::LOG_WARN, "%s: invalid area code:%s", MODULE_NAME, area_code_str.c_str());
  }
}

void PredefineIpList::updateIpListForArea(ApIpType type, rtc::AREA_CODE area_code) {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());

  std::string ip_list_json_str =
      config_service_->GetTdsValue(CONFIGURABLE_TAG_DEFAULT_IP, rtc::ConfigService::AB_TEST::A,
                                   area_code_to_config_param_map[type][area_code]);
  if (ip_list_json_str.empty()) {
    return;
  }

  any_document_t ip_list_doc(ip_list_json_str);
  if (!ip_list_doc.isValid()) {
    commons::log(commons::LOG_ERROR, "%s: invalid json format for ip list:%s", MODULE_NAME,
                 ip_list_json_str.c_str());
    return;
  }

  std::vector<ip_t> ip_list;
  auto child = ip_list_doc.getChild();
  while (child.isValid()) {
    std::string ipStr = child.getStringValue("");
    if (ipStr.empty()) {
      commons::log(commons::LOG_ERROR, "%s: empty ip found for area code:%d with type:%d",
                   MODULE_NAME, area_code, static_cast<int>(type));
      child = child.getNext();
      continue;
    }

    commons::log(commons::LOG_INFO, "%s: found config ip %s for area:%d with type:%d", MODULE_NAME,
                 commons::desensetizeIp(ipStr).c_str(), area_code, static_cast<int>(type));

    if (std::find(ip_list.begin(), ip_list.end(), ipStr) != ip_list.end()) {
      commons::log(commons::LOG_INFO, "%s: duplicate ip %s found for area:%d with type:%d",
                   MODULE_NAME, commons::desensetizeIp(ipStr).c_str(), area_code,
                   static_cast<int>(type));
      child = child.getNext();
      continue;
    }

    ip_list.emplace_back(std::move(ipStr));
    child = child.getNext();
  }

  if (ip_list.empty()) {
    // ip list should not be empty, if want to disable ip for area,
    // you should disable it through area code
    commons::log(commons::LOG_WARN, "%s: ip list empty for area:%d with type:%d", MODULE_NAME,
                 area_code, static_cast<int>(type));
    return;
  }

  area_ips_[type][area_code].swap(ip_list);
  std::shuffle(area_ips_[type][area_code].begin(), area_ips_[type][area_code].end(),
               getRndGenerator());
}

std::list<commons::ip_t> PredefineIpList::GetDefaultIps(ApIpType type) const {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  auto it = default_ips_.find(type);
  if (it == default_ips_.end()) {
    return std::list<commons::ip_t>();
  }
  return it->second;
}

std::string PredefineIpList::GetDefaultDomain(DomainType type) const {
  ASSERT_THREAD_IS(utils::major_worker()->getThreadId());
  auto it = default_domain_.find(type);
  if (it == default_domain_.end()) {
    return std::string();
  }
  return it->second;
}

std::string PredefineIpList::GetAreaCodeName() const { return area_code_name_; }

std::size_t PredefineIpList::GetAreaCount() const { return area_count_; }

}  // namespace rtc
}  // namespace agora
