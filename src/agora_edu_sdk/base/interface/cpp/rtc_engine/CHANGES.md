Note: Please update this file for every Agora API change you do. Simply fill in
your updates in the Working section below.


Agora High Level APIs (Working)
==================================================

API (yyyy-mm-dd)
==================================================
Purpose of this change

API file name #1
-------------
**Add:**
Short description
- Foo()
- Bar()

**Modified:**
Short description
- Changes Foo() to Foo1()
- Changes Bar() to Bar1()

**Deleted:**
Short description
- Deleted Foo()

API file name #2
-------------

API (2020-09-22)
==================================================
Support audio frame dump

IAgoraRtcEngine.h
-------------
**Add:**
- startAudioFrameDump()
- stopAudioFrameDump()


API (2020-09-16)
==================================================
Support SM4 Encryption

IAgoraRtcEngine.h
-------------
**Add:**
- IRtcEngine::enableEncryption

API (2020-09-10)
==================================================
Delete enum NETWORK_TYPE and add callback onNetworkTypeChanged

IAgoraRtcEngine.h
-------------
**Deleted:**
- Deleted enum NETWORK_TYPE

**Modified:**
- Changes Add onNetworkTypeChanged to IRtcEngineEventHandler


API (2020-09-04)
==================================================

rtc_engine_i.h
IAgoraRtcEngine.h
-------------
**Modified:**
Refine macros.

API (2020-09-01)
==================================================

Add v2.7.1 docs for high-level apis. For details, see jira MS-17241.

Merge docs for low-level apis from v2.7.0.2 to v2.7.1.

API (2020-08-21)
==================================================
Support audio device loopback test

IAgoraRtcEngine.h
-------------
**Add:**
- startAudioDeviceLoopbackTest()
- stopAudioDeviceLoopbackTest()

API (2020-08-18)
==================================================
Changes for warning

AgoraBase.h
-------------
**Modified:**
- Add warning WARN_CHANNEL_CONNECTION_IP_CHANGED/WARN_CHANNEL_CONNECTION_PORT_CHANGED;


API (2020-08-11)
==================================================
Support leave channel with options, options determines whether to do something when leave the channel.

IAgoraRtcEngine.h
-------------
**Add:**
When LeaveChannelOptions::stopAudioMixing = false, after leave the channel is still playing and mixing the music file.
- struct LeaveChannelOptions.
- leaveChannel(const LeaveChannelOptions& options)

API (2020-08-11)
==================================================
Support adjust loopback recording volume.

IAgoraRtcEngine.h
-------------
**Add:**
- adjustLoopbackRecordingVolume()
- getLoopbackRecordingVolume()


API (2020-08-02)
==================================================
**Deleted:**
- Deleted playEffect(int soundId, int loopCount, double pitch, double pan, int gain, bool publish = false)


API (2020-08-03)
==================================================
Modify data stream.

IAgoraRtcEngine.h
-------------
**Modified:**
Modify data stream apis
- Changes: add input args connectionId for createDataStream.
- Changes: add input args connectionId for sendStreamMessage.


API (2020-07-31)
==================================================
Make release() pure virtual in IMediaEngine.

IAgoraMediaEngine.h
-------------
**Modified:**
Make release() pure virtual in IMediaEngine.


API (2020-07-07)
==================================================
Modify ChannelMediaOptions.

IAgoraRtcEngine.h
-------------
**Modified:**
Rename pcmDataOnly of ChannelMediaOptions.
- Changes: Rename pcmDataOnly of ChannelMediaOptions to enableAudioRecordingOrPlayout.


API (2020-07-06)
==================================================
IAgoraRtcEngine.h
-------------
**Modify:**
Change include dir.


API (2020-07-01)
==================================================
IAgoraRtcEngine.h
-------------
**Modify:**
Remove string UID.


API (2020-06-27)
==================================================
Make include path correct


API (2020-07-03)
==================================================
Refine audio effect API

IAgoraRtcEngine.h
-------------
**Modified:**
- int playAllEffects(double pitch, double pan, int gain)
to int playAllEffects(int loopCount, double pitch, double pan, int gain, bool publish = false)

API (2020-06-21)
==================================================
Refine audio effect interface

IAgoraRtcEngine.h
-------------
**Add:**
- getEffectsVolume()
- setEffectsVolume()

**Modified:**
- preloadEffect(int soundId, const char* filePath)
- playEffect(int soundId, int loopCount, double pitch, double pan, int gain, bool publish = false)
- playEffect(int soundId, const char* filePath, int loopCount, double pitch, double pan, int gain, bool publish = false)


API (2020-06-17)
==================================================
Refine ChannelMediaOptions

IAgoraRtcEngine.h
-------------
**Modified:**
Refine ChannelMediaOptions
- `bool publishCameraTrack` to `base::Optional<bool> publishCameraTrack`
- `bool publishAudioTrack` to `base::Optional<bool> publishAudioTrack`
- `bool publishScreenTrack` to `base::Optional<bool> publishScreenTrack`
- `bool publishCustomAudioTrack` to `base::Optional<bool> publishCustomAudioTrack`
- `bool publishCustomVideoTrack` to `base::Optional<bool> publishCustomVideoTrack`
- `bool publishEncodedVideoTrack` to `base::Optional<bool> publishEncodedVideoTrack`
- `bool publishMediaPlayerAudioTrack` to `base::Optional<bool> publishMediaPlayerAudioTrack`
- `bool publishMediaPlayerVideoTrack` to `base::Optional<bool> publishMediaPlayerVideoTrack`
- `bool autoSubscribeAudio` to `base::Optional<bool> autoSubscribeAudio`
- `bool autoSubscribeVideo` to `base::Optional<bool> autoSubscribeVideo`
- `bool publishMediaPlayerId` to `base::Optional<bool> publishMediaPlayerId`
- `bool clientRoleType` to `base::Optional<CLIENT_ROLE_TYPE> clientRoleType`
- `bool defaultVideoStreamType` to `base::Optional<REMOTE_VIDEO_STREAM_TYPE> defaultVideoStreamType`
- `bool channelProfile` to `base::Optional<CHANNEL_PROFILE_TYPE> channelProfile`


API (2020-06-16)
==================================================
Modify ChannelMediaOptions.


IAgoraRtcEngine.h
-------------
**Modified:**
Add pcmDataOnly to ChannelMediaOptions.
- Changes: Add member pcmDataOnly for ChannelMediaOptions.


API (2020-06-16)
==================================================
Add interface to audio mixing volume

IAgoraRtcEngine.h
-------------
**Add:**
- adjustAudioMixingPublishVolume()
- getAudioMixingPublishVolume()
- adjustAudioMixingPlayoutVolume()
- getAudioMixingPlayoutVolume()


API (2020-06-16)
==================================================
IAgoraRtcEngine.h
-------------
**Add:**
- Add enableLoopbackRecording(conn_id_t connectionId, bool enabled), for supporting
to send audio pcm data got from loopback device by a specific connection.


API (2020-06-09)
==================================================
Modify the callback onBandwidthEstimationUpdated

IAgoraRtcEngine.h
-------------
**Modified:**
Modify the declaration of onBandwidthEstimationUpdated
- Changes "virtual void onBandwidthEstimationUpdated(int targetBitrateBps)" to "virtual void onBandwidthEstimationUpdated(const NetworkInfo& info)"


API (2020-06-03)
==================================================
IAgoraRtcEngine.h
-------------
**Add:**
- createMediaPlayer()
- destroyMediaPlayer(agora_refptr<IMediaPlayerSource> media_player)

**Modified:**
Audio effect interface

preloadEffect(int soundId, const char* filePath) -> preloadEffect(int& soundId, const char* filePath, int loopCount)
playEffect(int& soundId, const char* filePath, int loopCount, double pitch, double pan,
                 int gain, bool publish) -> playEffect(int soundId, const char* filePath, int loopCount, double pitch, double pan,
                 int gain, bool publish = false)
getEffectsVolume() -> getVolumeOfEffect(int soundId)

Add a new function playEffect(int soundId, double pitch, double pan, int gain, bool publish). It should be used with preloadEffect().
Add playAllEffects(double pitch, double pan, int gain).
Add unloadAllEffects().

Remove setEffectsVolume(int volume)


API (2020-06-01)
==================================================
IAgoraRtcEngine.h
-------------
**Add:**
- enableLocalVideoFilter(const char*, const char*, agora_refptr<IVideoFilter>, int)
- enableRemoteVideoFilter(const char*, const char*, agora_refptr<IVideoFilter>, int)


API (2020-05-29)
==================================================
IAgoraRtcEngine.h
-------------
**Modified:**
  - Move definition of type AREA_CODE from IAgoraRtcEngine.h to AgoraBase.h


API (2020-05-25)
==================================================
IAgoraRtcEngine.h
-------------
**Add:**
  - Add AREA_CODE type define


API (2020-05-20)
==================================================
IAgoraMediaPlayer.h
-------------
**Modified:**
- Refine the APIs' order of IMediaPlayer.
- Remove 'const' from the parameter of onPositionChanged() in IMediaPlayerObserver since unnecessary.


API (2020-05-14)
==================================================
Deprecated setAudioProfile(AUDIO_PROFILE_TYPE, AUDIO_SCENARIO_TYPE), add setAudioProfile(AUDIO_PROFILE_TYPE)

IAgoraRtcEngine.h
-------------
**Add:**
- setAudioProfile(AUDIO_PROFILE_TYPE)

**Deprecate:**
- setAudioProfile(AUDIO_PROFILE_TYPE, AUDIO_SCENARIO_TYPE)


API (2020-05-13)
==================================================
AgoraBase.h
-------------
**Modified:**
  - API annotations for EncodedVideoFrameInfo::framesPerSecond

IAgoraRtcEngine.h


API (2020-05-12)
==================================================

**Add:**
IAgoraMediaPlayer.h
-------------
Add relative log and unregister API in player.
setLogFile()
setLogFilter()
unregisterVideoFrameObserver()
unregisterAudioFrameObserver()


API (2020-05-12)
==================================================
Move lastmile and connection state enum to AgoraBase.h

IAgoraRtcEngine.h
-------------
**Deleted:**
Delete lastmile and connection state enum
- Deleted enum LASTMILE_PROBE_RESULT_STATE
- Deleted struct LastmileProbeOneWayResult
- Deleted struct LastmileProbeResult
- Deleted struct LastmileProbeConfig
- Deleted enum CONNECTION_CHANGED_REASON_TYPE
- Deleted enum AUDIO_REVERB_PRESET
- Deleted enum VOICE_CHANGER_PRESET
- Deleted struct ScreenCaptureParameters
- Deleted struct VideoCanvas


API (2020-05-09)
==================================================
IAgoraRtcEngine.h
--------------------
**Add:**

Add AUDIO_SCENARIO_TYPE audioScenario for RtcEngineContext


API (2020-05-08)
==================================================
Some structures are consistent with the definition in the old SDK,
resulting in conflicts when the new and old SDK are used at the same time.
Therefore, change the above the structures to agora::media::base namespace

AgoraBase.h
-------------
**Deleted:**
- AudioPcmFrame
- RENDER_MODE_TYPE
- IVideoFrameObserver
- IAudioFrameObserver

AgoraMediaBase.h
-------------
**Add:**
Move defines to agora::media::base namespace
- AudioPcmFrame
- RENDER_MODE_TYPE
- IVideoFrameObserver
- IAudioFrameObserver
- typedef void* view_t;
- typedef const char* user_id_t;

IAgoraMediaEngine.h
-------------
**Modified:**
Needed changes to namespace agora::media::base

IAgoraMediaPlayer.h
-------------
**Modified:**
Needed changes to namespace agora::media::base

IAgoraRtcEngine.h
-------------
**Modified:**
Needed changes to namespace agora::media::base


API (2020-05-08)
==================================================
AgoraBase.h
-------------
**Modified:**
  - Move LOG_LEVEL from AgoraBase.h to IAgoraLog.h


API (2020-05-06)
==================================================

IAgoraRtcEngine.h
**Added:**
  - Add interface setLogLevel(LOG_LEVEL)


API (2020-04-29)
==================================================
**Modified:**

API annotations in the IAgoraRtcEngine.h file.

API (2020-04-29)
==================================================
Refine LocalVideoStats and RemoteVideoStats

IAgoraRtcEngine.h
-------------
**Add:**
Add fields in LocalVideoStats
- encoderOutputFrameRate
- rendererOutputFrameRate
- targetFrameRate

Add fields in RemoteVideoStats
- decoderOutputFrameRate
- rendererOutputFrameRate
- packetLossRate
- totalFrozenTime
- frozenRate

**Deleted:**
Remove fields in RemoteVideoStats
- userId
- receivedFrameRate

API (2020-04-30)
==================================================
Modify comments of CHANNEL_PROFILE_TYPE default setting.

 AgoraBase.h
-------------
**Modified:**
Short description
- Changes the comment of CHANNEL_PROFILE_TYPE default setting from CHANNEL_PROFILE_COMMUNICATION to CHANNEL_PROFILE_LIVE_BROADCASTING.


API (2020-04-28)
==================================================
AgoraBase.h
-------------
**Add:**
  - Add enum CHANNEL_PROFILE_COMMUNICATION_1v1 for CHANNEL_PROFILE_TYPE

IAgoraRtcEngine.h
-------------
**Add:**
  - Add field channelProfile for RtcEngineContext


API (2020-04-27)
==================================================
AgoraRefPtr.h
**Add:**
Add interface HasOneRef


API (2020-04-22)
==================================================
AgoraBase.h
-------------
**Modified:**
Initialize member variable for RtcStats and RemoteAudioStats.


API (2020-04-20)
==================================================
AgoraBase.h
-------------
**Modified:**
Change default value of agora::rtc::VideoFormat from 0, 0, 0 to 640(width), 480(height), 15(fps)


API (2020-04-16)
==================================================
Fix a typo

AgoraMediaBase.h
-------------
**Modified:**
Change PLAY_ERROR_SRC_BUFFER_UNDERFLOW to PLAYER_ERROR_SRC_BUFFER_UNDERFLOW.


API (2020-04-17)
==================================================
Add Log Levels

AgoraBase.h
-------------
**Add:**
Add enum LOG_LEVEL for logging severities.


API (2020-04-13)
==================================================
Add internal states

AgoraMediaBase.h
-------------
**Modified:**
Add internal states for Media Player Source.


API (2020-04-10)
==================================================
Add field in struct LocalAudioStats

IAgoraRtcEngine.h
-------------
**Modified:**
Move LocalAudioStats to AgoraBase.h
add internalCodec in struct LocalAudioStats
- struct LocalAudioStats
- internalCodec


API (2020-04-10)
==================================================
Refine code comments

IAgoraRtcEngine.h
-------------
**Modified:**
Refine code comment for onRemoteAudioStats


API (2020-04-10)
==================================================

IAgoraRtcEngine.h
-------------
**Add:**
* IAgoraRtcEngine.h
- class IMetadataObserver
- class IRtcEngine
  - API registerMediaMetadataObserver()


API (2020-04-10)
==================================================

AgoraMediaBase.h
-------------
**Modified:**

- Refine the enum names of MEDIA_PLAYER_STATE and add some internal ones.

IAgoraMediaPlayer.h
-------------
**Modified:**

- Change one parameter's type of getStreamCount() and getStreamInfo() from int to int64_t to get aligned with the other APIs.

API (2020-04-10)
==================================================

AgoraBase.h
-------------
**Modified:**
  - Modified AUDIO_PROFILE_TYPE
  - Modified AUDIO_SCENARIO_TYPE

**Deleted:**
  - Deleted AUDIO_PROFILE_IOT
  - Deleted AUDIO_SCENARIO_IOT


API (2020-04-10)
==================================================

IAgoraRtcEngine.h
-------------
**Added:**
Support sending custom event to argus

- IRtcEngine::sendCustomReportMessage()


API (2020-03-27)
==================================================

IAgoraRtcEngine.h
-------------
**Modified:**
  - Modified the comment of onLastmileQuality
  - Modified the comment of startLastmileProbeTest

**Deleted:**
  - Deleted enableLastmileTest()
  - Deleted disableLastmileTest()


API (2020-03-18)
==================================================

AgoraBase.h
-------------
**Modified:**
- Replace FEATURE_ENABLE_UT with FEATURE_ENABLE_UT_SUPPORT


API (2020-03-16)
==================================================
add field in struct MediaStreamInfo

AgoraMediaBase.h
-------------
**Add:**
add audioBitsPerSample in struct MediaStreamInfo
- struct MediaStreamInfo
- audioBitsPerSample


API (2020-03-12)
==================================================

AgoraMediaBase.h
-------------

**Deleted:**
- Some comments in struct PacketOptions


API (2020-03-09)
==================================================

IAgoraRtcEngine.h
-------------
**Modified:**
  - updateChannelMediaOptions provider default connectionId param is agora::rtc::DEFAULT_CONNECTION_ID

API (2020-03-09)
==================================================

IAgoraRtcEngine.h
-------------
**Modified:**
  - Modified the comment of member availableBandwidth for struct LastmileProbeOneWayResult
  - Modified the comment of member expectedUplinkBitrate for struct LastmileProbeConfig



API (2020-03-05)
==================================================

AgoraMediaBase.h
-------------

**Deleted:**
- payload_type and ssrc fields in struct PacketOptions



API (2020-03-03)

AgoraBase.h
-------------

**Modified:**
- Change field name of sampleCount in EncodedAudioFrameInfo to samplesPerChannel


API (2020-02-26)


API (2020-02-27)
==================================================
Changes for warning

AgoraBase.h
-------------
**Modified:**
- Add warning WARN_CHANNEL_CONNECTION_UNRECOVERABLE;


API (2020-02-21)
==================================================

AgoraBase.h
-------------

**Add:**
- struct RemoteAudioStats
- LOCAL_AUDIO_STREAM_ERROR::LOCAL_AUDIO_STREAM_ERROR_DEVICE_BUSY
- LOCAL_AUDIO_STREAM_ERROR::LOCAL_AUDIO_STREAM_ERROR_RECORD_FAILURE
- RtcStats::txAudioBytes
- RtcStats::txVideoBytes
- RtcStats::rxAudioBytes
- RtcStats::rxVideoBytes

**Modified:**
- Changes LOCAL_AUDIO_STREAM_ERROR_ENCODE_FAILURE = 3 to LOCAL_AUDIO_STREAM_ERROR_ENCODE_FAILURE = 5
- Changes REMOTE_AUDIO_STREAM_STATE to REMOTE_AUDIO_STATE
- Changes REMOTE_AUDIO_STREAM_REASON to REMOTE_AUDIO_STATE_REASON

IAgoraRtcEngine.h
-------------
**Add:**
- onLocalAudioStats()
- onRemoteAudioStats()

**Modified:**
- Changes onFirstLocalAudioFrame to onFirstLocalAudioFramePublished
- Changes onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE) to onLocalAudioStateChanged(LOCAL_AUDIO_STREAM_STATE, LOCAL_AUDIO_STREAM_ERROR)
- Changes onRemoteAudioStateChanged(uid_t, REMOTE_AUDIO_STREAM_STATE) to onRemoteAudioStateChanged(uid_t, REMOTE_AUDIO_STATE, REMOTE_AUDIO_STATE_REASON, int)

**Deleted:**
- Deleted onRemoteAudioStats()
- Deleted onUserMuteAudio()
- Deleted onMicrophoneEnabled

API (2020-02-20)
==================================================
Support lastmile probe test

Add:
-------------
* IAgoraRtcEngine.h
  - class IRtcEngine
    - add API startLastmileProbeTest
	- add API stopLastmileProbeTest

API (2020-02-19)
=======================================

Modified:
------------------------

Modified API annotations in the following files:

* AgoraBase.h
* AgoraMediaBase.h
* IAgoraRtcEngine.h

API (2020-02-18)
==================================================

Modified:
-------------
* AgoraBase.h
  - struct `RtcStats`, rename field `cid` into `connectionId`

API (2020-02-15)
==================================================
API changes for unifying the definition of connection state and adding the API for getting connection state.

IAgoraRtcEngine.h
-------------
**Add:**
Add the API for getting the connection state.
- CONNECTION_STATE_TYPE IRtcEngine::getConnectionState(conn_id_t connectionId)

**Deleted:**
Delete the definition of enum CONNECTION_STATE_TYPE.
- Deleted enum CONNECTION_STATE_TYPE

AgoraBase.h
-------------
**Add:**
Add the definition for connection state.
- enum CONNECTION_STATE_TYPE

API (2020-02-15)
==================================================

Modified:
-------------
* IAgoraRtcEngine.h
  - Modified the comment of member publishMediaPlayerAudioTrack for struct ChannelMediaOptions

API (2020-02-12)
==================================================

Modified:
-------------
* AgoraBase.h
  - remove isScreenCapture and syncWithAudio in track info

API (2020-02-12)
==================================================

Modified:
-------------
* AgoraMediaBase.h
  - Add PLAY_ERROR_SRC_BUFFER_UNDERFLOW

API (2020-02-12)
==================================================

Add:
-------------
* IAgoraRtcEngine.h
  - AUDIO_REVERB_PRESET definition
  - VOICE_CHANGER_PRESET definition
  - class IRtcEngine
    - API setLocalVoiceReverbPreset()
	- API setLocalVoiceChanger()

API (2020-02-11)
==================================================

Add:
-------------
* IAgoraParameter.h
  - Add KEY_RTC_VIDEO_RESEND and KEY_RTC_AUDIO_RESEND


API (2020-02-13)
==================================================
Modified:
-------------
* AgoraRefPtr.h
  - Refine Agora shared pointer, re-struct code

API (2020-02-17)
==================================================

Add:
-------------
* AgoraBase.h
  - Add HEAAC2

API (2020-02-10)
==================================================

Add:
-------------
* IAgoraRtcEngine.h
 - class IRtcEngine
   - API setLogFileSize()


API (2020-02-06)
==================================================

Add:
-------------
* IAgoraRtcEngine.h
  - Add member publishMediaPlayerAudioTrack for struct ChannelMediaOptions

API (2020-02-05)

Add:
-------------
* AgoraRefPtr.h
  - Add member function reset() for class agora_refptr


API (2020-01-19)
==================================================

Modified:
-------------
* IAgoraRtcEngine.h AgoraBase.h
  - Move enum CHANNEL_PROFILE_TYPE to AgoraBase.h


API (2020-01-15)
==================================================

Add:
-------------
* IAgoraRtcEngine.h
  - class IRtcEngine
    - API startScreenCapture()


API (2020-01-13)
==================================================

Modified:
-------------
* AgoraMediaBase.h
  - Rename PLAYER_STATE_OPEN_COMPLETE of MEDIA_PLAYER_STATE to PLAYER_STATE_OPEN_COMPLETED


API (2020-01-09)
==================================================

Add:
-------------
* IAgoraMediaPlayer.h
  - class IMediaPlayer
    - API initialize()


API (2020-01-08)
==================================================

Add:
-------------
* IAgoraMediaPlayer.h
  - Add the following class
    - class IMediaPlayer
    - class IMediaPlayerObserver

* AgoraBase.h
  - Add the following class
    - class IVideoFrameObserver
    - class IAudioFrameObserver


API (2019-12-25)
==================================================

Modified:
-------------
* IAgoraRtcEngine.h
  - change API enableInEarMonitoring()


API (2019-12-15)
==================================================

Add:
-------------
* IAgoraRtcEngine.h
  - add API enableInEarMonitoring()

Modified:
-------------
* AgoraMediaBase.h
  - Delete unused class AudioFrame
  - Move ExternalVideoFrame from IAgoraMediaEngine.h
  - Move VIDEO_PIXEL_FORMAT definition out from class ExternalVideoFrame.
  - Move VideoFrame out from class IVideoFrameObserver.
* IAgoraMediaEngine.h
  - Move ExternalVideoFrame from this file to AgoraMediaBase.h
  - Delete the following class
    - class IVideoFrame
    - class IExternalVideoRenderCallback
    - class IExternalVideoRender
    - class IExternalVideoRenderFactory
  - Delete method registerVideoRenderFactory() in class IMediaEngine
* IAgoraRtcEngine.h
  - all platform supports setInEarMonitoringVolume()

Deleted:
-------------
* Delete class MEDIA_ENGINE_EVENT_CODE_TYPE in IAgoraRtcEngine.h
* Delete API setPlaybackDeviceVolume in interface IRtcEngine
* Delete API setVideoProfile in interface IRtcEngine
* Delete class RtcEngineParameters in IAgoraRtcEngine.h

API (2019-11-25)
==================================================

Add:
-------------


Modified:
-------------
* AgoraMediaBase.h
  - Delete unused class AudioFrame
  - Move ExternalVideoFrame from IAgoraMediaEngine.h
  - Move VIDEO_PIXEL_FORMAT definition out from class ExternalVideoFrame.
  - Move VideoFrame out from class IVideoFrameObserver.
* IAgoraMediaEngine
  - Move ExternalVideoFrame from this file to AgoraMediaBase.h
  - Delete the following class
    - class IVideoFrame
    - class IExternalVideoRenderCallback
    - class IExternalVideoRender
    - class IExternalVideoRenderFactory
  - Delete method registerVideoRenderFactory() in class IMediaEngine

Deleted:
-------------
* IAgoraLiveEngine.h
* IAgoraLivePublisher.h
* IAgoraLiveSubscriber.h
* IAgoraSignalingEngine.h
