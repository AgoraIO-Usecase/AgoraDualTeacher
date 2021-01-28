//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "AgoraRefPtr.h"
#include <functional>
#include <string>

namespace agora {
namespace rte {

enum DataReceiveType { DATA_RECEIVER_RTM };
enum DataRequestType { DATA_REQUSET_HTTPS };

// TODO(jxm): copied from rtm::CONNECTION_STATE, why not directly use that one?
enum DataReceiverConnState {
  /**
   * 1: The SDK has logged in the RTM service.
   */
  DATA_RECEIVER_CONN_STATE_CONNECTED = 1,
  /**
   * 2: The initial state. The SDK is disconnected from the RTM service.
   */
  DATA_RECEIVER_CONN_STATE_DISCONNECTED = 2,
  /**
   * 3: The SDK gives up logging in the RTM service, mainly because another instance has logged in
   * the RTM service with the same user ID.
   *
   * Call the logout() method before calling login to log in the RTM service again.
   */
  DATA_RECEIVER_CONN_STATE_ABORTED = 3,
};

struct DataTransferMethod {
  DataReceiveType data_receiver_type;
  DataRequestType data_request_type;

  DataTransferMethod()
      : data_receiver_type(DATA_RECEIVER_RTM), data_request_type(DATA_REQUSET_HTTPS) {}
};

class IDataParam : public RefCountInterface {
 public:
  virtual void AddString(const std::string& key, const std::string& val) = 0;
  virtual bool GetString(const std::string& key, std::string& val) = 0;
  virtual void AddInt(const std::string& key, int val) = 0;
  virtual bool GetInt(const std::string& key, int& val) = 0;
  virtual void AddUInt64(const std::string& key, uint64_t val) = 0;
  virtual bool GetUInt64(const std::string& key, uint64_t& val) = 0;
  virtual void AddPtr(const std::string& key, const void* val) = 0;
  virtual bool GetPtr(const std::string& key, const void*& val) = 0;
};

agora_refptr<IDataParam> CreateDataParam();

using data_request_callback = std::function<void(bool success, const std::string& data)>;

class IDataRequest : public RefCountInterface {
 public:
  virtual int DoRequest(agora_refptr<IDataParam> param, data_request_callback&& cb) = 0;

 protected:
  virtual ~IDataRequest() = default;
};

class IRteDataReceiverEventHandler {
 public:
  virtual void OnLoginSuccess(const std::string& user_token) = 0;
  virtual void OnLoginFailure() = 0;

  virtual void OnConnectionStateChanged(DataReceiverConnState state) = 0;

  virtual void OnMessageReceivedFromPeer(const std::string& peer_id,
                                         const std::string& message) = 0;

};

class IRteDataReceiver : public RefCountInterface {
 public:
  virtual int SetParam(agora_refptr<IDataParam> param) = 0;

  virtual int Login() = 0;
  virtual int Logout() = 0;

  virtual void RegisterEventHandler(IRteDataReceiverEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(IRteDataReceiverEventHandler* event_handler) = 0;

 protected:
  virtual ~IRteDataReceiver() {}
};

class ISceneDataReceiverEventHandler {
 public:
  virtual void OnJoinSuccess() = 0;
  virtual void OnJoinFailure() = 0;

  virtual void OnConnectionStateChanged(DataReceiverConnState state) = 0;

  virtual void OnMessageReceived(const std::string& message) = 0;
};

class ISceneDataReceiver : public RefCountInterface {
 public:
  virtual int SetParam(agora_refptr<IDataParam> param) = 0;

  virtual int Join() = 0;
  virtual int Leave() = 0;

  virtual void RegisterEventHandler(ISceneDataReceiverEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(ISceneDataReceiverEventHandler* event_handler) = 0;

 protected:
  virtual ~ISceneDataReceiver() {}
};

}  // namespace rte
}  // namespace agora
