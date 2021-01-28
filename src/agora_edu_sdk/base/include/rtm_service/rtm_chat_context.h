//
//  Agora Rtm SDK
//
//  Created by Junhao in 2018.
//  Copyright (c) 2018 Agora IO. All rights reserved.
//
#pragma once
#include <base/base_type.h>
#include <base/parameter_helper.h>
#include <sigslot.h>

#include <memory>
#include <string>

#include "rtm_service/rtm_context.h"
#include "rtm_service/rtm_packet_filter.h"
#include "rtm_service/rtm_service_protocol.h"
#include "rtm_service/rtm_signal_type.h"

namespace agora {
namespace rtm {

const uint32_t MESSAGE_SIZE_LIMIT = 32 * 1024;

class RtmChatManager;
class RtmCacheManager;
class RtmPeerManager;
class RtmChannelManager;
class RtmMessageManager;
class CompressionManager;
class ServerCommandManager;
class RtmParameterCollection;
class RtmChatReporter;

class RtmUtil {
 public:
  static bool isValidAttributeKey(const std::string& key);
  static bool isValidAttributeValue(const std::string& val);
  static int64_t allocUniqueId();
};

struct EnumClassHash {
  template <typename T>
  int operator()(T t) const {
    return static_cast<int>(t);
  }
};

class IRtmIdAllocator {
 public:
  virtual ~IRtmIdAllocator() {}
  virtual int32_t initialize() = 0;
  virtual int64_t allocateId() = 0;
};

// improvement: cache the base id, and don't allocate same one
class RtmIdAllocator : public IRtmIdAllocator {
 public:
  ~RtmIdAllocator() override {}
  int32_t initialize() override;
  int64_t allocateId() override;

 private:
  int64_t m_id;
};

class IRtmMessageManager {
 public:
  virtual int32_t sendMessage(message_packet_t& packet) = 0;
  virtual int32_t onReceiveMessage(message_packet_t& packet) = 0;
};

class IRtmCacheManager {};

class RtmTimeoutWatcher {
 public:
  typedef std::function<void()> repeat_handler;
  typedef std::function<void()> timeout_handler;

 private:
  const int interval = 500;
  class timeoutHandlerWrapper {
   public:
    timeoutHandlerWrapper(int ms, uint8_t maxRepeatCount, repeat_handler rpHandler,
                          timeout_handler toHandler)
        : m_ms(ms),
          interval(ms),
          m_repeatLeftCount(maxRepeatCount),
          m_rpHandler(rpHandler),
          m_toHandler(toHandler),
          m_bIsValid(true) {
      auto currentClock = std::chrono::system_clock::now();
      if (maxRepeatCount == 0) maxRepeatCount = 1;
      std::chrono::duration<int, std::milli> past_time(ms * maxRepeatCount);
      m_timeoutTs = currentClock + past_time;
    }

    bool isValid();
    void excute(int timeBetweenTwoExcute);
    void setInvalid() { m_bIsValid = false; }
    void reset() { m_ms = interval; }

   private:
    int interval;
    int m_ms;
    int16_t m_repeatLeftCount;
    repeat_handler m_rpHandler;
    timeout_handler m_toHandler;
    bool m_bIsValid;
    std::chrono::system_clock::time_point m_timeoutTs;
  };

 public:
  explicit RtmTimeoutWatcher(utils::worker_type& worker);
  ~RtmTimeoutWatcher();

  void watch(int64_t reqId, int ms, uint8_t maxRepeatCount, repeat_handler rpHandler,
             timeout_handler toHandler);
  void unWatch(int64_t reqId);
  bool isWatching(int64_t reqId);
  void reset(int64_t reqId);

 private:
  void onTimer();

 private:
  std::unique_ptr<agora::commons::timer_base> m_timer;
  std::unordered_map<int64_t, timeoutHandlerWrapper> m_map;
};

class RtmChatContext : public base::ParameterHasSlots,
                       public base::ParameterBase,
                       public agora::has_slots<> {
  enum CHAT_MODE { MODE_IDLE, MODE_IN_CHAT };

  struct SignalCollection {
    agora::base::NetworkMonitor::sig_network_changed rtm_network_changed;
    agora::base::NetworkMonitor::sig_dns64_responsed rtm_dns64_responsed;
    signal::sig_login login;
    signal::sig_logout logout;
    signal::sig_request_link_list request_link_list;
    signal::sig_link_event link_event;
    signal::sig_ap_link_res rtm_ap_link_res;

    signal::sig_tx_message tx_message;
    signal::sig_rx_message rx_message;
    signal::sig_tx_message_res tx_message_res;

    signal::sig_login_success login_success;
    signal::sig_login_failure login_failure;

    signal::sig_join_channel join_channel;
    signal::sig_join_channel_res join_channel_res;
    signal::sig_leave_channel leave_channel;
  };

 public:
  explicit RtmChatContext(RtmContext& ctx);
  ~RtmChatContext();
  int32_t login(const protocol::CmdLogin& cmd);
  int32_t logout();
  int32_t sendMessage(const protocol::CmdSendMessage& msg);
  bool isInChat() { return m_chatMode == MODE_IN_CHAT; }

  void resetContext();

 public:
  int32_t joinChannel(const std::string& channelId);
  int32_t leaveChannel(const std::string& channelId);
  int32_t getMemberList(const std::string& channelId);

 public:
  bool loggedIn() { return m_loggedIn; }
  uint64_t elapsed() { return commons::tick_ms() - m_tick0; }
  std::string sid() { return m_sid; }
  std::string userId() { return m_userId; }
  std::string token() { return m_token; }
  LOGIN_STRATEGY_TYPE loginStrategy() { return m_loginStrategy; }
  uint64_t instanceId() { return m_instanceId; }
  RtmContext& getRtmContext() { return m_rtmContext; }
  agora::base::BaseContext& getBaseContext() { return m_rtmContext.getBaseContext(); }
  RtmChatManager* chatManager() { return m_chatManager.get(); }
  RtmCacheManager* cacheManager() { return m_cacheManager.get(); }
  RtmChatReporter* chatReporter() { return m_chatReporter.get(); }
  RtmPeerManager* peerManager() { return m_peerManager.get(); }
  RtmChannelManager* channelManager() { return m_channelManager.get(); }
  RtmMessageManager* messageManager() { return m_messageManager.get(); }
  CompressionManager* compressionManager() { return m_compressionManager.get(); }
  ServerCommandManager* serverCommandManager() { return m_serverCommandManager.get(); }
  RtmTimeoutWatcher* timeoutWatcher() { return m_timeoutWatcher.get(); }

  utils::worker_type& worker() { return m_rtmContext.worker(); }
  transport::INetworkTransportHelper* GetTransportHelper() {
    return m_rtmContext.GetTransportHelper();
  }
  agora::commons::timer_base* createTimer(std::function<void()>&& f, uint64_t ms) {
    return getRtmContext().worker()->createTimer(std::move(f), ms);
  }

  int32_t onSetParameter(const std::string& key, const any_value_t& value) override { return 0; }
  int32_t onGetParameter(const std::string& key, const char* args, any_document_t& out) override {
    return 0;
  }

 public:
  void setLoggedIn(bool loggedIn) { m_loggedIn = loggedIn; }
  void setSid(const std::string& sid) { m_sid = sid; }
  void setToken(const std::string& token) { m_token = token; }
  void setTick0(uint64_t tick0) { m_tick0 = tick0; }
  void setUserId(const std::string& userId) { m_userId = userId; }
  void setLoginStrategy(LOGIN_STRATEGY_TYPE strategy) { m_loginStrategy = strategy; }
  bool isPeerMessage(int32_t type);
  bool isChannelMessage(int32_t type);

 private:
  void prepareContext(const protocol::CmdLogin& cmd);
  void cleanupContext();
  void clear();

 public:
  SignalCollection signals;
  std::unique_ptr<RtmParameterCollection> parameters;

 private:
  RtmContext& m_rtmContext;

  std::shared_ptr<RtmChatManager> m_chatManager;
  std::unique_ptr<RtmPeerManager> m_peerManager;
  std::unique_ptr<RtmChannelManager> m_channelManager;
  std::unique_ptr<RtmMessageManager> m_messageManager;
  std::unique_ptr<ServerCommandManager> m_serverCommandManager;
  std::unique_ptr<RtmTimeoutWatcher> m_timeoutWatcher;

  std::unique_ptr<CompressionManager> m_compressionManager;

  std::unique_ptr<RtmCacheManager> m_cacheManager;
  std::shared_ptr<RtmChatReporter> m_chatReporter;
  std::unique_ptr<IRtmIdAllocator> m_idAllocator;

  bool m_loggedIn;
  LOGIN_STRATEGY_TYPE m_loginStrategy;
  uint64_t m_tick0;
  std::string m_userId;
  std::string m_sid;
  std::string m_token;
  CHAT_MODE m_chatMode;
  uint64_t m_instanceId;
};

}  // namespace rtm
}  // namespace agora
