#pragma once
#ifndef RTE_RTEAPI_20210315_145756_H
#define RTE_RTEAPI_20210315_145756_H

#include "IAgoraRtmService.h"

#include "./RtmEventHandler.h"

class Rtm : public QObject
{
    Q_OBJECT
public:
    explicit Rtm();
    ~Rtm();
    bool init(const char *appid);
    bool enter(const char *token, const char *userid);
    void leave();
    void uninit();
    bool send(unsigned char* buff, int len, const char* peerId);
    bool joinRtmChannel(const char* channelId);
    bool leaveRtmChannel();
    bool sendRtmChannelMessage(const uint8_t* msg, int len);
    bool getMembers();

    static Rtm& GetRtmEngine();

    bool Dispatch(SDL_Event* event);
    void DispatchKBMessage(void* param);
    int width = 1920;
    int height = 1080;
signals:
    // rtm event signal
    void onRtmLeave(int errorCode);
    void onRtmEnter(bool);
    void funcReceivedMessageFromPeer(QString peerId, QString data, int size);
    //channel event signal
    void funJoinSucc();
    void funJoinFailed(QString error);
    void funcLeaveChannel(int error);
    void funMemberJoin(QString);
    void funMemberLeft(QString);
    void onGetMembersSuccess();
    void onGetMembersFailed();
    //
    bool parseReceiveMessage(QString peerId, QString data, int size);
    void onConnectionStateChanged(int state, int reason);
protected:
   
private:
   // agora::rtm::IRtmService *rtm;
    bool blogin = false;
    agora::rtm::IRtmService* rtmService;
    agora::rtm::IChannel* rtmChannel = NULL;
    RtmEventHandler rtmEventHandler;
};
 
#endif