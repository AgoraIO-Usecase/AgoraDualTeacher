#pragma once
#include <QObject>
#include <functional>
#include "IAgoraRtmService.h"
#include "src/GameControl.h"

using namespace agora::rtm;

union SDL_Event;
class Rtm;
class IJoypadHotPlug;
class RtmEventHandler : public QObject 
	, public agora::rtm::IRtmServiceEventHandler
	, public agora::rtm::IChannelEventHandler
{
	Q_OBJECT
public:
	RtmEventHandler(IJoypadHotPlug* joynotify);
	bool IsLogined() { return loginRtmSuccessFlag; }
	//
	int Dispatch(SDL_Event* event);
	void DispatchKBMessage(void* param);

protected:
	virtual void onLoginSuccess();
	virtual void onLoginFailure(LOGIN_ERR_CODE errorCode);
	virtual void onRenewTokenResult(const char* token, RENEW_TOKEN_ERR_CODE errorCode);
	virtual void onTokenExpired();
	virtual void onLogout(LOGOUT_ERR_CODE errorCode);
	virtual void onConnectionStateChanged(CONNECTION_STATE state, CONNECTION_CHANGE_REASON reason);
	virtual void onSendMessageResult(long long messageId, PEER_MESSAGE_ERR_CODE errorCode);
	virtual void onMessageReceivedFromPeer(const char* peerId, const IMessage* message);
	virtual void onImageMessageReceivedFromPeer(const char* peerId, const IImageMessage* message);
	virtual void onFileMessageReceivedFromPeer(const char* peerId, const IFileMessage* message);
	virtual void onMediaUploadingProgress(long long requestId, const MediaOperationProgress& progress);
	virtual void onMediaDownloadingProgress(long long requestId, const MediaOperationProgress& progress);
	virtual void onFileMediaUploadResult(long long requestId, IFileMessage* fileMessage, UPLOAD_MEDIA_ERR_CODE code);
	virtual void onImageMediaUploadResult(long long requestId, IImageMessage* imageMessage, UPLOAD_MEDIA_ERR_CODE code);
	virtual void onMediaDownloadToFileResult(long long requestId, DOWNLOAD_MEDIA_ERR_CODE code);
	virtual void onMediaDownloadToMemoryResult(long long requestId, const char* memory, long long length, DOWNLOAD_MEDIA_ERR_CODE code);
	virtual void onMediaCancelResult(long long requestId, CANCEL_MEDIA_ERR_CODE code);
	virtual void onQueryPeersOnlineStatusResult(long long requestId, const PeerOnlineStatus* peersStatus, int peerCount, QUERY_PEERS_ONLINE_STATUS_ERR errorCode);
	virtual void onSubscriptionRequestResult(long long requestId, PEER_SUBSCRIPTION_STATUS_ERR errorCode);
	virtual void onQueryPeersBySubscriptionOptionResult(long long requestId, const char* peerIds[], int peerCount, QUERY_PEERS_BY_SUBSCRIPTION_OPTION_ERR errorCode);
	virtual void onPeersOnlineStatusChanged(const PeerOnlineStatus peersStatus[], int peerCount);

	virtual void onGetChannelMemberCountResult(long long requestId, const ChannelMemberCount* channelMemberCounts, int channelCount, GET_CHANNEL_MEMBER_COUNT_ERR_CODE errorCode);
	//channel event
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

	//client control event
	void handleMouseMotion(SDL_Event* event);
	void handleAppQuit(SDL_Event* event);
	void handleWindowEvent(SDL_Event* event);
	void handleKeyDownEvent(SDL_Event* event);
	void handleKeyUpEvent(SDL_Event* event);
	void handleControllerButtonDown(SDL_Event* event);
	void handleControllerButtonUp(SDL_Event* event);
	void handleControllerDeviceAdded(SDL_Event* event);
	void handleControllerDeviceMoved(SDL_Event* event);
	void handleControllerAxisMotion(SDL_Event* event);
	void handleMouseWheel(SDL_Event* event);
	void handleMouseButtonDown(SDL_Event* event);
	void handleMouseButtonUp(SDL_Event* event);
	void handleHookKBEvent(PKBDLLHOOKSTRUCT p);
	
private:
	bool loginRtmSuccessFlag = false;
	bool joinedRtmChannelFlag = false;
	GameControl game_control_;
	IJoypadHotPlug* joynotify_;
};
