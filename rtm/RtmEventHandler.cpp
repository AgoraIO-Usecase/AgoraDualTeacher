#include <sstream>
#include <iostream>
#include "base.h"
#include "./RtmEventHandler.h"
#include "JoypadController.h"
#include "Rtm.h"
#include "SDL.h"
#include "../src/protocol.h"
#include "../src/Config.h"
#include <QDebug>
void WriteLog(const char* info) {
	//
#ifdef WIN32
	OutputDebugStringA(info);
#else
	qDebug() << info << "\n";
#endif
}


RtmEventHandler::RtmEventHandler(IJoypadHotPlug* joynotify) :
	joynotify_(joynotify)
{
	game_control_.Initialize();
}

void RtmEventHandler::onLoginSuccess()
{
	loginRtmSuccessFlag = true;
	Rtm::GetRtmEngine().joinRtmChannel(userinfo.channel_id.c_str());
	//emit Rtm::GetRtmEngine().onRtmEnter(true);
}

void RtmEventHandler::onLoginFailure(LOGIN_ERR_CODE errorCode)
{
	loginRtmSuccessFlag = false;
	emit Rtm::GetRtmEngine().onRtmEnter(false);
}

void RtmEventHandler::onRenewTokenResult(const char *token, RENEW_TOKEN_ERR_CODE errorCode)
{
	
}

void RtmEventHandler::onTokenExpired()
{
	
}

void RtmEventHandler::onLogout(LOGOUT_ERR_CODE errorCode)
{
	game_control_.ResetDevice();
	emit Rtm::GetRtmEngine().onRtmLeave((int)errorCode);
}


void RtmEventHandler::onConnectionStateChanged(CONNECTION_STATE state, CONNECTION_CHANGE_REASON reason)
{
	if (state == ERROR_CONNECTION_ABORTED) {
		game_control_.ResetDevice();
	}
	emit Rtm::GetRtmEngine().onConnectionStateChanged((int)state, (int)reason);
}

void RtmEventHandler::onSendMessageResult(long long messageId, PEER_MESSAGE_ERR_CODE errorCode)
{	

}

void RtmEventHandler::onMessageReceivedFromPeer(const char *peerId, const IMessage *message)
{
	//OutputDebugStringA("onMessageReceivedFromPeer\n");
	WriteLog(message->getRawMessageData());


	auto head = (PackageHead*)( const_cast<char*>(message->getRawMessageData()));
	std::uint32_t packet_size = head->size;
	std::uint32_t total_size = sizeof(PackageHead)+packet_size;

	auto client_packet = reinterpret_cast<const ClientPacket*>(head + 1);
	auto action = client_packet->action;
	if (action == ClientAction::kControl) {
		game_control_.Replay(reinterpret_cast<const ClientControl*>(client_packet), packet_size);
	}
}

void RtmEventHandler::onImageMessageReceivedFromPeer(const char *peerId, const IImageMessage *message)
{
}

void RtmEventHandler::onFileMessageReceivedFromPeer(const char *peerId, const IFileMessage *message)
{
	
}

void RtmEventHandler::onMediaUploadingProgress(long long requestId, const MediaOperationProgress &progress)
{
	
}

void RtmEventHandler::onMediaDownloadingProgress(long long requestId, const MediaOperationProgress &progress)
{
	
}

void RtmEventHandler::onFileMediaUploadResult(long long requestId, IFileMessage *fileMessage, UPLOAD_MEDIA_ERR_CODE code)
{
	 
}

void RtmEventHandler::onImageMediaUploadResult(long long requestId, IImageMessage *imageMessage, UPLOAD_MEDIA_ERR_CODE code)
{
	
}

void RtmEventHandler::onMediaDownloadToFileResult(long long requestId, DOWNLOAD_MEDIA_ERR_CODE code)
{
	
}

void RtmEventHandler::onMediaDownloadToMemoryResult(long long requestId, const char *memory, long long length, DOWNLOAD_MEDIA_ERR_CODE code)
{
	
}

void RtmEventHandler::onMediaCancelResult(long long requestId, CANCEL_MEDIA_ERR_CODE code)
{
	(long long)requestId;
	
}

void RtmEventHandler::onQueryPeersOnlineStatusResult(long long requestId, const PeerOnlineStatus *peersStatus, int peerCount, QUERY_PEERS_ONLINE_STATUS_ERR errorCode)
{
	
}

void RtmEventHandler::onSubscriptionRequestResult(long long requestId, PEER_SUBSCRIPTION_STATUS_ERR errorCode)
{
	
}

void RtmEventHandler::onQueryPeersBySubscriptionOptionResult(long long requestId, const char *peerIds[], int peerCount, QUERY_PEERS_BY_SUBSCRIPTION_OPTION_ERR errorCode)
{
	
}

void RtmEventHandler::onPeersOnlineStatusChanged(const PeerOnlineStatus peersStatus[], int peerCount)
{
	
}

void RtmEventHandler::onGetChannelMemberCountResult(long long requestId, const ChannelMemberCount *channelMemberCounts, int channelCount, GET_CHANNEL_MEMBER_COUNT_ERR_CODE errorCode)
{
	
}

//rtmChannel
void RtmEventHandler::onJoinSuccess()
{
	joinedRtmChannelFlag = true;
	if (!userinfo.host)  //client,confirm host joined
		Rtm::GetRtmEngine().getMembers();
	emit Rtm::GetRtmEngine().funJoinSucc();
}

void RtmEventHandler::onJoinFailure(JOIN_CHANNEL_ERR errorCode)
{
	joinedRtmChannelFlag = false;
	QString error = "";
	if (errorCode == JOIN_CHANNEL_ERR_ALREADY_JOINED) {
		error = QString::fromStdWString(L"already joined");
	}
	else {
		error = QString("%1").arg(errorCode);
	}
	emit Rtm::GetRtmEngine().funJoinFailed(error);
}

void RtmEventHandler::onLeave(LEAVE_CHANNEL_ERR errorCode)
{
	joinedRtmChannelFlag = false;
	emit Rtm::GetRtmEngine().funcLeaveChannel((int)errorCode);
}

void RtmEventHandler::onMessageReceived(const char* userId, const IMessage* message)
{
	int len = message->getRawMessageLength();
	if (len != 0) {
		const char* data = message->getRawMessageData();

	}
	else {
		const char* str = message->getText();

	}
}

void RtmEventHandler::onImageMessageReceived(const char* userId, const IImageMessage* message)
{

}

void RtmEventHandler::onFileMessageReceived(const char* userId, const IFileMessage* message)
{

}

void RtmEventHandler::onSendMessageResult(long long messageId, CHANNEL_MESSAGE_ERR_CODE state)
{

}

void RtmEventHandler::onMemberJoined(IChannelMember* member)
{
	emit Rtm::GetRtmEngine().funMemberJoin(QString::fromStdString(member->getUserId()));
}

void RtmEventHandler::onMemberLeft(IChannelMember* member)
{
	emit Rtm::GetRtmEngine().funMemberLeft(QString::fromStdString(member->getUserId()));
}

void RtmEventHandler::onGetMembers(IChannelMember** members, int userCount, GET_MEMBERS_ERR errorCode)
{
	bool bFind = false;
	for (int i = 0; i < userCount; ++i) {
		QString remoteUid = QString("%1").arg(userinfo.remote_uid);
		QString memberUid = QString::fromStdString(members[i]->getUserId());
		if (remoteUid.compare(memberUid) == 0) {
			bFind = true;
		}
	}
	if (bFind)
		emit Rtm::GetRtmEngine().onGetMembersSuccess();
	else
		emit Rtm::GetRtmEngine().onGetMembersFailed();
}

void RtmEventHandler::onAttributesUpdated(const IRtmChannelAttribute* attributes[], int numberOfAttributes)
{

}

void RtmEventHandler::onMemberCountUpdated(int memberCount)
{

}

int RtmEventHandler::Dispatch(SDL_Event* event)
{
	if (!loginRtmSuccessFlag)
		return 0;

	int exit = 0;
	uint32_t eventtype = event->type;
	switch (eventtype) {
		// 游戏控制事件
	case SDL_CONTROLLERBUTTONDOWN: handleControllerButtonDown(event); break;
	case SDL_CONTROLLERBUTTONUP: handleControllerButtonUp(event); break;
	case SDL_CONTROLLERDEVICEADDED: handleControllerDeviceAdded(event); break;
	case SDL_CONTROLLERDEVICEREMOVED: handleControllerDeviceMoved(event); break;
	case SDL_CONTROLLERDEVICEREMAPPED: break;
	case SDL_CONTROLLERAXISMOTION: handleControllerAxisMotion(event); break;

		// 鼠标控制事件
	case SDL_MOUSEMOTION: handleMouseMotion(event); break;
	case SDL_MOUSEBUTTONDOWN: handleMouseButtonDown(event); break;
	case SDL_MOUSEBUTTONUP: handleMouseButtonUp(event); break;
	case SDL_MOUSEWHEEL: handleMouseWheel(event); break;

		// 键盘控制事件
	case SDL_KEYDOWN: handleKeyDownEvent(event); break;
	case SDL_KEYUP: handleKeyUpEvent(event); break;

		// window系统事件
	case SDL_QUIT: handleAppQuit(event); exit = 1; break;
	case SDL_WINDOWEVENT: handleWindowEvent(event); break;

	default: break;
	}

	return exit;
}

void RtmEventHandler::DispatchKBMessage(void* param)
{
	PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)param;
	if (p == nullptr) return;
	handleHookKBEvent(p);
}

void RtmEventHandler::handleAppQuit(SDL_Event* event)
{
}

void RtmEventHandler::handleWindowEvent(SDL_Event* event)
{
}

void RtmEventHandler::handleKeyDownEvent(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ": virtual key = " << event->key.keysym.sym << ", scan code =" << event->key.keysym.scancode << "\n";
	WriteLog(ss.str().c_str());

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientKeyboard));
	auto head = (PackageHead*)(buffer.data());
	head->size = sizeof(ClientKeyboard);
	auto& keymsg = *reinterpret_cast<ClientKeyboard*>(head + 1);
	keymsg.action = ClientAction::kControl;
	keymsg.key_code = (event->key.keysym.scancode);
	keymsg.reserved = 0;
	keymsg.state = ButtonState::Pressed;
	keymsg.type = ControlType::kKeyboard;
	keymsg.timestamp = event->key.timestamp;

	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());
	
	//if (transport_)
	//	transport_->Send(buffer);
}

void RtmEventHandler::handleKeyUpEvent(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ": virtual key = " << event->key.keysym.sym << ", scan code =" << event->key.keysym.scancode << "\n";
	WriteLog(ss.str().c_str());

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientKeyboard));
	auto head = (PackageHead*)(buffer.data());
	head->size = (sizeof(ClientKeyboard));
	auto& keymsg = *reinterpret_cast<ClientKeyboard*>(head + 1);
	keymsg.action = ClientAction::kControl;
	keymsg.key_code = (event->key.keysym.scancode);
	keymsg.reserved = 0;
	keymsg.state = ButtonState::Released;
	keymsg.type = ControlType::kKeyboard;
	keymsg.timestamp = (event->key.timestamp);
	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());
	//if (transport_)
	//	transport_->Send(buffer);
}

void RtmEventHandler::handleControllerButtonDown(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ": which = " << (int)event->cbutton.which << ", button = " <<
		(int)event->cbutton.button << ", state = " << (int)event->cbutton.state << "\n";
	WriteLog(ss.str().c_str());

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientGamepadButton));
	auto head = (PackageHead*)(buffer.data());
	head->size = (sizeof(ClientGamepadButton));
	auto& motion = *reinterpret_cast<ClientGamepadButton*>(head + 1);
	motion.action = ClientAction::kControl;
	motion.type = ControlType::kGamepadButton;
	motion.timestamp = (event->cbutton.timestamp);
	motion.which = event->jbutton.which;
	motion.button = event->jbutton.button;
	motion.state = ButtonState::Pressed;
	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());

//	if (transport_)
//		transport_->Send(buffer);
}

void RtmEventHandler::handleControllerButtonUp(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ": which = " << (int)event->cbutton.which << ", button = " <<
		(int)event->cbutton.button << ", state = " << (int)event->cbutton.state << "\n";
	WriteLog(ss.str().c_str());

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientGamepadButton));
	auto head = (PackageHead*)(buffer.data());
	head->size = (sizeof(ClientGamepadButton));
	auto& motion = *reinterpret_cast<ClientGamepadButton*>(head + 1);
	motion.action = ClientAction::kControl;
	motion.type = ControlType::kGamepadButton;
	motion.timestamp = (event->cbutton.timestamp);
	motion.which = event->jbutton.which;
	motion.button = event->jbutton.button;
	motion.state = ButtonState::Released;
	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());

	//if (transport_)
	//	transport_->Send(buffer);
}

void RtmEventHandler::handleControllerDeviceAdded(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ", which = " << event->cdevice.which << "\n";
	WriteLog(ss.str().c_str());

	if (joynotify_)
		joynotify_->deviceAdded(event->cdevice.which);
}

void RtmEventHandler::handleControllerDeviceMoved(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ", which = " << event->cdevice.which << "\n";
	WriteLog(ss.str().c_str());

	if (joynotify_)
		joynotify_->deviceRemoved(event->cdevice.which);
}

void RtmEventHandler::handleControllerAxisMotion(SDL_Event* event)
{
	const int DEAD_ZONE = 32000;
	/*if (event->caxis.which != 0)
		return;*/

		/*bool bSend = false;
		switch (event->caxis.axis) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			if (event->caxis.value > DEAD_ZONE || event->caxis.value < -DEAD_ZONE)
				bSend = true;
			break;
		default:
			bSend = true;
			break;
		}

		if (bSend == false)
			return;*/

	std::stringstream ss;
	ss << __FUNCTION__ << ": which = " << (int)event->caxis.which << ", axis = " <<
		(int)event->caxis.axis << ", value = " << (int)event->caxis.value << "\n";
	//WriteLog(ss .str().c_str());

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientGamepadAxis));
	auto head = (PackageHead*)(buffer.data());
	head->size = (sizeof(ClientGamepadAxis));
	auto& motion = *reinterpret_cast<ClientGamepadAxis*>(head + 1);
	motion.action = ClientAction::kControl;
	motion.type = ControlType::kGamepadAxis;
	motion.timestamp = (event->caxis.timestamp);
	motion.axis = event->caxis.axis;
	motion.which = event->caxis.which;
	motion.value = (event->caxis.value);
	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());

	//if (transport_)
	//	transport_->Send(buffer);
}

void RtmEventHandler::handleMouseWheel(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ": x = " << event->wheel.x << ", y = " << event->wheel.y << "\n";
	WriteLog(ss.str().c_str());

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientMouseWheel));
	auto head = (PackageHead*)(buffer.data());
	head->size = (sizeof(ClientMouseWheel));
	auto& mouse = *reinterpret_cast<ClientMouseWheel*>(head + 1);
	mouse.x = (event->wheel.x);
	mouse.y = (event->wheel.y);
	mouse.action = ClientAction::kControl;
	mouse.type = ControlType::kMouseWheel;
	mouse.timestamp = (_time32(NULL));
	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());

	//if (transport_)
	//	transport_->Send(buffer);
}

void RtmEventHandler::handleMouseMotion(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ": xrel = " << event->button.x << ", yrel = " << event->button.y << "\n";
	WriteLog(ss.str().c_str());

	/*RECT rcClient;
	GetClientRect(&rcClient);*/
	int srcWidth = Rtm::GetRtmEngine().width;
	int srcHeight = Rtm::GetRtmEngine().height;

	uint16_t dx = (uint16_t)(1.0 * event->button.x / srcWidth * 1000);
	uint16_t dy = (uint16_t)(1.0 * event->button.y / srcHeight * 1000);

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientMouseMove));
	auto head = (PackageHead*)(buffer.data());
	head->size = (sizeof(ClientMouseMove));
	auto& mouse = *reinterpret_cast<ClientMouseMove*>(head + 1);
	mouse.x = (dx);
	mouse.y = (dy);
	mouse.action = ClientAction::kControl;
	mouse.type = ControlType::kMouseMove;
	mouse.timestamp = (event->button.timestamp);
	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());

	//if (transport_)
	//	transport_->Send(buffer);
}

void RtmEventHandler::handleMouseButtonDown(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ": x = " << event->button.x << ", y = " << event->button.y << "\n";
	WriteLog(ss.str().c_str());

	int srcWidth = Rtm::GetRtmEngine().width;
	int srcHeight = Rtm::GetRtmEngine().height;


	uint16_t dx = (uint16_t)(1.0 * event->button.x / srcWidth * 1000);
	uint16_t dy = (uint16_t)(1.0 * event->button.y / srcHeight * 1000);

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientMouseButton));
	auto head = (PackageHead*)(buffer.data());
	head->size = (sizeof(ClientMouseButton));
	auto& mouse = *reinterpret_cast<ClientMouseButton*>(head +1);
	mouse.x = (dx);
	mouse.y = (dy);
	mouse.state = ButtonState::Pressed;
	mouse.button = event->button.button;
	mouse.action = ClientAction::kControl;
	mouse.type = ControlType::kMouseButton;
	mouse.timestamp = (event->button.timestamp);
	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());

	//if (transport_)
	//	transport_->Send(buffer);
}

void RtmEventHandler::handleMouseButtonUp(SDL_Event* event)
{
	std::stringstream ss;
	ss << __FUNCTION__ << ": x = " << event->button.x << ", y = " << event->button.y << "\n";
	WriteLog(ss.str().c_str());

	int srcWidth = Rtm::GetRtmEngine().width;
	int srcHeight = Rtm::GetRtmEngine().height;


	uint16_t dx = (uint16_t)(1.0 * event->button.x / srcWidth * 1000);
	uint16_t dy = (uint16_t)(1.0 * event->button.y / srcHeight * 1000);

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientMouseButton));
	auto head = (PackageHead*)(buffer.data());
	head->size = (sizeof(ClientMouseButton));
	auto& mouse = *reinterpret_cast<ClientMouseButton*>(head +1);
	mouse.x = (dx);
	mouse.y = (dy);
	mouse.state = ButtonState::Released;
	mouse.button = event->button.button;
	mouse.action = ClientAction::kControl;
	mouse.type = ControlType::kMouseButton;
	mouse.timestamp = (event->button.timestamp);
	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());

	//if (transport_)
	//	transport_->Send(buffer);
}

#define USE_VK

void RtmEventHandler::handleHookKBEvent(PKBDLLHOOKSTRUCT p)
{
#ifndef USE_VK
	std::uint16_t scancode = MAKE_AGORA_SCANCODE((BYTE)p->scanCode, p->flags & LLKHF_EXTENDED);
#else
	std::uint16_t scancode = MAKE_AGORA_SCANCODE((BYTE)p->vkCode, p->flags & LLKHF_EXTENDED);
#endif
	BOOL isdown = !(p->flags & LLKHF_UP);

	std::stringstream ss;
	ss << __FUNCTION__ << ": vk = " << p->vkCode << ", scancode = " << scancode << ", button " << isdown << "\n";
	WriteLog(ss.str().c_str());

	std::string buffer;
	buffer.resize(sizeof(PackageHead) + sizeof(ClientKeyboard));
	auto head = (PackageHead*)(buffer.data());
	head->size = sizeof(ClientKeyboard);
	auto& keymsg = *reinterpret_cast<ClientKeyboard*>(head + 1);
	keymsg.action = ClientAction::kControl;
	keymsg.key_code = scancode;
	keymsg.reserved = 0;
	keymsg.state = (isdown ? ButtonState::Pressed : ButtonState::Released);
	keymsg.timestamp = p->time;

#ifndef USE_VK
	keymsg.type = ControlType::kKeyboard;
#else
	keymsg.type = ControlType::kKeyboardVk;
#endif

	Rtm::GetRtmEngine().send((unsigned char*)buffer.data(), buffer.length(), userinfo.remote_signalId.data());
}
 