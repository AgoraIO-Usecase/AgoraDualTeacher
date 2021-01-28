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

API (2020-09-04)
==================================================

AgoraBase.h
-------------
**Modified:**
Refine AGORA_API definitions to keep aligned with agora_api.h

AgoraRefPtr.h
-------------
**Modified:**
Add operator*().

IAgoraLog.h
-------------
**Modified:**
Add log related constants.

IAgoraMediaPlayerSource.h
-------------
**Modified:**
Change the 'int' parameters to 'int64_t' to keep aligned with the other APIs.


API (2020-09-09)
==================================================
Add enum NETWORK_TYPE.

AgoraBase.h
-------------
**Add:**
- enum NETWORK_TYPE


API (2020-09-04)
==================================================
Add more methods to IVideoFrameObserver.

AgoraMediaBase.h
-------------
**Modified:**
- Changes Add getVideoFormatPreference/getRotationApplied/getMirrorApplied to IVideoFrameObserver


API (2020-09-03)
==================================================

AgoraBase.h
-------------
**Modified:**
add members of RtcStats
- packetsBeforeFirstKeyFramePacket
- firstAudioPacketDurationAfterUnmute
- firstVideoPacketDurationAfterUnmute
- firstVideoKeyFramePacketDurationAfterUnmute
- firstVideoKeyFrameDecodedDurationAfterUnmute
- firstVideoKeyFrameRenderedDurationAfterUnmute


API (2020-08-19)
==================================================

AgoraMediaBase.h
-------------
**Add:** OpenGL type
- enum EGL_CONTEXT_TYPE

**Add:** Video Buffer types
- VIDEO_BUFFER_TYPE::VIDEO_BUFFER_ARRAY
- VIDEO_BUFFER_TYPE::VIDEO_BUFFER_TEXTURE

**Add:** Texture related parameter
- ExternalVideoFrame::eglContext
- ExternalVideoFrame::eglType
- ExternalVideoFrame::textureId


API (2020-08-04)
==================================================

AgoraBase.h
-------------
**Add:**
- AUDIO_CODEC_TYPE::AUDIO_CODEC_G722
- AUDIO_PROFILE_TYPE::AUDIO_PROFILE_IOT


API (2020-07-29)
==================================================

AgoraMediaBase.h
-------------
Add `struct AudioParameters`
Rename some fields of `struct AudioPcmFrame`


API (2020-07-14)
==================================================
IAgoraMediaPlayerSource.h
-------------
**Modify:**
Change include dir.


API (2020-07-06)
==================================================
AgoraBase.h
-------------
**Modify:**
Change include dir.


API (2020-07-01)
==================================================
AgoraBase.h
-------------
**Modify:**
Remove string UID.


API (2020-06-27)
==================================================
Make include path correct


API (2020-06-18)
==================================================
Modify EncodedVideoFrameInfo.

AgoraBase.h
-------------
**Modify:**
Modify EncodedVideoFrameInfo.
- Remove packetizationMode.


API (2020-06-17)
Change namespace to avoid confliect with media_server_library

AgoraOptional.h
-------------
**Modified:**
Change namespace
- Changes base::Optional to base_utils::Optional


API (2020-06-17)
==================================================
Fix header file macro

AgoraOptional.h
-------------
**Add:**
Add string_value to Optional
- Optional::string_value()

**Modified:**
- `BASE_OPTIONAL_H_` to `AGORA_OPTIONAL_H_`


API (2020-05-29)
==================================================
Updated docs for the following header files:
- AgoraBase.h
- AgoraMediaBase.h


API (2020-06-22)
==================================================

IAgoraMediaPlayerSource.h
-------------
**Modified:**
- setLooping(bool looping) to setLoopCount(int loopCount)


API (2020-06-10)
==================================================
Add NetworkInfo for feedback to user

AgoraBase.h
-------------
**Add:**
Add NetworkInfo
- struct NetworkInfo


API (2020-06-08)
==================================================

IAgoraMediaPlayerSource.h
-------------
**Add:**
Add new Api in Media Player
- takeScreenshot()
- selectInternalSubtitle()
- setExternalSubtitle()


API (2020-06-08)
==================================================

AgoraBase.h
-------------
**Modified:**
- REMOTE_VIDEO_STREAM_STATE rename to REMOTE_VIDEO_STATE
- Changes REMOTE_VIDEO_STATE_PLAYING to REMOTE_VIDEO_STATE_DECODING

REMOTE_VIDEO_STATE_REASON
- REMOTE_VIDEO_STATE_REASON_INTERNAL
- REMOTE_VIDEO_STATE_REASON_NETWORK_CONGESTION
- REMOTE_VIDEO_STATE_REASON_NETWORK_RECOVERY
- REMOTE_VIDEO_STATE_REASON_LOCAL_MUTED
- REMOTE_VIDEO_STATE_REASON_LOCAL_UNMUTED
- REMOTE_VIDEO_STATE_REASON_REMOTE_MUTED
- REMOTE_VIDEO_STATE_REASON_REMOTE_UNMUTED
- REMOTE_VIDEO_STATE_REASON_REMOTE_OFFLINE
- REMOTE_VIDEO_STATE_REASON_AUDIO_FALLBACK
- REMOTE_VIDEO_STATE_REASON_AUDIO_FALLBACK_RECOVERY


API (2020-06-03)
=========================

Move IAgoraMediaPlayerSource.h from api2 to here, and add getSourceId() for IMediaPlayerSource

**Add:**
AgoraMediaBase.h
-------------
Add enum MEDIA_PLAYER_SOURCE_TYPE.



API (2020-05-31)
=========================

AgoraMediaBase.h
-------------
**Add:**
- Add enum MEDIA_PLAYER_PLAYBACK_SPEED.

IAgoraMeidaPlayerSource.h
-------------
**Add:**
Add new Api in Media Player
- changePlaybackSpeed()
- selectAudioTrack()
- setPlayerOption()

AgoraMeidaPlayerTypes.h
-------------
**Add:**
Move Media Player related declaration from AgoraMediaBase.h to AgoraMeidaPlayerTypes.h
- enum MEDIA_PLAYER_STATE
- enum MEDIA_PLAYER_ERROR
- enum MEDIA_PLAYER_PLAYBACK_SPEED
- enum MEDIA_STREAM_TYPE
- enum MEDIA_PLAYER_EVENT
- struct MediaStreamInfo
- enum MEDIA_PLAYER_METADATA_TYPE

AgoraMediaBase.h
-------------
**Delete:**
Move Media Player related declaration from AgoraMediaBase.h to AgoraMeidaPlayerTypes.h
- enum MEDIA_PLAYER_STATE
- enum MEDIA_PLAYER_ERROR
- enum MEDIA_PLAYER_PLAYBACK_SPEED
- enum MEDIA_STREAM_TYPE
- enum MEDIA_PLAYER_EVENT
- struct MediaStreamInfo
- enum MEDIA_PLAYER_METADATA_TYPE


API (2020-06-09)
==================================================
Modify PacketOptions.

AgoraMediaBase.h
-------------
**Modified:**
Add audioLevelIndication into PacketOptions.
- PacketOptions: Add member audioLevelIndication


API (2020-05-28)
==================================================
Modify VideoEncoderConfiguration and VIDEO_CODEC_TYPE

AgoraBase.h
-------------
**Modified:**
- Add member enableGenericH264 for VideoEncoderConfiguration
- Add new type VIDEO_CODEC_GENERIC(6) for enum VIDEO_CODEC_TYPE
- Add new type VIDEO_FRAME_TYPE_DROPPABLE_FRAME(6) for enum VIDEO_FRAME_TYPE


API (2020-05-22)
=========================
**Add:**
AgoraMediaBase.h
-------------
Add enum VIDEO_OBSERVER_POSITION.


API (2020-05-12)
=========================

**Add:**
AgoraMediaBase.h
-------------
Add data member videoRotation in struct MediaStreamInfo.

API (2020-05-12)
==================================================
Add lastmile and connection state enum

AgoraBase.h
-------------
**Add:**
Add lastmile and connection state enum
- enum LASTMILE_PROBE_RESULT_STATE
- struct LastmileProbeOneWayResult
- struct LastmileProbeResult
- struct LastmileProbeConfig
- enum CONNECTION_CHANGED_REASON_TYPE
- struct LastmileProbeConfig
- enum AUDIO_REVERB_PRESET
- struct ScreenCaptureParameters
- enum VOICE_CHANGER_PRESET


API (2020-05-09)

AgoraBase.h
--------------------
**Add:**
Add AUDIO_SCENARIO_HIGH_DEFINITION for AUDIO_SCENARIO_TYPE
