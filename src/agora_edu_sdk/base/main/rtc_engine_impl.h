//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once

#include <atomic>
#include <memory>
#include <utility>

#include "api2/NGIAgoraAudioDeviceManager.h"
#include "api2/NGIAgoraExtensionControl.h"
#include "api2/NGIAgoraLocalUser.h"
#include "api2/NGIAgoraRtcConnection.h"
#include "api2/internal/media_node_factory_i.h"
#include "channel_manager.h"
#include "core/agora_service_impl.h"
#include "extension_node_manager.h"
#include "facilities/tools/api_logger.h"
#include "internal/rtc_engine_i.h"
#include "local_track_manager.h"
#include "media_player_manager.h"
#include "rtc_engine/internal/media_engine_i.h"

namespace agora {
namespace rtc {
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
class BaseStreamProxy;
#endif  // FEATURE_RTMP_STREAMING_SERVICE

struct RtcImage;

class RtcEngine : public IRtcEngineEx,
                  public agora::has_slots<>,
                  public IAudioDeviceManagerObserver {
 public:
  enum VIDEO_SOURCE_TYPE { VIDEO_SOURCE_CAMERA, VIDEO_SOURCE_SCREEN, VIDEO_SOURCE_EXTERNAL };

  RtcEngine();

 public:  // IRtcEngineEx
  int initialize(const RtcEngineContext& context) override;
  const char* getVersion(int* build) override;
  const char* getErrorDescription(int code) override;
  // int uid version
  int joinChannel(const char* token, const char* channelId, const char* info,
                  uid_t userId) override;
  int joinChannel(const char* token, const char* channelId, uid_t userId,
                  const ChannelMediaOptions& options) override;
  int leaveChannel() override;
  int leaveChannel(const LeaveChannelOptions& options) override;
  int startEchoTest() override;
  int stopEchoTest() override;
  int startLastmileProbeTest(const LastmileProbeConfig& config) override;
  int stopLastmileProbeTest() override;
  int enableVideo() override;
  int disableVideo() override;
  int startPreview() override;
  int stopPreview() override;
  int enableAudio() override;
  int disableAudio() override;
  int enableLocalAudio(bool enabled) override;
  int enableLocalVideo(bool enabled) override;
  int setDefaultMuteAllRemoteAudioStreams(bool mute) override;
  int setDefaultMuteAllRemoteVideoStreams(bool mute) override;
  int muteAllRemoteAudioStreams(bool mute) override;
  int muteAllRemoteVideoStreams(bool mute) override;
  int muteLocalAudioStream(bool mute) override;
  int muteLocalVideoStream(bool mute) override;
  int muteRemoteAudioStream(uid_t uid, bool mute,
                            conn_id_t connectionId = DEFAULT_CONNECTION_ID) override;
  int muteRemoteVideoStream(uid_t uid, bool mute,
                            conn_id_t connectionId = DEFAULT_CONNECTION_ID) override;
  int setRemoteVideoStreamType(uid_t userId, REMOTE_VIDEO_STREAM_TYPE streamType) override;
  int setRemoteDefaultVideoStreamType(REMOTE_VIDEO_STREAM_TYPE streamType) override;
  int enableAudioVolumeIndication(int interval, int smooth) override;
  int startAudioRecording(const char* filePath, AUDIO_RECORDING_QUALITY_TYPE quality) override;
  int stopAudioRecording() override;

  agora_refptr<IMediaPlayerSource> createMediaPlayer() override;
  int destroyMediaPlayer(agora_refptr<IMediaPlayerSource> media_player) override;

  int startAudioMixing(const char* filePath, bool loopback, bool replace, int cycle) override;
  int stopAudioMixing() override;
  int pauseAudioMixing() override;
  int resumeAudioMixing() override;
  int adjustAudioMixingVolume(int volume) override;
  int adjustAudioMixingPublishVolume(int volume) override;
  int getAudioMixingPublishVolume() override;
  int adjustAudioMixingPlayoutVolume(int volume) override;
  int getAudioMixingPlayoutVolume() override;
  int getAudioMixingDuration() override;
  int getAudioMixingCurrentPosition() override;
  int setAudioMixingPosition(int pos /*in ms*/) override;
  int preloadEffect(int soundId, const char* filePath) override;
  int playEffect(int soundId, const char* filePath, int loopCount, double pitch, double pan,
                 int gain, bool publish) override;
  int playAllEffects(int loopCount, double pitch, double pan, int gain, bool publish) override;

  int getEffectsVolume() override;
  int setEffectsVolume(int volume) override;

  int getVolumeOfEffect(int soundId) override;
  int setVolumeOfEffect(int soundId, int volume) override;

  int pauseEffect(int soundId) override;
  int pauseAllEffects() override;

  int resumeEffect(int soundId) override;
  int resumeAllEffects() override;

  int stopEffect(int soundId) override;
  int stopAllEffects() override;

  int unloadEffect(int soundId) override;
  int unloadAllEffects() override;

  int setLocalVoicePitch(double pitch) override;
  int setLocalVoiceEqualization(AUDIO_EQUALIZATION_BAND_FREQUENCY bandFrequency,
                                int bandGain) override;
  int setLocalVoiceReverb(AUDIO_REVERB_TYPE reverbKey, int value) override;
  int setLocalVoiceReverbPreset(AUDIO_REVERB_PRESET reverbPreset) override;
  int setLocalVoiceChanger(VOICE_CHANGER_PRESET voiceChanger) override;
  int pauseAudio() override;
  int resumeAudio() override;
  int setLogFile(const char* filePath) override;
  int setLogFilter(unsigned int filter) override;
  int setLogLevel(LOG_LEVEL level) override;
  int setLogFileSize(unsigned int fileSizeInKBytes) override;
  int setLocalRenderMode(media::base::RENDER_MODE_TYPE renderMode) override;
  int setRemoteRenderMode(uid_t userId, media::base::RENDER_MODE_TYPE renderMode,
                          conn_id_t connectionId = DEFAULT_CONNECTION_ID) override;
  int setLocalVideoMirrorMode(VIDEO_MIRROR_MODE_TYPE mirrorMode) override;
  int enableDualStreamMode(bool enabled) override;

  int setExternalAudioSource(bool enabled, int sampleRate, int channels, int sourceNumber) override;

  int setRecordingAudioFrameParameters(int sampleRate, int channel,
                                       RAW_AUDIO_FRAME_OP_MODE_TYPE mode,
                                       int samplesPerCall) override;
  int setPlaybackAudioFrameParameters(int sampleRate, int channel,
                                      RAW_AUDIO_FRAME_OP_MODE_TYPE mode,
                                      int samplesPerCall) override;
  int setMixedAudioFrameParameters(int sampleRate, int channel, int samplesPerCall) override;
  int setPlaybackAudioFrameBeforeMixingParameters(int sampleRate, int channel) override;
  int adjustRecordingSignalVolume(int volume) override;
  int muteRecordingSignal(bool mute) override;
  int adjustPlaybackSignalVolume(int volume) override;
  int enableWebSdkInteroperability(bool enabled) override;
  int enableLoopbackRecording(bool enabled) override;
  int enableLoopbackRecording(conn_id_t connectionId, bool enabled) override;
  int adjustLoopbackRecordingVolume(int volume) override;
  int getLoopbackRecordingVolume() override;
  int enableInEarMonitoring(bool enabled, bool includeAudioFilter) override;
  int setInEarMonitoringVolume(int volume) override;

#if defined(__ANDROID__) || (defined(__APPLE__) && TARGET_OS_IOS)
  int switchCamera() override;
  int setDefaultAudioRouteToSpeakerphone(bool defaultToSpeaker) override;
  int setEnableSpeakerphone(bool speakerOn) override;
  bool isSpeakerphoneEnabled() override;
#endif  // __ANDROID__ || (__APPLE__ && TARGET_OS_IOS)
  int getCallId(util::AString& callId) override;
  int rate(const char* callId, int rating,
           const char* description) override;  // 0~10
  int complain(const char* callId, const char* description) override;
  int setupRemoteVideo(const VideoCanvas& canvas,
                       conn_id_t connectionId = DEFAULT_CONNECTION_ID) override;
  int setupLocalVideo(const VideoCanvas& canvas) override;
  int queryInterface(INTERFACE_ID_TYPE iid, void** inter) override;
  int registerPacketObserver(IPacketObserver* observer) override;
  int setVideoEncoderConfiguration(const VideoEncoderConfiguration& config,
                                   conn_id_t connectionId = DEFAULT_CONNECTION_ID) override;
  int setAudioProfile(AUDIO_PROFILE_TYPE profile, AUDIO_SCENARIO_TYPE scenario) override;
  int setAudioProfile(AUDIO_PROFILE_TYPE profile) override;
  int renewToken(const char* token) override;
  int setChannelProfile(CHANNEL_PROFILE_TYPE profile) override;
  int setClientRole(CLIENT_ROLE_TYPE role) override;
  int addVideoWatermark(const RtcImage& watermark) override;
  int clearVideoWatermarks() override;
  int setRemoteUserPriority(uid_t uid, PRIORITY_TYPE userPriority) override;
  void release(bool sync) override;
  int sendCustomReportMessage(const char* id, const char* category, const char* event,
                              const char* label, int value, conn_id_t connectionId = 0) override;

 public:
  int initialize(const RtcEngineContext2& context) override;
  int initializeEx(const RtcEngineContextEx& context) override;
  int setParameters(const char* parameters) override;
  int getParameters(const char* key, any_document_t& results) override;
  int setProfile(const char* profile, bool merge) override;
  int getProfile(any_document_t& result) override;
  int notifyNetworkChange(commons::network::network_info_t&& networkInfo) override;
  int getOptionsByVideoProfile(int profile, VideoNetOptions& options) override;
  int setEncryptionMode(const char* encryptionMode) override;
  int setEncryptionSecret(const char* secret) override;
  int enableEncryption(bool enabled, const EncryptionConfig& config,
                       conn_id_t connectionId = DEFAULT_CONNECTION_ID) override;

  int createDataStream(int* streamId, bool reliable, bool ordered,
                       conn_id_t connectionId = 0) override;
  int sendStreamMessage(int streamId, const char* data, size_t length,
                        conn_id_t connectionId = 0) override;

#if defined TARGET_OS_MAC && !TARGET_OS_IPHONE
  int startScreenCaptureByDisplayId(unsigned int displayId, const Rectangle& regionRect,
                                    const ScreenCaptureParameters& captureParams) override;
#endif

#if defined(_WIN32)
  int startScreenCaptureByScreenRect(const Rectangle& screenRect, const Rectangle& regionRect,
                                     const ScreenCaptureParameters& captureParams) override;
#endif

#if defined(__ANDROID__)
  int startScreenCapture(void* mediaProjectionPermissionResultData,
                         const ScreenCaptureParameters& captureParams) override;
#endif

#if defined(_WIN32) || !(TARGET_OS_IPHONE) && (TARGET_OS_MAC)
  int startScreenCaptureByWindowId(view_t windowId, const Rectangle& regionRect,
                                   const ScreenCaptureParameters& captureParams) override;

  int setScreenCaptureContentHint(VIDEO_CONTENT_HINT contentHint) override;

  int updateScreenCaptureRegion(const Rectangle& regionRect) override;
#endif

#if defined(_WIN32) || (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC)) || defined(__ANDROID__)

  int updateScreenCaptureParameters(const ScreenCaptureParameters& captureParams) override;

  int stopScreenCapture() override;
#endif

#if defined(_WIN32)
  void SetScreenCaptureSource(bool allow_magnification_api, bool allow_directx_capturer) override;
#endif  // _WIN32
  int setLogLevelEx(unsigned int filter) override;

  int simulateOnSetParameters(const std::string& parameters) override;

  int setCameraDevice(const char dev_id[MAX_DEVICE_ID_LENGTH]) override;

  int addPublishStreamUrl(const char* url, bool transcodingEnabled) override;
  int removePublishStreamUrl(const char* url) override;
  int setLiveTranscoding(const LiveTranscoding& transcoding) override;
  int addInjectStreamUrl(const char* url, const InjectStreamConfig& config) override;
  int addInjectStreamUrl2(const char* url, protocol::CmdInjectStreamConfig& config) override;
  int enableYuvDumper(bool enable) override;
  int setVideoConfigParam(const char* params) override;
  int getVideoConfigParam(char* param, size_t size) override;
  int setExternalVideoConfigEx(const VideoEncoderConfiguration& config,
                               conn_id_t connectionId) override;
  int removeInjectStreamUrl(const char* url) override;

  bool registerEventHandler(IRtcEngineEventHandler* eventHandler) override;
  bool unregisterEventHandler(IRtcEngineEventHandler* eventHandler) override;

  int reportWebAgentVideoStats(const WebAgentVideoStats& stats) override;
  int reportArgusCounters(int* counterId, int* value, int count, user_id_t userId) override;

  int reportRecordingArgusEvent(uint32_t* eventIds, int64_t* value, int count,
                                RecordingEventType eventType) override;
  int printLog(int level, const char* message) override;
  int setVideoProfileEx(int width, int height, int frameRate, int bitrate) override;
  int runOnWorkerThread(std::function<void(void)>&& f) override;
  CONNECTION_STATE_TYPE getConnectionState(conn_id_t connectionId) override;
  int registerMediaMetadataObserver(IMetadataObserver* observer,
                                    IMetadataObserver::METADATA_TYPE type) override;

#if (defined(__APPLE__) && !(TARGET_OS_IOS) && (TARGET_OS_MAC))
  int monitorDeviceChange(bool enabled) override;
  IVideoDeviceManager* getVideoDeviceManager();
#endif

  int setAudioOptionParams(const char* params) override;
  int getAudioOptionParams(char* params) override;
  int setAudioSessionParams(const char* params) override;
  int getAudioSessionParams(char* params) override;

  bool isMicrophoneOn() override;
  int enableLocalVideoFilter(const char* name, const char* vendor,
                             agora_refptr<IVideoFilter> filter, int enable) override;

  int enableLocalVideoFilter(const char* name, const char* vendor, int enable) override;

  int setLocalVideoFilterProperty(const char* name, const char* vendor, const char* key,
                                  const void* value, int size) override;

  int getLocalVideoFilterProperty(const char* name, const char* vendor, const char* key,
                                  void* value, int size) override;

  int enableRemoteVideoFilter(const char* name, const char* vendor,
                              agora_refptr<IVideoFilter> filter, int enable) override;

  int enableRemoteVideoFilter(const char* name, const char* vendor, int enable) override;

  int setRemoteVideoFilterProperty(const char* name, const char* vendor, const char* key,
                                   const void* value, int size) override;

  int getRemoteVideoFilterProperty(const char* name, const char* vendor, const char* key,
                                   void* value, int size) override;

  int startAudioFrameDump(const char* channel_id, uid_t user_id, const char* location,
                          const char* uuid, const char* passwd, long duration_ms,  // NOLINT
                          bool auto_upload) override;

  int stopAudioFrameDump(const char* channel_id, uid_t user_id, const char* location) override;

 public:
  RtcEngineContextEx getRtcEngineContext() const { return rtc_contextex_; }

  AudioRoute getDefaultAudioRoute() const { return default_audio_route_; }

  agora_refptr<IRtcConnection> getConnection(conn_id_t connectionId = DEFAULT_CONNECTION_ID) const {
    API_LOGGER_MEMBER("connectionId:%d", connectionId);

    return channel_manager_->getConnectionById(connectionId);
  }

  base::IAgoraService* getAgoraService() { return service_ptr_ex_; }

  agora_refptr<INGAudioDeviceManager> getAudioDeviceManager() { return audio_device_manager_; }

  agora_refptr<IMediaNodeFactory> getMediaNodeFactory() { return media_node_factory_ex_; }

 private:
  int createLocalCameraTrackForDefaultChannel();

  int startService(const RtcEngineContextEx& context);
  int stopService(bool waitForAll = false);

  int doSetParameters(const char* format, ...);
  int setObject(const char* key, const char* format, ...);
  int convertPath(const char* filePath, util::AString& value);
  bool is_valid_str(const char* k) const { return k && *k != '\0'; }
  bool isValidChannelId(const char* channelId) const {
    return (channelId && base::AgoraService::isValidChannelId(channelId));
  }
  int playEffect(int soundId, int loopCount, double pitch, double pan, int gain, bool publish);
  int publishAudioEffect(int soundId);
  int publishAllAudioEffect();
  int unpublishAudioEffect(int soundId);
  int unpublishAllAudioEffect();

 public:  // IAudioDeviceManagerObserver
  void onDeviceStateChanged() override;
  void onRoutingChanged(AudioRoute route) override;

  template <class T>
  bool serializeEvent(const T& p, std::string& result) {
    commons::packer pk;
    pk << p;
    pk.pack();
    result = std::string(pk.buffer(), pk.length());
    return true;
  }

 public:  // For multiple channels
  int joinChannelEx(const char* token, const char* channelId, uid_t uid,
                    const ChannelMediaOptions& options, IRtcEngineEventHandler* eventHandler,
                    conn_id_t* connectionId) override;
  int updateChannelMediaOptions(
      const ChannelMediaOptions& options,
      conn_id_t connectionId = agora::rtc::DEFAULT_CONNECTION_ID) override;
  int leaveChannelEx(const char* channelId, conn_id_t connectionId) override;

 public:  // for Media Engine
  const ChannelMediaOptions& defaultChannMediaOptions() const {
    return default_channel_media_options_;
  }

  LocalTrackManager* localTrackManager() const { return local_track_manager_.get(); }

  ChannelManager* channelManager() const { return channel_manager_.get(); }

  int setExternalVideoSource(bool enabled, bool useTexture, bool encoded);

  int registerAudioFrameObserver(agora::media::IAudioFrameObserver* observer);

 private:
  void createDefaultConnectionIfNeeded(const RtcEngineContextEx& rtc_contextex);
  IRtcConnectionEx* getDefaultConnection();
  int initLowLevelModules(const RtcEngineContextEx& context);
  void cleanupLocalMediaTracks();
  int doEnableInEarMonitoring(bool enabled, bool includeAudioFilter);
  void resetPublishMediaOptions();

  int enableLocalAudioInternal(bool enabled, bool changePublishState = true);
  int enableLocalVideoInternal(bool enabled, bool changePublishState = true);

 private:
  ~RtcEngine();

 private:
  base::IAgoraServiceEx* service_ptr_ex_ = nullptr;
  std::atomic_bool m_initialized = {false};

  RtcEngineContextEx rtc_contextex_;

  // Default channel
  agora_refptr<IRtcConnection> default_connection_;
  conn_id_t default_connection_id_ = 0;
  ILocalUser* local_user_ = nullptr;
  // Configurations only for default channel
  ChannelMediaOptions default_channel_media_options_;
  std::string default_channel_id_;
  // Stored publishAudioTrack flag if `::StartAudioMixing` changed the default one
  absl::optional<bool> stored_publishAudioTrack_for_mixing_;

  std::unique_ptr<LocalTrackManager> local_track_manager_;
  std::unique_ptr<MediaPlayerManager> media_player_manager_;
  std::unique_ptr<ChannelManager> channel_manager_;

  agora_refptr<IMediaNodeFactoryEx> media_node_factory_ex_;
  agora_refptr<ILocalVideoTrack> local_screen_track_;

  agora_refptr<INGAudioDeviceManager> audio_device_manager_;
  AudioRoute default_audio_route_ = ROUTE_DEFAULT;
  AudioRoute cur_audio_route_ = ROUTE_DEFAULT;
  bool default_audio_route_set_ = false;
  bool in_ear_monitoring_enabled_ = false;
  bool ear_monitoring_include_audio_filter_ = true;

  bool recording_signal_muted_ = false;
  int prev_recording_signal_volume_ = 100;

  ExtensionNodes local_extensions_;
#if defined(FEATURE_RTMP_STREAMING_SERVICE)
  std::unique_ptr<BaseStreamProxy> live_stream_proxy_;
#endif  // FEATURE_RTMP_STREAMING_SERVICE
};

}  // namespace rtc
}  // namespace agora
