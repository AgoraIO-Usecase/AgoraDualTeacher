#pragma once

#include <QObject>
#include <functional>
#include "IAgoraRtmService.h"

using namespace agora::rtm;
class Rtm;
class RtmChannelHandler : public QObject , public agora::rtm::IChannelEventHandler //,
{
    Q_OBJECT
public:
    RtmChannelHandler();
    
protected:
    virtual void onJoinSuccess();
    virtual void onJoinFailure(JOIN_CHANNEL_ERR errorCode);
    virtual void onLeave(LEAVE_CHANNEL_ERR errorCode);
    virtual void onMessageReceived(const char* userId, const IMessage* message);
    virtual void onImageMessageReceived(const char* userId, const IImageMessage* message);
    virtual void onFileMessageReceived(const char* userId, const IFileMessage* message);
    virtual void onSendMessageResult(long long messageId, CHANNEL_MESSAGE_ERR_CODE state);
    virtual void onMemberJoined(IChannelMember* member);
    virtual void onMemberLeft(IChannelMember* member);
    virtual void onGetMembers(IChannelMember** members, int userCount, GET_MEMBERS_ERR errorCode);
    virtual void onAttributesUpdated(const IRtmChannelAttribute* attributes[], int numberOfAttributes);
    virtual void onMemberCountUpdated(int memberCount);

private:
    Rtm* rtm;
};
