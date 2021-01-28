//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <IAgoraSignalingEngine.h>
#include <sigslot.h>
#include <memory>
#include <string>
#include <vector>
#include "main/async_event_handler.h"
#include "main/stream_signaling_event_handler.h"

namespace agora {
namespace commons {
struct perf_counter_data;
}
namespace signaling {
class SignalingContext;

struct AccountInfo {
  std::string name;
  agora::rtc::uid_t uid;
};
using AccountList = std::vector<AccountInfo>;
class SignalingEngineNotification : public base::INotification, public agora::has_slots<> {
  using event_handle_type = commons::AsyncEventHandler<StreamSignalingEventHandler>;

 public:
  SignalingEngineNotification(SignalingContext& ctx, ISignalingEngineEventHandler* eh);
  ~SignalingEngineNotification();
  void stopAsyncHandler(bool waitForExit);
  void suppressNotification(bool suppressed) { m_notificationSuppressed = suppressed; }
  bool isNotificationSuppressed() const;
  bool getQueuePerfCounters(commons::perf_counter_data& counters) const;
  ISignalingEngineEventHandler* getEventHandler();

 public:
  void onUserJoinChannel(const char* userAccount);
  void onUserLeaveChannel(const char* userAccount);
  void onChannelUserListUpdated(AccountList&& userAccounts);
  void onChannelAttributesUpdated(const char** userAccounts, size_t count);
  void onWarning(int warn, const char* msg);
  void onError(int err, const char* msg);
  void onConnectionLost();
  void onConnectionInterrupted();
  void onConnectionRestored();

 public:
  void suppressApiCallNotification(bool suppressed) override { m_apiCallSuppressed = suppressed; }
  bool isApiCallNotificationSuppressed() const override { return m_apiCallSuppressed; }
  void onApiCallExecuted(int err, const char* api, const char* result) override;

 private:
  SignalingContext& m_context;
  std::unique_ptr<event_handle_type> m_eh;
  bool m_apiCallSuppressed;
  bool m_notificationSuppressed;
};

}  // namespace signaling
}  // namespace agora
