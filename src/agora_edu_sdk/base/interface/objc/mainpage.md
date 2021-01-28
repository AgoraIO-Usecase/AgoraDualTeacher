Agora provides ensured quality of experience (QoE) for worldwide Internet-based voice and video communications through a virtual global network optimized for real-time web and mobile-to-mobile applications.

- The `AgoraRtcEngineKit` class is the entry point of the Agora SDK providing API methods for apps to easily start voice and video communication.
- The Agora SDK uses delegate callbacks in the `AgoraRtcEngineDelegate` protocol to report runtime events to the app.
<!-- - The `AgoraRtcChannel` class provides methods that enable real-time communications in a specified channel. By creating multiple RtcChannel instances, users can join multiple channels.
- The [AgoraRtcChannelDelegate](AgoraRtcChannelDelegate) class provides callbacks that report events and statistics of a specified channel. -->

This page provides Reference for Agora high-level APIs.

- [Channel management](#channelmanagement)
- [Channel events](#channelevents)
- [Audio management](#audiomanagement)
- [Video management](#videomanagement)
- [Local media events](#localmediaevents)
- [Remote media events](#remotemediaevents)
- [Statistics events](#statisticsevents)
- [Audio volume indication](#audiovolumeindication)
- [Audio route control](#audioroutecontrol)
- [Music file playback and mixing](#audiomixing)
- [Audio effect file playback](#playback)
- [Voice changer and reverberation](#voicechanger)
- [In-ear monitor](#inearmonitor)
- [Last-mile test](#lastmiletest)
- [Camera control](#cameracontrol)
- [Dual video stream](#dualvideostream)
- [External audio source](#externalaudiosource)
- [External video source](#externalvideosource)
- [Media metadata](#metadata)
<!-- - [Screen sharing](#screensharing)
- [CDN publisher](#cdnpublisher)
- [External audio data](#externalaudiodata)
- [Raw audio data](#rawaudiodata)
- [Data stream](#datastream)
- [Miscellaneous methods](#miscmethods)
<!-- - [Device manager](#devicemanager) -->
- [Miscellaneous events](#miscevents)

## <a name="channelmanagement"></a>Channel management

| Method                                                       | Function                                             |
| :----------------------------------------------------------- | :--------------------------------------------------- |
|  {@link AgoraRtcEngineKit.sharedEngineWithConfig:delegate: sharedEngineWithConfig}  | Initializes RtcEngine.              |
| {@link AgoraRtcEngineKit.destroy destroy}                       | Destroys the RtcEngine instance.                     |
| {@link AgoraRtcEngineKit.setChannelProfile: setChannelProfile} | Sets the channel profile.          |
| {@link AgoraRtcEngineKit.setClientRole: setClientRole}         | Sets the role of the user. |
| {@link AgoraRtcEngineKit.joinChannelByToken:channelId:info:uid:joinSuccess: joinChannelByToken}1 | Joins a channel.                     |
| {@link AgoraRtcEngineKit.joinChannelByToken:channelId:uid:mediaOptions:joinSuccess: joinChannelByToken}2 | Joins a channel with media options.                     |
| {@link AgoraRtcEngineKit.leaveChannel: leaveChannel}            | Allows a user to leave a channel.                    |
| {@link AgoraRtcEngineKit.renewToken: renewToken}                | Renews the Token.                                    |
| {@link AgoraRtcEngineKit.getConnectionState: getConnectionState} | Gets the connection state of the app. |

## <a name="channelevents"></a>Channel events

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:didJoinChannel:withUid:elapsed: didJoinChannel} | Occurs when a user joins a channel.                        |
| {@link AgoraRtcEngineDelegate.rtcEngine:didRejoinChannel:withUid:elapsed: didRejoinChannel} | Occurs when a user rejoins a channel.                      |
| {@link AgoraRtcEngineDelegate.rtcEngine:didLeaveChannelWithStats: didLeaveChannelWithStats} | Occurs when a user leaves a channel.                       |
| {@link AgoraRtcEngineDelegate.rtcEngine:didClientRoleChanged:newRole: didClientRoleChanged} | Occurs when the user role switches in a live broadcast.       |
| {@link AgoraRtcEngineDelegate.rtcEngine:didJoinedOfUid:elapsed: didJoinedOfUid} | Occurs when a remote user joins a channel.                 |
| {@link AgoraRtcEngineDelegate.rtcEngine:didOfflineOfUid:reason: didOfflineOfUid} | Occurs when a remote user leaves a channel.                |
| {@link AgoraRtcEngineDelegate.rtcEngineConnectionDidLost: rtcEngineConnectionDidLost} | Occurs when the connection between the SDK is interrupted, and the SDK cannot reconnect to the edge server in 10 seconds. |
| {@link AgoraRtcEngineDelegate.rtcEngine:tokenPrivilegeWillExpire: tokenPrivilegeWillExpire} | Occurs when the token expires in 30 seconds.             |
| {@link AgoraRtcEngineDelegate.rtcEngineRequestToken: rtcEngineRequestToken} | Occurs when the token expires.                               |
<!-- | [connectionChangedToState]([AgoraRtcEngineDelegate.rtcEngine:connectionChangedToState:reason:} | Occurs when the connection state of the app changes.         |
| [networkTypeChangedToType]([AgoraRtcEngineDelegate.rtcEngine:networkTypeChangedToType:}                                     | Occurs when the network type changes.                                            |
| [rtcEngineConnectionDidBanned]([AgoraRtcEngineDelegate.rtcEngineConnectionDidBanned:} | Occurs when your connection is banned by the Agora Server.   | -->

## <a name="audiomanagement"></a>Audio management

| Method                                                       | Function                                                     |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineKit.enableAudio enableAudio}               | Enables the audio module.                                    |
| {@link AgoraRtcEngineKit.disableAudio disableAudio}             | Disables the audio module.                                   |
| {@link AgoraRtcEngineKit.setAudioProfile:scenario: setAudioProfile1} | Sets the audio parameters and application scenarios.         |
| {@link AgoraRtcEngineKit.setAudioProfile: setAudioProfile2} | Sets the audio parameters.         |
| {@link AgoraRtcEngineKit.adjustRecordingSignalVolume: adjustRecordingSignalVolume}| Adjusts the recording volume.                                |
| {@link AgoraRtcEngineKit.enableLocalAudio: enableLocalAudio}   | Enables or disables the local audio capture.            |
| {@link AgoraRtcEngineKit.muteLocalAudioStream: muteLocalAudioStream} | Stops or resumes sending the local audio stream.              |
| {@link AgoraRtcEngineKit.muteRemoteAudioStream:mute: muteRemoteAudioStream} | Stops or resumes receiving the audio stream of a specified user.       |
| {@link AgoraRtcEngineKit.muteAllRemoteAudioStreams: muteAllRemoteAudioStreams} | Stops or resumes receiving all remote audio streams.    |
<!-- | [adjustRecordingSignalVolume]([AgoraRtcEngineKit.adjustRecordingSignalVolume:} | Adjusts the recording volume.                                |
| [adjustUserPlaybackSignalVolume]([AgoraRtcEngineKit.adjustUserPlaybackSignalVolume:volume:} | Adjusts the playback volume of a specified remote user.                              |
| [adjustPlaybackSignalVolume]([AgoraRtcEngineKit.adjustPlaybackSignalVolume:} | Adjusts the playback volume of all remote users.                                 |
 -->

## <a name="videomanagement"></a>Video management

| Method                                                       | Function                                              |
| :----------------------------------------------------------- | :---------------------------------------------------- |
| {@link AgoraRtcEngineKit.enableVideo enableVideo}               | Enables the video module.                             |
| {@link AgoraRtcEngineKit.disableVideo disableVideo}             | Disables the video module.                            |
| {@link AgoraRtcEngineKit.setupLocalVideo: setupLocalVideo}      | Initializes the local video view.                         |
| {@link AgoraRtcEngineKit.setLocalRenderMode: setLocalRenderMode} | Updates the display mode of the local video view.                    |
| {@link AgoraRtcEngineKit.startPreview startPreview}             | Starts the local video preview.                       |
| {@link AgoraRtcEngineKit.stopPreview stopPreview}               | Stops the local video preview.                        |
| {@link AgoraRtcEngineKit.enableLocalVideo: enableLocalVideo}   | Enables/Disables the local video capture.             |
| {@link AgoraRtcEngineKit.muteLocalVideoStream: muteLocalVideoStream} | Sends/Stops sending the local video.                  |
| {@link AgoraRtcEngineKit.muteRemoteVideoStream:mute: muteRemoteVideoStream} | Receives/Stops receiving a specified video stream.    |
| {@link AgoraRtcEngineKit.muteAllRemoteVideoStreams: muteAllRemoteVideoStreams} | Receives/Stops receiving all remote video streams.       |
<!-- | [setVideoEncoderConfiguration]([AgoraRtcEngineKit.setVideoEncoderConfiguration:} | Sets the video encoder configuration.                 |
| [setupRemoteVideo]([AgoraRtcEngineKit.setupRemoteVideo:}    | Initializes the video view of a remote user.                 |
| [setRemoteRenderMode]([AgoraRtcEngineKit.setRemoteRenderMode:renderMode:mirrorMode:} | Updates the display mode of the video view of a remote user.           | -->

## <a name="localmediaevents"></a>Local media events

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:localAudioStateChanged:error: localAudioStateChange} | Occurs when the local audio state changes.            |
| {@link AgoraRtcEngineDelegate.rtcEngine:firstLocalAudioFramePublished: firstLocalAudioFramePublished} | Occurs when the first audio frame is published. |
| {@link AgoraRtcEngineDelegate.rtcEngine:localVideoStateChangedOfState:error: localVideoStateChange} | Occurs when the state of the local video changes.               |


## <a name="remotemediaevents"></a>Remote media events

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:remoteAudioStateChangedOfUid:state:reason:elapsed: remoteAudioStateChangedOfUid} | Occurs when the remote audio state changes. |
|{@link AgoraRtcEngineDelegate.rtcEngine:remoteVideoStateChangedOfUid:state:reason:elapsed: remoteVideoStateChangedOfUid}  | Occurs when the remote video stream state changes.           |


## <a name="statisticsevents"></a>Statistics events

> After joining a channel, SDK triggers this group of callbacks once every two seconds.

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:localAudioStats: localAudioStats} | Reports the statistics of the local audio stream.                  |
| {@link AgoraRtcEngineDelegate.rtcEngine:remoteAudioStats: remoteAudioStats} | Reports the statistics of the audio stream from each remote user/host. |
| {@link AgoraRtcEngineDelegate.rtcEngine:reportRtcStats: reportRtcStats} | Reports the statistics of the Rtc Engine.   |
| {@link AgoraRtcEngineDelegate.rtcEngine:localVideoStats: localVideoStats} | Reports the statistics of the uploading local video streams. |
| {@link AgoraRtcEngineDelegate.rtcEngine:remoteVideoStats: remoteVideoStats} | Reports the statistics of the video stream from each remote user/host. |

| Method                                                       | Function                                              |
| :----------------------------------------------------------- | :---------------------------------------------------- |
| [setBeautyEffectOptions]([AgoraRtcEngineKit.setBeautyEffectOptions:options:} | Enables/Disables image enhancement and sets the options. (iOS only)  |

## <a name="multichannels"></a>Multi-channel management

| API                                                          | Function                           |
| :----------------------------------------------------------- | :--------------------------------- |
| [createRtcChannel]([AgoraRtcEngineKit.createRtcChannel:} | Initializes and gets an [AgoraRtcChannel](AgoraRtcChannel) instance. To join multiple channels, create multiple [AgoraRtcChannel](AgoraRtcChannel) objects.      |
| [AgoraRtcChannel](AgoraRtcChannel) | Provides methods that enable real-time communications in a specified channel.  |
| [AgoraRtcChannelDelegate](AgoraRtcChannelDelegate) | Provides callbacks that report events and statistics in a specified channel.      |

## <a name="screensharing"></a>Screen sharing

> This group of methods is for macOS only.

| Method                                                       | Function                           |
| :----------------------------------------------------------- | :--------------------------------- |
| [startScreenCaptureByDisplayId]([AgoraRtcEngineKit.startScreenCaptureByDisplayId:rectangle:parameters:} | Shares the whole or part of a screen by specifying the display ID.      |
| [startScreenCaptureByWindowId]([AgoraRtcEngineKit.startScreenCaptureByWindowId:rectangle:parameters:} | Shares the whole or part of a window by specifying the window ID.  |
| [setScreenCaptureContentHint]([AgoraRtcEngineKit.setScreenCaptureContentHint:} | Sets the content hint for screen sharing.       |
| [updateScreenCaptureParameters]([AgoraRtcEngineKit.updateScreenCaptureParameters:} | Updates the screen sharing parameters.  |
| [updateScreenCaptureRegion]([AgoraRtcEngineKit.updateScreenCaptureRegion:} | Updates the screen-sharing region. |
| [stopScreenCapture]([AgoraRtcEngineKit.stopScreenCapture}   | Stops screen sharing.              |
-->

## <a name="audiovolumeindication"></a>Audio volume indication

| Method                                                       | Function                                               |
| :----------------------------------------------------------- | :----------------------------------------------------- |
| {@link AgoraRtcEngineKit.enableAudioVolumeIndication:smooth: enableAudioVolumeIndication} | Enables the SDK to regularly report to the application on which users are speaking and the speakers' volume. |

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:reportAudioVolumeIndicationOfSpeakers:totalVolume: reportAudioVolumeIndicationOfSpeakers} | Reports which users are speaking and the speakers' volume at the moment.  |

## <a name="audioroutecontrol"></a>Audio route control

> This group of methods is for iOS only.

| Method                                                       | Function                                                |
| :----------------------------------------------------------- | :------------------------------------------------------ |
| {@link AgoraRtcEngineKit.setDefaultAudioRouteToSpeakerphone: setDefaultAudioRouteToSpeakerphone} | Sets the default audio route.                           |
| {@link AgoraRtcEngineKit.setEnableSpeakerphone: setEnableSpeakerphone} | Enables/Disables the audio route to the speakerphone. |
| {@link AgoraRtcEngineKit.isSpeakerphoneEnabled isSpeakerphoneEnabled} | Checks whether the speakerphone is enabled.            |

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:didAudioRouteChanged: didAudioRouteChanged} | Occurs when the local audio route changes.                   |

## Music file playback and mixing

| Method                                                       | Function                                                |
| :----------------------------------------------------------- | :------------------------------------------------------ |
| {@link AgoraRtcEngineKit.startAudioMixing:loopback:replace:cycle: startAudioMixing} | Starts playing and mixing the music file.               |
| {@link AgoraRtcEngineKit.stopAudioMixing stopAudioMixing}       | Stops playing and mixing the music file.                |
| {@link AgoraRtcEngineKit.pauseAudioMixing pauseAudioMixing}     | Pauses playing and mixing the music file.               |
| {@link AgoraRtcEngineKit.resumeAudioMixing resumeAudioMixing}   | Resumes playing and mixing the music file.              |
| {@link AgoraRtcEngineKit.adjustAudioMixingVolume: adjustAudioMixingVolume} | Adjusts the volume of audio mixing.                     |
| {@link AgoraRtcEngineKit.getAudioMixingDuration getAudioMixingDuration} | Gets the duration (ms) of the music file.               |
| {@link AgoraRtcEngineKit.getAudioMixingCurrentPosition getAudioMixingCurrentPosition} | Gets the playback position (ms) of the music file.      |
| {@link AgoraRtcEngineKit.setAudioMixingPosition: setAudioMixingPosition} | Sets the playback position of the music file.           |

| Event                                                     | Description                                           |
| :----------------------------------------------------------- | :---------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:audioMixingStateChanged:errorType: audioMixingStateChanged} | Occurs when the state of the local user's audio mixing file changes. |

## Audio effect file playback

| Method                                                       | Function                                                     |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineKit.getEffectsVolume getEffectsVolume}     | Gets the volume of the audio effects.                        |
| {@link AgoraRtcEngineKit.setEffectsVolume: setEffectsVolume}     | Sets the volume of the audio effects.                       |
| {@link AgoraRtcEngineKit.setVolumeOfEffect:withVolume: setVolumeOfEffect} | Sets the volume of a specified audio effect.                 |
| {@link AgoraRtcEngineKit.playEffect:filePath:loopCount:pitch:pan:gain:publish: playEffect} | Plays a specified audio effect.                              |
| {@link AgoraRtcEngineKit.stopEffect: stopEffect}                 | Stops playing a specified audio effect.                      |
| {@link AgoraRtcEngineKit.stopAllEffects stopAllEffects}         | Stops playing all audio effects.                             |
| {@link AgoraRtcEngineKit.preloadEffect:filePath: preloadEffect} | Preloads a specified audio effect file into the memory.      |
| {@link AgoraRtcEngineKit.unloadEffect: unloadEffect}            | Releases a specified preloaded audio effect from the memory. |
| {@link AgoraRtcEngineKit.pauseEffect: pauseEffect}              | Pauses a specified audio effect.                             |
| {@link AgoraRtcEngineKit.pauseAllEffects pauseAllEffects}       | Pauses all audio effects.                                    |
| {@link AgoraRtcEngineKit.resumeEffect: resumeEffect}            | Resumes playing a specified audio effect.                    |
| {@link AgoraRtcEngineKit.resumeAllEffects resumeAllEffects}     | Resumes playing all audio effects.                           |

|    事件       |  功能   |
| :----------------------------------------------------------- | :-------------------------------------------- |
|{@link AgoraRtcEngineDelegate.rtcEngineDidAudioEffectFinish:soundId: rtcEngineDidAudioEffectFinish}| Occurs when the local audio effect playback finishes.  |


## <a name="#voicechanger"></a>Voice changer and reverberation

| Method                                                       | Function                                      |
| :----------------------------------------------------------- | :-------------------------------------------- |
| {@link AgoraRtcEngineKit.setLocalVoiceChanger: setLocalVoiceChanger} | Sets the local voice changer option.           |
| {@link AgoraRtcEngineKit.setLocalVoiceReverbPreset: setLocalVoiceReverbPreset} | Sets the preset local voice reverberation effect. |

## <a name="inearmonitor"></a>In-ear monitor

> This group of methods is for iOS only.

| Method                                                       | Function                               |
| :----------------------------------------------------------- | :------------------------------------- |
| {@link AgoraRtcEngineKit.enableInEarMonitoring: enableInEarMonitoring} | Enables in-ear monitoring.             |
| {@link AgoraRtcEngineKit.setInEarMonitoringVolume: setInEarMonitoringVolume} | Sets the volume of the in-ear monitor. |

<!--
## Sound position indication

| Method                                                       | Function                                      |
| :----------------------------------------------------------- | :-------------------------------------------- |
| [enableSoundPositionIndication]([AgoraRtcEngineKit.enableSoundPositionIndication:} | Enables/Disables stereo panning for remote users. |
| [setRemoteVoicePosition]([AgoraRtcEngineKit.setRemoteVoicePosition:pan:gain:} | Sets the sound position and gain of a remote user.      |

## <a name="cdnpublisher"></a>CDN publisher

> This group of methods is applicable to Interactive Broadcast only.

| Method                                                       | Function                                               |
| :----------------------------------------------------------- | :----------------------------------------------------- |
| [addPublishStreamUrl]([AgoraRtcEngineKit.addPublishStreamUrl:transcodingEnabled:} | Adds a CDN live stream address.                             |
| [removePublishStreamUrl]([AgoraRtcEngineKit.removePublishStreamUrl:} | Removes a CDN live stream address.                          |
| [setLiveTranscoding]([AgoraRtcEngineKit.setLiveTranscoding:} | Sets the video layout and audio settings for CDN live. |

| Delegate                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| [rtmpStreamingChangedToState]([AgoraRtcEngineDelegate.rtcEngine:rtmpStreamingChangedToState:state:errorCode:} | Occurs when the RTMP streaming status changes.       |
| [rtcEngineTranscodingUpdated]([AgoraRtcEngineDelegate.rtcEngineTranscodingUpdated:} | Occurs when the publisher’s transcoding settings are updated. |

## Media stream relay across channels

| Method                                                       | Function                                               |
| :----------------------------------------------------------- | :----------------------------------------------------- |
| [startChannelMediaRelay]([AgoraRtcEngineKit.startChannelMediaRelay:} | Starts to relay media streams across channels.     |
| [updateChannelMediaRelay]([AgoraRtcEngineKit.updateChannelMediaRelay:} | Updates the channels for media stream relay. |
| [stopChannelMediaRelay]([AgoraRtcEngineKit.stopChannelMediaRelay} | Stops the media stream relay. |

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| [channelMediaRelayStateDidChange]([AgoraRtcEngineDelegate.rtcEngine:channelMediaRelayStateDidChange:error:} | Occurs when the state of the media stream relay changes.      |
| [didReceiveChannelMediaRelayEvent]([AgoraRtcEngineDelegate.rtcEngine:didReceiveChannelMediaRelayEvent:} | Reports events during the media stream relay. |  -->

## <a name="lastmiletest"></a>Last-mile test

| Method                                                       | Function                                      |
| :----------------------------------------------------------- | :-------------------------------------------- |
| {@link AgoraRtcEngineKit.startLastmileProbeTest: startLastmileProbeTest} | Starts the last-mile network probe test. |
| {@link AgoraRtcEngineKit.stopLastmileProbeTest stopLastmileProbeTest} | Stops the last-mile network probe test. |

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:lastmileProbeTestResult: lastmileProbeTestResult} | Reports the last-mile network probe result. |

## <a name="cameracontrol"></a>Camera control

> This group of methods is for iOS only.

| Method                                                       | Function                                                     |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineKit.switchCamera switchCamera}             | Switches between front and rear cameras.                 |
<!-- | [isCameraZoomSupported]([AgoraRtcEngineKit.isCameraZoomSupported} | Checks whether the camera zoom function is supported.        |
| [isCameraTorchSupported]([AgoraRtcEngineKit.isCameraTorchSupported} | Checks whether the camera flash function is supported.       |
| [isCameraFocusPositionInPreviewSupported]([AgoraRtcEngineKit.isCameraFocusPositionInPreviewSupported} | Checks whether the camera manual focus function is supported. |
| [isCameraExposurePositionSupported]([AgoraRtcEngineKit.isCameraExposurePositionSupported} | Checks whether the camera manual exposure function is supported. |
| [isCameraAutoFocusFaceModeSupported]([AgoraRtcEngineKit.isCameraAutoFocusFaceModeSupported} | Checks whether the camera auto-face focus function is supported. |
| [setCameraZoomFactor]([AgoraRtcEngineKit.setCameraZoomFactor:} | Sets the camera zoom ratio.                                  |
| [setCameraFocusPositionInPreview]([AgoraRtcEngineKit.setCameraFocusPositionInPreview:} | Sets the manual focus position.                              |
| [setCameraExposurePosition]([AgoraRtcEngineKit.setCameraExposurePosition:} | Sets the manual exposure position.                           |
| [setCameraTorchOn]([AgoraRtcEngineKit.setCameraTorchOn:}    | Enables the camera flash function.                           |
| [setCameraAutoFocusFaceModeEnabled]([AgoraRtcEngineKit.setCameraAutoFocusFaceModeEnabled:} | Enables the camera auto-face focus function.                 |

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| [cameraFocusDidChangedToRect]([AgoraRtcEngineDelegate.rtcEngine:cameraFocusDidChangedToRect:} | Occurs when a camera focus area changes. (iOS only)          |
| [cameraExposureDidChangedToRect]([AgoraRtcEngineDelegate.rtcEngine:cameraExposureDidChangedToRect:} | Occurs when a camera exposure area changes. (iOS only)       | -->

## <a name="dualvideostream"></a>Dual video stream

| Method                                                       | Function                                                     |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineKit.enableDualStreamMode: enableDualStreamMode} | Enables/Disables the dual-stream mode.                       |
| {@link AgoraRtcEngineKit.setRemoteDefaultVideoStreamType: setRemoteDefaultVideoStreamType} | Sets the default video stream type of the remote stream. |
<!-- | [setRemoteVideoStream]([AgoraRtcEngineKit.setRemoteVideoStream:type:} | Sets the video stream type of the remote stream.         |

## Stream fallback

| Method                                                       | Function                                                     |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| [setRemoteUserPriority]([AgoraRtcEngineKit.setRemoteUserPriority:type:} | Sets the priority of a remote user's stream.     |
| [setLocalPublishFallbackOption]([AgoraRtcEngineKit.setLocalPublishFallbackOption:} | Sets the fallback option for the published stream under unreliable network conditions. |
| [setRemoteSubscribeFallbackOption]([AgoraRtcEngineKit.setRemoteSubscribeFallbackOption:} | Sets the fallback option for the remote stream under unreliable network conditions. |

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| [didLocalPublishFallbackToAudioOnly]([AgoraRtcEngineDelegate.rtcEngine:didLocalPublishFallbackToAudioOnly:} | Occurs when the published video stream falls back to an audio-only stream, or when it switches back to the video stream. |
| [didRemoteSubscribeFallbackToAudioOnly]([AgoraRtcEngineDelegate.rtcEngine:didRemoteSubscribeFallbackToAudioOnly:byUid:} | Occurs when the remote video stream falls back to an audio-only stream, or when it switches back to the video stream. | -->

<!-- ## Custom video module

| Method                                                       | Function                        |
| :----------------------------------------------------------- | :------------------------------ |
| [setVideoSource]([AgoraRtcEngineKit.setVideoSource:}        | Sets the video source.          |
| [setLocalVideoRenderer]([AgoraRtcEngineKit.setLocalVideoRenderer:} | Sets the local video renderer.  |
| [setRemoteVideoRenderer]([AgoraRtcEngineKit.setRemoteVideoRenderer:forUserId:} | Sets the remote video renderer. |
| [videoSource]([AgoraRtcEngineKit.videoSource}               | Gets the video source.          |
| [localVideoRenderer]([AgoraRtcEngineKit.localVideoRenderer} | Gets the local video renderer.  |
| [remoteVideoRendererOfUserId]([AgoraRtcEngineKit.remoteVideoRendererOfUserId:} | Gets the remote video renderer. | -->

<!-- ## <a name="externalaudiodata"></a>External audio data (push-mode only)

| Method                                                       | Function                                        |
| :----------------------------------------------------------- | :---------------------------------------------- |
| [enableExternalAudioSourceWithSampleRate]([AgoraRtcEngineKit.enableExternalAudioSourceWithSampleRate:channelsPerFrame:} | Enables the external audio source.              |
| [disableExternalAudioSource]([AgoraRtcEngineKit.disableExternalAudioSource} | Disables the external audio source.             |
| [pushExternalAudioFrameRawData]([AgoraRtcEngineKit.pushExternalAudioFrameRawData:samples:timestamp:} | Pushes the external raw audio frame data.       |
| [pushExternalAudioFrameSampleBuffer]([AgoraRtcEngineKit.pushExternalAudioFrameSampleBuffer:} | Pushes the external CMSampleBuffer audio frame. |

 ## External audio sink

| Method                                                       | Function                              |
| :----------------------------------------------------------- | :------------------------------------ |
| [enableExternalAudioSink]([AgoraRtcEngineKit.enableExternalAudioSink:channels:} | Enables the external audio sink. |
| [disableExternalAudioSink]([AgoraRtcEngineKit.disableExternalAudioSink} | Disables the external audio sink. |
| [pullPlaybackAudioFrameRawData]([AgoraRtcEngineKit.pullPlaybackAudioFrameRawData:lengthInByte:} | Pulls the remote audio data in the RawData format. |
| [pullPlaybackAudioFrameSampleBufferByLengthInByte]([AgoraRtcEngineKit.pullPlaybackAudioFrameSampleBufferByLengthInByte:} | Pulls the remote audio data in the SampleBuffer format. | -->

## <a name="externalvideodata"></a>External video data (push-mode only)

| Method                                                       | Function                              |
| :----------------------------------------------------------- | :------------------------------------ |
| {@link AgoraRtcEngineKit.setExternalVideoSource:useTexture:pushMode: setExternalVideoSource} | Configures the external video source. |
| {@link AgoraRtcEngineKit.pushExternalVideoFrame: pushExternalVideoFrame} | Pushes the external video frame.      |


## <a name="rawaudiodata"></a>Raw audio data

> You can use the C++ APIs to implement this function, see [C++ Raw Audio Data](../../../cpp/index.html#rawaudio).

## <a name="metadata"></a>Media metadata

This group of methods is applicable to the live interactive streaming only.

> Do not implement {@link AgoraMediaMetadataDataSource.metadataMaxSize metadataMaxSize}, {@link AgoraMediaMetadataDataSource.readyToSendMetadataAtTimestamp: readyToSendMetadataAtTimestamp}, and{@link AgoraMediaMetadataDelegate.receiveMetadata:fromUser:atTimestamp: receiveMetadata} in {@link AgoraRtcEngineDelegate}.

| Method                                                         | Function               |
| :----------------------------------------------------------- | :----------------- |
| {@link AgoraRtcEngineKit.setMediaMetadataDataSource:withType: setMediaMetadataDataSource} | Sets the data source of the media metadata. |
| {@link AgoraRtcEngineKit.setMediaMetadataDelegate:withType: setMediaMetadataDelegate} |  Sets the delegate of the metadata. |

| Event                                                         | Description                   |
| :----------------------------------------------------------- | :--------------------- |
| {@link AgoraMediaMetadataDataSource.metadataMaxSize metadataMaxSize} | Occurs when the SDK requests the maximum size of the metadata.              |
| {@link AgoraMediaMetadataDataSource.readyToSendMetadataAtTimestamp: readyToSendMetadataAtTimestamp} | Occurs when the sender is ready to send the metadata. |
| {@link AgoraMediaMetadataDelegate.receiveMetadata:fromUser:atTimestamp: receiveMetadata} | Occurs when the receiver receives the metadata. |

## <a name="rawaudiodata"></a> Raw audio data

## Audio recorder

| Method                                                       | Function                                 |
| :----------------------------------------------------------- | :--------------------------------------- |
| [startAudioRecording]([AgoraRtcEngineKit.startAudioRecording:sampleRate:quality:} | Starts an audio recording on the client. |
| [stopAudioRecording]([AgoraRtcEngineKit.stopAudioRecording} | Stops an audio recording on the client.  |

## <a name="datastream"></a>Data stream

| Method                                                       | Function                    |
| :----------------------------------------------------------- | :-------------------------- |
| {@link AgoraRtcEngineKit.createDataStream:reliable:ordered: createDataStream} | Creates a data stream.      |
| {@link AgoraRtcEngineKit.sendStreamMessage:data: sendStreamMessage} | Sends data stream messages. |

| Event                                                     | Description                                               |
| :----------------------------------------------------------- | :-------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:receiveStreamMessageFromUid:streamId:data: receiveStreamMessageFromUid} | Occurs when the local user receives the data stream from a remote user within five seconds.      |
| {@link AgoraRtcEngineDelegate.rtcEngine:didOccurStreamMessageErrorFromUid:streamId:error:missed:cached: didOccurStreamMessageErrorFromUid} | Occurs when the local user fails to receive the data stream from the remote user within five seconds. |

## <a name="miscmethods"></a>Miscellaneous methods

| Method                                                       | Function                                                     |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineKit.getCallId getCallId}                   | Gets the current call ID.                                    |
| {@link AgoraRtcEngineKit.rate:rating:description: rate}            | Allows a user to rate a call after the call ends.            |
| {@link AgoraRtcEngineKit.complain:description: complain}           | Allows a user to complain about the call quality after a call ends. |
| {@link AgoraRtcEngineKit.getSdkVersion getSdkVersion}           | Gets the Agora SDK version.                                  |
| {@link AgoraRtcEngineKit.getErrorDescription: getErrorDescription}           | Retrieves the description of a warning or error code.                                  |
| {@link AgoraRtcEngineKit.setLogFile: setLogFile}                | Specifies an SDK output log file.                            |
| {@link AgoraRtcEngineKit.setLogFilter: setLogFilter}            | Sets the output log filter level of the SDK.                        |
| {@link AgoraRtcEngineKit.setLogLevel: setLogLevel}            | Sets the output log level of the SDK.                        |
| {@link AgoraRtcEngineKit.sendCustomReportMessage:category:event:label:value: sendCustomReportMessage} | Reports and analyzes customized messages. |



## <a name="miscevents"></a>Miscellaneous events

| Event                                                     | Description                                                  |
| :----------------------------------------------------------- | :----------------------------------------------------------- |
| {@link AgoraRtcEngineDelegate.rtcEngine:didApiCallExecute:api:result: didApiCallExecute} | Occurs when an API method is executed.                       |
