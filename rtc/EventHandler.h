#pragma once

#include "IAgoraRtcEngine.h"
#include <QObject>
using namespace agora::rtc;

class EventHandler : public QObject, public IRtcEngineEventHandler
{
    Q_OBJECT
public:
    void init();
    virtual void onJoinChannelSuccess(const char* channel, uid_t userId, int elapsed)override;
    virtual void onRejoinChannelSuccess(const char* channel, uid_t userId, int elapsed)override;
    virtual void onWarning(int warn, const char* msg)override;
    virtual void onError(int err, const char* msg)override;
    virtual void onAudioQuality(uid_t userId, int quality, unsigned short delay,
        unsigned short lost)override;
    virtual void onLastmileProbeResult(const LastmileProbeResult& result)override;
    virtual void onAudioVolumeIndication(const AudioVolumeInfo* speakers, unsigned int speakerNumber,
        int totalVolume)override;
    virtual void onLeaveChannel(const RtcStats& stats)override;

    virtual void onRtcStats(const RtcStats& stats)override;
    virtual void onAudioDeviceStateChanged(const char* deviceId, int deviceType, int deviceState)override;
    virtual void onAudioMixingFinished()override;
   
    virtual void onAudioEffectFinished(int soundId)override;
    virtual void onVideoDeviceStateChanged(const char* deviceId, int deviceType, int deviceState)override;
    virtual void onNetworkQuality(uid_t userId, int txQuality, int rxQuality)override;
    virtual void onIntraRequestReceived()override;
    virtual void onLastmileQuality(int quality)override;
    virtual void onFirstLocalVideoFrame(int width, int height, int elapsed)override;
    virtual void onFirstLocalVideoFramePublished(int elapsed)override;
    virtual void onFirstRemoteVideoDecoded(uid_t userId, int width, int height, int elapsed)override;
    virtual void onVideoSizeChanged(uid_t userId, int width, int height, int rotation)override;
    virtual void onRemoteVideoStateChanged(uid_t userId, REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason, int elapsed)override;
    virtual void onFirstRemoteVideoFrame(uid_t userId, int width, int height, int elapsed)override;

    virtual void onUserJoined(uid_t userId, int elapsed)override;

    virtual void onUserOffline(uid_t userId, USER_OFFLINE_REASON_TYPE reason)override;

    virtual void onUserMuteVideo(uid_t userId, bool muted)override;

    virtual void onUserEnableVideo(uid_t userId, bool enabled)override;

    virtual void onUserEnableLocalVideo(uid_t userId, bool enabled)override;

    virtual void onApiCallExecuted(int err, const char* api, const char* result)override;

    virtual void onLocalAudioStats(const LocalAudioStats& stats)override;


    virtual void onRemoteAudioStats(const RemoteAudioStats& stats)override;

    virtual void onLocalVideoStats(const LocalVideoStats& stats)override;
    virtual void onRemoteVideoStats(const RemoteVideoStats& stats)override;
    //virtual void onRemoteVideoStreamInfoUpdated(const RemoteVideoStreamInfo& info)override;

    virtual void onCameraReady()override;

    virtual void onCameraFocusAreaChanged(int x, int y, int width, int height)override;

    virtual void onCameraExposureAreaChanged(int x, int y, int width, int height)override;

    virtual void onVideoStopped()override;
    virtual void onAudioMixingStateChanged(AUDIO_MIXING_STATE_TYPE state, AUDIO_MIXING_ERROR_TYPE errorCode)override;
    virtual void onConnectionLost()override;
    virtual void onConnectionInterrupted()override;

    virtual void onConnectionBanned()override;

    virtual void onStreamMessage(uid_t userId, int streamId, const char* data, size_t length, uint64_t sentTs)override;
    virtual void onStreamMessageError(uid_t userId, int streamId, int code, int missed, int cached)override;

    virtual void onRequestToken()override;

    virtual void onTokenPrivilegeWillExpire(const char* token)override;

    virtual void onFirstLocalAudioFramePublished(int elapsed)override;
    virtual void onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE state, LOCAL_AUDIO_STREAM_ERROR error)override;

    virtual void onRemoteAudioStateChanged(uid_t uid, REMOTE_AUDIO_STATE state, REMOTE_AUDIO_STATE_REASON reason, int elapsed)override;
    virtual void onActiveSpeaker(uid_t userId)override;
    virtual void onClientRoleChanged(CLIENT_ROLE_TYPE oldRole, CLIENT_ROLE_TYPE newRole)override;
    virtual void onAudioDeviceVolumeChanged(MEDIA_DEVICE_TYPE deviceType, int volume, bool muted)override;
    virtual void onRtmpStreamingStateChanged(const char* url, RTMP_STREAM_PUBLISH_STATE state,
        RTMP_STREAM_PUBLISH_ERROR errCode)override;
    virtual void onStreamPublished(const char* url, int error)override;
    virtual void onStreamUnpublished(const char* url)override;
    virtual void onTranscodingUpdated()override;
   
    virtual void onAudioRoutingChanged(int routing)override;
    virtual void onChannelMediaRelayStateChanged(int state, int code)override;
    virtual void onChannelMediaRelayEvent(int code)override;


    virtual void onLocalPublishFallbackToAudioOnly(bool isFallbackOrRecover)override;

    virtual void onRemoteSubscribeFallbackToAudioOnly(uid_t userId, bool isFallbackOrRecover)override;
    virtual void onRemoteAudioTransportStats(uid_t userId, unsigned short delay, unsigned short lost,
        unsigned short rxKBitRate)override;
    virtual void onRemoteVideoTransportStats(uid_t userId, unsigned short delay, unsigned short lost,
        unsigned short rxKBitRate)override;
    virtual void onConnectionStateChanged(
        CONNECTION_STATE_TYPE state, CONNECTION_CHANGED_REASON_TYPE reason)override;
    virtual void onNetworkTypeChanged(NETWORK_TYPE type)override;
    virtual void onLocalUserRegistered(uid_t uid, const char* userAccount)override;
    virtual void onUserInfoUpdated(uid_t uid, const UserInfo& info)override;
    virtual void onAudioSubscribeStateChanged(const char* channel, uid_t uid, STREAM_SUBSCRIBE_STATE oldState, STREAM_SUBSCRIBE_STATE newState, int elapseSinceLastState)override;
    virtual void onVideoSubscribeStateChanged(const char* channel, uid_t uid, STREAM_SUBSCRIBE_STATE oldState, STREAM_SUBSCRIBE_STATE newState, int elapseSinceLastState)override;
    virtual void onAudioPublishStateChanged(const char* channel, STREAM_PUBLISH_STATE oldState, STREAM_PUBLISH_STATE newState, int elapseSinceLastState)override;
    virtual void onVideoPublishStateChanged(const char* channel, STREAM_PUBLISH_STATE oldState, STREAM_PUBLISH_STATE newState, int elapseSinceLastState)override;
private:
       
};
