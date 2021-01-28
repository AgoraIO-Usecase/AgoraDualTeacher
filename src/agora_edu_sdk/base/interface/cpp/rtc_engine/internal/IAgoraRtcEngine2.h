//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include "IAgoraRtcEngine.h"

namespace agora {
namespace rtc {

class IRtcEngineEventHandler2;
struct RtcEngineContext2
{
  IRtcEngineEventHandler2* eventHandler;
  const char* appId;
  // For android, it the context(Activity or Application
  void* context;
  // Used to deprecate enalbeAudio and enableVideo before joinChannel()
  bool enableAudio;
  bool enableVideo;
  unsigned int areaCode;

  RtcEngineContext2()
      : eventHandler(NULL)
      , appId(NULL)
      , context(NULL)
      , enableAudio(true)
      , enableVideo(false)
      , areaCode(AREA_CODE_GLOB)
  {}
};

class IRtcEngine2 : public IRtcEngine
{
public:
    using IRtcEngine::initialize;
    virtual int initialize(const RtcEngineContext2& context) = 0;

    /**
    * Specify video stream parameters based on video profile
    * @param [in] width
    *        width of video resolution in pixel
    * @param [in] height
    *        height of video resolution in pixel
    * @param [in] frameRate
    *        frame rate in fps
    * @param [in] bitrate
    *        bit rate in kbps
    * @return return 0 if success or an error code
    */
    virtual int setVideoProfileEx(int width, int height, int frameRate, int bitrate) = 0;
};

class IRtcEngineEventHandler2 : public IRtcEngineEventHandler
{
public:
    using IRtcEngineEventHandler::onJoinChannelSuccess;
    using IRtcEngineEventHandler::onRejoinChannelSuccess;
    using IRtcEngineEventHandler::onAudioQuality;
    using IRtcEngineEventHandler::onNetworkQuality;
    using IRtcEngineEventHandler::onFirstRemoteVideoDecoded;
    using IRtcEngineEventHandler::onVideoSizeChanged;
    using IRtcEngineEventHandler::onRemoteVideoStateChanged;
    using IRtcEngineEventHandler::onLocalVideoStateChanged;
    using IRtcEngineEventHandler::onFirstRemoteVideoFrame;
    using IRtcEngineEventHandler::onUserJoined;
    using IRtcEngineEventHandler::onUserOffline;
    using IRtcEngineEventHandler::onUserMuteVideo;
    using IRtcEngineEventHandler::onUserEnableVideo;
    using IRtcEngineEventHandler::onUserEnableLocalVideo;
    using IRtcEngineEventHandler::onStreamMessage;
    using IRtcEngineEventHandler::onStreamMessageError;
    using IRtcEngineEventHandler::onActiveSpeaker;
    using IRtcEngineEventHandler::onStreamInjectedStatus;
    using IRtcEngineEventHandler::onRemoteSubscribeFallbackToAudioOnly;

    /**
    * when join channel success, the function will be called
    * @param [in] channel
    *        the channel name you have joined
    * @param [in] userId
    *        the userId of you in this channel
    * @param [in] elapsed
    *        the time elapsed in ms from the joinChannel been called to joining completed
    */
    virtual void onJoinChannelSuccess(const char* channel, user_id_t userId, int elapsed) {
        (void)channel;
        (void)userId;
        (void)elapsed;
    }

    /**
    * when join channel success, the function will be called
    * @param [in] channel
    *        the channel name you have joined
    * @param [in] userId
    *        the userId of you in this channel
    * @param [in] elapsed
    *        the time elapsed in ms elapsed
    */
    virtual void onRejoinChannelSuccess(const char* channel, user_id_t userId, int elapsed) {
        (void)channel;
        (void)userId;
        (void)elapsed;
    }

    /**
    * when audio quality message come, the function will be called
    * @param [in] userId
    *        the userId of the peer
    * @param [in] quality
    *        the quality of the remote user, see QUALITY_TYPE for value definition
    * @param [in] delay
    *        the average time of the audio packages delayed
    * @param [in] lost
    *        the rate of the audio packages lost
    */
    virtual void onAudioQuality(user_id_t userId, int quality, unsigned short delay, unsigned short lost) {
        (void)userId;
        (void)quality;
        (void)delay;
        (void)lost;
    }

    /**
    * report the network quality
    * @param [in] userId
    *        the userId of the remote user
    * @param [in] txQuality
    *        the score of the send network quality 0~5 the higher the better
    * @param [in] rxQuality
    *        the score of the recv network quality 0~5 the higher the better
    */
    virtual void onNetworkQuality(user_id_t userId, int txQuality, int rxQuality) {
        (void)userId;
        (void)txQuality;
        (void)rxQuality;
    }

    /**
    * when the first remote video frame decoded, the function will be called
    * @param [in] userId
    *        the userId of the remote user
    * @param [in] width
    *        the width of the video frame
    * @param [in] height
    *        the height of the video frame
    * @param [in] elapsed
    *        the time elapsed from channel joined in ms
    */
    virtual void onFirstRemoteVideoDecoded(user_id_t userId, int width, int height, int elapsed) {
        (void)userId;
        (void)width;
        (void)height;
        (void)elapsed;
    }

    /**
    * when video size changed or rotation changed, the function will be called
    * @param [in] userId
    *        the userId of the remote user or local user (0)
    * @param [in] width
    *        the new width of the video
    * @param [in] height
    *        the new height of the video
    * @param [in] rotation
    *        the rotation of the video
    */
    virtual void onVideoSizeChanged(user_id_t userId, int width, int height, int rotation) {
        (void)userId;
        (void)width;
        (void)height;
        (void)rotation;
    }

    virtual void onRemoteVideoStateChanged(user_id_t userId, REMOTE_VIDEO_STATE state, REMOTE_VIDEO_STATE_REASON reason, int elapsed) {
        (void)userId;
        (void)state;
        (void)reason;
        (void)elapsed;
    }

    virtual void onLocalVideoStateChanged(LOCAL_VIDEO_STREAM_STATE state,
                                          LOCAL_VIDEO_STREAM_ERROR errorCode) {
      (void)state;
      (void)errorCode;
    }

    /**
    * when the first remote video frame displayed, the function will be called
    * @param [in] userId
    *        the userId of the remote user
    * @param [in] width
    *        the width of the video frame
    * @param [in] height
    *        the height of the video frame
    * @param [in] elapsed
    *        the time elapsed from remote user called joinChannel in ms
    */
    virtual void onFirstRemoteVideoFrame(user_id_t userId, int width, int height, int elapsed) {
        (void)userId;
        (void)width;
        (void)height;
        (void)elapsed;
    }

    /**
    * when any other user joined in the same channel, the function will be called
    * @param [in] userId
    *        the userId of the remote user
    * @param [in] elapsed
    *        the time elapsed from remote used called joinChannel to joining completed in ms
    */
    virtual void onUserJoined(user_id_t userId, int elapsed) {
        (void)userId;
        (void)elapsed;
    }

    /**
    * when user offline(exit channel or offline by accident), the function will be called
    * @param [in] userId
    *        the userId of the remote user
    */
    virtual void onUserOffline(user_id_t userId, USER_OFFLINE_REASON_TYPE reason) {
        (void)userId;
        (void)reason;
    }

    /**
    * when remote user muted the video stream, the function will be called
    * @deprecated Use onRemoteVideoStateChanged instead of.
    * @param [in] userId
    *        the userId of the remote user
    * @param [in] muted
    *        true: the remote user muted the video stream, false: the remote user unmuted the video stream
    */
    virtual void onUserMuteVideo(user_id_t userId, bool muted) {
      (void)userId;
      (void)muted;
    }

    /**
    * when remote user enable video function, the function will be called
    * @deprecated Use onRemoteVideoStateChanged instead of.
    * @param [in] userId
    *        the userId of the remote user
    * @param [in] enabled
    *        true: the remote user has enabled video function, false: the remote user has disabled video function
    */
    virtual void onUserEnableVideo(user_id_t userId, bool enabled) {
      (void)userId;
      (void)enabled;
    }

    /**
     * when remote user enable local video function, the function will be called
     * @deprecated Use onRemoteVideoStateChanged instead of.
     * @param [in] userId
     *        the userId of the remote user
     * @param [in] enabled
     *        true: the remote user has enabled local video function, false: the remote user has disabled local video function
     */
    virtual void onUserEnableLocalVideo(user_id_t userId, bool enabled) {
      (void)userId;
      (void)enabled;
    }

    /**
    * when stream message received, the function will be called
    * @param [in] userId
    *        userId of the peer who sends the message
    * @param [in] streamId
    *        APP can create multiple streams for sending messages of different purposes
    * @param [in] data
    *        the message data
    * @param [in] length
    *        the message length, in bytes
    *        frame rate
    */
    virtual void onStreamMessage(user_id_t userId, int streamId, const char* data, size_t length) {
        (void)userId;
        (void)streamId;
        (void)data;
        (void)length;
    }

    /**
    *
    */
    virtual void onStreamMessageError(user_id_t userId, int streamId, int code, int missed, int cached) {
        (void)userId;
        (void)streamId;
        (void)code;
        (void)missed;
        (void)cached;
    }

    /** @param [in] userId
    *        the speaker userId who is talking in the channel
    */
    virtual void onActiveSpeaker(user_id_t userId) {
        (void)userId;
    }

    virtual void onPublishingRequestAnswered(user_id_t userId, int response, int error) {
        (void)userId;
        (void)response;
        (void)error;
    }

    virtual void onPublishingRequestReceived(user_id_t userId) {
        (void)userId;
    }

    virtual void onUnpublishingRequestReceived(user_id_t userId) {
        (void)userId;
    }

    virtual void onStreamInjectedStatus(const char* url, user_id_t userId, int status) {
        (void)url;
        (void)userId;
        (void)status;
    }
    virtual void onRemoteSubscribeFallbackToAudioOnly(user_id_t userId, bool isFallbackOrRecover) {
        (void)userId;
        (void)isFallbackOrRecover;
    }
  virtual void onRtmpStreamingStateChanged(const char* url, RTMP_STREAM_PUBLISH_STATE state,
                                           RTMP_STREAM_PUBLISH_ERROR errCode){
        (void) url;
        (void) state;
        (void) errCode;
  };
  virtual void onStreamPublished(const char* url, int error){
        (void)url;
        (void)error;
  };
  virtual void onStreamUnpublished(const char* url){
        (void)url;
  };
  virtual void onTranscodingUpdated(){};

  virtual void onUserAccountUpdated(uid_t uid, const char* user_account){
    (void)uid;
    (void)user_account;
  }
};

}}
