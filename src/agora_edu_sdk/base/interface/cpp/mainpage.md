# Agora High-Level API Reference for All Platforms

Agora provides ensured quality of experience (QoE) for worldwide Internet-based voice and video communications through a virtual global network optimized for real-time web and mobile-to-mobile applications.

This page provides Agora high-level API reference.

* [Channel management](#channelmanagement)
* [Channel events](#channelevents)
* [Audio management](#audiomanagement)
* [Video management](#videomanagement)
* [Local media events](#localmediaevents)
* [Remote media events](#remotemediaevents)
* [Statistics events](#statisticsevents)
* [In-ear monitor](#inearmonitor)
* [Music file playback and mixing](#audiomixing)
* [Audio Effect Playback](#audioeffect)
* [Voice changer and reverberation](#voicechanger)
* [Last-mile test](#lastmiletest)
* [Screen capture](#screencapture)
* [Dual video stream](#dualvideostream)
* [Encryption](#encryption)
* [CDN publisher](#cdnpublisher)
* [Channel media relay](#mediarelay)
* [External audio data](#externalaudiodata)
* [Raw audio data](#rawaudiodata)
* [External video data](#externalvideodata)
* [Raw video data](#rawvideodata)
* [Media Metadata](#metadata)
* [Camera control](#cameracontrol)
* [Multiple channels](#multichannels)
* [Audio route](#audioroute)
* [Audio volume indication](#audiovolumeindication)
* [Data stream](#datastream)
* [Video device manager](#videodevicemanager)
* [Video device collection](#videodevicecollection)
* [Audio device manager](#audiodevicemanager)
* [Audio device collection](#audiodevicecollection)
* [Miscellaneous Video Control](#loopbackrecording)
* [Miscellaneous methods](#miscmethods)
* [Miscellaneous events](#miscevents)

<a name="channelmanagement"></a>

### Channel management

| Method                                                       | Description                        |
| ------------------------------------------------------------ | ---------------------------------- |
| \ref createAgoraRtcEngine "createAgoraRtcEngine"       | Creates an IRtcEngine object and returns the pointer. |
| \ref agora::rtc::IRtcEngine::initialize "initialize"         | Initializes the Agora SDK service. |
| \ref agora::rtc::IRtcEngine::release "release"               | Releases the IRtcEngine object.    |
| \ref agora::rtc::IRtcEngine::setChannelProfile "setChannelProfile" | Sets the channel profile.          |
| \ref agora::rtc::IRtcEngine::joinChannel "joinChannel"1       | Joins a channel.                   |
| \ref agora::rtc::IRtcEngine::joinChannel(const char* token, const char* channelId, uid_t userId, const ChannelMediaOptions& options) "joinChannel"2 | Joins a channel with media options. |
| \ref agora::rtc::IRtcEngine::updateChannelMediaOptions "updateChannelMediaOptions" | Updates the channel media options after joining a channel. |
| \ref agora::rtc::IRtcEngine::leaveChannel "leaveChannel"     | Leaves the channel.                |
| \ref agora::rtc::IRtcEngine::setClientRole "setClientRole"   | Sets the role of the user.                 |
| \ref agora::rtc::IRtcEngine::renewToken "renewToken"         | Renews the token.                  |
| \ref agora::rtc::IRtcEngine::getConnectionState "getConnectionState" | Gets the connection state of the app. |



<a name="channelevents"></a>

### Channel events

| Event                                                        | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngineEventHandler::onJoinChannelSuccess "onJoinChannelSuccess" | Occurs when the local user successfully joins the specified channel. |
| \ref agora::rtc::IRtcEngineEventHandler::onRejoinChannelSuccess "onRejoinChannelSuccess" | Occurs when the local user rejoins the channel after being disconnected dut ro network problems. |
| \ref agora::rtc::IRtcEngineEventHandler::onClientRoleChanged "onClientRoleChanged" | Occurs when the user role in a Live-Broadcast channel has switched. |
| \ref agora::rtc::IRtcEngineEventHandler::onLeaveChannel "onLeaveChannel" | Occurs when the local user successfully leaves the channel.  |
| \ref agora::rtc::IRtcEngineEventHandler::onUserJoined "onUserJoined" | Occurs when a remote user or broadcaster joins the channel.  |
| \ref agora::rtc::IRtcEngineEventHandler::onUserOffline "onUserOffline" | Occurs when a remote user or broadcaster goes offline.       |
| \ref agora::rtc::IRtcEngineEventHandler::onConnectionLost "onConnectionLost" | Occurs when the SDK cannot reconnect to the server 10 seconds after its connections to the server is interrupted. |
| \ref agora::rtc::IRtcEngineEventHandler::onRequestToken "onRequestToken" | Occurs when the token has expired.                           |
| \ref agora::rtc::IRtcEngineEventHandler::onTokenPrivilegeWillExpire "onTokenPrivilegeWillExpire" | Occurs when the token will expire in 30 seconds.             |

<a name="audiomanagement"></a>

### Audio management

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::enableAudio "enableAudio"       | Enables the audio.                                           |
| \ref agora::rtc::IRtcEngine::disableAudio "disableAudio"     | Disables the audio.                                          |
| \ref agora::rtc::IRtcEngine::setAudioProfile(AUDIO_PROFILE_TYPE) "setAudioProfile" | Sets the audio parameters and application scenarios.         |
| \ref agora::rtc::IRtcEngine::adjustRecordingSignalVolume "adjustRecordingSignalVolume" | Adjusts the recording volume.     |
| \ref agora::rtc::IRtcEngine::enableLocalAudio "enableLocalAudio" | Enables or disables the local audio capture. \re             |
| \ref agora::rtc::IRtcEngine::muteLocalAudioStream "muteLocalAudioStream" | Stops or resumes sending the local audio stream.             |
| \ref agora::rtc::IRtcEngine::muteAllRemoteAudioStreams "muteAllRemoteAudioStreams" | Stops or resumes receiving all remote audio stream.          |
| \ref agora::rtc::IRtcEngine::setDefaultMuteAllRemoteAudioStreams "setDefaultMuteAllRemoteAudioStreams" | Determines whether to receive all remote audio streams by default. |
| \ref agora::rtc::IRtcEngine::muteRemoteAudioStream "muteRemoteAudioStream" | Stops or resumes receiving the audio stream of a specified user. |

<a name="videomanagement"></a>

### Video management

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::enableVideo "enableVideo"       | Enables the video.                                           |
| \ref agora::rtc::IRtcEngine::disableVideo "disableVideo"     | Disables the video.                                          |
| \ref agora::rtc::IRtcEngine::setVideoEncoderConfiguration "setVideoEncoderConfiguration" | Sets the video encoder configuration.                        |
| \ref agora::rtc::IRtcEngine::startPreview "startPreview"     | Starts the local video preview before joining a channel.     |
| \ref agora::rtc::IRtcEngine::stopPreview "stopPreview"       | Stops the local video preview.                               |
| \ref agora::rtc::IRtcEngine::setupLocalVideo "setupLocalVideo" | Initializes the local video view.                            |
| \ref agora::rtc::IRtcEngine::setupRemoteVideo "setupRemoteVideo" | Initializes the video view of a remote user.                 |
| \ref agora::rtc::IRtcEngine::setLocalRenderMode "setLocalRenderMode" | Updates the display mode of the local video view.            |
| \ref agora::rtc::IRtcEngine::setRemoteRenderMode "setRemoteRenderMode" | Updates the display more of the video view of a remote user. |
| \ref agora::rtc::IRtcEngine::setLocalVideoMirrorMode "setLocalVideoMirrorMode" | Sets the local video mirror mode.                            |
| \ref agora::rtc::IRtcEngine::enableLocalVideo "enableLocalVideo" | Disables or re-enables the local video capture.              |
| \ref agora::rtc::IRtcEngine::muteLocalVideoStream "muteLocalVideoStream" | Stops or resumes sending the local video stream.             |
| \ref agora::rtc::IRtcEngine::muteAllRemoteVideoStreams "muteAllRemoteVideoStreams" | Stops or resumes receiving all remote video streams.         |
| \ref agora::rtc::IRtcEngine::setDefaultMuteAllRemoteVideoStreams "setDefaultMuteAllRemoteVideoStreams" | Determines whether to receive all remote video streams by default. |
| \ref agora::rtc::IRtcEngine::muteRemoteVideoStream "muteRemoteVideoStream" | Stops or resumes receiving the video stream of a specified user. |


<a name="localmediaevents"></a>

### Local media events

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onLocalAudioStateChanged "onLocalAudioStateChanged" | Occurs when the local audio state changes.            |
| \ref agora::rtc::IRtcEngineEventHandler::onLocalVideoStateChanged "onLocalVideoStateChanged" | Occurs when the local video state changes.          |
| \ref agora::rtc::IRtcEngineEventHandler::onFirstLocalAudioFramePublished "onFirstLocalAudioFramePublished"| Occurs when the first audio frame is published. |

<a name="remotemediaevents"></a>

### Remote media events

| Event                                                        | Description                                |
| ------------------------------------------------------------ | ------------------------------------------ |
| \ref agora::rtc::IRtcEngineEventHandler::onRemoteAudioStateChanged "onRemoteAudioStateChanged" | Occurs when the remote audio state changes. |
| \ref agora::rtc::IRtcEngineEventHandler::onRemoteVideoStateChanged "onRemoteVideoStateChanged" | Occurs when the remote video state changes. |



<a name="statisticsevents"></a>

### Statistics events

| Event                                                        | Description                                 |
| ------------------------------------------------------------ | ------------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onRemoteAudioStats "onRemoteAudioStats" | Reports the statistics of the remote audio. |
| \ref agora::rtc::IRtcEngineEventHandler::onRtcStats "onRtcStats" | Reports the statistics of the current call. |
| \ref agora::rtc::IRtcEngineEventHandler::onNetworkQuality "onNetworkQuality" | Reports the network quality of each user.   |
| \ref agora::rtc::IRtcEngineEventHandler::onLocalVideoStats "onLocalVideoStats" | Reports the statistics of the local video.  |
| \ref agora::rtc::IRtcEngineEventHandler::onRemoteVideoStats "onRemoteVideoStats" | Reports the statistics of the remote video. |
| \ref agora::rtc::IRtcEngineEventHandler::onLocalAudioStats "onLocalAudioStats" | Reports the statistics of the local audio.  |
| \ref agora::rtc::IRtcEngineEventHandler::onRemoteAudioStats "onRemoteAudioStats" | Reports the statistics of the remote audio. |

<a name="inearmonitor"></a>

### In-ear monitor

| Method                                                       | Description                               |
| ------------------------------------------------------------ | ----------------------------------------- |
| \ref agora::rtc::IRtcEngine::enableInEarMonitoring "enableInEarMonitoring" | Enables in-ear monitoring.                |
| \ref agora::rtc::IRtcEngine::setInEarMonitoringVolume "setInEarMonitoringVolume" | Sets the volume of the in-ear monitoring. |

<a name="audiomixing"></a>

### Music file playback and mixing

| Method                                                       | Function                                                |
| :----------------------------------------------------------- | :------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::startAudioMixing "startAudioMixing" | Starts playing and mixing the music file.               |
| \ref agora::rtc::IRtcEngine::stopAudioMixing "stopAudioMixing"       | Stops playing and mixing the music file.                |
| \ref agora::rtc::IRtcEngine::pauseAudioMixing "pauseAudioMixing"   | Pauses playing and mixing the music file.               |
| \ref agora::rtc::IRtcEngine::resumeAudioMixing "resumeAudioMixing"   | Resumes playing and mixing the music file.              |
| \ref agora::rtc::IRtcEngine::adjustAudioMixingVolume "adjustAudioMixingVolume" | Adjusts the volume of audio mixing.                     |
| \ref agora::rtc::IRtcEngine::getAudioMixingDuration "getAudioMixingDuration" | Gets the duration (ms) of the music file.               |
| \ref agora::rtc::IRtcEngine::getAudioMixingCurrentPosition "getAudioMixingCurrentPosition" | Gets the playback position (ms) of the music file.      |
| \ref agora::rtc::IRtcEngine::setAudioMixingPosition "setAudioMixingPosition" | Sets the playback position of the music file.           |

| Event                                                     | Description                                           |
| :----------------------------------------------------------- | :---------------------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onAudioMixingStateChanged "onAudioMixingStateChanged" | Occurs when the state of the local user's audio mixing file changes. |

<a name="audioeffect"></a>

### Audio Effect Playback

| Method                                                    | Description                               |
| :----------------------------------------------------------- | :------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::getEffectsVolume "getEffectsVolume" | Gets the volume of audio effects.|
| \ref agora::rtc::IRtcEngine::setEffectsVolume "setEffectsVolume" |Sets the volume of audio effects.   |
| \ref agora::rtc::IRtcEngine::getVolumeOfEffect "getVolumeOfEffect" |Gets the volume of the specified audio effect.   |
| \ref agora::rtc::IRtcEngine::setVolumeOfEffect "setVolumeOfEffect" | Sets the volume of the specified audio effect. |
| \ref agora::rtc::IRtcEngine::preloadEffect "preloadEffect"|Preloads a specified audio effect.  |
| \ref agora::rtc::IRtcEngine::playEffect "playEffect" | Plays a specified audio effect.|
| \ref agora::rtc::IRtcEngine::playAllEffects "playAllEffects" | Plays all audio effects.  |
| \ref agora::rtc::IRtcEngine::pauseEffect "pauseEffect" | Pauses playing the specified audio effect.  |
| \ref agora::rtc::IRtcEngine::pauseAllEffects "pauseAllEffects" | Pauses playing audio effects.  |
| \ref agora::rtc::IRtcEngine::resumeEffect "resumeEffect" | Resumes playing the specified audio effect. |
| \ref agora::rtc::IRtcEngine::resumeAllEffects "resumeAllEffects"| Resumes playing audio effects. |
| \ref agora::rtc::IRtcEngine::stopEffect "stopEffect"|Stops playing the specified audio effect.  |
| \ref agora::rtc::IRtcEngine::stopAllEffects "stopAllEffects" | Stops playing audio effects.|
| \ref agora::rtc::IRtcEngine::unloadEffect "unloadEffect" | Releases the specified preloaded audio effect from the memory.  |
| \ref agora::rtc::IRtcEngine::unloadAllEffects "unloadAllEffects" | Releases preloaded audio effects from the memory. |


<a name="voicechanger"></a>

### Voice changer and reverberation

| Method | Description |
| --------- | ----------------|
| \ref agora::rtc::IRtcEngine::setLocalVoicePitch "setLocalVoicePitch" | Changes the voice pitch of the local speaker. |
| \ref agora::rtc::IRtcEngine::setLocalVoiceEqualization "setLocalVoiceEqualization" | Sets the local voice equalization effect.     |
| \ref agora::rtc::IRtcEngine::setLocalVoiceReverb "setLocalVoiceReverb" | Sets the local voice reverberation.           |
| \ref agora::rtc::IRtcEngine::setLocalVoiceReverbPreset "setLocalVoiceReverbPreset" | Sets the preset local voice reverberation effect. |
| \ref agora::rtc::IRtcEngine::setLocalVoiceChanger "setLocalVoiceChanger"| Sets the local voice changer option.           |


<a name="lastmiletest"></a>

### Last-mile test

| Method                                                       | Description                 |
| ------------------------------------------------------------ | --------------------------- |
| \ref agora::rtc::IRtcEngine::startLastmileProbeTest "startLastmileProbeTest" | Starts the last-mile network probe test. |
| \ref agora::rtc::IRtcEngine::stopLastmileProbeTest "stopLastmileProbeTest" | Stops the last-mile network probe test. |

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onLastmileProbeResult "onLastmileProbeResult" | Reports the last-mile network probe result. |
| \ref agora::rtc::IRtcEngineEventHandler::onLastmileQuality "onLastmileProbeQuality" | Reports the last-mile network quality of the local user. |

<a name="screencapture"></a>

### Screen capture

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::startScreenCaptureByScreenRect "startScreenCaptureByScreenRect" | Shares the whole or part of a screen by specifying the screen rect. |
| \ref agora::rtc::IRtcEngine::setScreenCaptureContentHint "setScreenCaptureContentHint" | Sets the content hint for screen sharing.                       |
| \ref agora::rtc::IRtcEngine::updateScreenCaptureRegion "updateScreenCaptureRegion" | Updates the screen sharing region. |
| \ref agora::rtc::IRtcEngine::updateScreenCaptureParameters "updateScreenCaptureParameters" | Updates the screen sharing parameters.                       |
| \ref agora::rtc::IRtcEngine::stopScreenCapture "stopScreenCapture" | Stops the screen sharing.                                    |

<a name="dualvideostream"></a>

### Dual video stream

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::enableDualStreamMode "enableDualStreamMode" | Enables or disables the dual video stream mode.              |
| \ref agora::rtc::IRtcEngine::setRemoteVideoStreamType "setRemoteVideoStreamType" | Sets the remote video stream type.                           |
| \ref agora::rtc::IRtcEngine::setRemoteDefaultVideoStreamType "setRemoteDefaultVideoStreamType" | Sets the default stream type of the remote video if the remote user has enabled dual-stream. |

<a name="encryption"></a>

### Encryption

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::registerPacketObserver "registerPacketObserver" | Registers a packet observer.              |


<a name="cdnpublisher"></a>

### CDN publisher

| Method                                                       | Description                                            |
| ------------------------------------------------------------ | ------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::addPublishStreamUrl "addPublishStreamUrl" | Publishes the local stream to the CDN.                 |
| \ref agora::rtc::IRtcEngine::removePublishStreamUrl "removePublishStreamUrl" | Removes an RTMP stream from the CDN.                   |
| \ref agora::rtc::IRtcEngine::setLiveTranscoding "setLiveTranscoding" | Sets the video layout and audio settings for CDN live. |

| Event                                                        | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngineEventHandler::onRtmpStreamingStateChanged "onRtmpStreamingStateChanged" | Occurs when the state of the RTMP streaming state has changed. |
| \ref agora::rtc::IRtcEngineEventHandler::onTranscodingUpdated "onTranscodingUpdated" | Occurs when the publisher's transcoding settings are updated. |

<a name="mediarelay"></a>

### Channel media relay

| Event                                                        | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngineEventHandler::onChannelMediaRelayStateChanged "onChannelMediaRelayStateChanged" | Occurs when the state of the media stream relay changes. |
| \ref agora::rtc::IRtcEngineEventHandler::onChannelMediaRelayEvent "onChannelMediaRelayEvent" | Reports events during the media stream relay. |

<a name="externalaudiodata"></a>

### External audio data 

| Method                                                       | Description                                |
| ------------------------------------------------------------ | ------------------------------------------ |
| \ref agora::rtc::IRtcEngine::setExternalAudioSource "setExternalAudioSource" | Sets the external audio source.            |
| \ref agora::media::IMediaEngine::pushAudioFrame "pushAudioFrame" | Pushes the external audio data to the app. |

<a name="rawaudiodata"></a>

### Raw audio data

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::media::IMediaEngine::registerAudioFrameObserver "registerAudioFrameObserver" | Registers an audio frame observer object.                    |
| \ref agora::rtc::IRtcEngine::setRecordingAudioFrameParameters "setRecordingAudioFrameParameters" | Sets the audio recording format for the `onRecordAudioFrame` callback. |
| \ref agora::rtc::IRtcEngine::setPlaybackAudioFrameParameters "setPlaybackAudioFrameParameters" | Sets the audio playback format for the `onPlaybackAudioFrame` callback. |
| \ref agora::rtc::IRtcEngine::setMixedAudioFrameParameters "setMixedAudioFrameParameters" | Sets the mixed audio format for the `onMixedAudioFrame` callback. |
| \ref agora::rtc::IRtcEngine::setPlaybackAudioFrameBeforeMixingParameters "setPlaybackAudioFrameBeforeMixingParameters" | Sets the audio playback format before mixing in the `onPlaybackAudioFrameBeforeMixing` callback. |

| Event                                                        | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::media::IAudioFrameObserver::onRecordAudioFrame "onRecordAudioFrame" | Occurs when the recorded audio frame is received.            |
| \ref agora::media::IAudioFrameObserver::onPlaybackAudioFrame "onPlaybackAudioFrame" | Occurs when the playback audio frame is received.            |
| \ref agora::media::IAudioFrameObserver::onMixedAudioFrame "onMixedAudioFrame" | Occurs when the mixed audio data is received.                |
| \ref agora::media::IAudioFrameObserver::onPlaybackAudioFrameBeforeMixing "onPlaybackAudioFrameBeforeMixing" | Occurs when the playback audio frame before mixing is received. |


<a name="externalvideodata"></a>

### External video data

| Method                                                       | Description                                 |
| ------------------------------------------------------------ | ------------------------------------------- |
| \ref agora::media::IMediaEngine::setExternalVideoSource "setExternalVideoSource" | Sets the external video source.             |
| \ref agora::media::IMediaEngine::pushVideoFrame "pushVideoFrame" | Pushes the external video frame to the app. |
| \ref agora::media::IMediaEngine::pushEncodedVideoImage "pushEncodedVideoImage" | Pushes the encoded video image to the app.  |

<a name="rawvideodata"></a>

### Raw video data

| Method                                                       | Description                                              |
| ------------------------------------------------------------ | -------------------------------------------------------- |
| \ref agora::media::IMediaEngine::registerVideoFrameObserver "registerVideoFrameObserver" | Registers a video frame observer object.                 |
| \ref agora::media::IMediaEngine::registerVideoEncodedImageReceiver "registerVideoEncodedImageReceiver" | Registers a receiver object for the encoded video image. |

| Event                                                        | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::media::IVideoFrameObserver::onCaptureVideoFrame "onCaptureVideoFrame" | Occurs each time The SDK receives a video frame captured by the local camera. |
| \ref agora::media::IVideoFrameObserver::onRenderVideoFrame "onRenderVideoFrame" | Occurs each time the SDK receives a video frame sent by the remote user. |
| \ref agora::rtc::IVideoEncodedImageReceiver::OnEncodedVideoImageReceived "OnEncodedVideoImageReceived" | Occurs each time the SDK receives an encoded video image.    |
<a name="metadata"></a>

### Media Metadata

> Do not implement \ref agora::rtc::IMetadataObserver::getMaxMetadataSize "getMaxMetadataSize", \ref agora::rtc::IMetadataObserver::onReadyToSendMetadata "onReadyToSendMetadata", and \ref agora::rtc::IMetadataObserver::onMetadataReceived "onMetadataReceived" in \ref agora::rtc::IRtcEngineEventHandler "IRtcEngineEventHandler".

|Method     | Description         |
| --------------| ------------------ |
| \ref agora::rtc::IRtcEngine::registerMediaMetadataObserver "registerMediaMetadataObserver" | Registers the metadata observer. |
| \ref agora::rtc::IMetadataObserver::getMaxMetadataSize "getMaxMetadataSize" | Gets the maximum size of the metadata. |
| \ref agora::rtc::IMetadataObserver::onReadyToSendMetadata "onReadyToSendMetadata" | Notification for starting sending the metadata. |
| \ref agora::rtc::IMetadataObserver::onMetadataReceived "onMetadataReceived" | Occurs when received the metadata. |

<a name="cameracontrol"></a>

### Camera control

> This group of methods is for Android and iOS only.

| Method                                                       | Function                                                     |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| \ref agora::rtc::IRtcEngine::switchCamera "switchCamera"            | Switches between front and rear cameras.                 |

<a name="multichannels"></a>

### Multiple channels

| Method                                                       | Description                                     |
| ------------------------------------------------------------ | ----------------------------------------------- |
| \ref agora::rtc::IRtcEngine::joinChannelEx "joinChannelEx"   | Joins a channel with the connection ID.                                |
| \ref agora::rtc::IRtcEngine::leaveChannelEx "leaveChannelEx" | Leaves the channel with the connection ID.                               |


<a name="audioroute"></a>

### Audio route

> This group of methods is for Android and iOS only.

| Method          | Description                   |
| ---------------- | ----------------------------- |
| \ref agora::rtc::IRtcEngine::setDefaultAudioRouteToSpeakerphone "setDefaultAudioRouteToSpeakerphone" |Sets the default audio route.|
| \ref agora::rtc::IRtcEngine::setEnableSpeakerphone "setEnableSpeakerphone" |Enables or disables the speakerphone temporarily.|
| \ref agora::rtc::IRtcEngine::isSpeakerphoneEnabled "isSpeakerphoneEnabled" |Checks whether the speakerphone is enabled.|

| Event           | Description                   |
|----------------- | ------------------------------ |
| \ref agora::rtc::IRtcEngineEventHandler::onAudioRoutingChanged "onAudioRoutingChanged" | Occurs when the audio route changes. |

<a name="audiovolumeindication"></a>

### Audio volume indication

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::enableAudioVolumeIndication "enableAudioVolumeIndication" | Enables the SDK to regularly report to the application on which users are speaking and the speakers' volume.|

| Event                                                        | Description                                                |
| ------------------------------------------------------------ | ---------------------------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onAudioVolumeIndication "onAudioVolumeIndication" | Reports which users are speaking and the speakers' volume at the moment.|
<a name="datastream"></a>

### Data Stream

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::createDataStream "createDataStream" |Creates a data stream. |
| \ref agora::rtc::IRtcEngine::sendStreamMessage "sendStreamMessage" |Sends a data stream. |

| Event                                                        | Description                                                |
| ------------------------------------------------------------ | ---------------------------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onStreamMessage "onStreamMessage" |Occurs when the user receives the data stream. |
| \ref agora::rtc::IRtcEngineEventHandler::onStreamMessageError "onStreamMessageError" |Occurs when the user fails to receive the data stream.|





<a name="videodevicemanager"></a>

### Video device manager

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IVideoDeviceManager::setDevice "setDevice"  | Specifies a device with the device ID.        |
| \ref agora::rtc::IVideoDeviceManager::getDevice "getDevice"  | Gets the device ID of the video capture device that is in use.  |
| \ref agora::rtc::IVideoDeviceManager::release "release"      | Releases all IVideoDeviceManager resources.               |

| Event                                                        | Description                                                |
| ------------------------------------------------------------ | ---------------------------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onVideoDeviceStateChanged "onVideoDeviceStateChanged" | Occurs when the video device state changes.|

<a name="videodevicecollection"></a>

### Video device collection

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IVideoDeviceCollection::getCount "getCount" | Gets the total number of the video capture devices in the system. |
| \ref agora::rtc::IVideoDeviceCollection::getDevice "getDevice" | Gets the device ID and device name of the specified video capture device.    |
| \ref agora::rtc::IVideoDeviceCollection::setDevice "setDevice" | Specifies a device with the device ID.   |
| \ref agora::rtc::IVideoDeviceCollection::release "release"   | Releases all IVideoDeviceCollection resources.    |

<a name="audiodevicemanager"></a>

### Audio device manager

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IAudioDeviceManager::enumeratePlaybackDevices "enumeratePlaybackDevices" | Enumerates the audio playback devices in the system.         |
| \ref agora::rtc::IAudioDeviceManager::enumerateRecordingDevices "enumerateRecordingDevices" | Enumerates the audio recording devices in the system.   |
| \ref agora::rtc::IAudioDeviceManager::setPlaybackDevice "setPlaybackDevice" | Specifies an audio playback device with the device ID. |
| \ref agora::rtc::IAudioDeviceManager::getPlaybackDevice "getPlaybackDevice" | Gets the ID of the audio playback device.      |
| \ref agora::rtc::IAudioDeviceManager::getPlaybackDeviceInfo "getPlaybackDeviceInfo" | Gets the device ID and device name of the audio playback device. |
| \ref agora::rtc::IAudioDeviceManager::setPlaybackDeviceVolume "setPlaybackDeviceVolume" | Sets the volume of the audio playback device.      |
| \ref agora::rtc::IAudioDeviceManager::getPlaybackDeviceVolume "getPlaybackDeviceVolume" | Gets the volume of the audio playback device.    |
| \ref agora::rtc::IAudioDeviceManager::setRecordingDevice "setRecordingDevice" | Specifies an audio recording device with the device ID.  |
| \ref agora::rtc::IAudioDeviceManager::getRecordingDevice "getRecordingDevice" | Gets the ID of the audio recording device.        |
| \ref agora::rtc::IAudioDeviceManager::getRecordingDeviceInfo "getRecordingDeviceInfo" | Gets the device ID and device name of the audio recording device.|
| \ref agora::rtc::IAudioDeviceManager::setRecordingDeviceVolume "setRecordingDeviceVolume" | Sets the volume of the audio recording device.    |
| \ref agora::rtc::IAudioDeviceManager::getRecordingDeviceVolume "getRecordingDeviceVolume" | Gets the volume of the audio recording device.|
| \ref agora::rtc::IAudioDeviceManager::setPlaybackDeviceMute "setPlaybackDeviceMute" | Mutes or unmutes the audio playback device.    |
| \ref agora::rtc::IAudioDeviceManager::getPlaybackDeviceMute "getPlaybackDeviceMute" | Checks whether the audio playback device is muted.   |
| \ref agora::rtc::IAudioDeviceManager::setRecordingDeviceMute "setRecordingDeviceMute" | Mutes or unmutes the audio recording device.   |
| \ref agora::rtc::IAudioDeviceManager::getRecordingDeviceMute "getRecordingDeviceMute" | Checks whether the audio recording device is muted.   |
| \ref agora::rtc::IAudioDeviceManager::release "release"      |Releases all IAudioDeviceManager resources.             |

| Event                                                        | Description                                                |
| ------------------------------------------------------------ | ---------------------------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onAudioDeviceStateChanged "onAudioDeviceStateChanged" | Occurs when the audio device state changes.|
| \ref agora::rtc::IRtcEngineEventHandler::onAudioDeviceVolumeChanged "onAudioDeviceVolumeChanged" | Occurs when the audio device volume changes.|


<a name="audiodevicecollection"></a>

### Audio device collection

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IAudioDeviceCollection::getCount "getCount" | Gets the total number of the audio playback or recording devices in the system.|
| \ref agora::rtc::IAudioDeviceCollection::getDevice "getDevice" |Gets the device ID and device name of the specified audio playback or recording device.   |
| \ref agora::rtc::IAudioDeviceCollection::setDevice "setDevice" |Specifies a device with the device ID.      |
| \ref agora::rtc::IAudioDeviceCollection::setApplicationVolume "setApplicationVolume" | Sets the volume of the app.    |
| \ref agora::rtc::IAudioDeviceCollection::getApplicationVolume "getApplicationVolume" | Gets the volume of the app.   |
| \ref agora::rtc::IAudioDeviceCollection::setApplicationMute "setApplicationMute" |Mutes or unmutes the app.        |
| \ref agora::rtc::IAudioDeviceCollection::isApplicationMute "isApplicationMute" | Checks whether the app is muted. |
| \ref agora::rtc::IAudioDeviceCollection::release "release"   | Releases all IAudioDeviceCollection resources.    |


<a name="loopbackrecording"></a>

### Miscellaneous Video Control

| Method                                                       | Description                                                  |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| \ref agora::rtc::IRtcEngine::enableLoopbackRecording(bool enabled) "enableLoopbackRecording"1 | Enables loopback recording.|
| \ref agora::rtc::IRtcEngine::enableLoopbackRecording(conn_id_t connectionId, bool enabled) "enableLoopbackRecording"2 |Sets the connection ID and enables loopback recording.  |


<a name="miscmethods"></a>

### Miscellaneous methods

| Method                                                       | Description                                       |
| ------------------------------------------------------------ | ------------------------------------------------- |
| \ref agora::rtc::IRtcEngine::sendCustomReportMessage "sendCustomReportMessage"           | Reports customized messages.   |
| \ref agora::rtc::IRtcEngine::getCallId "getCallId"           | Gets the current call ID.                         |
| \ref agora::rtc::IRtcEngine::rate "rate"                     | Allows a user to rate the call.                   |
| \ref agora::rtc::IRtcEngine::complain "complain"             | Allows a user to complain about the call quality. |
| \ref agora::rtc::IRtcEngine::getVersion "getVersion"         | Gets the SDK version.                             |
| \ref agora::rtc::IRtcEngine::getErrorDescription "getErrorDescription" | Gets the warning or error description.            |
| \ref agora::rtc::IRtcEngine::setLogFile "setLogFile"         | Specifies an SDK output log file.                 |
| \ref agora::rtc::IRtcEngine::setLogFileSize "setLogFileSize"         | Sets the log file size (KB).                |
| \ref agora::rtc::IRtcEngine::setLogFilter "setLogFilter"     | Sets the output log filter level of the SDK.          |
| \ref agora::rtc::IRtcEngine::setLogLevel "setLogLevel"     | Sets the output log level of the SDK.  |




<a name="miscevents"></a>

### Miscellaneous events

| Method                                                       | Description                            |
| ------------------------------------------------------------ | -------------------------------------- |
| \ref agora::rtc::IRtcEngineEventHandler::onApiCallExecuted "onApiCallExecuted" | Occurs when an API method is executed. |


