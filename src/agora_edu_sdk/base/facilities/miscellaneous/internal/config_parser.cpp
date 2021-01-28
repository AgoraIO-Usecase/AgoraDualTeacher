//
//  Agora Media SDK
//
//  Created by Zheng, Ender in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "config_parser.h"

#include <sstream>

#include "api2/internal/config_engine_i.h"
#include "utils/log/log.h"

namespace agora {
namespace rtc {

const char MODULE_NAME[] = "[CP]";

static const std::unordered_set<std::string> kApprovedKeys = {
    CONFIGURABLE_KEY_RTC_UPLOAD_LOG_REQUEST,
    CONFIGURABLE_KEY_RTC_VIDEO_PLAYOUT_DELAY_MAX,
    CONFIGURABLE_KEY_RTC_VIDEO_PLAYOUT_DELAY_MIN,
    CONFIGURABLE_KEY_RTC_WIN_ALLOW_MAGNIFICATION,
    CONFIGURABLE_KEY_RTC_WIN_ALLOW_DIRECTX,
    CONFIGURABLE_KEY_SDK_DEBUG_ENABLE,
    CONFIGURABLE_KEY_RTC_AEC3_ENABLE,
    CONFIGURABLE_KEY_RTC_AUDIO_ADM_LAYER,
    CONFIGURABLE_KEY_RTC_REPORT_CONFIG,
    CONFIGURABLE_KEY_RTC_IP_AREACODE,
    CONFIGURABLE_KEY_RTC_IP_AREACODE_CN,
    CONFIGURABLE_KEY_RTC_IP_AREACODE_NA,
    CONFIGURABLE_KEY_RTC_IP_AREACODE_EUR,
    CONFIGURABLE_KEY_RTC_IP_AREACODE_AS,
    CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_CN,
    CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_NA,
    CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_EUR,
    CONFIGURABLE_KEY_RTC_IP_TLS_AREACODE_AS,
    CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_CN,
    CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_NA,
    CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_EUR,
    CONFIGURABLE_KEY_RTC_PROXY_IP_AREACODE_AS,
    CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_CN,
    CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_NA,
    CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_EUR,
    CONFIGURABLE_KEY_RTC_PROXY_IP_TLS_AREACODE_AS,
    CONFIGURABLE_KEY_RTC_ENABLE_DNS,
    CONFIGURABLE_KEY_RTC_FIRST_FRAME_DECODED_TIMEOUT,
    CONFIGURABLE_KEY_RTC_JOIN_TO_FIRST_DECODED_TIMEOUT,
    CONFIGURABLE_KEY_RTC_VIDEO_ENABLED_HW_ENCODER,
    CONFIGURABLE_KEY_RTC_P2P_SWITCH,
    CONFIGURABLE_KEY_RTC_VIDEO_ENABLE_HW_DECODER,
    CONFIGURABLE_KEY_RTC_ENABLE_DUMP,
    CONFIGURABLE_KEY_RTC_ENABLE_DUMP_FILE,
    CONFIGURABLE_KEY_RTC_ENABLE_DUMP_UPLOAD,
    CONFIGURABLE_KEY_VIDEO_DEGRADATION_PREFERENCE,
#if defined(FEATURE_ENABLE_UT_SUPPORT)
    "test-key1",
    "test-key2",
    "test-key3",
#endif
};

std::unordered_map<std::string, std::string> ConfigParser::ParseCds(
    const std::string& cds_json_str) {
  std::unordered_map<std::string, std::string> ret;
  if (cds_json_str.empty()) {
    commons::log(commons::LOG_WARN, "%s: empty CDS JSON string in ParseCds()", MODULE_NAME);
    return ret;
  }

  any_document_t cds_policy_doc(cds_json_str);
  if (!cds_policy_doc.isValid()) {
    commons::log(commons::LOG_ERROR, "%s: failed to parse CDS JSON string in ParseCds()",
                 MODULE_NAME);
    return ret;
  }

  // always check these two keys
  if (!cds_policy_doc.isObject("configs") || !cds_policy_doc.isString("version")) {
    commons::log(commons::LOG_INFO,
                 "%s: failed to find 'configs' as object or 'version' as string in ParseCds()",
                 MODULE_NAME);
    return ret;
  }

  // and also make sure the version key shouldn't be empty
  if (std::string(cds_policy_doc.getStringValue("version", "")).empty()) {
    commons::log(commons::LOG_INFO, "%s: value of 'version' is empty in ParseCds()", MODULE_NAME);
    return ret;
  }

  auto configs = cds_policy_doc.getObject("configs");
  for (auto item = cds_policy_doc.getChild(); item.isValid(); item = item.getNext()) {
    auto key = item.getName();
    if (kApprovedKeys.find(key) == kApprovedKeys.end()) continue;
    ret[key] = item.toString();
  }

  return ret;
}

std::unordered_map<std::string, TdsCacheItem> ConfigParser::ParseTds(
    const std::string& tds_json_str) {
  /*
     {
         "feature1": {
             "A" : {
                 "config1": "value1",
                 "config2": "value2"
             },
             "B": {
                 "config1": "value1",
                 "config2": "value2"
             }
          }
     }
   */
  std::unordered_map<std::string, TdsCacheItem> ret;

  if (tds_json_str.empty()) {
    commons::log(commons::LOG_WARN, "%s: empty TDS JSON string in ParseTds()", MODULE_NAME);
    return ret;
  }

  any_document_t tds_policy_doc(tds_json_str);
  if (!tds_policy_doc.isValid()) {
    commons::log(commons::LOG_ERROR, "%s: failed to parse TDS JSON string in ParseTds()",
                 MODULE_NAME);
    return ret;
  }

  for (auto feature = tds_policy_doc.getChild(); feature.isValid(); feature = feature.getNext()) {
    std::string feature_tag = feature.getName();
    TdsCacheItem v(feature.toString());
    v.in_call = (feature_tag.find("_in_call") != std::string::npos);
    v.store = (feature_tag.find("_store") != std::string::npos);
    ret[feature_tag] = v;
  }
  return ret;
}

TdsCacheItem::TdsCacheItem(const std::string& s) {
  any_document_t feature(s);
  TdsCacheItem::Plan* p = nullptr;

  for (auto ab = feature.getChild(); ab.isValid(); ab = ab.getNext()) {
    std::string plan = ab.getName();
    if (plan == "A" || plan == "a") {
      p = &PlanA;
    } else if (plan == "B" || plan == "b") {
      p = &PlanB;
    }

    if (!p) continue;

    for (auto conf = ab.getChild(); conf.isValid(); conf = conf.getNext()) {
      if (kApprovedKeys.find(conf.getName()) == kApprovedKeys.end()) continue;
      p->configs[conf.getName()] = conf.toString();
    }
  }
}

bool TdsCacheItem::operator==(const TdsCacheItem& rhs) const {
  if (in_call != rhs.in_call) return false;
  if (store != rhs.store) return false;
  return (PlanA.configs == rhs.PlanA.configs && PlanB.configs == rhs.PlanB.configs);
}

TdsCacheItem::operator std::string() const {
  std::stringstream ss;
  ss << "{";

  ss << "\"A\":{";
  size_t i = 0;
  for (auto itor = PlanA.configs.begin(); itor != PlanA.configs.end(); i++, itor++) {
    ss << "\"" << itor->first << "\":\"" << itor->second << "\"";
    if (i != (PlanA.configs.size() - 1)) ss << ",";
  }
  ss << "},";

  ss << "\"B\":{";
  i = 0;
  for (auto itor = PlanB.configs.begin(); itor != PlanB.configs.end(); i++, itor++) {
    ss << "\"" << itor->first << "\":\"" << itor->second << "\"";
    if (i != (PlanB.configs.size() - 1)) ss << ",";
  }
  ss << "}";

  ss << "}";

  return ss.str();
}

}  // namespace rtc
}  // namespace agora
