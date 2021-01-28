//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include "rtc/rtc_context.h"

#include "call_engine/call_context.h"
#include "engine_adapter/media_engine_manager.h"
#include "utils/log/log.h"
#include "utils/tools/json_util.h"
#include "utils/tools/util.h"
#include "video_profile.h"

using namespace agora::commons;

namespace agora {
namespace rtc {

RtcContext::RtcContext(base::BaseContext& baseContext, const RtcEngineContextEx& context)
    : m_baseContext(baseContext),
      m_worker(utils::major_worker()),
      m_notification(*this, context),
#if defined(FEATURE_P2P)
      is_p2p_switch_enabled_(context.is_p2p_switch_enabled_),
#endif
      m_packetObserver(nullptr),
      m_connectionId(context.connectionId) {
  m_configEngine.setWorker(m_worker.get(), &m_notification, true);
  m_baseContext.configEngineManager().registerConfigEngine("rtc.", &m_configEngine);
}

RtcContext::~RtcContext() { m_baseContext.configEngineManager().unregisterConfigEngine("rtc."); }

#ifdef FEATURE_ENABLE_UT_SUPPORT
RtcContext::RtcContext(agora::base::BaseContext& baseContext, const RtcEngineContextEx& context,
                       bool)
    : m_baseContext(baseContext), m_notification(*this, context) {}

void RtcContext::setCallContext(CallContext* context) { m_callContext.reset(context); }
#endif

void RtcContext::startService(const RtcEngineContextEx& context) {
  m_callContext.reset(new CallContext(*this, m_baseContext.getConfigDir()));
  m_notification.onCallContextCreated();
  getReportService().setReportType(m_callContext->parameters->net.reportType.value());
}

void RtcContext::stopService() { m_callContext.reset(); }

SafeUserIdManager* RtcContext::safeUserIdManager() { return getCallContext()->safeUserIdManager(); }

UserIdManagerImpl* RtcContext::internalUserIdManager() {
  return getCallContext()->internalUserIdManager();
}

void RtcContext::applyProfile() {
  if (!m_profile) return;

  any_document_t doc;
  doc.setObjectType();
  any_document_t ae = m_profile->getObject("audioEngine");
#if 0  // detach m_profile
    any_value_t node;
    while ((node = cJSON_DetachItemFromArray(ae.getRoot(), 0)) != nullptr) {
        if (!strncmp(node->string, "che.", 4)) {
            doc.addItemToObject(node->string, node);
        } else {
            std::string name = std::string("che.audio.") + node->string;
            doc.addItemToObject(name.c_str(), node);
        }
    }
#else  // duplicate from m_profile
  for (auto it = ae.getChild(); it.isValid(); it = it.getNext()) {
    const char* key = it.getName();
    auto value = it.getRoot();
    if (key && *key != '\0') {
      std::string name = std::string("che.audio.") + key;
      doc.addItemToObject(name.c_str(), cJSON_Duplicate(value, 1));
    }
  }
#endif
  if (doc.isValid()) getConfigEngine().onSetParameters(&m_notification, doc, false, true, true);
}

void RtcContext::applyConfiguration(const std::string& config, uint64_t elapsed) {
  if (m_callContext) {
    m_callContext->applyConfiguration(config, elapsed);
  }
}

void RtcContext::applyABTestConfiguration(std::list<signal::ABTestData>& abTestData) {
  if (m_callContext) {
    m_callContext->applyABTestConfiguration(abTestData);
  }
}

void RtcContext::reportAPEvent(const signal::APEventData& ed) {
  if (m_callContext) {
    m_callContext->reportAPEvent(ed);
  }
}

void RtcContext::onLogging(int level, const char* format, ...) {
  char buf[256];
  va_list args;
  va_start(args, format);
  vsnprintf(buf, sizeof(buf) - 1, format, args);
  va_end(args);
  m_notification.onLogging(level, buf);
}

bool RtcContext::getVideoOptionsByProfile(int profile, bool swapWidthAndHeight,
                                          VideoNetOptions& options) {
#ifdef FEATURE_VIDEO
  return VideoProfile::getVideoOptionsByProfile(profile, swapWidthAndHeight, options);
#else
  return false;
#endif
}

int RtcContext::setVideoProfileEx(int profile, int width, int height, int frameRate, int bitrate) {
#ifdef FEATURE_VIDEO
  return VideoProfile::setVideoProfile(profile, width, height, frameRate, bitrate);
#else
  return 0;
#endif
}

base::ReportService& RtcContext::getReportService() { return m_baseContext.getReportService(); }

}  // namespace rtc
}  // namespace agora
