//
//  Agora Media SDK
//
//  Copyright (c) 2015 Agora IO. All rights reserved.
//

#ifndef _AGORA_SDK_H
#define _AGORA_SDK_H

#include <string>

#if defined(_WIN32)
#include <cstdint>
#endif

#if defined(__APPLE__)
#define MEDIA_VERSION
#endif

/*


----------- Account :

     login          ---->
                    <---- onLoginSuccesse
                    <---- onLoginFailed

                    <.... Temporal state :
                          reconnecting
                          connectFailed




     [network down] ---->
     logout         ---->
                    <---- onLogout


----------- Channel :

     channelJoin    ---->
                    <---- onChannelJoinSuccess
                    <---- onChannelJoinFailed
                    <....


     [kicked]       ---->
     channelLeave   ---->
                    <---- onChannelLeaved


----------- Channel :


: call out

     channelInvite  ---->
                    [ invite sending stage]
                    <---- onInviteReceivedByPeer
                    <---- onInviteFailed

                    [ invite sending stage]
                    <---- onInviteAcceptedByPeer
                    <---- onInviteRefusedByPeer

                    [ invite ending stage]
                    <---- onInviteEndByPeer

                    [ error ]

: call in

                    <---- onInviteReceived
channelInviteAccept ---->
channelInviteRefuse ---->


*/

typedef enum {
  SUCCESS = 0,

  LOGOUT_E_OTHER = 100,         //
  LOGOUT_E_USER = 101,          // logout by user
  LOGOUT_E_NET = 102,           // network failure
  LOGOUT_E_KICKED = 103,        // login in other device
  LOGOUT_E_PACKET = 104,        //
  LOGOUT_E_TOKENEXPIRED = 105,  // token expired
  LOGOUT_E_OLDVERSION = 106,    //
  LOGOUT_E_TOKENWRONG = 107,
  LOGOUT_E_ALREADY_LOGOUT = 108,

  LOGIN_E_OTHER = 200,
  LOGIN_E_NET = 201,
  LOGIN_E_FAILED = 202,
  LOGIN_E_CANCEL = 203,
  LOGIN_E_TOKENEXPIRED = 204,
  LOGIN_E_OLDVERSION = 205,
  LOGIN_E_TOKENWRONG = 206,
  LOGIN_E_TOKEN_KICKED = 207,
  LOGIN_E_ALREADY_LOGIN = 208,
  LOGIN_E_INVALID_USER = 209,  // login account is invalid

  JOINCHANNEL_E_OTHER = 300,

  SENDMESSAGE_E_OTHER = 400,
  SENDMESSAGE_E_TIMEOUT = 401,

  QUERYUSERNUM_E_OTHER = 500,
  QUERYUSERNUM_E_TIMEOUT = 501,
  QUERYUSERNUM_E_BYUSER = 502,

  LEAVECHANNEL_E_OTHER = 600,
  LEAVECHANNEL_E_KICKED = 601,
  LEAVECHANNEL_E_BYUSER = 602,
  LEAVECHANNEL_E_LOGOUT = 603,
  LEAVECHANNEL_E_DISCONN = 604,

  INVITE_E_OTHER = 700,
  INVITE_E_REINVITE = 701,
  INVITE_E_NET = 702,
  INVITE_E_PEEROFFLINE = 703,
  INVITE_E_TIMEOUT = 704,
  INVITE_E_CANTRECV = 705,

  GENERAL_E = 1000,
  GENERAL_E_FAILED = 1001,
  GENERAL_E_UNKNOWN = 1002,
  GENERAL_E_NOT_LOGIN = 1003,
  GENERAL_E_WRONG_PARAM = 1004,
  GENERAL_E_LARGE_PARAM = 1005,
} AGORA_E_CODE;

namespace agora_sdk {
class ICallBack {
 public:
  // connection
  virtual void onReconnecting(uint32_t nretry) {}
  virtual void onReconnected(int fd) {}

  // login
  virtual void onLoginSuccess(uint32_t uid, int fd) {}
  virtual void onLogout(int ecode) {}
  virtual void onLoginFailed(int ecode) {}

  // channel
  virtual void onChannelJoined(std::string channelID) {}
  virtual void onChannelJoinFailed(std::string channelID, int ecode) {}
  virtual void onChannelLeaved(std::string channelID, int ecode) {}

  virtual void onChannelUserJoined(std::string account, uint32_t uid) {}
  virtual void onChannelUserLeaved(std::string account, uint32_t uid) {}
  virtual void onChannelUserList(int n, char** accounts, uint32_t* uids) {}

  virtual void onChannelQueryUserNumResult(std::string channelID, int ecode,
                                           int num) {}
  virtual void onChannelQueryUserIsIn(std::string channelID,
                                      std::string account, int isIn) {}
  virtual void onChannelAttrUpdated(std::string channelID, std::string name,
                                    std::string value, std::string type) {}

  // invite
  virtual void onInviteReceived(std::string channelID, std::string account,
                                uint32_t uid, std::string extra) {}
  virtual void onInviteReceivedByPeer(std::string channelID,
                                      std::string account, uint32_t uid) {}
  virtual void onInviteAcceptedByPeer(std::string channelID,
                                      std::string account, uint32_t uid,
                                      std::string extra) {}
  virtual void onInviteRefusedByPeer(std::string channelID, std::string account,
                                     uint32_t uid, std::string extra) {}
  virtual void onInviteFailed(std::string channelID, std::string account,
                              uint32_t uid, int ecode, std::string extra) {}
  virtual void onInviteEndByPeer(std::string channelID, std::string account,
                                 uint32_t uid, std::string extra) {}
  virtual void onInviteEndByMyself(std::string channelID, std::string account,
                                   uint32_t uid) {}
  virtual void onInviteMsg(std::string channelID, std::string account,
                           uint32_t uid, std::string msgType,
                           std::string msgData, std::string extra) {}

  // message
  virtual void onMessageSendError(std::string messageID, int ecode) {}
  virtual void onMessageSendProgress(std::string account, std::string messageID,
                                     std::string type, std::string info) {}
  virtual void onMessageSendSuccess(std::string messageID) {}
  virtual void onMessageAppReceived(std::string msg) {}
  virtual void onMessageInstantReceive(std::string account, uint32_t uid,
                                       std::string msg) {}
  virtual void onMessageChannelReceive(std::string channelID,
                                       std::string account, uint32_t uid,
                                       std::string msg) {}

  // other
  virtual void onLog(std::string txt) {}
  virtual void onInvokeRet(std::string callID, std::string err,
                           std::string resp) {}
  virtual void onMsg(std::string from, std::string t, std::string msg) {}
  virtual void onUserAttrResult(std::string account, std::string name,
                                std::string value) {}
  virtual void onUserAttrAllResult(std::string account, std::string value) {}
  virtual void onError(std::string name, int ecode, std::string desc) {}
  virtual void onQueryUserStatusResult(std::string name, std::string status) {}

  // xx
  virtual void onDbg(std::string a, std::string b) {}
  virtual void onBCCall_result(std::string reason, std::string json_ret,
                               std::string callID) {}
};

class IAgoraAPI {
 public:
  // callback
  virtual void callbackSet(ICallBack* handler) = 0;
  virtual ICallBack* callbackGet() = 0;

  // login
  virtual void login(std::string vendorID, std::string account,
                     std::string token, uint32_t uid, std::string deviceID) = 0;
  virtual void login2(std::string vendorID, std::string account,
                      std::string token, uint32_t uid, std::string deviceID,
                      int retry_time_in_s, int retry_count) = 0;
  virtual void logout() = 0;

  // channel
  virtual void channelJoin(std::string channelID) = 0;
  virtual void channelLeave(std::string channelID) = 0;
  virtual void channelQueryUserNum(std::string channelID) = 0;
  virtual void channelQueryUserIsIn(std::string channelID,
                                    std::string account) = 0;
  virtual void channelSetAttr(std::string channelID, std::string name,
                              std::string value) = 0;
  virtual void channelDelAttr(std::string channelID, std::string name) = 0;
  virtual void channelClearAttr(std::string channelID) = 0;

  // invite
  virtual void channelInviteUser(std::string channelID, std::string account,
                                 uint32_t uid = 0) = 0;
  virtual void channelInviteUser2(std::string channelID, std::string account,
                                  std::string extra) = 0;
  virtual void channelInvitePhone(std::string channelID, std::string phoneNum,
                                  uint32_t uid = 0) = 0;
  virtual void channelInvitePhone2(std::string channelID, std::string phoneNum,
                                   std::string sourcesNum) = 0;
  virtual void channelInvitePhone3(std::string channelID, std::string phoneNum,
                                   std::string sourcesNum,
                                   std::string extra) = 0;
  virtual void channelInviteDTMF(std::string channelID, std::string phoneNum,
                                 std::string dtmf) = 0;

  virtual void channelInviteAccept(std::string channelID, std::string account,
                                   uint32_t uid, std::string extra) = 0;
  virtual void channelInviteRefuse(std::string channelID, std::string account,
                                   uint32_t uid, std::string extra) = 0;
  virtual void channelInviteEnd(std::string channelID, std::string account,
                                uint32_t uid) = 0;

  // message
  virtual void messageAppSend(std::string msg, std::string msgID) = 0;
  virtual void messageInstantSend(std::string account, uint32_t uid,
                                  std::string msg, std::string msgID) = 0;
  virtual void messageInstantSend2(std::string account, uint32_t uid,
                                   std::string msg, std::string msgID,
                                   std::string options) = 0;
  virtual void messageChannelSend(std::string channelID, std::string msg,
                                  std::string msgID) = 0;
  virtual void messageChannelSendForce(std::string channelID, std::string msg,
                                       std::string msgID) = 0;
  virtual void messagePushSend(std::string account, uint32_t uid,
                               std::string msg, std::string msgID) = 0;
  virtual void messageChatSend(std::string account, uint32_t uid,
                               std::string msg, std::string msgID) = 0;
  virtual void messageDTMFSend(uint32_t uid, std::string msg,
                               std::string msgID) = 0;

  // other
  virtual void setBackground(uint32_t bOut) = 0;
  virtual void setNetworkStatus(uint32_t bOut) = 0;
  virtual void ping() = 0;
  virtual void setAttr(std::string name, std::string value) = 0;
  virtual void getAttr(std::string name) = 0;
  virtual void getAttrAll() = 0;
  virtual void getUserAttr(std::string account, std::string name) = 0;
  virtual void getUserAttrAll(std::string account) = 0;
  virtual void queryUserStatus(std::string account) = 0;

  // query
  virtual void invoke(std::string name, std::string req,
                      std::string callID) = 0;

  // start SDK loop
  virtual void start() = 0;

  // stop SDK loop
  virtual void stop() = 0;

  virtual bool isOnline() = 0;
  virtual int getStatus() = 0;
  virtual int getSdkVersion() = 0;

  // broadcast
  virtual void bc_call(std::string func, std::string json_args,
                       std::string callID) = 0;

  virtual void dbg(std::string a, std::string b) = 0;

  virtual void destroy() = 0;
};
}  // namespace agora_sdk

#if defined _WIN32 || __MINGW__
#define LIB_PRE __declspec(dllexport)
#elif defined __unix__
#define LIB_PRE __attribute__((visibility("default")))
#else
#define LIB_PRE __attribute__((visibility("default")))
#endif

#ifdef MEDIA_VERSION
#ifdef FEATURE_SIGNALING_ENGINE
extern "C" LIB_PRE agora_sdk::IAgoraAPI* getAgoraSDKInstanceM();
#endif
extern "C" LIB_PRE agora_sdk::IAgoraAPI* createAgoraSDKInstanceM();
extern "C" LIB_PRE void* getInstanceCookieM(agora_sdk::IAgoraAPI*);
extern "C" LIB_PRE void setInstanceCookieM(agora_sdk::IAgoraAPI*, void*);
#else
extern "C" LIB_PRE agora_sdk::IAgoraAPI* getAgoraSDKInstance();
extern "C" LIB_PRE agora_sdk::IAgoraAPI* createAgoraSDKInstance();
extern "C" LIB_PRE void* getInstanceCookie(agora_sdk::IAgoraAPI*);
extern "C" LIB_PRE void setInstanceCookie(agora_sdk::IAgoraAPI*, void*);
#endif

#endif
