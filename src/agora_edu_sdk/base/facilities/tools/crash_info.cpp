//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "facilities/tools/crash_info.h"

#include <algorithm>
#include <fstream>
#include <sstream>

#include "base/AgoraBase.h"
#include "utils/tools/json_wrapper.h"

namespace agora {
namespace utils {
const char* kCrashInfoPath = "global/crashinfo";
const char* kCrashContextKey = "crash_ctx";
const char* kCrashGeneralContextKey = "crash_general_ctx";

CrashGeneralContext::CrashGeneralContext(const std::string& s) {
  any_document_t json_root(s);

  if (!json_root.isValid()) {
    return;
  }

  serviceId = json_root.getStringValue("serviceId", "");
  sdkVersion = json_root.getStringValue("sdkVersion", "");
  deviceId = json_root.getStringValue("deviceId", "");
  appId = json_root.getStringValue("appId", "");
  channelMode = json_root.getIntValue("channelMode", 0);
  clientType = json_root.getIntValue("clientType", 0);
  buildNo = json_root.getIntValue("buildNo", 0);

  channelInfo.sessionId = json_root.getStringValue("sessionId", "");
  channelInfo.channelName = json_root.getStringValue("channelName", "");
  channelInfo.networkType = json_root.getIntValue("networkType", 0);
  channelInfo.clientRole = json_root.getIntValue("clientRole", 0);
}

CrashGeneralContext::operator std::string() const {
  std::stringstream ss;
  ss << "{";
  ss << "\"serviceId\":\"" << serviceId << "\",";
  ss << "\"sessionId\":\"" << channelInfo.sessionId << "\",";
  ss << "\"channelName\":\"" << channelInfo.channelName << "\",";
  ss << "\"sdkVersion\":\"" << sdkVersion << "\",";
  ss << "\"deviceId\":\"" << deviceId << "\",";
  ss << "\"appId\":\"" << appId << "\",";
  ss << "\"networkType\":" << channelInfo.networkType << ",";
  ss << "\"channelMode\":" << channelMode << ",";
  ss << "\"clientType\":" << clientType << ",";
  ss << "\"clientRole\":" << channelInfo.clientRole << ",";
  ss << "\"buildNo\":" << buildNo;
  ss << "}";
  return ss.str();
}

std::string replaceBackSlash(const std::string& s) {
  std::string tmpStr = s;
  std::replace(tmpStr.begin(), tmpStr.end(), '\\', '/');
  return tmpStr;
}

CrashContext::operator std::string() const {
  bool dumpFileExist = false;
  {
    std::ifstream f(dumpFile.c_str());
    if (f.is_open()) {
      dumpFileExist = true;
    }
  }

  std::stringstream ss;
  ss << "{";
  ss << "\"crashVer\":" << crashVer << ",";
  ss << "\"crashTs\":\"" << std::to_string(crashTs) << "\",";
  ss << "\"crashAddr\":\"" << std::to_string(crashAddr) << "\",";
  ss << "\"loadAddrBegin\":\"" << std::to_string(loadAddrBegin) << "\",";
  ss << "\"loadAddrEnd\":\"" << std::to_string(loadAddrEnd) << "\",";
  ss << "\"crashId\":\"" << crashId << "\",";
  ss << "\"logFile\":\"" << replaceBackSlash(logFile) << "\",";
  ss << "\"dumpFile\":\"" << replaceBackSlash(dumpFile) << "\",";
  ss << "\"dmpType\":" << dmpType << ",";
  ss << "\"isDumpFile\":" << (dumpFileExist ? "\"true\"" : "\"false\"");
  ss << "}";
  return ss.str();
}

static uint64_t stringToUint64(const std::string& str) {
  uint64_t val = 0;
  std::istringstream iss(str);
  iss >> val;
  return val;
}

CrashContext::CrashContext(const std::string& s) {
  any_document_t json_root(s);

  if (!json_root.isValid()) {
    return;
  }
  crashVer = json_root.getIntValue("crashVer", 0);
  crashTs = stringToUint64(json_root.getStringValue("crashTs", ""));
  crashAddr = stringToUint64(json_root.getStringValue("crashAddr", ""));
  loadAddrBegin = stringToUint64(json_root.getStringValue("loadAddrBegin", ""));
  loadAddrEnd = stringToUint64(json_root.getStringValue("loadAddrEnd", ""));
  crashId = json_root.getStringValue("crashId", "");
  logFile = json_root.getStringValue("logFile", "");
  dumpFile = json_root.getStringValue("dumpFile", "");
  dmpType = json_root.getIntValue("dmpType", 0);
  std::string isDumpFileStr = json_root.getStringValue("isDumpFile", "");
  isDumpFile = (isDumpFileStr == "true");
}

}  // namespace utils
}  // namespace agora
