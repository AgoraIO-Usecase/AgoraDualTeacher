
#include "./EventHandler.h"
#include "Rtc.h"

#define LOG_FUN(...) LOG_I(__FUNCTION__##__VA_ARGS__)


void EventHandler::init()
{
    
}

void EventHandler::onJoinChannelSuccess(const char* channel, uid_t userId, int elapsed) {
  //  LOG_FUN(" %s,%d,%d",channel,userId,elapsed);
    emit Rtc::GetRtcEngine().onJoinChannelSuccess();
   
}

void EventHandler::onRejoinChannelSuccess(const char* channel, uid_t userId, int elapsed) {
   // LOG_FUN(" %s,%d,%d", channel, userId, elapsed);
    //app->onEnter(true);
}


void EventHandler::onWarning(int warn, const char* msg) {
    (void)warn;
    (void)msg;
 //   LOG_I("%s : %d",msg,warn);
}

void EventHandler::onError(int err, const char* msg) {
    (void)err;
    (void)msg;
  //  LOG_FUN();
}


void EventHandler::onAudioQuality(uid_t userId, int quality, unsigned short delay, unsigned short lost) {
    (void)userId;
    (void)quality;
    (void)delay;
    (void)lost;
  
}


void EventHandler::onLastmileProbeResult(const LastmileProbeResult& result) {
    (void)result;
    
}

void EventHandler::onAudioVolumeIndication(const AudioVolumeInfo* speakers, unsigned int speakerNumber, int totalVolume) {
    //LOG_FUN();
    /*for (int i = 0; i < speakerNumber; ++i) {
        if (speakers[i].uid == 123) {
            char sVolume[20] = { 0 };
            sprintf_s(sVolume, "Volume:%d\n", speakers[i].volume);
            //OutputDebugStringA(sVolume);
        }
    }*/
   
}

void EventHandler::onLeaveChannel(const RtcStats& stats) { 
   // (void)stats; LOG_FUN();
   // app->onRtcLeave();
    emit Rtc::GetRtcEngine().onLeaveChannel();
}

void EventHandler::onRtcStats(const RtcStats& stats) {
    Rtc::GetRtcEngine().SetRtcStats(stats);
    //lastmileDelay
    //rxAudioKBitRate
    //rxVideoKBitRate
   /* LOG_V("onRtcStats() duration:%d, tx/rx Bytes(%d/%d){Audio(%d/%d),Video(%d/%d)},KBitRate(%d/%d){Audio(%d/%d),Video(%d/%d)},lmDelay(%d),userCount(%d),cpuAppUsage(%.3f),cpuTotalUsage(%.3f),connectTimeMs(%d),txPacketLossRate(%d),rxPacketLossRate(%d)",
           stats.duration,stats.txBytes,stats.rxBytes,stats.txAudioBytes, stats.rxAudioBytes, stats.txVideoBytes, stats.rxVideoBytes,
        stats.txKBitRate, stats.rxKBitRate, stats.txAudioKBitRate, stats.rxAudioKBitRate, stats.txVideoKBitRate, stats.rxVideoKBitRate,
        stats.lastmileDelay, stats.userCount, stats.cpuAppUsage, stats.cpuTotalUsage, stats.connectTimeMs, stats.txPacketLossRate, stats.rxPacketLossRate);*/
}


void EventHandler::onAudioDeviceStateChanged(const char* deviceId, int deviceType, int deviceState) {
    (void)deviceId;
    (void)deviceType;
    (void)deviceState;
    
} 


void EventHandler::onAudioMixingFinished() {  }

void EventHandler::onAudioEffectFinished(int soundId) {  }


void EventHandler::onVideoDeviceStateChanged(const char* deviceId, int deviceType, int deviceState) {
    (void)deviceId;
    (void)deviceType;
    (void)deviceState;
   // LOG_I("onVideoDeviceStateChanged %s:%d %d",deviceId,deviceType,deviceState);
}

void EventHandler::onNetworkQuality(uid_t userId, int txQuality, int rxQuality) {
    (void)userId;
    (void)txQuality;
    (void)rxQuality;
   // LOG_V("onNetworkQuality() tx:%d rx:%d",txQuality,rxQuality);
}


void EventHandler::onIntraRequestReceived() {  }


void EventHandler::onLastmileQuality(int quality) { (void)quality; 
}


void EventHandler::onFirstLocalVideoFrame(int width, int height, int elapsed) {
    (void)width;
    (void)height;
    (void)elapsed;
    
}

void EventHandler::onFirstLocalVideoFramePublished(int elapsed) { (void)elapsed; }
 

void EventHandler::onFirstRemoteVideoDecoded(uid_t userId, int width, int height, int elapsed) {
    (void)userId;
    (void)width;
    (void)height;
    (void)elapsed;
    emit  Rtc::GetRtcEngine().onFirstRemoteVideoDecoded(userId, width, height, elapsed);
}

void EventHandler::onVideoSizeChanged(uid_t userId, int width, int height, int rotation) {
    (void)userId;
    (void)width;
    (void)height;
    (void)rotation;
   // LOG_I("onVideoSizeChanged %d,%d,%d,%d", userId, height, width, rotation);
}
 
void EventHandler::onRemoteVideoStateChanged(uid_t userId, REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason, int elapsed) {
    (void)userId;
    (void)state;
    (void)reason;
    (void)elapsed;
   // LOG_I("onRemoteVideoStateChanged %d,%d,%d",userId,state,reason);
}
 

void EventHandler::onFirstRemoteVideoFrame(uid_t userId, int width, int height, int elapsed) {
    //app->onShowFrame(userId, width, height, elapsed);
}
 
void EventHandler::onUserJoined(uid_t userId, int elapsed) {
    emit Rtc::GetRtcEngine().onUserJoined((unsigned int)userId, elapsed);
}
 
void EventHandler::onUserOffline(uid_t userId, USER_OFFLINE_REASON_TYPE reason) {
    emit Rtc::GetRtcEngine().onUserOffline((unsigned int)userId);
}

void EventHandler::onUserMuteVideo(uid_t userId, bool muted) {
    (void)userId;
    (void)muted;
    
}

void EventHandler::onUserEnableVideo(uid_t userId, bool enabled) {
    (void)userId;
    (void)enabled;
    
}

void EventHandler::onUserEnableLocalVideo(uid_t userId, bool enabled) {
    (void)userId;
    (void)enabled;
    
}

void EventHandler::onApiCallExecuted(int err, const char* api, const char* result) {
    (void)err;
    (void)api;
    (void)result;
    
}
 
void EventHandler::onLocalAudioStats(const LocalAudioStats& stats) { (void)stats; //
}
 

void EventHandler::onRemoteAudioStats(const RemoteAudioStats& stats) { 
   
}
 

void EventHandler::onLocalVideoStats(const LocalVideoStats& stats) { (void)stats; //
}
 

void EventHandler::onRemoteVideoStats(const RemoteVideoStats& stats) { 
    Rtc::GetRtcEngine().SetRemoteVideoInfo(stats);
}
 
void EventHandler::onCameraReady() {  }

void EventHandler::onCameraFocusAreaChanged(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    
}

void EventHandler::onCameraExposureAreaChanged(int x, int y, int width, int height) {
    (void)x;
    (void)y;
    (void)width;
    (void)height;
    
}

void EventHandler::onVideoStopped() {  }
 
void EventHandler::onAudioMixingStateChanged(AUDIO_MIXING_STATE_TYPE state, AUDIO_MIXING_ERROR_TYPE errorCode) {
    (void)state;
    (void)errorCode;
    
}
 
void EventHandler::onConnectionLost() {  }
 

void EventHandler::onConnectionInterrupted() {  }

/** Occurs when your connection is banned by the Agora Server.
*/

void EventHandler::onConnectionBanned() {  }


static inline unsigned int toInt(const unsigned char* data) {
    unsigned int ret = (data[3]) | (((unsigned int)data[2]) << 8) | (((unsigned int)data[1]) << 16) | (((unsigned int)data[0]) << 24);
    return ret;
}

void EventHandler::onStreamMessage(uid_t userId, int streamId, const char* data, size_t length, uint64_t sentTs) {
    emit Rtc::GetRtcEngine().onStreamMessage((unsigned int)userId, streamId, QString(data), length, sentTs);
}
 

void EventHandler::onStreamMessageError(uid_t userId, int streamId, int code, int missed, int cached) {
    agora::ERROR_CODE_TYPE err = (agora::ERROR_CODE_TYPE)code;
  
}

void EventHandler::onRequestToken() {  }
 

void EventHandler::onTokenPrivilegeWillExpire(const char* token) { (void)token; 
}
 

void EventHandler::onFirstLocalAudioFramePublished(int elapsed) { (void)elapsed; 
}
 

void EventHandler::onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE state, LOCAL_AUDIO_STREAM_ERROR error) {
    (void)state;
    (void)error;
    //
}
 

void EventHandler::onRemoteAudioStateChanged(uid_t uid, REMOTE_AUDIO_STATE state, REMOTE_AUDIO_STATE_REASON reason, int elapsed) {
    (void)uid;
    (void)state;
    (void)reason;
    (void)elapsed;
    
}
 

void EventHandler::onActiveSpeaker(uid_t userId) { (void)userId; 
}
 

void EventHandler::onClientRoleChanged(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole) {  }
 
void EventHandler::onAudioDeviceVolumeChanged(MEDIA_DEVICE_TYPE deviceType, int volume, bool muted) {
    (void)deviceType;
    (void)volume;
    (void)muted;
    
}
 
void EventHandler::onRtmpStreamingStateChanged(const char* url, RTMP_STREAM_PUBLISH_STATE state, RTMP_STREAM_PUBLISH_ERROR errCode) {
    (void)url;
    (void)state;
    (void)errCode;
    
}
 
void EventHandler::onStreamPublished(const char* url, int error) {
    (void)url;
    (void)error;
    
}
 
void EventHandler::onStreamUnpublished(const char* url) { (void)url; 
}
 

void EventHandler::onTranscodingUpdated() {  }

void EventHandler::onAudioRoutingChanged(int routing) { (void)routing; 
}
 

void EventHandler::onChannelMediaRelayStateChanged(int state, int code) {
    (void)state;
    (void)code;
    
}
 

void EventHandler::onChannelMediaRelayEvent(int code) { (void)code; 
}

void EventHandler::onLocalPublishFallbackToAudioOnly(bool isFallbackOrRecover) {
    (void)isFallbackOrRecover;
}

void EventHandler::onRemoteSubscribeFallbackToAudioOnly(uid_t userId, bool isFallbackOrRecover) {
    (void)userId;
    (void)isFallbackOrRecover;
    
}
 

void EventHandler::onRemoteAudioTransportStats(uid_t userId, unsigned short delay, unsigned short lost, unsigned short rxKBitRate) {
    (void)userId;
    (void)delay;
    (void)lost;
    (void)rxKBitRate;
    //
}
 
void EventHandler::onRemoteVideoTransportStats(uid_t userId, unsigned short delay, unsigned short lost, unsigned short rxKBitRate) {
    (void)userId;
    (void)delay;
    (void)lost;
    (void)rxKBitRate;
  //  
}
 

void EventHandler::onConnectionStateChanged(CONNECTION_STATE_TYPE state, CONNECTION_CHANGED_REASON_TYPE reason) {
    (void)state;
    (void)reason;
    //LOG_I("onConnectionStateChanged:%d %d",state,reason);
}
 

void EventHandler::onNetworkTypeChanged(NETWORK_TYPE type) {
    (void)type;
    
}
 
/*void EventHandler::onEncryptionError(ENCRYPTION_ERROR_TYPE errorType) {
    (void)errorType;
    
}*/
 
void EventHandler::onLocalUserRegistered(uid_t uid, const char* userAccount) {
    (void)uid;
    (void)userAccount;
    
}
 

void EventHandler::onUserInfoUpdated(uid_t uid, const UserInfo& info) {
    (void)uid;
    (void)info;
    
}
 
void EventHandler::onAudioSubscribeStateChanged(const char* channel, uid_t uid, STREAM_SUBSCRIBE_STATE oldState, STREAM_SUBSCRIBE_STATE newState, int elapseSinceLastState) {
    (void)channel;
    (void)uid;
    (void)oldState;
    (void)newState;
    (void)elapseSinceLastState;
    
}
 

void EventHandler::onVideoSubscribeStateChanged(const char* channel, uid_t uid, STREAM_SUBSCRIBE_STATE oldState, STREAM_SUBSCRIBE_STATE newState, int elapseSinceLastState) {
    (void)channel;
    (void)uid;
    (void)oldState;
    (void)newState;
    (void)elapseSinceLastState;
    
}
 
void EventHandler::onAudioPublishStateChanged(const char* channel, STREAM_PUBLISH_STATE oldState, STREAM_PUBLISH_STATE newState, int elapseSinceLastState) {
    (void)channel;
    (void)oldState;
    (void)newState;
    (void)elapseSinceLastState;
    
}
 
void EventHandler::onVideoPublishStateChanged(const char* channel, STREAM_PUBLISH_STATE oldState, STREAM_PUBLISH_STATE newState, int elapseSinceLastState) {
    (void)channel;
    (void)oldState;
    (void)newState;
    (void)elapseSinceLastState;
    
}
