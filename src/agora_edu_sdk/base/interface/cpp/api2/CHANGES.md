Note: Please update this file for every Agora API change you do. Simply fill in
your updates in the Working section below.


Agora Low Level APIs (Working)
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

API (2020-10-30)
==================================================
Refine API comments.

IAgoraService.h
NGIAgoraAudioTrack.h
NGIAgoraMediaNodeFactory.h
NGIAgoraRtcConnectiono.h
-------------
Refine comments.

API (2020-10-27)
==================================================
Add callback for encryption error

NGIAgoraRtcConnection.h
-------------
**Add:**
- IRtcConnectionObserver::onEncryptionError

API (2020-09-16)
==================================================
fix ios and mac marco

API file name #2
-------------

API (2020-09-16)
==================================================
Support SM4 Encryption.

NGIAgoraRtcConnection.h
-------------
**Add:**
- IRtcConnection::enableEncryption

API (2020-09-17)
==================================================
Add member fields to RemoteAudioTrackStats

NGIAgoraAudioTrack.h
-------------
**Add:**
- RemoteAudioTrackStats::expanded_speech_samples
- RemoteAudioTrackStats::expanded_noise_samples
- RemoteAudioTrackStats::timestamps_since_last_report
- RemoteAudioTrackStats::min_sequence_number
- RemoteAudioTrackStats::max_sequence_number

API (2020-08-28)
==================================================

Add v2.7.1 docs for high-level apis. For details, see jira MS-17241.
-------------
Merge docs for low-level apis from v2.7.0.2 to v2.7.1.

API (2020-09-14)
==================================================

NGIAgoraAudioDeviceManager.h
-------------
**Modified:**
- Modify change marco

API (2020-09-07)
==================================================

NGIAgoraAudioDeviceManager.h
-------------
**Deleted:**
- Delete INGAudioDeviceManager::startMicrophoneTest and INGAudioDeviceManager::stopMicrophoneTest
- Delete IAudioDeviceManagerObserver::onVolumeIndication


API (2020-09-02)
==================================================
Modify enum VIDEO_FRAME_TYPE

packet_i.h
-------------
**Modified:**
- Changes remove DROPPABLE_FRAME from VIDEO_FRAME_TYPE


API (2020-08-21)
==================================================
agora_service_i.h
-------------
**Modified:**
Remove dup 'appId'.

IAgoraRtmService.h
-------------
**Modified:**
Merge changes from arsenal.

IAgoraRtmService.h
-------------
**Modified:**
Modify macro definition

NGIAgoraAudioDeviceManager.h
NGIAgoraCameraCapturer.h
NGIAgoraScreenCapturer.h
-------------
**Modified:**
Refine macro defition and add comments for #endif.

NGIAgoraLocalUser.h
-------------
**Modified:**
Fix typo.

NGIAgoraMediaNodeFactory.h
-------------
**Modified:**
Fix return value of setProperty() and getProperty().


API (2020-09-04)
==================================================
Add new callback to IRtcConnectionObserver.

NGIAgoraRtcConnection.h
-------------
**Modified:**
- Changes add onNetworkTypeChanged to IRtcConnectionObserver


API (2020-08-13)
==================================================

IAgoraRtmService.h
-------------
**Modified:**
- Changes onSendMessageState(long long messageId, CHANNEL_MESSAGE_STATE state) to
 onSendMessageState(int64_t messageId, CHANNEL_MESSAGE_STATE state)


API (2020-08-21)
==================================================
NGIAgoraVideoTrack.h
-------------
**Modified:**
- Change addVideoFilter(filter) to addVideoFilter(filter, position)

API (2020-08-07)
==================================================
NGIAgoraMediaNodeFactory.h
-------------
**Add:**
- `IVideoFrameTransceiver`class
- createVideoFrameTransceiver function for IMediaNodeFactory

IAgoraService.h
-------------
**Add:**
- createTranscodedVideoTrack function for IAgoraService


API (2020-08-04)
==================================================
API change to support string UID.

IAgoraService.h
-------------
**Modified:**
Modify AgoraServiceConfiguration to include string uid switch
- bool useStringUid
**Add:**
Add Low level api to IAgoraService to support register user account
- IAgoraService::registerLocalUserAccount


API (2020-08-11)
==================================================
Fix return error

NGIAgoraMediaNodeFactory.h
NGIAgoraLocalUser.h
-------------
**Modified:**
- Modify setProperty and getProperty return type
- Fix spell error


API (2020-07-31)
==================================================

NGIAgoraAudioTrack.h
-------------
Add `mean_waiting_time` field for RemoteAudioTrackStats


API (2020-07-29)
==================================================

NGIAgoraAudioDeviceManager.h
-------------
Add `getPlayoutAudioParameters(AudioParameters* params) const` function for INGAudioDeviceManager
Add `getRecordAudioParameters(AudioParameters* params) const` function for INGAudioDeviceManager


API (2020-08-03)
==================================================

IAgoraService.h
**Add:**
- AgoraServiceConfiguration add logDir


API (2020-07-27)
==================================================

NGIAgoraAudioTrack.h
-------------
Remove free_pcm_data_list_size from ILocalAudioTrack::LocalAudioTrackStats


API (2020-07-17)
==================================================
Modify comment.

AgoraRefCountedObject.h
IAgoraRtmpStreamingService.h
IAgoraService.h
NGIAgoraAudioDeviceManager.h
NGIAgoraAudioTrack.h
NGIAgoraCameraCapturer.h
NGIAgoraExtensionControl.h
NGIAgoraLocalUser.h
NGIAgoraMediaNodeFactory.h
NGIAgoraRtcConnection.h
NGIAgoraScreenCapturer.h
NGIAgoraVideoFrame.h
NGIAgoraVideoMixerSource.h
NGIAgoraVideoTrack.h
-------------
**Modify:**
- Remove api2 prefix.


API (2020-07-16)
==================================================
add onConnecting

NGIAgoraRtcConnection.h
-------------
**Modified:**
add onConnecting to IRtcConnectionObserver


API (2020-07-13)
==================================================
NGIAgoraAudioTrack.h
-------------
**Add:**
- add "uint32_t effect_type" member to ILocalAudioTrack::LocalAudioTrackStats


API (2020-07-03)
==================================================
Modify comment.

NGIAgoraLocalUser.h
-------------
**Modify:**
Modify comment.
- Changes changes comment about pcmDataOnly.

NGIAgoraRtcConnection.h
-------------
**Modify:**
Modify AudioSubscriptionOptions and RtcConnectionConfiguration.
- Changes remove pcmDataOnly from AudioSubscriptionOptions.
- Changes add enableAudioRecordingOrPlayout into RtcConnectionConfiguration.


API (2020-07-01)
==================================================
IAgoraService.h
-------------
**Modify:**
Remove media engine type.


API (2020-06-27)
==================================================
Make include path correct


API (2020-06-18)
==================================================
Modify VideoEncodedImageData.

video_node_i.h
-------------
**Modify:**
Modify VideoEncodedImageData.
- Remove packetizationMode.


API (2020-06-18)
==================================================

Move Move IRecordingDeviceSource from NGIAgoraMediaNodeFactory.h
to NGIAgoraAudioDeviceManager.h


API (2020-06-17)
==================================================

agora_service_i.h
-------------
**Deleted:**
Delete unused api
- Deleted createLocalAudioTrack(const rtc::AudioOptions& audioOptions)


API (2020-06-16)
==================================================
IAgoraRtmService.h
-------------
**Modify:**
Fix compile error
- Remove the incorrect type conversion code in class IChannelEventHandler


API (2020-06-15)
==================================================

IAgoraService.h
-------------
**Add:**
- Add createRecordingDeviceAudioTrack() for IAgoraService.

NGIAgoraMediaNodeFactory.h
-------------
**Add:**
- Add createRecordingDeviceSource() for IMediaNodeFactory.
- Add interface IRecordingDeviceSource



API (2020-06-10)
==================================================

NGIAgoraVideoFrame.h
-------------
**Modify:**
Fix compile error
- unnamed class used in typedef name won't compile in C++20


API (2020-06-09)
==================================================
Modify onMediaPacketReceived.

NGIAgoraMediaNodeFactory.h
-------------
**Modified:**
Modify the declaration of onMediaPacketReceived.
- Changes onMediaPacketReceived(const uint8_t *packet, size_t length)
  to      onMediaPacketReceived(const uint8_t *packet, size_t length, const agora::media::base::PacketOptions& options)

API (2020-06-04)
==================================================
Modify the callback onBandwidthEstimationUpdated

NGIAgoraRtcConnection.h
-------------
**Modified:**
Modify the declaration of onBandwidthEstimationUpdated
- Changes "virtual void onBandwidthEstimationUpdated(int target_bitrate_bps)" to "virtual void onBandwidthEstimationUpdated(const NetworkInfo& info)"


API (2020-06-04)
==================================================
Add SenderOptions and modify createCustomVideoTrack

IAgoraService.h
-------------
**Add:**
Add SenderOptions
- struct SenderOptions

**Modified:**
Change the declaration of createCustomVideoTrack
- Changes "virtual agora_refptr<rtc::ILocalVideoTrack> createCustomVideoTrack(
               agora_refptr<rtc::IVideoEncodedImageSender> videoSource, bool syncWithAudioTrack = false, TCcMode ccMode = CC_ENABLED)"
  To "virtual agora_refptr<rtc::ILocalVideoTrack> createCustomVideoTrack(
          agora_refptr<rtc::IVideoEncodedImageSender> videoSource, SenderOptions& options)"

- Remove option syncWithAudioTrack  from all related apis


API (2020-05-29)
==================================================
Updated docs for the following header files:
- NGIAgoraVideoTrack.h
- NGIAgoraRtcConnection.h
- NGIAgoraMediaNodeFactory.h
- NGIAgoraLocalUser.h
- NGIAgoraAudioTrack.h
- IAgoraService.h


API (2020-05-27)
==================================================

NGIAgoraMediaNodeFactory.h
-------------
**Modify:**
- Add a parameter for createMediaPlayerSource()


API (2020-05-26)
==================================================
NGIAgoraAudioTrack.h
-------------
**Modified:**
- Init RemoteAudioTrackStats


API (2020-05-26)
==================================================
Move createObservableVideoSink() from IMediaNodeFactory to IMediaNodeFactoryEx

NGIAgoraMediaNodeFactory.h
-------------
**Deleted:**
- Delete createObservableVideoSink()


API (2020-05-20)
==================================================
NGIAgoraMediaPlayerSource.h
-------------
**Modified:**
- Rename registerPlayerObserver()/unregisterPlayerObserver() to registerPlayerSourceObserver()/unregisterPlayerSourceObserver() in IMediaPlayerSource.
- Remove 'const' from the parameter of onPlayerSourceStateChanged(), onPositionChanged() and onPlayerEvent() in IMediaPlayerSourceObserver since unnecessary.


API (2020-05-20)
==================================================
NGIAgoraVideoTrack.h
-------------
**Modified:**
- Change addRenderer(renderer) to addRenderer(renderer, position)


API (2020-05-19)
==================================================
NGIAgoraMediaNodeFactory.h
-------------
**Add:**
- bool isExternal()

NGIAgoraVideoTrack.h
-------------
**Modified:**
- remove "bool internal" parameter form funciton addVideoFilter


API (2020-05-19)
==================================================
NGIAgoraCameraCapturer.h
-------------
**Deleted:**
- Delete ICameraCaptureObserver interface
- Delete ICameraCaptureObserver::registerCameraCaptureObserver
- Delete ICameraCaptureObserver::unregisterCameraCaptureObserver
- Delete ICameraCaptureObserver::getCaptureState

NGIAgoraLocalUser.h
-------------
**Modified:**
- Annotation of ILocalUserObserver::onLocalVideoTrackStateChanged


API (2020-05-12)
==================================================
Remove illegal header files included and add including header files.

NGIAgoraLocalUser.h
-------------
**Deleted:**
Delete the IAgoraRtcEngine.h included
- Deleted #include "IAgoraRtcEngine.h"

NGIAgoraMediaNodeFactory.h
-------------
**Deleted:**
Delete the IAgoraRtcEngine.h included
- Deleted #include "IAgoraRtcEngine.h"

internal/agora_service_i.h
-------------
**Add:**
Add the IAgoraLog.h including
- #include "base/IAgoraLog.h"


API (2020-05-09)
=========================

IAgoraService.h
--------------------
**Add:**

Add AUDIO_SCENARIO_TYPE audioScenario for AgoraServiceConfiguration


API (2020-05-08)
==================================================
NGIAgoraLocalUser.h
NGIAgoraMediaNodeFactory.h
NGIAgoraMediaPlayerSource.h
internal/media_node_factory_i.h
internal/rtc_connection_i.h
-------------
**Modified:**

- Change namespace


API (2020-05-08)
==================================================
NGIAgoraExtensionControl.h
-------------
**Modified:**
  - Move LOG_LEVEL from AgoraBase.h to IAgoraLog.h and change namespace to commons

NGIAgoraMediaNodeFactory.h
-------------
**Modify:**
- Remove parameter |view| in createVideoRenderer



- Change namespace

API (2020-05-07)
==================================================
AgoraOptional.h
-------------
**Modified:**
- Changes optional.h to AgoraOptional.h

API (2020-04-29)
==================================================
Refine RemoteVideoTrackStats

NGIAgoraVideoTrack.h
-------------
**Modified:**
Rename fields
- streamType to rxStreamType

**Add:**
Add fields
- decoderOutputFrameRate
- rendererOutputFrameRate
- packetLossRate
- totalFrozenTime
- frozenRate

API (2020-04-26)
==================================================
NGIAgoraCameraCapturer.h
-------------
**Deleted:**
- ICameraCapturer::CAPTURE_STATE::CAPTURE_STATE_STOPPING
- ICameraCapturer::CAPTURE_STATE::CAPTURE_STATE_STARTING
- switchCamera()

API (2020-04-20)
==================================================
NGIAgoraMediaNodeFactory.h
-------------
**Add:**
- IVideoSinkBase::onDataStreamWillStart()
- IVideoSinkBase::onDataStreamWillStop()
- IVideoFilter::onDataStreamWillStart()
- IVideoFilter::onDataStreamWillStop()


API (2020-04-17)
==================================================
NGIAgoraExtensionControl.h
-------------
**Add:**
Add IExtensionControl interfaces for agora extensions
- getCapabilities
- registerExtensionProvider
- unregisterExtensionProvider
- createVideoFrame
- copyVideoFrame
- recycleVideoCache
- dumpVideoFrame
- log

NGIAgoraExtension.h
-------------
**Deleted:**
file deleted

NGIAgoraMediaNodeProvider.h
-------------
**Deleted:**
file deleted

NGIAgoraMediaNodeFactory
-------------
**Deleted:**
- registerMediaNodeProvider
- unregisterMediaNodeProvider


API (2020-04-16)
==================================================
NGIAgoraVideoFrame.h
**Add:**
Add IVideoFrame interfaces for external video frames
- type
- format
- width
- height
- size
- rotation
- setRotation
- timestampUs
- setTimestampUs
- data
- mutableData
- resize
- textureId
- fill


API (2020-04-13)
==================================================
NGIAgoraMediaPlayerSource.h
-------------
**Modified:**

- Refine the API order of IMediaPlayerSource().


API (2020-04-12)
Add send intra request api.
==================================================
NGIAgoraLocalUser.h
-------------
**Add:**
Add send intra request api.
- sendIntraRequest()


API (2020-04-11)
==================================================
NGIAgoraVideoTrack.h
-------------
**Modified:**
- IVideoTrack::addVideoFilter()

API (2020-04-10)
==================================================
NGIAgoraRtcConnection.h
-------------
**Added:**
Support sending custom event to argus
- IRtcConnection::sendCustomReportMessage().

IAgoraRtmService.h
-------------
**Added:**
Add parameter 'eventSpace', used during report RTM events
- IAgoraRtmService::initialize(const char *appId, IRtmServiceEventHandler *eventHandler, uint64_t eventSpace).


API (2020-04-10)
==================================================
Add field in struct LocalAudioTrackStats

NGIAgoraAudioTrack.h
-------------
*Deleted:**
delete struct LocalAudioTrackStats, use LocalAudioStats instead
- struct LocalAudioTrackStats


API (2020-04-10)
==================================================
NGIAgoraMediaPlayerSource.h
-------------
**Modified:**
- Change one parameter's type of getStreamCount() and getStreamInfo() from int to int64_t to get aligned with the other APIs.


API (2020-04-09)
==================================================
NGIAgoraVideoMixerSource.h
-------------
**Add:**
- Video Mixer related data structures and interfaces (POC)

IAgoraService.h
-------------
**Add:**
- createMixedVideoTrack (POC)

NGIAgoraMediaNodeFactory.h
-------------
**Add:**
- createVideoMixer (POC)


API (2020-04-03)
==================================================
NGIAgoraCameraCapturer.h
-------------
**Modified:**
Add __APPLE__ macro judge.

API (2020-04-03)
==================================================
Add intra request callback and network observer.

NGIAgoraLocalUser.h
-------------
**Add:**
Add intra request callback.
- ILocalUserObserver::onIntraRequestReceived()

NGIAgoraRtcConnection.h
-------------
**Add:**
Add network observer and its register/unregister methods.
- class INetworkObserver
- IRtcConnection::registerNetworkObserver()
- IRtcConnection::unregisterNetworkObserver()

NGIAgoraCameraCapturer.h
-------------
**Add:**
Add destructor of ICameraCaptureObserver
- ICameraCaptureObserver::~ICameraCaptureObserver()

NGIAgoraMediaNodeFactory.h
-------------
**Modified:**
Modify the implement of BeautyOptions constructor
- BeautyOptions::BeautyOptions()


API (2020-04-2)
==================================================
NGIAgoraLocalUser.h
-------------
*Add:**
 elapsed in onUserVideoTrackStateChanged onVideoTrackPublishSuccess

API (2020-03-31)
==================================================
Move appId from AgoraServiceConfigurationEx to AgoraServiceConfiguration

IAgoraService.h
-------------
*Add:**
struct AgoraServiceConfiguration {
  const char* appId = nullptr;
}


API (2020-03-30)
==================================================
NGIAgoraCameraCapturer.h
-------------
**Modified:**
Refine the API order of IDeviceInfo.


API (2020-03-30)
==================================================
IAgoraService.h
-------------
**Modified:**
Remove enabled parameter from IAgoraService::createLocalAudioTrack()


API (2020-03-27)
==================================================
NGIAgoraRtcConnection.h
-------------
**Modified:**
  - Modified the comment of onLastmileQuality
  - Modified the comment of startLastmileProbeTest

**Deleted:**
  - Deleted enableLastmileTest()
  - Deleted disableLastmileTest()


API (2020-03-16)
==================================================
NGIAgoraAudioTrack.h
-------------
**Add:**
Add isEnabled to ILocalAudioTrack to get the local audio track enabled status
- ILocalAudioTrack::isEnabled()


API (2020-03-16)
==================================================
NGIAgoraRtcConnection.h
-------------
**Add:**
- enum RECV_TYPE


API (2020-03-12)
==================================================
Add 'received_bytes' for 'RemoteAudioTrackStats'

NGIAgoraAudioTrack.h
-------------
**Add:**
- RemoteAudioTrackStats::received_bytes


API (2020-03-06)
==================================================
Add 'virtual' for ~IAgoraService() and ~IAgoraServiceEx()

IAgoraService.h
-------------
**Modified:**
- Add 'virtual' for ~IAgoraService()

agora_service_i.h
-------------
**Modified:**
- Add 'virtual' for ~IAgoraServiceEx()


API (2020-02-27)
==================================================
new cc type

AgoraBase.h
-------------
**Modified:**
- Add new cc type CONGESTION_CONTROLLER_TYPE_AUT_CC;


API (2020-02-25)
==================================================
Changes for AudioSessionConfiguration

IAgoraService.h
-------------
**Modified:**
- Change Optional<int> ioBufferDuration; to Optional<double> ioBufferDuration;


API (2020-02-23)
==================================================
API changes for supporting data channel

NGIAgoraRtcConnection.h
-------------
**Add:**
Add data channel api to IRtcConnection
- createDataStream()
- sendStreamMessage()

Add data channel callback api to IRtcConnectionObserver
- onStreamMessage()
- onStreamMessageError()


API (2020-02-21)
==================================================
Changes for audio video stats and state

NGIAgoraAudioTrack.h
-------------
**Add:**
Add two structs of audio track stats.
- add struct RemoteAudioTrackStats
- add struct LocalAudioTrackStats

**Modified:**
- Changes getStatistics(RemoteAudioStats& stats) to getStatistics(RemoteAudioTrackStats& stats)
- Changes REMOTE_AUDIO_STATE getState() to REMOTE_AUDIO_STATE getState()

NGIAgoraLocalUser.h
-------------
**Add:**
Add callback functions of audio track statistics.
- onLocalAudioTrackStatistics()
- onRemoteAudioTrackStatistics()

**Modified:**
Modify the type of field codec_name
- Changes std::string codec_name to char codec_name[media::kMaxCodecNameLength]
- Changes onUserAudioTrackStateChanged(user_id_t, agora_refptr<rtc::IRemoteAudioTrack>, REMOTE_AUDIO_STREAM_STATE, REMOTE_AUDIO_STREAM_REASON)
to onUserAudioTrackStateChanged(user_id_t, REMOTE_AUDIO_STATE, REMOTE_AUDIO_STATE_REASON, int) = 0;

NGIAgoraAudioDeviceManager.h
-------------
**Deleted:**
- Deleted onMicrophoneEnabled()


API (2020-02-20)
==================================================
Support lastmile probe test

NGIAgoraRtcConnection.h
-------------
**Add:**
Add callback for getting lastmile probe test results and two functions to control lastmile probe test
- void onLastmileProbeResult(const LastmileProbeResult& result)
- int startLastmileProbeTest(const LastmileProbeConfig& config)
- int stopLastmileProbeTest()


API (2020-02-10)
==================================================
API changes for create video track parameter

IAgoraService.h
-------------
**Modified:**

  - createCameraVideoTrack(), createScreenVideoTrack(), createCustomVideoTrack()
  remove `bool enable`


API (2020-02-15)
==================================================
API changes for unifying the definition of connection state in AgoraBase.h

NGIAgoraRtcConnection.h
-------------
**Deleted:**
Delete the definition of TConnectionState.
* Deleted enum TConnectionState


API (2020-02-06)
==================================================
API changes for media packet and media control packet sender and receiver

IAgoraService.h
--------------------------
**Add:**

Add two functions to create media tracks with IMediaPacketSender, which is created from media node factory.
- agora_refptr<rtc::ILocalAudioTrack> createCustomAudioTrack(agora_refptr<rtc::IMediaPacketSender> source)
- agora_refptr<rtc::ILocalVideoTrack> createCustomVideoTrack(agora_refptr<rtc::IMediaPacketSender> source)

NGIAgoraAudioTrack.h
--------------------------
**Add:**
Add two functions to register and unregister media packet receiver.

- int registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver)
- int unregisterMediaPacketReceiver(IMediaPacketReceiver* packetReceiver)

NGIAgoraLocalUser.h
--------------------------
**Add:**
Add function to get IMediaControlPacketSender.
Add two functions to register and unregister media control packet receiver.

- IMediaControlPacketSender* getMediaControlPacketSender()
- int registerMediaControlPacketReceiver(IMediaControlPacketReceiver* ctrlPacketReceiver)
- int unregisterMediaControlPacketReceiver(IMediaControlPacketReceiver* ctrlPacketReceiver)

NGIAgoraVideoTrack.h
--------------------------
**Add:**
Add two functions to register and unregister media packet receiver.

- int registerMediaPacketReceiver(IMediaPacketReceiver* packetReceiver)
- int unregisterMediaPacketReceiver(IMediaPacketReceiver* packetReceiver)

NGIAgoraMediaNodeFactory.h
--------------------------
**Add:**
Add function to create media packet sender as well as related definitions

- agora_refptr<IMediaPacketSender> createMediaPacketSender()
- IMediaPacketSender
- IMediaControlPacketSender
- IMediaPacketReceiver
- IMediaControlPacketReceiver


API (2020-01-21)
==================================================
API changes for getting audio filter.

Audio Filter (api2/NGIAgoraMediaNodeFactory.h)
--------------------------

**Add:**

Main methods:

- const char * getName() const

Local Audio Track (api2/NGIAgoraAudioTrack.h)
--------------------------

**Add:**

Main methods:

- agora_refptr<IAudioFilter> getAudioFilter(const char *name) const


API (2020-02-10)
==================================================
Deleted:
-------------
* IAgoraService.h
  - class IAgoraService
    -API createSignalingEngine()


API (2020-01-20)
==================================================
Add:
-------------
* NGIAgoraLocalUser.h
  - class ILocalUser
    - API adjustPlaybackSignalVolume()
    - API getPlaybackSignalVolume()


API (2020-01-19)
==================================================
Modified:
-------------
* NGIAgoraRtcConnection.h
  - struct RtcConnectionConfiguration
    add CHANNEL_PROFILE_TYPE  channelProfile


API (2020-01-17)
==================================================
Modified:
-------------
* IAgoraService.h
  - createCameraVideoTrack(), createScreenVideoTrack(), createCustomVideoTrack()
  and createCustomVideoTrack() change argument to `bool enable = false`


API (2020-01-15)
==================================================
Add:
-------------
* NGIAgoraScreenCapturer.h
  - class IScreenCapturer
    - API initWithMediaProjectionPermissionResultData()


API (2020-01-13)
==================================================
Modified:
-------------
* NGIAgoraMediaPlayerSource.h
  - Rename onPlayerStateChanged() of IMediaPlayerSourceObserver to onPlayerSourceStateChanged


API (2020-01-09)
==================================================
Deleted:
-------------
* NGIAgoraMediaPlayerSource.h
  - class IMediaPlayerSource
    - API registerVideoFrameObserver()
    - API unregisterVideoFrameObserver()


API (2020-01-08)
==================================================
Add:
-------------
* NGIAgoraMediaPlayerSource.h
  - Add the following class
    - class IMediaPlayerSource
    - class IMediaPlayerSourceObserver

* NGIAgoraMediaNodeFactory.h
  - class IMediaNodeFactory
    - Add API createMediaPlayerSource()

* IAgoraService.h
  - class IAgoraService
    - Add API createMediaPlayerVideoTrack()
    - Add API createMediaPlayerAudioTrack()


Deleted:
-------------
* NGIAgoraMediaPlayer.h
  - Delete class IMediaPlayer
  - Delete class IMediaPlayerObserver

* IAgoraService.h
  - class IAgoraService
    - Delete API createMediaPlayer()


API (2019-12-24)
==================================================
Macro cleanup

IAgoraService (api2/IAgoraService.h)
 - remove FEATURE_RTMP_STREAMING_SERVICE and FEATURE_RTM_SERVICE


API (2019-12-22)
==================================================
API changes

Add:
-------------

Modified:
-------------
* AgoraBase.h
  - rename RemoteVideoTrackInfo to VideoTrackInfo
  - relevant changes in other interfaces

* IAgoraRtcEngine.h
  - struct VideoCanvas add field isScreenView

* IAgoraService.h
  - createCameraVideoTrack(), createScreenVideoTrack(), createCustomVideoTrack()
  and createCustomVideoTrack() add argument `bool enable = true`

* NGIAgoraLocalUser.h
  - rename RemoteVideoTrackInfo to VideoTrackInfo

* NGIAgoraMediaNodeFactory.h
  - rename RemoteVideoTrackInfo to VideoTrackInfo

* NGIAgoraVideoTrack.h
  - rename RemoteVideoTrackInfo to VideoTrackInfo


API (2019-12-03)
==================================================
API changes for graceful exiting

IAgoraService (api2/IAgoraService.h)
 - add release()


API (2019-12-10)
==================================================
Local Audio Track (api2/NGIAgoraAudioTrack.h)
--------------------------

**Add:**

Main methods:

- int enableEarMonitor(bool enable, bool includeAudioFilter)

API (2019-12-04)
==================================================
API changes for supporting create RTMP streaming serivce.

Add:
-------------
* IAgoraRtmpStreamingService.h
* IAgoraRtmService.h

Modified:
-------------
* IAgoraService.h
  - Add method `agora_refptr<rtc::IRtmpStreamingService> createRtmpStreamingService`
  - Add method `rtm::IRtmService* createRtmService()`


API (2019-11-29)
==================================================
API changes for supporting audio filter.

Audio Filter (api2/NGIAgoraMediaNodeFactory.h)
--------------------------

Base class of custom audio filter, application can implement its own
filter and add it to audio track.

New interface and enum definition:

```
class IAudioFilterBase : public RefCountInterface {
public:
 // Return false if filter decide to drop the frame.
 virtual bool adaptAudioFrame(const AudioPcmFrame& inAudioFrame,
                              AudioPcmFrame& adaptedFrame) = 0;
};

class IAudioFilter : public IAudioFilterBase {
 public:
  virtual void setEnabled(bool enable) = 0;
  virtual bool isEnabled() = 0;
};

enum AUDIO_FILTER_TYPE {
  AUDIO_FILTER_EFFECT,
  AUDIO_FILTER_ANS,
  AUDIO_FILTER_VOICE_CHANGE
};
```


Media Node Factory (api2/NGIAgoraMediaNodeFactory.h)
--------------------------

**Add:**

Main methods:

- agora_refptr<IAudioFilter> createAudioFilter(AUDIO_FILTER_TYPE type)


Local Audio Track (api2/NGIAgoraAudioTrack.h)
--------------------------

**Add:**

Internal enum:

```
  enum AudioFilterPosition {
    Default
  };
```

Main methods:

- bool addAudioFilter(agora_refptr<IAudioFilter> filter, AudioFilterPosition position)
- bool removeAudioFilter(agora_refptr<IAudioFilter> filter, AudioFilterPosition position)


API (2019-11-28)
==================================================
API cleanup

Modified:
-------------
* NGIAgoraMediaNodeFactory
  - modify class IVideoFrameSender, use agora ExternalVideoFrame instead of webrtc VideoFrame
  - modify class IVideoFilterBase
    - use agora VideoFrame instead of webrtc VideoFrame
    - remove onSinkWantsChanged
  - modify class IVideoSinkBase
    - Add a interface to distinguish external or internal video sink
  - move class IVideoFrameAdapter to internal interface
  - move createVideoFrameAdapter() to internal interface

* NGIAgoraAudioTrack.h
  - IAudioTrack
    - virtual int adjustPlayoutVolume(int volume) = 0;
    - virtual int getPlayoutVolume(int* volume) = 0;
  - ILocalAudioTrack
    - virtual int adjustPublishVolume(int volume) = 0;
    - virtual int getPublishVolume(int* volume) = 0;


API (2019-11-14)
==================================================
Refine Media Statistics and event report

Add:
-------------

Modified:
-------------
* NGIAgoraAudioTrack.h
  - Add struct LocalAudioTrackStats
  - Add method GetStats()

* NGIAgoraLocalUser.h
  - rename LocalAudioStats to LocalAudioDetailedStats, since LocalAudioStats belong to high level API

Deleted:
-------------
* AgoraBase.h
  - remove AUDIO_CODEC_AAC in AUDIO_CODEC_TYPE


API (2019-11-25)
==================================================
API changes for extension

 - move `interface/cpp/api2/IAgoraParameter.h` to `interface/cpp/IAgoraParameter.h`


API (2019-11-21)
==================================================
API changes for extension

 - move `interface/cpp/AgoraParameter.h` to `interface/cpp/api2/IAgoraParameter.h`


API (2019-11-19)
==================================================
API changes.

Audio Frame Observer (IAgoraMediaEngine.h)
--------------------------

**Modified:**

Main field(s)

| Old  | New |
|---|---|
|`IAudioFrameObserver::AudioFrame::samples`  |  `IAudioFrameObserver::AudioFrame::samplesPerChannel` |


API (2019-11-12)
==================================================
API changes for extension

Agora Extension (interface/cpp/api2/NGIAgoraExtension.h)
-------------
 - struct `agora_extension`
 - struct `agora_core`


API (2019-11-11)
==================================================
API changes for supporting audio local playback.

Local Audio Track (api2/NGIAgoraAudioTrack.h)
--------------------------

**Add:**

Main methods:

- void enableLocalPlayback(bool enable)
- int adjustPlayoutVolume(int volume)
- int adjustPublishVolume(int volume)


API (2019-11-08)
==================================================
API changes for default client role is audience.

Rtc Engine (interface/cpp/IAgoraRtcEngine.h)
-------------
 - struct `ChannelMediaOptions` add field `clientRoleType`.

Rtc Connection (interface/cpp/api2/NGIAgoraRtcConnection.h)
-------------
 - struct `RtcConnectionConfiguration` add field `clientRoleType`.

Local User (api2/NGIAgoraLocalUser.h)
-------------

**Add:**

Main methods:

 - CLIENT_ROLE_TYPE getUserRole();


API (2019-11-05)
==================================================
API changes for supporting register/unregister
encoded image receiver.

**Deleted:**

move IVideoEncodedImageReceiver to AgoraBase.h


API (2019-11-04)
==================================================
Media Player (api2/NGIAgoraMediaPlayer.h)
-------------

**Add:**

Main methods:

 - int setLooping(bool looping)


**Deleted:**

Main methods:

 - int close()


API (2019-10-30)
==================================================
API changes for supporting audio simple audio player.

Media Player (api2/NGIAgoraMediaPlayer.h)
-------------

This interface provide access to a media player.

Main methods are:

 - int open(const char* url)
 - int play()
 - int playFromPosition(int position)
 - int stop()
 - int pause()
 - int resume()
 - int getDuration(int& duration)

 - int getCurrentPosition(int& currentPosition)
 - int setCurrentPosition(int newPos)
 - int close()


 - int registerPlayerObserver(IMediaPlayerObserver* observer)
 - int unregisterPlayerObserver(IMediaPlayerObserver* observer)
 - agora_refptr<rtc::ILocalAudioTrack> getPlayerAudioTrack()
 - agora_refptr<rtc::ILocalVideoTrack> getPlayerVideoTrack()

The denifinion of IMediaPlayerObserver:
```
class IMediaPlayerObserver {
public:
  virtual ~IMediaPlayerObserver() = default;
  virtual void onPlayerStateChanged(const IMediaPlayer::PlayerState& state) = 0;
  virtual void onPositionChanged(const int position) = 0;
};
```


API (2019-10-17)
==================================================
API changes for supporting audio tx mixer.

Local User (api2/NGIAgoraLocalUser.h)
--------------------------

**Add:**

Internal struct

- struct ANAStats;
- struct AudioProcessingStats;
- struct LocalAudioStats;

Main methods:

- int setAudioEncoderConfiguration(const rtc::AudioEncoderConfiguration& config)
- bool getLocalAudioStatistics(LocalAudioStats& stats)

**Modified:**

Main methods

| Old  | New |
|---|---|
|`int setPlaybackAudioFrameParameters(size_t bytesPerSample, size_t numberOfChannels, uint32_t sampleRateHz)`  |  `int setPlaybackAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz)` |
|`int setRecordingAudioFrameParameters(size_t bytesPerSample, size_t numberOfChannels, uint32_t sampleRateHz)`  |  `int setPlaybackAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz)` |
|`int setMixedAudioFrameParameters(size_t bytesPerSample, size_t numberOfChannels, uint32_t sampleRateHz)`  |  `int setPlaybackAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz)` |
|`int setPlaybackAudioFrameBeforeMixingParameters(size_t bytesPerSample, size_t numberOfChannels, uint32_t sampleRateHz)`  |  `int setPlaybackAudioFrameParameters(size_t numberOfChannels, uint32_t sampleRateHz)` |


agora_refptr (AgoraRefPtr.h)
--------------------------

**Add:**

Main methods

- bool operator==(const agora_refptr<T>& r)
- bool operator<(const agora_refptr<T>& r) const


Audio PCM Data (api2/internal/audio_node_i.h)
--------------------------

**Modified:**

Main field(s)

| Old  | New |
|---|---|
| number_of_samples  | samples_per_channel |


Local Audio Track (api2/NGIAgoraAudioTrack.h)
--------------

**Deleted:**

Internal struct

- struct ANAStats;
- struct AudioProcessingStats;
- struct LocalAudioStats;

Main methods:

- int setAudioEncoderConfiguration(const rtc::AudioEncoderConfiguration& config)
- bool getStatistics(LocalAudioStats& stats)


API (2019-10-13 recorded frome release/2.6.0)
==================================================
The full details of API descritpion can be seen in https://confluence.agoralab.co/display/MS/API+Introduction


Agora Low Level APIs
============================
Agora Service
-------------
Agora Communications as a Service (CaaS) provides ensured quality of experience (QoE) for worldwide Internet-based voice and video communications through a virtual global network that is especially optimized for real-time web and mobile-to-mobile applications.

The IAgoraService is the entry point of Agora Low Level APIs. This interface is used by applications to create access points to Agora Modules to enable real-time communication, including RTC Connection, Media Tracks, Audio Device Manager and etc. In addition, in order to customize different user scenarios, the interface allows application to configure service and media sessions on the global level, i.e., AgoraServiceConfiguration or AudioSessionConfiguration.

Main methods are:

- initialize(appId, serviceConfiguration)
- createRtcConnection()
- createLocalAudioTrack()
- createAudioDeviceManager()
- createMediaNodeFactory()
- createLocalVideoTrack()
- setAudioSessionConfiguration()
- getAudioSessionConfiguration()
- setLogFile()
- setLogFilter()
- release()

RTC Connection
-------------
This allows an application to establish a connection to a Agora Channel. In order to establish a connection, application must provide an App ID or Token and the channel identifier you want to join.

Connecting to a Agora Channel is done asynchronous, application can listen connection state by provided RTC Connection Observer. With connected to a channel, application can use [Local User](##Local User) to publish and subscribe media stream/data from Agora Channel.

RTC connection also monitors remote users in the channel. Once a remote user joined or left a channel application will be notified.

Main methods are:

- connect()
- disconnect()
- getLocalUser()
- getConnectionInfo()
- getRemoteUsers()
- getUserInfo()
- release()

- IConnectionObserver::onConnected()
- IConnectionObserver::onDisconnected()
- IConnectionObserver::onConnecting()
- IConnectionObserver::onConnectionFailure()
- IConnectionObserver::onUserJoined()
- IConnectionObserver::onUserleft()

Local User (get from RtcConnection)
-------------
This interface represents a combination of Publisher and Subscriber. Each Rtc Connection has its own Local User, and application can get the Local User by IRtcConnection::getLocalUser()

The Local User has two clients roles, broadcaster(Publisher with or w/o Subscriber) and audience(Subscriber only). The Publisher publish local Audio and Video tracks to a channel, like a host in LIVE BROADCAST profile or a participant in COMMUNICATION profile.
The Subscriber subscribes and receives remote Audio and Video tracks from different users in the channel.

Main methods are:

- publishAudio()
- unpublishAudio()
- publishVideo()
- unpublishVideo()
- subscribeAudio()
- subscribeAllAudio()
- unsubscribeAudio()
- unsubscribeAllAudio()
- subscribeVideo()
- subscribeAllVideo()
- unsubscribeVideo()
- unsubscribeAllVideo()
- registerLocalUserObserver()
- unregisterLocalUserObserver()
- release()

- ILocaUserlObserver::onAudioTrackPublishSuccess()
- ILocaUserlObserver::onAudioTrackPublicationFailure()
- ILocaUserlObserver::onVideoTrackPublishSuccess()
- ILocaUserlObserver::onVideoTrackPublicationFailure()
- ILocaUserlObserver::onUserAudioTrackSubscribed()
- ILocaUserlObserver::onUserAudioTrackUnsubscribed()
- ILocaUserlObserver::onUserAudioTrackStateChanged()
- ILocaUserlObserver::onUserAudioTrackStatistics()
- ILocaUserlObserver::onUserAudioTrackSubscriptionFailure()
- ILocaUserlObserver::onUserVideoTrackSubscribed()
- ILocaUserlObserver::onUserVideoTrackUnsubscribed()
- ILocaUserlObserver::onUserVideoTrackStateChanged()
- ILocaUserlObserver::onUserVideoTrackStatistics()
- ILocaUserlObserver::onUserVideoTrackSubscriptionFailure()

- ILocaUserlObserver::onUserNetworkQuality()
- ILocaUserlObserver::onAudioVolumeIndication()


Local Audio Track
--------------
A presentation of a PCM stream. A Local Audio Track can be created directly by IAgoraService::createLocalTrack(), which originates from default audio recording device, i.e., built-in microphone. Application can also change default audio recording devices by IAudioDeviceManager APIs if multiple recording devices available in system. Local Audio Track can also be created from IMediaPlayer().

After local tracks are created, Application can publish one or multiple local audio tracks through ILocalUser::PublishAudio(). By natual, all published tracks will be mixed first, then encoded into a single audio stream and send to Agora Channel.

Main methods are:
- setEnabled()
- isEnabled()
- adjustSignalingVolume()
- getSignalingVolume()
- release()

Remote Audio Track
---------
A presentation of a remote audio track.

Main methods are:
- setPlaybackEnabled()
- isPlaybackEnabled()
- getState()

Audio Device Manager
-------------
The interface provides access to connected audio recording devices like microphones and audio playout devices like speakers.

Main methods are:

- setMicrophoneVolume()
- getMicrophoneVolume()
- setSpeakerVolume()
- getSpeakerVolume()
- setMicrophoneMute()
- getMicrophoneMute()
- setSpeakerMute()
- getSpeakerMute()

- getNumberOfPlayoutDevices()
- getNumberOfRecordingDevices()
- getPlayoutDeviceInfo()
- getRecordingDeviceInfo()
- setPlayoutDevice()
- setRecordingDevice()

- startMicrophoneTest()
- stopMicrophoneTest()

- registerObserver()
- unregisterObserver()

- onVolumeIndication()
- onDeviceStateChanged()
- onRoutingChanged()

Video Node Factory
----------------
This factory interface is used to create video nodes to build video track.

Main methods are:

- createCameraCapturer()
- createScreenCapturer()
- createCustomVideoSource()
- createVideoFilter()
- createVideoRenderer()

Video Camera Capturer
----------------
This interface is used to control camera as a video source. It also manages camera devices in the windows and MacOS.

Main methods are:

- setCameraSource()
- switchCamera()
- startCapture()
- stopCapture()
- getCaptureFormat()
- getCaptureState()
- registerVideoFrameObserver()
- unregisterVideoFrameObserver()
- registerCameraCaptureObserver()
- unregisterCameraCaptureObserver()

Video Screen Capturer
----------------
This interface is used to control screen sharing as video source.

Main methods are:

- release()

Custom Video Source
----------------
This interface allows application to push external video frame to the Agora SDK.

Main method is:

- pushVideoFrame()

Video Filter & Video Filter Base
----------------
The Video Filter is an intermediate node, which can be added in the video track by IVideoTrack::addVideoFilter(). It reads video frames from the previous node in the track, e.g., a video source or another video filter and then writes video frames back after frame adaption.

Agora provides several built-in video filters, e.g., video beauty filter, which can be created by IMediaNodeFactory::createVideoFilter().

Application can also write its own video filter to modify video frame by implementing IVideoFilterBase::adaptVideoFrame() and add the video filter in the track by IVideoTrack::addVideoFilter().

Main methods are:

- setFilterParameters()
- setEnabled()
- isEnabled()
- registerVideoFrameObserver()
- unregisterVideoFrameObserver()


Video Renderer & Video Sink Base
----------------
The Video Renderer inherits Video Sink Base, which is used to render video frame. Agora provides built-in video renderer by IMediaNodeFactory::createVideoRenderer() and application can add it in the video track by IVideoTrack::addVideoRenderer()
The Video Sink Base is video frame observer, which can reads video frame from built-in video sources, video filters and video track.

Main methods are:

- setRenderMode()
- setMirror()

Local Video Track
----------------
A presentation of local video track that originates from one video source, e.g., camera capturer, screen capturer or a custom video source. Local Video Track can be created by IAgoraService::createVideoTrack()

Agora allows application to customize video track by selecting or adding different video nodes which can be created by IMediaNodeFactory, e.g., Video Source, Video Filter and Video Renderer. You can add multiple nodes and the order of nodes in the pipeline depends on the order of addition by the application.

After video track customized, Application can publish one or multiple local video tracks through ILocalUser::PublishVideo() and each video track has its own video send stream. e.g., you can send camera capturing stream and screen sharing stream simultaneously, and remote users can receive two video streams accordingly.

Main methods are:

- setEnabled()
- isEnabled()
- addVideoFilter()
- removeVideoFilter()
- addVideoRenderer()
- removeVideoRenderer()
- release()

Remote Video Track
----------------
A presentation of a remote video track. Similar with local video track, it also supports customization by adding video filters and renderers.

Main methods are:

- isEnabled()
- addVideoFilter()
- removeVideoFilter()
- addVideoRenderer()
- removeVideoRenderer()
- release()


Terminologies & Basics
==============================
APP ID
-----------
Each “vendor�organization using the Agora SDK (across platforms) has a unique App ID which identifies that organization. All the communication sessions that are created across the Agora Global Network for one App ID are isolated from all other sessions for other App IDs. Therefore, communication sessions will not be connected together across vendors. Statistics, management, and billing are all separated according to the App ID. If an organization has multiple applications that should be kept completely separate, like they are built by completely different teams, then they would use multiple App IDs in this case. If applications need to communicate with each other, then a single App ID should be used.

Dynamical Keys
----------
Using your App ID directly is easy and works well for the initial development of applications, However, if someone illicitly obtains your App ID, then they will be able to perform the same operations in their own client applications. This will allow them to join sessions that belong to you and are billed to you. To prevent this and to secure your applications, Agora recommends to use dynamic keys for large-scale production applications.

Dynamical Keys has constructed within server-side code by APP Cert and other key materials, and APP Cert is never accessible in any client code, which makes Dynamical Key is more secure than static App ID.

Dynamical Key has expiration dates and also contains client permissions as different roles to access Agora Global Network, like host and audience.

Please also review [Agora's Security Keys](https://docs.agora.io/en/2.0.2/product/Interactive%20Broadcast/Product%20Overview/key?platform=All%20Platforms) for more technical details.

Agora Channel
-------------
Think of a channel as a meeting room.  To set up communications between two or more instances of an application, the instances will join the same channel.  Channel names are unique within the domain of an App ID.  The process for app instances to exchange channel names is managed outside the SDK, and is handled by the application developer.

Channels are hosted on the Agora.io Global Network and a channel will automatically be created the first time an app instance tries to join, and destroyed when the last app instance leaved.

User & User Information
--------------
User represents a participant in the channel. Each User has a unique string ID for its identity. Application can get current users-in-channel information and monitor user joining and leaving event by provided RTC Connection Observer after connected to a channel.

There are two kinds of users in channel, Local User and remote users. Local User is application itself which has two roles as Publisher to publish local streams and Subscriber to subscribe remote streams. Remote user represents the remote participant in the channel, who produces and publishes remote stream from remote end.

After connection established, application can get basic user info including User Id and user's media stream info. If Subscriber has subscribed specific remote user, application may also monitor remote user's stream track state.
