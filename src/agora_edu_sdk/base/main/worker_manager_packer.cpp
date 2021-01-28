//
//  Agora Media SDK
//
//  Created by Albert Zhang in 2018-11.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//

#include <base/base_type.h>
#include <base/worker_manager_packer.h>
#include <base/worker_manager_protocol.h>

#define name_to_str(name_31415926) (#name_31415926)

namespace agora {
namespace rtc {
namespace jpacker {

std::string jpack(const protocol::PAllocateRequest& req) {
  cjson::cJSON* root = cjson::cJSON_CreateObject();

  cjson::cJSON* item = cjson::cJSON_CreateString(req.command.c_str());
  cJSON_AddItemToObject(root, name_to_str(command), item);

  item = cjson::cJSON_CreateString(req.sid.c_str());
  cJSON_AddItemToObject(root, name_to_str(sid), item);

  item = cjson::cJSON_CreateString(req.appId.c_str());
  cJSON_AddItemToObject(root, name_to_str(appId), item);

  item = cjson::cJSON_CreateString(req.uid.c_str());
  cJSON_AddItemToObject(root, name_to_str(uid), item);

  item = cjson::cJSON_CreateString(req.token.c_str());
  cJSON_AddItemToObject(root, name_to_str(token), item);

  item = cjson::cJSON_CreateNumber(req.ts);
  cJSON_AddItemToObject(root, name_to_str(ts), item);

  item = cjson::cJSON_CreateNumber(req.seq);
  cJSON_AddItemToObject(root, name_to_str(seq), item);

  item = cjson::cJSON_CreateString(req.cname.c_str());
  cJSON_AddItemToObject(root, name_to_str(cname), item);

  item = cjson::cJSON_CreateNumber(req.requestId);
  cJSON_AddItemToObject(root, name_to_str(requestId), item);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
  cJSON_Delete(root);
  return str;
}

std::string jpack(const protocol::PProxyJoin& req) {
  cjson::cJSON* root = cjson::cJSON_CreateObject();
  cjson::cJSON* item = cjson::cJSON_CreateString(req.appId.c_str());
  cJSON_AddItemToObject(root, name_to_str(appId), item);

  item = cjson::cJSON_CreateString(req.cname.c_str());
  cJSON_AddItemToObject(root, name_to_str(cname), item);

  item = cjson::cJSON_CreateString(req.uid.c_str());
  cJSON_AddItemToObject(root, name_to_str(uid), item);

  item = cjson::cJSON_CreateString(req.sdkVersion.c_str());
  cJSON_AddItemToObject(root, name_to_str(sdkVersion), item);

  item = cjson::cJSON_CreateString(req.sid.c_str());
  cJSON_AddItemToObject(root, name_to_str(sid), item);

  item = cjson::cJSON_CreateNumber(req.seq);
  cJSON_AddItemToObject(root, name_to_str(seq), item);

  item = cjson::cJSON_CreateNumber(req.ts);
  cJSON_AddItemToObject(root, name_to_str(ts), item);

  item = cjson::cJSON_CreateNumber(req.requestId);
  cJSON_AddItemToObject(root, name_to_str(requestId), item);

  item = cjson::cJSON_CreateBool(req.allocate);
  cJSON_AddItemToObject(root, name_to_str(allocate), item);

  cjson::cJSON* request = cjson::cJSON_CreateObject();
  item = cjson::cJSON_CreateString(req.clientRequest.command.c_str());
  cJSON_AddItemToObject(request, name_to_str(command), item);

  cJSON_AddItemToObject(root, name_to_str(clientRequest), request);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
  cJSON_Delete(root);
  return str;
}

std::string jpack(const protocol::PSetSourceChannel& req) {
  cjson::cJSON* root = cjson::cJSON_CreateObject();
  cjson::cJSON* item = cjson::cJSON_CreateString(req.appId.c_str());
  cJSON_AddItemToObject(root, name_to_str(appId), item);

  item = cjson::cJSON_CreateString(req.cname.c_str());
  cJSON_AddItemToObject(root, name_to_str(cname), item);

  item = cjson::cJSON_CreateString(req.uid.c_str());
  cJSON_AddItemToObject(root, name_to_str(uid), item);

  item = cjson::cJSON_CreateString(req.sdkVersion.c_str());
  cJSON_AddItemToObject(root, name_to_str(sdkVersion), item);

  item = cjson::cJSON_CreateString(req.sid.c_str());
  cJSON_AddItemToObject(root, name_to_str(sid), item);

  item = cjson::cJSON_CreateNumber(req.seq);
  cJSON_AddItemToObject(root, name_to_str(seq), item);

  item = cjson::cJSON_CreateNumber(req.ts);
  cJSON_AddItemToObject(root, name_to_str(ts), item);

  item = cjson::cJSON_CreateNumber(req.requestId);
  cJSON_AddItemToObject(root, name_to_str(requestId), item);

  item = cjson::cJSON_CreateBool(req.allocate);
  cJSON_AddItemToObject(root, name_to_str(allocate), item);

  cjson::cJSON* request = cjson::cJSON_CreateObject();
  item = cjson::cJSON_CreateString(req.clientRequest.command.c_str());
  cJSON_AddItemToObject(request, name_to_str(command), item);
  item = cjson::cJSON_CreateString(req.clientRequest.token.c_str());
  cJSON_AddItemToObject(request, name_to_str(token), item);
  item = cjson::cJSON_CreateString(req.clientRequest.channelName.c_str());
  cJSON_AddItemToObject(request, name_to_str(channelName), item);
  item = cjson::cJSON_CreateString(req.clientRequest.uid.c_str());
  cJSON_AddItemToObject(request, name_to_str(uid), item);

  cJSON_AddItemToObject(root, name_to_str(clientRequest), request);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
  cJSON_Delete(root);
  return str;
}

std::string jpack(const protocol::PSetDestChannel& req) {
  cjson::cJSON* root = cjson::cJSON_CreateObject();
  cjson::cJSON* item = cjson::cJSON_CreateString(req.appId.c_str());
  cJSON_AddItemToObject(root, name_to_str(appId), item);

  item = cjson::cJSON_CreateString(req.cname.c_str());
  cJSON_AddItemToObject(root, name_to_str(cname), item);

  item = cjson::cJSON_CreateString(req.uid.c_str());
  cJSON_AddItemToObject(root, name_to_str(uid), item);

  item = cjson::cJSON_CreateString(req.sdkVersion.c_str());
  cJSON_AddItemToObject(root, name_to_str(sdkVersion), item);

  item = cjson::cJSON_CreateString(req.sid.c_str());
  cJSON_AddItemToObject(root, name_to_str(sid), item);

  item = cjson::cJSON_CreateNumber(req.seq);
  cJSON_AddItemToObject(root, name_to_str(seq), item);

  item = cjson::cJSON_CreateNumber(req.ts);
  cJSON_AddItemToObject(root, name_to_str(ts), item);

  item = cjson::cJSON_CreateNumber(req.requestId);
  cJSON_AddItemToObject(root, name_to_str(requestId), item);

  item = cjson::cJSON_CreateBool(req.allocate);
  cJSON_AddItemToObject(root, name_to_str(allocate), item);

  cjson::cJSON* request = cjson::cJSON_CreateObject();
  item = cjson::cJSON_CreateString(req.clientRequest.command.c_str());
  cJSON_AddItemToObject(request, name_to_str(command), item);
  item = cjson::cJSON_CreateString(req.clientRequest.channelName.c_str());
  cJSON_AddItemToObject(request, name_to_str(channelName), item);
  item = cjson::cJSON_CreateString(req.clientRequest.token.c_str());
  cJSON_AddItemToObject(request, name_to_str(token), item);
  item = cjson::cJSON_CreateString(req.clientRequest.uid.c_str());
  cJSON_AddItemToObject(request, name_to_str(uid), item);
  cJSON_AddItemToObject(root, name_to_str(clientRequest), request);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
  cJSON_Delete(root);
  return str;
}

std::string jpack(const protocol::PPacketTransferControl& req) {
  cjson::cJSON* root = cjson::cJSON_CreateObject();
  cjson::cJSON* item = cjson::cJSON_CreateString(req.appId.c_str());
  cJSON_AddItemToObject(root, name_to_str(appId), item);

  item = cjson::cJSON_CreateString(req.cname.c_str());
  cJSON_AddItemToObject(root, name_to_str(cname), item);

  item = cjson::cJSON_CreateString(req.uid.c_str());
  cJSON_AddItemToObject(root, name_to_str(uid), item);

  item = cjson::cJSON_CreateString(req.sdkVersion.c_str());
  cJSON_AddItemToObject(root, name_to_str(sdkVersion), item);

  item = cjson::cJSON_CreateString(req.sid.c_str());
  cJSON_AddItemToObject(root, name_to_str(sid), item);

  item = cjson::cJSON_CreateNumber(req.seq);
  cJSON_AddItemToObject(root, name_to_str(seq), item);

  item = cjson::cJSON_CreateNumber(req.ts);
  cJSON_AddItemToObject(root, name_to_str(ts), item);

  item = cjson::cJSON_CreateNumber(req.requestId);
  cJSON_AddItemToObject(root, name_to_str(requestId), item);

  item = cjson::cJSON_CreateBool(req.allocate);
  cJSON_AddItemToObject(root, name_to_str(allocate), item);

  cjson::cJSON* request = cjson::cJSON_CreateObject();
  item = cjson::cJSON_CreateString(req.clientRequest.command.c_str());
  cJSON_AddItemToObject(request, name_to_str(command), item);

  cJSON_AddItemToObject(root, name_to_str(clientRequest), request);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
  cJSON_Delete(root);
  return str;
}

std::string jpack(const protocol::PReconnect& req) {
  cjson::cJSON* root = cjson::cJSON_CreateObject();
  cjson::cJSON* item = cjson::cJSON_CreateString(req.appId.c_str());
  cJSON_AddItemToObject(root, name_to_str(appId), item);

  item = cjson::cJSON_CreateString(req.cname.c_str());
  cJSON_AddItemToObject(root, name_to_str(cname), item);

  item = cjson::cJSON_CreateString(req.uid.c_str());
  cJSON_AddItemToObject(root, name_to_str(uid), item);

  item = cjson::cJSON_CreateString(req.sdkVersion.c_str());
  cJSON_AddItemToObject(root, name_to_str(sdkVersion), item);

  item = cjson::cJSON_CreateString(req.sid.c_str());
  cJSON_AddItemToObject(root, name_to_str(sid), item);

  item = cjson::cJSON_CreateNumber(req.seq);
  cJSON_AddItemToObject(root, name_to_str(seq), item);

  item = cjson::cJSON_CreateNumber(req.ts);
  cJSON_AddItemToObject(root, name_to_str(ts), item);

  item = cjson::cJSON_CreateNumber(req.requestId);
  cJSON_AddItemToObject(root, name_to_str(requestId), item);

  item = cjson::cJSON_CreateBool(req.allocate);
  cJSON_AddItemToObject(root, name_to_str(allocate), item);

  cjson::cJSON* request = cjson::cJSON_CreateObject();
  item = cjson::cJSON_CreateString(req.clientRequest.command.c_str());
  cJSON_AddItemToObject(request, name_to_str(command), item);

  cJSON_AddItemToObject(root, name_to_str(clientRequest), request);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
  cJSON_Delete(root);
  return str;
}

std::string jpack(const protocol::PPacketHeartbeat& req) {
  cjson::cJSON* root = cjson::cJSON_CreateObject();
  cjson::cJSON* item = cjson::cJSON_CreateString(req.appId.c_str());
  cJSON_AddItemToObject(root, name_to_str(appId), item);

  item = cjson::cJSON_CreateString(req.cname.c_str());
  cJSON_AddItemToObject(root, name_to_str(cname), item);

  item = cjson::cJSON_CreateString(req.uid.c_str());
  cJSON_AddItemToObject(root, name_to_str(uid), item);

  item = cjson::cJSON_CreateString(req.command.c_str());
  cJSON_AddItemToObject(root, name_to_str(command), item);

  item = cjson::cJSON_CreateString(req.sid.c_str());
  cJSON_AddItemToObject(root, name_to_str(sid), item);

  item = cjson::cJSON_CreateNumber(req.ts);
  cJSON_AddItemToObject(root, name_to_str(ts), item);

  item = cjson::cJSON_CreateNumber(req.requestId);
  cJSON_AddItemToObject(root, name_to_str(requestId), item);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
  cJSON_Delete(root);
  return str;
}

std::string jpack(const protocol::PSetSourceUserId& req) {
  cjson::cJSON* root = cjson::cJSON_CreateObject();
  cjson::cJSON* item = cjson::cJSON_CreateString(req.appId.c_str());
  cJSON_AddItemToObject(root, name_to_str(appId), item);

  item = cjson::cJSON_CreateString(req.cname.c_str());
  cJSON_AddItemToObject(root, name_to_str(cname), item);

  item = cjson::cJSON_CreateString(req.uid.c_str());
  cJSON_AddItemToObject(root, name_to_str(uid), item);

  item = cjson::cJSON_CreateString(req.sdkVersion.c_str());
  cJSON_AddItemToObject(root, name_to_str(sdkVersion), item);

  item = cjson::cJSON_CreateString(req.sid.c_str());
  cJSON_AddItemToObject(root, name_to_str(sid), item);

  item = cjson::cJSON_CreateNumber(req.seq);
  cJSON_AddItemToObject(root, name_to_str(seq), item);

  item = cjson::cJSON_CreateNumber(req.ts);
  cJSON_AddItemToObject(root, name_to_str(ts), item);

  item = cjson::cJSON_CreateNumber(req.requestId);
  cJSON_AddItemToObject(root, name_to_str(requestId), item);

  item = cjson::cJSON_CreateBool(req.allocate);
  cJSON_AddItemToObject(root, name_to_str(allocate), item);

  cjson::cJSON* request = cjson::cJSON_CreateObject();
  item = cjson::cJSON_CreateString(req.clientRequest.command.c_str());
  cJSON_AddItemToObject(request, name_to_str(command), item);
  item = cjson::cJSON_CreateString(req.clientRequest.uid.c_str());
  cJSON_AddItemToObject(request, name_to_str(uid), item);
  cJSON_AddItemToObject(root, name_to_str(clientRequest), request);

  char* output = cjson::cJSON_Print(root);
  std::string str = output;
  free(output);
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\t'), str.end());
  cJSON_Delete(root);
  return str;
}

int junpack(protocol::PAllocateResponse& res, const std::string& json) {
  commons::cjson::JsonWrapper jwrapper;
  jwrapper.parse(json.c_str());
  res.sid = jwrapper.getStringValue("sid", "");
  res.ts = jwrapper.getUIntValue("ts", 0);
  res.seq = jwrapper.getUIntValue("seq", 0);
  res.cname = jwrapper.getStringValue("cname", "");
  res.requestId = jwrapper.getUIntValue("requestId", 0);
  res.code = jwrapper.getUIntValue("code", 0);
  res.reason = jwrapper.getStringValue("reason", "");
  res.vid = jwrapper.getUIntValue("vid", 0);
  res.serverTs = jwrapper.getUIntValue("serverTs", 0);
  res.workerToken = jwrapper.getStringValue("workerToken", "");
  auto serversJson = jwrapper.getArray("servers");
  if (serversJson.isValid()) {
    auto serverJson = serversJson.getChild();
    while (serverJson.isValid()) {
      protocol::PServerInfo serverInfo;
      serverInfo.address = serverJson.getStringValue("address", "");
      serverInfo.tcp = serverJson.getUIntValue("tcp", 0);
      serverInfo.tcps = serverJson.getUIntValue("tcps", 0);
      res.servers.emplace_back(std::move(serverInfo));
      serverJson = serverJson.getNext();
    }
  }
  return 0;
}

int junpack(protocol::PCrossChannelResponse& res, const std::string& json) {
  commons::cjson::JsonWrapper jwrapper;
  jwrapper.parse(json.c_str());
  res.command = jwrapper.getStringValue("command", "");
  res.appId = jwrapper.getUIntValue("appId", 0);
  res.seq = jwrapper.getUIntValue("seq", 0);
  res.cname = jwrapper.getStringValue("cname", "");
  res.code = jwrapper.getUIntValue("code", 0);
  res.reason = jwrapper.getStringValue("reason", "");
  res.requestId = jwrapper.getUIntValue("requestId", 0);
  auto serverResponse = jwrapper.getObject("serverResponse");
  if (serverResponse.isValid()) {
    res.serverResponse.command = serverResponse.getStringValue("command", "");
    res.serverResponse.result = serverResponse.getUIntValue("result", 0);
  }
  return 0;
}

int junpack(protocol::PCrossChannelStatus& res, const std::string& json) {
  commons::cjson::JsonWrapper jwrapper;
  jwrapper.parse(json.c_str());
  res.command = jwrapper.getStringValue("command", "");
  res.appId = jwrapper.getUIntValue("appId", 0);
  res.seq = jwrapper.getUIntValue("seq", 0);
  res.cname = jwrapper.getStringValue("cname", "");
  res.code = jwrapper.getUIntValue("code", 0);
  res.reason = jwrapper.getStringValue("reason", "");
  res.requestId = jwrapper.getUIntValue("requestId", 0);
  auto serverStatus = jwrapper.getObject("serverStatus");
  if (serverStatus.isValid()) {
    res.serverStatus.command = serverStatus.getStringValue("command", "");
    res.serverStatus.state = serverStatus.getUIntValue("state", 0);
  }
  return 0;
}

int junpack(protocol::PProxyResponse& res, const std::string& json) {
  commons::cjson::JsonWrapper jwrapper;
  jwrapper.parse(json.c_str());
  res.code = jwrapper.getUIntValue("code", 0);
  res.reason = jwrapper.getStringValue("reason", "");
  auto serverResponse = jwrapper.getObject("serverResponse");
  if (serverResponse.isValid()) {
    res.workerPort = serverResponse.getUIntValue("port", 0);
  }
  return 0;
}

int junpack(protocol::PHeartbeatResponse& res, const std::string& json) {
  commons::cjson::JsonWrapper jwrapper;
  jwrapper.parse(json.c_str());
  res.command = jwrapper.getStringValue("command", "");
  res.sid = jwrapper.getStringValue("sid", "");
  res.appId = jwrapper.getUIntValue("appId", 0);
  res.cname = jwrapper.getStringValue("cname", "");
  res.requestId = jwrapper.getUIntValue("requestId", 0);
  res.uid = jwrapper.getStringValue("uid", "");
  return 0;
}

}  // namespace jpacker
}  // namespace rtc
}  // namespace agora
