//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#include <signaling/signaling_notification.h>
#include <signaling_engine/signaling_context.h>
#include <IAgoraSignalingEngine.h>

using namespace std::placeholders;
using namespace agora::commons;
namespace agora {
	namespace signaling {
SignalingEngineNotification::SignalingEngineNotification(SignalingContext& ctx, ISignalingEngineEventHandler* eh)
	:m_context(ctx)
	, m_apiCallSuppressed(false)
	, m_notificationSuppressed(false)
{
    if (eh)
    {
        m_eh = agora::commons::make_unique<event_handle_type>();
        m_eh->setEventHandler(eh, false);
    }
}

SignalingEngineNotification::~SignalingEngineNotification()
{
}

void SignalingEngineNotification::stopAsyncHandler(bool waitForExit)
{
	if (m_eh)
		m_eh->stop(waitForExit);
}

inline bool SignalingEngineNotification::isNotificationSuppressed() const
{
	return m_notificationSuppressed || !m_eh;
}

bool SignalingEngineNotification::getQueuePerfCounters(commons::perf_counter_data& counters) const
{
	return m_eh ? m_eh->getQueuePerfCounters(counters) : false;
}

void SignalingEngineNotification::onApiCallExecuted(int err, const char* api, const char* result)
{
    if (!api || isNotificationSuppressed() || m_apiCallSuppressed) return;
    if (err < 0)
        err = -err;
    log_if(LOG_DEBUG, "api call executed: err %d api '%s' result '%s'", err, api, result ? result : "");
    m_eh->onApiCallExecuted(err, api, result);
}

void SignalingEngineNotification::onChannelUserListUpdated(AccountList&& userAccounts)
{
    if (userAccounts.empty() || isNotificationSuppressed()) return;
    std::vector<const char*> accounts;
    for (const auto& acc : userAccounts)
    {
        accounts.push_back(acc.name.c_str());
    }
    m_eh->onChannelUserListUpdated(&accounts[0], accounts.size());
}

void SignalingEngineNotification::onChannelAttributesUpdated(const char** userAccounts, size_t count)
{
    if (isNotificationSuppressed()) return;
    m_eh->onChannelAttributesUpdated(userAccounts, count);
}

void SignalingEngineNotification::onUserJoinChannel(const char* userAccount)
{
    if (isNotificationSuppressed()) return;
    m_eh->onUserJoinChannel(userAccount);
}

void SignalingEngineNotification::onUserLeaveChannel(const char* userAccount)
{
    if (isNotificationSuppressed()) return;
    m_eh->onUserLeaveChannel(userAccount);
}

void SignalingEngineNotification::onWarning(int warn, const char* msg)
{
    if (!warn || isNotificationSuppressed()) return;
    m_eh->onWarning(warn, msg);
}

void SignalingEngineNotification::onError(int err, const char* msg)
{
    if (!err || isNotificationSuppressed()) return;
    m_eh->onWarning(err, msg);
}

void SignalingEngineNotification::onConnectionLost()
{
    if (isNotificationSuppressed()) return;
    m_eh->onConnectionLost();
}

void SignalingEngineNotification::onConnectionInterrupted()
{
    if (isNotificationSuppressed()) return;
    m_eh->onConnectionInterrupted();
}

void SignalingEngineNotification::onConnectionRestored()
{
    if (isNotificationSuppressed()) return;
    m_eh->onConnectionRestored();
}

}}
