//
// AgoraEnumerates.h
// AgoraRtcEngineKit
//
// Copyright (c) 2018 Agora. All rights reserved.
//

#import <Foundation/Foundation.h>

/**
 * Warning code.
 *
 * Warning codes occur when the SDK encounters an error that may be recovered automatically.
 * These are only notifications, and can generally be ignored. For example, when the SDK loses connection to the server, the SDK reports the AgoraWarningCodeOpenChannelTimeout(106) warning and tries to reconnect automatically.
 */
typedef NS_ENUM(NSInteger, AgoraWarningCode) {
    /** 8: The specified view is invalid. Specify a view when using the video call function. */
    AgoraWarningCodeInvalidView = 8,
    /** 16: Failed to initialize the video function, possibly caused by a lack of resources. The users cannot see the video while the voice communication is not affected. */
    AgoraWarningCodeInitVideo = 16,
     /** 20: The request is pending, usually due to some module not being ready, and the SDK postpones processing the request. */
    AgoraWarningCodePending = 20,
    /** 103: No channel resources are available. Maybe because the server cannot allocate any channel resource. */
    AgoraWarningCodeNoAvailableChannel = 103,
    /** 104: A timeout occurs when looking up the channel. When joining a channel, the SDK looks up the specified channel. The warning usually occurs when the network condition is too poor for the SDK to connect to the server. */
    AgoraWarningCodeLookupChannelTimeout = 104,
    /** 105: The server rejects the request to look up the channel. The server cannot process this request or the request is illegal. */
    AgoraWarningCodeLookupChannelRejected = 105,
    /** 106: The server rejects the request to look up the channel. The server cannot process this request or the request is illegal. */
    AgoraWarningCodeOpenChannelTimeout = 106,
    /** 107: The server rejects the request to open the channel. The server cannot process this request or the request is illegal. */
    AgoraWarningCodeOpenChannelRejected = 107,
    /** 111: A timeout occurs when switching to the live video. */
    AgoraWarningCodeSwitchLiveVideoTimeout = 111,
    /** 118: A timeout occurs when setting the client role in the live broadcast profile. */
    AgoraWarningCodeSetClientRoleTimeout = 118,
    /** 119: The client role is unauthorized. */
    AgoraWarningCodeSetClientRoleNotAuthorized = 119,
    /** 121: The ticket to open the channel is invalid. */
    AgoraWarningCodeOpenChannelInvalidTicket = 121,
    /** 122: Try connecting to another server. */
    AgoraWarningCodeOpenChannelTryNextVos = 122,
    /** 701: An error occurs in opening the audio mixing file. */
    AgoraWarningCodeAudioMixingOpenError = 701,
    /** 1014: Audio Device Module: a warning occurs in the playback device. */
    AgoraWarningCodeAdmRuntimePlayoutWarning = 1014,
    /** 1016: Audio Device Module: a warning occurs in the recording device. */
    AgoraWarningCodeAdmRuntimeRecordingWarning = 1016,
    /** 1019: Audio Device Module: no valid audio data is collected. */
    AgoraWarningCodeAdmRecordAudioSilence = 1019,
    /** 1020: Audio Device Module: a playback device fails. */
    AgoraWarningCodeAdmPlaybackMalfunction = 1020,
    /** 1021: Audio Device Module: a recording device fails. */
    AgoraWarningCodeAdmRecordMalfunction = 1021,
    /** 1025: Call is interrupted by system events such as phone call or Siri. */
    AgoraWarningCodeAdmInterruption = 1025,
    /** 1031: Audio Device Module: the recorded audio is too low. */
    AgoraWarningCodeAdmRecordAudioLowlevel = 1031,
    /** 1032: Audio Device Module: the playback audio is too low. */
    AgoraWarningCodeAdmPlayoutAudioLowlevel = 1032,
    /** 1051: Audio Device Module: howling is detected. */
    AgoraWarningCodeApmHowling = 1051,
    /** 1052: Audio Device Module: the device is in the glitch state. */
    AgoraWarningCodeAdmGlitchState = 1052,
    /** 1053: Audio Device Module: the underlying audio settings have changed. */
    AgoraWarningCodeAdmImproperSettings = 1053,
};

/**
 * Error code.
 *
 * Error codes occur when the SDK encounters an error that cannot be recovered automatically without any app intervention.
 * For example, the SDK reports the {@link AgoraErrorCodeStartCall}(1002) error if it fails to start a call.
 */
typedef NS_ENUM(NSInteger, AgoraErrorCode) {
    /** 0: No error occurs. */
    AgoraErrorCodeNoError = 0,
    /** 1: A general error occurs (no specified reason). */
    AgoraErrorCodeFailed = 1,
    /** 2: An invalid parameter is used. For example, the specific channel name includes illegal characters. */
    AgoraErrorCodeInvalidArgument = 2,
    /**
     * 3: The SDK module is not ready.
     *
     * Possible solutions：
     * - Check the audio device.
     * - Check the completeness of the app.
     * - Re-initialize the SDK.
     */
    AgoraErrorCodeNotReady = 3,
    /** 4: The current state of the SDK does not support this function. */
    AgoraErrorCodeNotSupported = 4,
    /** 5: The request is rejected. This is for internal SDK use only, and is not returned to the app through any method or callback. */
    AgoraErrorCodeRefused = 5,
    /** 6: The buffer size is not big enough to store the returned data. */
    AgoraErrorCodeBufferTooSmall = 6,
    /** 7: The SDK is not initialized before calling this method. */
    AgoraErrorCodeNotInitialized = 7,
    /** 9: No permission exists. Check if the user has granted access to the audio or video device. */
    AgoraErrorCodeNoPermission = 9,
    /** 10: An API method timeout occurs. Some API methods require the SDK to return the execution result, and this error occurs if the request takes too long (over 10 seconds) for the SDK to process. */
    AgoraErrorCodeTimedOut = 10,
    /** 11: The request is canceled. This is for internal SDK use only, and is not returned to the app through any method or callback. */
    AgoraErrorCodeCanceled = 11,
    /** 12: The method is called too often. This is for internal SDK use only, and is not returned to the app through any method or callback. */
    AgoraErrorCodeTooOften = 12,
    /** 13: The SDK fails to bind to the network socket. This is for internal SDK use only, and is not returned to the app through any method or callback. */
    AgoraErrorCodeBindSocket = 13,
    /** 14: The network is unavailable. This is for internal SDK use only, and is not returned to the app through any method or callback. */
    AgoraErrorCodeNetDown = 14,
    /** 15: No network buffers are available. This is for internal SDK use only, and is not returned to the app through any method or callback. */
    AgoraErrorCodeNoBufs = 15,
    /**
     * 17: The request to join the channel is rejected.
     *
     * Possible reasons:
     * - The user is already in the channel, and still calls the API method to join the channel, for example `joinChannelByToken`.
     * - The user tries joining the channel during the echo test. Please join the channel after the echo test ends.
    */
    AgoraErrorCodeJoinChannelRejected = 17,
    /**
     * 18: The request to leave the channel is rejected.
     *
     * Possible reasons are:
     * - The user left the channel and still calls the API method to leave the channel, for example, {@link AgoraRtcEngineKit.leaveChannel: leaveChannel}.
     * - The user has not joined the channel and calls the API method to leave the channel.
     */
    AgoraErrorCodeLeaveChannelRejected = 18,
    /** 19: The resources are occupied and cannot be used. */
    AgoraErrorCodeAlreadyInUse = 19,
    /** 20: The SDK gave up the request due to too many requests.  */
    AgoraErrorCodeAbort = 20,
    /** 21: In Windows, specific firewall settings cause the SDK to fail to initialize and crash. */
    AgoraErrorCodeInitNetEngine = 21,
    /** 22: The app uses too much of the system resources and the SDK fails to allocate the resources. */
    AgoraErrorCodeResourceLimited = 22,
    /** 101: The specified App ID is invalid. Please try to rejoin the channel with a valid App ID.*/
    AgoraErrorCodeInvalidAppId = 101,
    /** 102: The specified channel name is invalid. Please try to rejoin the channel with a valid channel name. */
    AgoraErrorCodeInvalidChannelId = 102,
    /**
     * 109: The token expired.
     *
     * Possible reasons are:
     * - Authorized Timestamp expired: The timestamp is represented by the number of seconds elapsed since 1/1/1970. The user can use the token to access the Agora service within 24 hours after the token is generated. If the user does not access the Agora service after 24 hours, this token is no longer valid.
     * - Call Expiration Timestamp expired: The timestamp is the exact time when a user can no longer use the Agora service (for example, when a user is forced to leave an ongoing call). When a value is set for the Call Expiration Timestamp, it does not mean that the token will expire, but that the user will be banned from the channel.
     */
    AgoraErrorCodeTokenExpired = 109,
    /**
     * 110: The token is invalid.
     *
     * Possible reasons are:
     * - The App Certificate for the project is enabled in Console, but the user is using the App ID. Once the App Certificate is enabled, the user must use a token.
     * - The uid is mandatory, and users must set the same uid as the one set in the `joinChannelByToken` method.
     */
    AgoraErrorCodeInvalidToken = 110,
    /** 111: The Internet connection is interrupted. This applies to the Agora Web SDK only. */
    AgoraErrorCodeConnectionInterrupted = 111,
    /** 112: The Internet connection is lost. This applies to the Agora Web SDK only. */
    AgoraErrorCodeConnectionLost = 112,
    /** 113: The user is not in the channel when calling the `sendStreamMessage` or `getUserInfoByUserAccount` method. */
    AgoraErrorCodeNotInChannel = 113,
    /** 114: The size of the sent data is over 1024 bytes when the user calls the `sendStreamMessage` method. */
    AgoraErrorCodeSizeTooLarge = 114,
    /** 115: The bitrate of the sent data exceeds the limit of 6 Kbps when the user calls the `sendStreamMessage` method. */
    AgoraErrorCodeBitrateLimit = 115,
    /** 116: Too many data streams (over five streams) are created when the user calls the `createDataStream` method. */
    AgoraErrorCodeTooManyDataStreams = 116,
    /** 120: Decryption fails. The user may have used a different encryption password to join the channel. Check your settings or try rejoining the channel. */
    AgoraErrorCodeDecryptionFailed = 120,
    /** 124: Incorrect watermark file parameter. */
    AgoraErrorCodeWatermarkParam = 124,
    /** 125: Incorrect watermark file path. */
    AgoraErrorCodeWatermarkPath = 125,
    /** 126: Incorrect watermark file format. */
    AgoraErrorCodeWatermarkPng = 126,
    /** 127: Incorrect watermark file information. */
    AgoraErrorCodeWatermarkInfo = 127,
    /** 128: Incorrect watermark file data format. */
    AgoraErrorCodeWatermarkAGRB = 128,
    /** 129: An error occurs in reading the watermark file. */
    AgoraErrorCodeWatermarkRead = 129,
    /** 130: The encrypted stream is not allowed to publish. */
    AgoraErrorCodeEncryptedStreamNotAllowedPublish = 130,
    /** Stream publishing failed. */
    AgoraErrorCodePublishFailed = 150,
    /** 1001: Fails to load the media engine. */
    AgoraErrorCodeLoadMediaEngine = 1001,
    /** 1002: Fails to start the call after enabling the media engine. */
    AgoraErrorCodeStartCall = 1002,
    /** 1003: Fails to start the camera. */
    AgoraErrorCodeStartCamera = 1003,
    /** 1004: Fails to start the video rendering module. */
    AgoraErrorCodeStartVideoRender = 1004,
    /** 1005: A general error occurs in the Audio Device Module (the reason is not classified specifically). Check if the audio device is used by another app, or try rejoining the channel. */
    AgoraErrorCodeAdmGeneralError = 1005,
    /** 1006: Audio Device Module: An error occurs in using the Java resources. */
    AgoraErrorCodeAdmJavaResource = 1006,
    /** 1007: Audio Device Module: An error occurs in setting the sampling frequency. */
    AgoraErrorCodeAdmSampleRate = 1007,
    /** 1008: Audio Device Module: An error occurs in initializing the playback device. */
    AgoraErrorCodeAdmInitPlayout = 1008,
    /** 1009: Audio Device Module: An error occurs in starting the playback device. */
    AgoraErrorCodeAdmStartPlayout = 1009,
    /** 1010: Audio Device Module: An error occurs in stopping the playback device. */
    AgoraErrorCodeAdmStopPlayout = 1010,
    /** 1011: Audio Device Module: An error occurs in initializing the recording device. */
    AgoraErrorCodeAdmInitRecording = 1011,
    /** 1012: Audio Device Module: An error occurs in starting the recording device. */
    AgoraErrorCodeAdmStartRecording = 1012,
    /** 1013: Audio Device Module: An error occurs in stopping the recording device. */
    AgoraErrorCodeAdmStopRecording = 1013,
    /** 1015: Audio Device Module: A playback error occurs. Check your playback device, or try rejoining the channel. */
    AgoraErrorCodeAdmRuntimePlayoutError = 1015,
    /** 1017: Audio Device Module: A recording error occurs. */
    AgoraErrorCodeAdmRuntimeRecordingError = 1017,
    /** 1018: Audio Device Module: Fails to record. */
    AgoraErrorCodeAdmRecordAudioFailed = 1018,
    /** 1020: Audio Device Module: Abnormal audio playback frequency. */
    AgoraErrorCodeAdmPlayAbnormalFrequency = 1020,
    /** 1021: Audio Device Module: Abnormal audio recording frequency. */
    AgoraErrorCodeAdmRecordAbnormalFrequency = 1021,
    /** 1022: Audio Device Module: An error occurs in initializing the loopback device. */
    AgoraErrorCodeAdmInitLoopback  = 1022,
    /** 1023: Audio Device Module: An error occurs in starting the loopback device. */
    AgoraErrorCodeAdmStartLoopback = 1023,
    /** 1359: No recording device exists. */
    AgoraErrorCodeAdmNoRecordingDevice = 1359,
    /** 1360: No playback device exists. */
    AgoraErrorCodeAdmNoPlayoutDevice = 1360,
    /** 1501: Video Device Module: The camera is unauthorized. */
    AgoraErrorCodeVdmCameraNotAuthorized = 1501,
    /** 1600: Video Device Module: An unknown error occurs. */
    AgoraErrorCodeVcmUnknownError = 1600,
    /** 1601: Video Device Module: An error occurs in initializing the video encoder. */
    AgoraErrorCodeVcmEncoderInitError = 1601,
    /** 1602: Video Device Module: An error occurs in video encoding. */
    AgoraErrorCodeVcmEncoderEncodeError = 1602,
    /** @deprecated
     * 1603: Video Device Module: An error occurs in setting the video encoder.
     */
    AgoraErrorCodeVcmEncoderSetError = 1603,
};

/** @deprecated
 Video Profile */
typedef NS_ENUM(NSInteger, AgoraVideoProfile) {
    /** Invalid profile */
    AgoraVideoProfileInvalid = -1,
    /** 160 x 120  @ 15 fps */
    AgoraVideoProfileLandscape120P = 0,
    /** 120 x 120 @ 15 fps */
    AgoraVideoProfileLandscape120P_3 = 2, // iOS
    /** 320 x 180 @ 15 fps */
    AgoraVideoProfileLandscape180P = 10, // iOS
    /** 180 x 180  @ 15 fps */
    AgoraVideoProfileLandscape180P_3 = 12, // iOS
    /** 240 x 180 @ 15 fps */
    AgoraVideoProfileLandscape180P_4 = 13, // iOS
    /** 320 x 240 @ 15 fps */
    AgoraVideoProfileLandscape240P = 20,
    /** 240 x 240 @ 15 fps */
    AgoraVideoProfileLandscape240P_3 = 22, //iOS
    /** 424 x 240 @ 15 fps */
    AgoraVideoProfileLandscape240P_4 = 23, // iOS
    /** 640 x 360 @ 15 fps */
    AgoraVideoProfileLandscape360P = 30,
    /** 360 x 360 @ 15 fps */
    AgoraVideoProfileLandscape360P_3 = 32, // iOS
    /** 640 x 360 @ 30 fps */
    AgoraVideoProfileLandscape360P_4 = 33,
    /** 360 x 360 @ 30 fps */
    AgoraVideoProfileLandscape360P_6 = 35,
    /** 480 x 360 @ 15 fps */
    AgoraVideoProfileLandscape360P_7 = 36,
    /** 480 x 360 @ 30 fps */
    AgoraVideoProfileLandscape360P_8 = 37,
    /** 640 x 360 @ 15 fps */
    AgoraVideoProfileLandscape360P_9 = 38,
    /** 640 x 360 @ 24 fps */
    AgoraVideoProfileLandscape360P_10 = 39,
    /** 640 x 360 @ 24 fps */
    AgoraVideoProfileLandscape360P_11 = 100,
    /** 640 x 480 @ 15 fps */
    AgoraVideoProfileLandscape480P = 40,
    /** 480 x 480 @ 15 fps */
    AgoraVideoProfileLandscape480P_3 = 42, // iOS
    /** 640 x 480 @ 30 fps */
    AgoraVideoProfileLandscape480P_4 = 43,
     /** 480 x 480 @ 30 fps */
    AgoraVideoProfileLandscape480P_6 = 45,
    /** 848 x 480 @ 15 fps */
    AgoraVideoProfileLandscape480P_8 = 47,
    /** 848 x 480 @ 30 fps */
    AgoraVideoProfileLandscape480P_9 = 48,
    /** 640 x 480 @ 10 fps */
    AgoraVideoProfileLandscape480P_10 = 49,
    /** 1280 x 720 @ 15 fps */
    AgoraVideoProfileLandscape720P = 50,
    /** 1280 x 720 @ 30 fps */
    AgoraVideoProfileLandscape720P_3 = 52,
    /** 960 x 720 @ 15 fps */
    AgoraVideoProfileLandscape720P_5 = 54,
    /** 960 x 720 @ 30 fps */
    AgoraVideoProfileLandscape720P_6 = 55,
    /** 1920 x 1080 @ 15 fps */
    AgoraVideoProfileLandscape1080P = 60,
    /** 1920 x 1080 @ 30 fps */
    AgoraVideoProfileLandscape1080P_3 = 62,
    /** 1920 x 1080 @ 60 fps */
    AgoraVideoProfileLandscape1080P_5 = 64,
    /** 2560 x 1440 @ 30 fps */
    AgoraVideoProfileLandscape1440P = 66,
    /** 2560 x 1440 @ 60 fps */
    AgoraVideoProfileLandscape1440P_2 = 67,
    /** 3840 x 2160 @ 30 fps */
    AgoraVideoProfileLandscape4K = 70,
    /** 3840 x 2160 @ 60 fps */
    AgoraVideoProfileLandscape4K_3 = 72,
    /** 120 x 160 @ 15 fps */
    AgoraVideoProfilePortrait120P = 1000,
    /** 120 x 120 @ 15 fps */
    AgoraVideoProfilePortrait120P_3 = 1002, //iOS
    /** 180 x 320 @ 15 fps */
    AgoraVideoProfilePortrait180P = 1010, // iOS
    /** 180 x 180 @ 15 fps */
    AgoraVideoProfilePortrait180P_3 = 1012, // iOS
    /** 180 x 240 @ 15 fps */
    AgoraVideoProfilePortrait180P_4 = 1013, // iOS
    /** 240 x 320 @ 15 fps */
    AgoraVideoProfilePortrait240P = 1020,
    /** 240 x 240 @ 15 fps */
    AgoraVideoProfilePortrait240P_3 = 1022, // iOS
    /** 240 x 424 @ 15 fps */
    AgoraVideoProfilePortrait240P_4 = 1023,
    /** 360 x 640 @ 15 fps */
    AgoraVideoProfilePortrait360P = 1030, // iOS
    /** 360 x 360 @ 15 fps */
    AgoraVideoProfilePortrait360P_3 = 1032, // iOS
    /** 360 x 640 @ 30 fps */
    AgoraVideoProfilePortrait360P_4 = 1033,
    /** 360 x 360 @ 30 fps */
    AgoraVideoProfilePortrait360P_6 = 1035,
    /** 360 x 480 @ 15 fps */
    AgoraVideoProfilePortrait360P_7 = 1036,
    /** 360 x 480 @ 30 fps */
    AgoraVideoProfilePortrait360P_8 = 1037,
    /** 360 x 640 @ 15 fps */
    AgoraVideoProfilePortrait360P_9 = 1038,
    /** 360 x 640 @ 24 fps */
    AgoraVideoProfilePortrait360P_10 = 1039,
    /** 360 x 640 @ 24 fps */
    AgoraVideoProfilePortrait360P_11 = 1100,
    /** 480 x 640 @ 15 fps */
    AgoraVideoProfilePortrait480P = 1040,
    /** 480 x 480 @ 15 fps */
    AgoraVideoProfilePortrait480P_3 = 1042, // iOS
    /** 480 x 640 @ 30 fps */
    AgoraVideoProfilePortrait480P_4 = 1043,
    /** 480 x 480 @ 30 fps */
    AgoraVideoProfilePortrait480P_6 = 1045,
    /** 480 x 848 @ 15 fps */
    AgoraVideoProfilePortrait480P_8 = 1047,
    /** 480 x 848 @ 30 fps */
    AgoraVideoProfilePortrait480P_9 = 1048,
    /** 480 x 640 @ 10 fps */
    AgoraVideoProfilePortrait480P_10 = 1049,
    /** 720 x 1280 @ 15 fps */
    AgoraVideoProfilePortrait720P = 1050,
    /** 720 x 1280 @ 30 fps */
    AgoraVideoProfilePortrait720P_3 = 1052,
    /** 720 x 960 @ 15 fps */
    AgoraVideoProfilePortrait720P_5 = 1054,
     /** 720 x 960 @ 30 fps */
    AgoraVideoProfilePortrait720P_6 = 1055,
    /** 1080 x 1920 @ 15 fps */
    AgoraVideoProfilePortrait1080P = 1060, // macOS
    /** 1080 x 1920 @ 30 fps */
    AgoraVideoProfilePortrait1080P_3 = 1062, // macOS
    /** 1080 x 1920 @ 60 fps */
    AgoraVideoProfilePortrait1080P_5 = 1064, // macOS
    /** 1440 x 2560 @ 30 fps */
    AgoraVideoProfilePortrait1440P = 1066, // macOS
    /** 1440 x 2560 @ 60 fps */
    AgoraVideoProfilePortrait1440P_2 = 1067, // macOS
    /** 2160 x 3840 @ 30 fps */
    AgoraVideoProfilePortrait4K = 1070, // macOS
    /** 2160 x 3840 @ 60 fps */
    AgoraVideoProfilePortrait4K_3 = 1072, // macOS
    /** Default 640 x 360 @ 15 fps */
    AgoraVideoProfileDEFAULT = AgoraVideoProfileLandscape360P,
};

typedef NS_ENUM(NSInteger, AgoraVideoCodecType) {
  AgoraVideoCodecTypeVP8 = 1,
  AgoraVideoCodecTypeH264 = 2,
  AgoraVideoCodecTypeVP9 = 5,
};

/** Video frame rate */
typedef NS_ENUM(NSInteger, AgoraVideoFrameRate) {
    /** 1 fps */
    AgoraVideoFrameRateFps1 = 1,
    /** 7 fps */
    AgoraVideoFrameRateFps7 = 7,
    /** 10 fps */
    AgoraVideoFrameRateFps10 = 10,
    /** 15 fps */
    AgoraVideoFrameRateFps15 = 15,
    /** 24 fps */
    AgoraVideoFrameRateFps24 = 24,
    /** 30 fps */
    AgoraVideoFrameRateFps30 = 30,
    /** 60 fps */
    AgoraVideoFrameRateFps60 = 60,
};

/** Video output orientation mode

  Note: When a custom video source is used, if you set AgoraVideoOutputOrientationMode as AgoraVideoOutputOrientationModeFixedLandscape(1) or AgoraVideoOutputOrientationModeFixedPortrait(2), when the rotated video image has a different orientation than the specified output orientation, the video encoder will first crop it and then encode it.
 */
typedef NS_ENUM(NSInteger, AgoraVideoOutputOrientationMode) {
    /** Adaptive mode (Default).

     The video encoder adapts to the orientation mode of the video input device.

     - If the width of the captured video from the SDK is larger than the height, the video sent out by the encoder is in landscape mode. The encoder also sends out the rotational information of the video, and the receiving end will rotate the received video based on the rotational information.
     - When a custom video source is used, the output video from the encoder inherits the orientation of the original video. If the original video is in  portrait mode, the output video from the encoder is also in portrait mode. The encoder also sends out the rotational information of the video to the receiver.
     */
    AgoraVideoOutputOrientationModeAdaptative = 0,
    /** Landscape mode.

     The video encoder always sends out the video in landscape mode. The original video is rotated before being sent out and the rotational information is therefore 0. This mode applies to scenarios involving CDN streaming.
     */
    AgoraVideoOutputOrientationModeFixedLandscape = 1,
     /** Portrait mode.

      The video encoder always sends out the video in portrait mode. The original video is rotated before being sent out and the rotational information is therefore 0. This mode applies to scenarios involving CDN streaming.
      */
    AgoraVideoOutputOrientationModeFixedPortrait = 2,
};

/**
 * The channel profile.
 */
typedef NS_ENUM(NSInteger, AgoraChannelProfile) {
    /**
     * 0: Communication.
     *
     * This profile prioritizes smoothness and applies to the one-to-one scenario.
     */
    AgoraChannelProfileCommunication = 0,
    /**
     * 1: (Default) Live Broadcast.
     *
     * This profile prioritizes supporting a large audience in a live broadcast channel.
     */
    AgoraChannelProfileLiveBroadcasting = 1,
    /**
     * @deprecated
     * 2: Gaming.
     *
     */
    AgoraChannelProfileGame = 2,
    /**
     * 3: Cloud Gaming.
     *
     * This profile prioritizes low end-to-end latency and applies to scenarios where users interact
     * with each other, and any delay affects the user experience.
     */
    AgoraChannelProfileCloudGaming = 3,

    /**
     * 4： 1-to-1 communication.
     */
    AgoraChannelProfileCommunication_1v1 = 4,
};

/** Client role. */
typedef NS_ENUM(NSInteger, AgoraClientRole) {
    /** Broadcaster */
    AgoraClientRoleBroadcaster = 1,
    /** Audience */
    AgoraClientRoleAudience = 2,
};


/** Media type */
typedef NS_ENUM(NSInteger, AgoraMediaType) {
    /** No audio and video */
    AgoraMediaTypeNone = 0,
    /** Audio only */
    AgoraMediaTypeAudioOnly = 1,
    /** Video only */
    AgoraMediaTypeVideoOnly = 2,
    /** Audio and video */
    AgoraMediaTypeAudioAndVideo = 3,
};


/** Encryption mode */
typedef NS_ENUM(NSInteger, AgoraEncryptionMode) {
    /** When it is set to a NULL string, the encryption is in 128-bit AES encryption, XTS mode by default. */
    AgoraEncryptionModeNone = 0,
    /** 128-bit AES encryption, XTS mode */
    AgoraEncryptionModeAES128XTS = 1,
    /** 128-bit AES encryption, ECB mode */
    AgoraEncryptionModeAES256XTS = 2,
    /** 256-bit AES encryption, XTS mode */
    AgoraEncryptionModeAES128ECB = 3,

    /** 4: 128-bit SM4 encryption, ECB mode. */
    AgoraEncryptionModeSM4128ECB = 4,
    /** Enumerator boundary */
    AgoraEncryptionModeEnd,
};

/** Reason for the user being offline */
typedef NS_ENUM(NSUInteger, AgoraUserOfflineReason) {
    /** A user has quit the call. */
    AgoraUserOfflineReasonQuit = 0,
    /** The SDK timed out and the user dropped offline because it has not received any data package within a certain period of time. If a user quits the call and the message is not passed to the SDK (due to an unreliable channel), the SDK assumes the event has timed out. */
    AgoraUserOfflineReasonDropped = 1,
    /** User switched to an audience */
    AgoraUserOfflineReasonBecomeAudience = 2,
};

/** Status of importing an external video stream in a live broadcast */
typedef NS_ENUM(NSUInteger, AgoraInjectStreamStatus) {
    /** The external video stream imported successfully. */
    AgoraInjectStreamStatusStartSuccess = 0,
    /** The external video stream already exists. */
    AgoraInjectStreamStatusStartAlreadyExists = 1,
    /** The external video stream import is unauthorized */
    AgoraInjectStreamStatusStartUnauthorized = 2,
    /** Import external video stream timeout. */
    AgoraInjectStreamStatusStartTimedout = 3,
    /** The external video stream failed to import. */
    AgoraInjectStreamStatusStartFailed = 4,
    /** The xternal video stream imports successfully. */
    AgoraInjectStreamStatusStopSuccess = 5,
    /** No external video stream is found. */
    AgoraInjectStreamStatusStopNotFound = 6,
    /** The external video stream is stopped from being unauthorized. */
    AgoraInjectStreamStatusStopUnauthorized = 7,
    /** Importing the external video stream timeout. */
    AgoraInjectStreamStatusStopTimedout = 8,
    /** Importing the external video stream failed. */
    AgoraInjectStreamStatusStopFailed = 9,
    /** The external video stream is broken. */
    AgoraInjectStreamStatusBroken = 10,
};

/** Output log filter level */
typedef NS_ENUM(NSUInteger, AgoraLogFilter) {
    /** Do not output any log information. */
    AgoraLogFilterOff = 0,
    /** Output all API log information */
    AgoraLogFilterDebug = 0x080f,
    /** Output CRITICAL, ERROR, WARNING, and INFO level log information. */
    AgoraLogFilterInfo = 0x000f,
    /** Outputs CRITICAL, ERROR, and WARNING level log information. */
    AgoraLogFilterWarning = 0x000e,
    /** Outputs CRITICAL and ERROR level log information */
    AgoraLogFilterError = 0x000c,
    /** Outputs CRITICAL level log information. */
    AgoraLogFilterCritical = 0x0008,
};

/** Audio recording quality */
typedef NS_ENUM(NSInteger, AgoraAudioRecordingQuality) {
   /** Low quality, file size around 1.2 MB after 10 minutes of recording. */
    AgoraAudioRecordingQualityLow = 0,
    /** Medium quality, file size around 2 MB after 10 minutes of recording. */
    AgoraAudioRecordingQualityMedium = 1,
    /** High quality, file size around 3.75 MB after 10 minutes of recording. */
    AgoraAudioRecordingQualityHigh = 2
};

/** Video stream lifecyle of CDN Live.                                                                                                   */
typedef NS_ENUM(NSInteger, AgoraRtmpStreamLifeCycle) {
    /** Bound to the channel lifecycle. */
    AgoraRtmpStreamLifeCycleBindToChannel = 1,
    /** Bound to the owner of the RTMP stream. */
    AgoraRtmpStreamLifeCycleBindToOwner = 2,
};

/** Network quality */
typedef NS_ENUM(NSUInteger, AgoraNetworkQuality) {
    /** The network quality is unknown. */
    AgoraNetworkQualityUnknown = 0,
    /**  The network quality is excellent. */
    AgoraNetworkQualityExcellent = 1,
    /** The network quality is quite good, but the bitrate may be slightly lower than excellent. */
    AgoraNetworkQualityGood = 2,
    /** Users can feel the communication slightly impaired. */
    AgoraNetworkQualityPoor = 3,
    /** Users can communicate only not very smoothly. */
    AgoraNetworkQualityBad = 4,
     /** The network is so bad that users can hardly communicate. */
    AgoraNetworkQualityVBad = 5,
     /** The network is down  and users cannot communicate at all. */
    AgoraNetworkQualityDown = 6,
};

/**
 * The state of the probe test.
 */
typedef NS_ENUM(NSUInteger, AgoraLastmileProbeResultState) {
  /**
   * 1: The last-mile network probe test is complete.
   */
  AgoraLastmileProbeResultComplete = 1,
  /**
   * 2: The last-mile network probe test is incomplete and the bandwidth estimation is not available, probably due to limited test resources.
   */
  AgoraLastmileProbeResultIncompleteNoBwe = 2,
  /**
   * 3: The last-mile network probe test is not carried out, probably due to poor network conditions.
   */
  AgoraLastmileProbeResultUnavailable = 3,
};

/** Video stream type */
typedef NS_ENUM(NSInteger, AgoraVideoStreamType) {
    /** High-video stream */
    AgoraVideoStreamTypeHigh = 0,
    /** Low-video stream */
    AgoraVideoStreamTypeLow = 1,
};

/** Video display mode */
typedef NS_ENUM(NSUInteger, AgoraVideoRenderMode) {
    /** Hidden(1): Uniformly scale the video until it fills the visible boundaries (cropped). One dimension of the video may have clipped contents. */
    AgoraVideoRenderModeHidden = 1,

    /** Fit(2): Uniformly scale the video until one of its dimension fits the boundary (zoomed to fit). Areas that are not filled due to the disparity in the aspect ratio will be filled with black. */
    AgoraVideoRenderModeFit = 2,

    /** @deprecated
     Adaptive(3)：This mode is obsolete.
     */
    AgoraVideoRenderModeAdaptive __deprecated_enum_msg("AgoraVideoRenderModeAdaptive is deprecated.") = 3,
};

/** Video codec profile type */
typedef NS_ENUM(NSInteger, AgoraVideoCodecProfileType) {
    /** Baseline video codec profile */
    AgoraVideoCodecProfileTypeBaseLine = 66,
    /** Main video codec profile */
    AgoraVideoCodecProfileTypeMain = 77,
    /** High Video codec profile (default) */
    AgoraVideoCodecProfileTypeHigh = 100
};

/** Video mirror mode */
typedef NS_ENUM(NSUInteger, AgoraVideoMirrorMode) {
    /**
     * 0: The default mirror mode (the SDK determines the mirror mode).
     */
    AgoraVideoMirrorModeAuto = 0,
    /**
     * 1: Enable the mirror mode.
     */
    AgoraVideoMirrorModeEnabled = 1,
    /**
     * 2: Disable the mirror mode.
     */
    AgoraVideoMirrorModeDisabled = 2,
};

/** States of the local video. */
typedef NS_ENUM(NSUInteger, AgoraVideoLocalState) {
  /**
   * 0: The local video is in the initial state.
   */
  AgoraVideoLocalStateStopped = 0,
  /**
   * 1: The capturer starts successfully.
   */
  AgoraVideoLocalStateCapturing = 1,
  /**
   * 2: The first video frame is encoded successfully.
   */
  AgoraVideoLocalStateEncoding = 2,
  /**
   * 3: The local video fails to start.
   */
  AgoraVideoLocalStateFailed = 3
};

/** States of the local video. */
typedef NS_ENUM(NSUInteger, AgoraLocalVideoStreamError) {
  /**
   * 0: The local video is normal.
  */
  AgoraLocalVideoStreamErrorOK = 0,
  /**
   * 1: No specified reason for the local video failure.
  */
  AgoraLocalVideoStreamErrorFailure = 1,
  /**
   * 2: No permission to use the local video device.
   */
  AgoraLocalVideoStreamErrorDeviceNoPermission = 2,
  /**
   * 3: The local video capturer is in use.
   */
  AgoraLocalVideoStreamErrorDeviceBusy = 3,
  /**
   * 4: The local video capture fails. Check whether the capturer is working properly.
   */
  AgoraLocalVideoStreamErrorCaptureFailure = 4,
  /**
   * 5: The local video encoding fails.
   */
  AgoraLocalVideoStreamErrorEncodeFailure = 5
};

/** The state of the remote video. */
typedef NS_ENUM(NSUInteger, AgoraVideoRemoteState) {
    /** 0: The remote video is in the default state, probably due to `AgoraVideoRemoteStateReasonLocalMuted(3)`, `AgoraVideoRemoteStateReasonRemoteMuted(5)`, or `AgoraVideoRemoteStateReasonRemoteOffline(7)`. */
    AgoraVideoRemoteStateStopped = 0,
    /** 1: The first remote video packet is received. */
    AgoraVideoRemoteStateStarting = 1,
    /** 2: The remote video stream is decoded and plays normally, probably due to `AgoraVideoRemoteStateReasonNetworkRecovery(2)`, `AgoraVideoRemoteStateReasonLocalUnmuted(4)`, `AgoraVideoRemoteStateReasonRemoteUnmuted(6)`, or `AgoraVideoRemoteStateReasonAudioFallbackRecovery(9)`. */
    AgoraVideoRemoteStateDecoding = 2,
    /** 3: The remote video is frozen, probably due to `AgoraVideoRemoteStateReasonNetworkCongestion(1)` or `AgoraVideoRemoteStateReasonAudioFallback(8)`. */
    AgoraVideoRemoteStateFrozen = 3,
    /** 4: The remote video fails to start, probably due to `AgoraVideoRemoteStateReasonInternal(0)`. */
    AgoraVideoRemoteStateFailed = 4,
};

/**
 * The reason of the remote video state change.
 */
typedef NS_ENUM(NSUInteger, AgoraVideoRemoteReason) {
      /**
      * 0: Internal reasons.
      */
      AgoraVideoRemoteReasonInternal = 0,

      /**
      * 1: Network congestion.
      */
      AgoraVideoRemoteReasonCongestion = 1,

      /**
      * 2: Network recovery.
      */
      AgoraVideoRemoteReasonRecovery = 2,

      /**
      * 3: The local user stops receiving the remote video stream or disables the video module.
      */
      AgoraVideoRemoteReasonLocalMuted = 3,

      /**
      * 4: The local user resumes receiving the remote video stream or enables the video module.
      */
      AgoraVideoRemoteReasonLocalUnmuted = 4,

      /**
      * 5: The remote user stops sending the video stream or disables the video module.
      */
      AgoraVideoRemoteReasonRemoteMuted = 5,

      /**
      * 6: The remote user resumes sending the video stream or enables the video module.
      */
      AgoraVideoRemoteReasonRemoteUnmuted = 6,

      /**
      * 7: The remote user leaves the channel.
      */
      AgoraVideoRemoteReasonRemoteOffline = 7,

      /**
      * 8: The remote media stream falls back to the audio-only stream due to poor network conditions.
      */
      AgoraVideoRemoteReasonAudioFallback = 8,

      /**
      * 9: The remote media stream switches back to the video stream after the network conditions improve.
      */
      AgoraVideoRemoteReasonAudioFallbackRecovery = 9
};

/**
 * The state of the local audio.
 */
typedef NS_ENUM(NSUInteger, AgoraAudioLocalState) {
    /**
     * 0: The local audio is in the initial state.
     */
    AgoraAudioLocalStateStopped = 0,
    /**
     * 1: The recording device starts successfully.
     */
    AgoraAudioLocalStateRecording = 1,
    /**
     * 2: The first audio frame encodes successfully.
     */
    AgoraAudioLocalStateEncoding = 2,
    /**
     * 3: The local audio fails to start.
     */
    AgoraAudioLocalStateFailed = 3,
};

/**
 * The error information of the local audio.
 */
typedef NS_ENUM(NSUInteger, AgoraAudioLocalError) {
    /**
     * 0: No error.
     */
    AgoraAudioLocalErrorOK = 0,
    /**
     * 1: No specified reason for the local audio failure.
     */
    AgoraAudioLocalErrorFailure = 1,
    /**
     * 2: No permission to use the local audio device.
     */
    AgoraAudioLocalErrorDeviceNoPermission = 2,
    /**
     * 3: The microphone is in use.
     */
    AgoraAudioLocalErrorDeviceBusy = 3,
    /**
     * 4: The local audio recording fails. Check whether the recording device is working properly.
     */
    AgoraAudioLocalErrorRecordFailure = 4,
    /**
     * 5: The local audio encoding fails.
     */
    AgoraAudioLocalErrorEncodeFailure = 5,
};

/**
 * The state of the remote audio.
 */
typedef NS_ENUM(NSUInteger, AgoraAudioRemoteState) {
    /**
     * 0: The remote audio stops (the default state). The following are possible reasons:
     * - {@link AgoraAudioRemoteReasonLocalMuted}(3)
     * - {@link AgoraAudioRemoteReasonRemoteMuted}(5)
     * - {@link AgoraAudioRemoteReasonRemoteOffline}(7)
     */
    AgoraAudioRemoteStateStopped = 0,
    /**
     * 1: The first remote audio packet is received.
     */
    AgoraAudioRemoteStateStarting = 1,
    /**
     * 2: The remote audio stream is decoded and plays normally. The following are possible reasons:
     * - {@link AgoraAudioRemoteReasonNetworkRecovery}(2)
     * - {@link AgoraAudioRemoteReasonLocalUnmuted}(4)
     * - {@link AgoraAudioRemoteReasonRemoteUnmuted}(6)
     */
    AgoraAudioRemoteStateDecoding = 2,
    /**
     * 3: The remote audio is frozen. The possible reason is {@link AgoraAudioRemoteReasonNetworkCongestion}(1).
     */
    AgoraAudioRemoteStateFrozen = 3,
    /**
     * 4: The remote audio fails to start. The possible reason is {@link AgoraAudioRemoteReasonInternal}(0).
     */
    AgoraAudioRemoteStateFailed = 4,
};

/**
 * The reason of the remote audio state change.
 */
typedef NS_ENUM(NSUInteger, AgoraAudioRemoteReason) {
      /**
       * 0: Internal reasons.
       */
      AgoraAudioRemoteReasonInternal = 0,
      /**
       * 1: Network congestion.
       */
      AgoraAudioRemoteReasonNetworkCongestion = 1,
      /**
       * 2: Network recovery.
       */
      AgoraAudioRemoteReasonNetworkRecovery = 2,
      /**
       * 3: The local user stops receiving the remote audio stream or disables the audio module.
       */
      AgoraAudioRemoteReasonLocalMuted = 3,
      /**
       * 4: The local user resumes receiving the remote audio stream or enables the audio module.
       */
      AgoraAudioRemoteReasonLocalUnmuted = 4,
      /**
       * 5: The remote user stops sending the audio stream or disables the audio module.
       */
      AgoraAudioRemoteReasonRemoteMuted = 5,
      /**
       * 6: The remote user resumes sending the audio stream or enables the audio module.
       */
      AgoraAudioRemoteReasonRemoteUnmuted = 6,
      /**
       * 7: The remote user leaves the channel.
       */
      AgoraAudioRemoteReasonRemoteOffline = 7,
};

/** Stream fallback option */
typedef NS_ENUM(NSInteger, AgoraStreamFallbackOptions) {
    /** Default option */
    AgoraStreamFallbackOptionDisabled = 0,
    /** Under poor network conditions, the SDK will send or receive AgoraVideoStreamTypeLow. You can only set this option in [setRemoteSubscribeFallbackOption]([AgoraRtcEngineKit setRemoteSubscribeFallbackOption:]). Nothing happens when you set this in  [setLocalPublishFallbackOption]([AgoraRtcEngineKit setLocalPublishFallbackOption:]). */
    AgoraStreamFallbackOptionVideoStreamLow = 1,
    /** Under poor network conditions, the SDK may receive AgoraVideoStreamTypeLow first, but if the network still does not allow displaying the video, the SDK will send or receive audio only. */
    AgoraStreamFallbackOptionAudioOnly = 2,
};

/** Audio sampling rate */
typedef NS_ENUM(NSInteger, AgoraAudioSampleRateType) {
    /** 32 kHz */
    AgoraAudioSampleRateType32000 = 32000,
    /** 44.1 kHz */
    AgoraAudioSampleRateType44100 = 44100,
    /** 48 kHz */
    AgoraAudioSampleRateType48000 = 48000,
};

/**
 * Audio profile types.
 */
typedef NS_ENUM(NSInteger, AgoraAudioProfile) {
  /**
   * 0: The default audio profile.
   * - In the Communication profile, it represents a sample rate of 16 kHz, music encoding, mono, and a bitrate
   * of up to 16 Kbps.
   * - In the Live-broadcast profile, it represents a sample rate of 48 kHz, music encoding, mono, and a bitrate
   * of up to 64 Kbps.
   */
    AgoraAudioProfileDefault = 0,
    /**
     * 1: A sample rate of 32 kHz, audio encoding, mono, and a bitrate up to 18 Kbps.
     */
    AgoraAudioProfileSpeechStandard = 1,
    /**
     * 2: A sample rate of 48 kHz, music encoding, mono, and a bitrate of up to 64 Kbps.
     */
    AgoraAudioProfileMusicStandard = 2,
    /**
     * 3: A sample rate of 48 kHz, music encoding, stereo, and a bitrate of up to 80
     * Kbps.
     */
    AgoraAudioProfileMusicStandardStereo = 3,
    /**
     * 4: A sample rate of 48 kHz, music encoding, mono, and a bitrate of up to 96 Kbps.
     */
    AgoraAudioProfileMusicHighQuality = 4,
    /**
     * 5: A sample rate of 48 kHz, music encoding, stereo, and a bitrate of up to 128 Kbps.
     */
    AgoraAudioProfileMusicHighQualityStereo = 5,
};

/**
 * Audio application scenarios.
 */
typedef NS_ENUM(NSInteger, AgoraAudioScenario) {
    /**
     * 0: (Recommended) The default audio scenario.
     */
    AgoraAudioScenarioDefault = 0,
    /**
     * 1: The entertainment scenario, which supports voice during gameplay.
     */
    AgoraAudioScenarioChatRoomEntertainment = 1,
    /**
     * 2: The education scenario, which prioritizes smoothness and stability.
     */
    AgoraAudioScenarioEducation = 2,
    /**
     * 3: (Recommended) The live gaming scenario, which needs to enable gaming
     * audio effects in the speaker. Choose this scenario to achieve high-fidelity
     * music playback.
     */
    AgoraAudioScenarioGameStreaming = 3,
    /**
     * 4: The showroom scenario, which optimizes the audio quality with professional
     * external equipment.
     */
    AgoraAudioScenarioShowRoom = 4,
    /**
     * 5: The gaming scenario.
     */
    AgoraAudioScenarioChatRoomGaming = 5,
    /**
     * 6: (Recommended) The scenario requiring high-quality audio.
     */
    AgoraAudioScenarioHighDefinition = 6
};

/**
 * The audio output routing.
 */
typedef NS_ENUM(NSInteger, AgoraAudioOutputRouting) {
    /**
     * -1: The default audio route.
     */
    AgoraAudioOutputRoutingDefault = -1,
    /**
     * 0: Headset.
     */
    AgoraAudioOutputRoutingHeadset = 0,
    /**
     * 1: Earpiece.
     */
    AgoraAudioOutputRoutingEarpiece = 1,
    /**
     * 2: Headset with no microphone.
     */
    AgoraAudioOutputRoutingHeadsetNoMic = 2,
    /**
     * 3: Speakerphone.
     */
    AgoraAudioOutputRoutingSpeakerphone = 3,
    /**
     * 4: Loudspeaker.
     */
    AgoraAudioOutputRoutingLoudspeaker = 4,
    /**
     * 5: Bluetooth headset.
     */
    AgoraAudioOutputRoutingHeadsetBluetooth = 5
};

/** Use mode of the onRecordAudioFrame callback */
typedef NS_ENUM(NSInteger, AgoraAudioRawFrameOperationMode) {
    /** Read-only mode: Users only read the AudioFrame data without modifying anything. For example, when users acquire data with the Agora SDK then push the RTMP streams. */
    AgoraAudioRawFrameOperationModeReadOnly = 0,
    /** Write-only mode: Users replace the AudioFrame data with their own data and pass them to the SDK for encoding. For example, when users acquire data. */
    AgoraAudioRawFrameOperationModeWriteOnly = 1,
    /** Read and write mode: Users read the data from AudioFrame, modify it, and then play it. For example, when users have their own sound-effect processing module and do some voice pre-processing such as a voice change. */
    AgoraAudioRawFrameOperationModeReadWrite = 2,
};

/** Audio equalization band frequency */
typedef NS_ENUM(NSInteger, AgoraAudioEqualizationBandFrequency) {
    /** 31 Hz */
    AgoraAudioEqualizationBand31 = 0,
    /** 62 Hz */
    AgoraAudioEqualizationBand62 = 1,
    /** 125 Hz */
    AgoraAudioEqualizationBand125 = 2,
    /** 250 Hz */
    AgoraAudioEqualizationBand250 = 3,
    /** 500 Hz */
    AgoraAudioEqualizationBand500 = 4,
    /** 1 kHz */
    AgoraAudioEqualizationBand1K = 5,
    /** 2 kHz */
    AgoraAudioEqualizationBand2K = 6,
    /** 4 kHz */
    AgoraAudioEqualizationBand4K = 7,
    /** 8 kHz */
    AgoraAudioEqualizationBand8K = 8,
    /** 16 kHz */
    AgoraAudioEqualizationBand16K = 9,
};

/** Audio reverberation type */
typedef NS_ENUM(NSInteger, AgoraAudioReverbType) {
    /** (dB, -20 to 10), the level of the dry signal */
    AgoraAudioReverbDryLevel = 0,
    /** (dB, -20 to 10), the level of the early reflection signal (wet signal) */
    AgoraAudioReverbWetLevel = 1,
    /** (0 to 100), the room size of the reflection */
    AgoraAudioReverbRoomSize = 2,
    /** (ms, 0 to 200), the length of the initial delay of the wet signal in ms */
    AgoraAudioReverbWetDelay = 3,
     /** (0 to 100), the strength of the late reverberation */
    AgoraAudioReverbStrength = 4,
};

/** The preset audio voice configuration used to change the voice effect. */
typedef NS_ENUM(NSInteger, AgoraAudioVoiceChanger) {
    /** The original voice (no local voice change). */
    AgoraAudioVoiceChangerOff = 0x00000000,
    /** The voice of an old man. */
    AgoraAudioVoiceChangerOldMan = 0x02020200,
    /** The voice of a little boy. */
    AgoraAudioVoiceChangerBabyBoy = 0x02020300,
    /** The voice of a little girl. */
    AgoraAudioVoiceChangerBabyGirl = 0x02020500,
    /** The voice of Zhu Bajie, a character in Journey to the West who has a voice like that of a growling bear. */
    AgoraAudioVoiceChangerZhuBaJie = 0x02020600,
    /** The ethereal voice. */
    AgoraAudioVoiceChangerEthereal = 0x02010700,
    /** The voice of Hulk. */
    AgoraAudioVoiceChangerHulk = 0x02020700,
    /** A more vigorous voice. */
    AgoraAudioVoiceBeautyVigorous = 0x01030100,
    /** A deeper voice. */
    AgoraAudioVoiceBeautyDeep = 0x01030200,
    /** A mellower voice. */
    AgoraAudioVoiceBeautyMellow = 0x01030300,
    /** Falsetto. */
    AgoraAudioVoiceBeautyFalsetto = 0x01030400,
    /** A fuller voice. */
    AgoraAudioVoiceBeautyFull = 0x01030500,
    /** A clearer voice. */
    AgoraAudioVoiceBeautyClear = 0x01030600,
    /** A more resounding voice. */
    AgoraAudioVoiceBeautyResounding = 0x01030700,
    /** A more ringing voice. */
    AgoraAudioVoiceBeautyRinging = 0x01030800,
    /** A more spatially resonant voice. */
    AgoraAudioVoiceBeautySpacial = 0x02010600,
    /** (For male only) A more magnetic voice. Do not use it when the speaker is a female; otherwise, voice distortion occurs. */
    AgoraAudioGeneralBeautyVoiceMaleMagnetic = 0x01010100,
    /** (For female only) A fresher voice. Do not use it when the speaker is a male; otherwise, voice distortion occurs. */
    AgoraAudioGeneralBeautyVoiceFemaleFresh = 0x01010200,
    /** (For female only) A more vital voice. Do not use it when the speaker is a male; otherwise, voice distortion occurs. */
    AgoraAudioGeneralBeautyVoiceFemaleVitality = 0x01010300,
};

/** The preset local voice reverberation option. */
typedef NS_ENUM(NSInteger, AgoraAudioReverbPreset) {
    /** The original voice (no local voice reverberation). */
    AgoraAudioReverbPresetOff = 0x00000000,
    /** The reverberation style typical of a KTV venue (enhanced). */
    AgoraAudioReverbPresetFxKTV = 0x02010100,
    /** The reverberation style typical of a concert hall (enhanced). */
    AgoraAudioReverbPresetFxVocalConcert = 0x02010200,
    /** The reverberation style typical of an uncle's voice. */
    AgoraAudioReverbPresetFxUncle = 0x02020100,
    /** The reverberation style typical of a little sister's voice. */
    AgoraAudioReverbPresetFxSister = 0x02020400,
    /** The reverberation style typical of a recording studio (enhanced). */
    AgoraAudioReverbPresetFxStudio = 0x02010300,
    /** The reverberation style typical of popular music (enhanced). */
    AgoraAudioReverbPresetFxPopular = 0x02030200,
    /** The reverberation style typical of R&B music (enhanced). */
    AgoraAudioReverbPresetFxRNB = 0x02030100,
    /** The reverberation style typical of the vintage phonograph. */
    AgoraAudioReverbPresetFxPhonograph = 0x02010400
};

/** Audio session restriction */
typedef NS_OPTIONS(NSUInteger, AgoraAudioSessionOperationRestriction) {
    /** No restriction, the SDK has full control on the audio session operations. */
    AgoraAudioSessionOperationRestrictionNone              = 0,
    /** The SDK will not change the audio session category */
    AgoraAudioSessionOperationRestrictionSetCategory       = 1,
    /** The SDK will not change any setting of the audio session (category, mode, categoryOptions) */
    AgoraAudioSessionOperationRestrictionConfigureSession  = 1 << 1,
    /** The SDK will keep the audio session active when leaving a channel */
    AgoraAudioSessionOperationRestrictionDeactivateSession = 1 << 2,
    /** The SDK will not configure the audio session anymore */
    AgoraAudioSessionOperationRestrictionAll               = 1 << 7
};

/** Media device type */
typedef NS_ENUM(NSInteger, AgoraMediaDeviceType) {
    /** Unknown device*/
    AgoraMediaDeviceTypeAudioUnknown = -1,
    /** Microphone device */
    AgoraMediaDeviceTypeAudioRecording = 0,
    /** Audio playback device */
    AgoraMediaDeviceTypeAudioPlayout = 1,
    /** Video render device*/
    AgoraMediaDeviceTypeVideoRender = 2,
    /** Video capture device*/
    AgoraMediaDeviceTypeVideoCapture = 3,
};

/** Video frame format */
typedef NS_ENUM(NSInteger, AgoraVideoFormat) {
    /** i420 video frame fromat*/
    AgoraVideoFormatI420 = 1,
    /** BGRA video frame fromat*/
    AgoraVideoFormatBGRA = 2,
    /** NV21 video frame fromat*/
    AgoraVideoFormatNV21 = 3,
    /** RGBA video frame fromat*/
    AgoraVideoFormatRGBA = 4,
    /** IMC2 video frame fromat*/
    AgoraVideoFormatIMC2 = 5,
    /** ARGB video frame fromat*/
    AgoraVideoFormatARGB = 7,
    /** NV12 video frame fromat*/
    AgoraVideoFormatNV12 = 8,
    /** iOS texture (CVPixelBufferRef)*/
    AgoraVideoFormatCVPixel = 12,
};

/** The connection state of the SDK. */
typedef NS_ENUM(NSInteger, AgoraConnectionState) {
  /**
   * 1: The SDK is disconnected from the edge server.
   */
  AgoraConnectionStateDisconnected = 1,
  /**
   * 2: The SDK is connecting to the edge server.
   */
  AgoraConnectionStateConnecting = 2,
  /**
   * 3: The SDK is connected to the edge server and has joined a channel. You can now publish or subscribe to a media stream in the channel.
   */
  AgoraConnectionStateConnected = 3,
  /**
   * 4: The SDK keeps rejoining the channel after being disconnected from a joined channel because of network issues.
   */
  AgoraConnectionStateReconnecting = 4,
  /**
   * 5: The SDK fails to connect to the edge server or join the channel.
   */
  AgoraConnectionStateFailed = 5,
};

/** The network type. */
typedef NS_ENUM(NSInteger, AgoraNetworkType) {
  /**
   * -1: The network type is unknown.
   */
  AgoraNetworkTypeUnknown = -1,
  /**
   * 0: The network type is disconnected.
   */
  AgoraNetworkTypeDisconnected = 0,
  /**
   * 1: The network type is LAN.
   */
  AgoraNetworkTypeLAN = 1,
  /**
   * 2: The network type is Wi-Fi.
   */
  AgoraNetworkTypeWIFI = 2,
  /**
   * 3: The network type is mobile 2G.
   */
  AgoraNetworkType2G = 3,
  /**
   * 4: The network type is mobile 3G.
   */
  AgoraNetworkType3G = 4,
  /**
   * 5: The network type is mobile 4G.
   */
  AgoraNetworkType4G = 5,
};

/** The video encoding degradation preference under limited bandwidth. */
typedef NS_ENUM(NSInteger, AgoraDegradationPreference) {
    /** (Default) Degrades the frame rate to guarantee the video quality. */
    AgoraDegradationMaintainQuality = 0,
    /** Degrades the video quality to guarantee the frame rate. */
    AgoraDegradationMaintainFramerate = 1,
    /** Reserved for future use. */
    AgoraDegradationBalanced = 2,
};

typedef NS_ENUM(NSInteger, AgoraConnectionChangedReason) {
  /**
   * 0: The SDK is connecting to Agora's edge server.
   */
  AgoraConnectionChangedReasonConnecting = 0,
  /**
   * 1: The SDK has joined the channel successfully.
   */
  AgoraConnectionChangedReasonJoinSuccess = 1,
  /**
   * 2: The connection between the SDK and Agora's edge server is interrupted.
   */
  AgoraConnectionChangedReasonInterrupted = 2,
  /**
   * 3: The connection between the SDK and Agora's edge server is banned by Agora's edge server.
   */
  AgoraConnectionChangedReasonBannedByServer = 3,
  /**
   * 4: The SDK fails to join the channel for more than 20 minutes and stops reconnecting to the channel.
   */
  AgoraConnectionChangedReasonJoinFailed = 4,
  /**
   * 5: The SDK has left the channel.
   */
  AgoraConnectionChangedReasonLeaveChannel = 5,
  /**
   * 6: The connection failed since Appid is not valid.
   */
  AgoraConnectionChangedReasonInvalidAppId = 6,
  /**
   * 7: The connection failed since channel name is not valid.
   */
  AgoraConnectionChangedReasonInvalidChannelName = 7,
  /**
   * 8: The connection failed since token is not valid, possibly because:
   - The App Certificate for the project is enabled in Console, but you do not use Token when
   joining the channel. If you enable the App Certificate, you must use a token to join the channel.
   - The uid that you specify in the `joinChannel` method
   is different from the uid that you pass for generating the token.
   */
  AgoraConnectionChangedReasonInvalidToken = 8,
  /**
   * 9: The connection failed since token is expired.
   */
  AgoraConnectionChangedReasonTokenExpired = 9,
  /**
   * 10: The connection is rejected by server.
   */
  AgoraConnectionChangedReasonRejectedByServer = 10,
  /**
   * 11: The connection changed to reconnecting since SDK has set a proxy server.
   */
  AgoraConnectionChangedReasonSettingProxyServer = 11,
  /**
   * 12: When SDK is in connection failed, the renew token operation will make it connecting.
   */
  AgoraConnectionChangedReasonRenewToken = 12,
  /**
   * 13: The IP Address of SDK client has changed. i.e., Network type or IP/Port changed by network
     operator might change client IP address.
   */
  AgoraConnectionChangedReasonClientIpAddressChanged = 13,
  /**
   * 14: Timeout for the keep-alive of the connection between the SDK and Agora's edge server. The
     connection state changes to CONNECTION_STATE_RECONNECTING(4).
   */
  AgoraConnectionChangedReasonKeepAliveTimeout = 14,
  /**
   * 15: The SDK has rejoined the channel successfully.
   */
  AgoraConnectionChangedReasonRejoinSuccess = 15,
  /**
   * 16: The connection between the SDK and Agora's edge server is lost.
   */
  AgoraConnectionChangedReasonLost = 16,
  /**
   * 17: The change of connection state is caused by echo test.
   */
  AgoraConnectionChangedReasonEchoTest = 17,
};

/**
 * The playback state of the audio mixing file.
 */
typedef NS_ENUM(NSInteger, AgoraAudioMixingStateType) {
  /**
   * 710: The audio mixing file is playing.
   */
  AgoraAudioMixingStateTypePlaying = 710,
  /**
   * 711: The audio mixing file pauses playing.
   */
  AgoraAudioMixingStateTypePaused = 711,
  /**
   * 713: The audio mixing file stops playing.
   */
  AgoraAudioMixingStateTypeStopped = 713,
  /**
   * 714: The audio mixing file playback fails. See `AgoraAudioMixingErrorType` for details.
   */
  AgoraAudioMixingStateTypeFailed = 714,
};

/**
 * The audio mixing error code.
 */
typedef NS_ENUM(NSInteger, AgoraAudioMixingErrorType) {
  /**
   * 701: The SDK cannot open the audio file.
   */
  AgoraAudioMixingErrorTypeCanNotOpen = 701,
  /**
   * 702: The SDK opens the audio mixing file too frequently. Ensure that the time interval between calling {@link AgoraRtcEngineKit.startAudioMixing:loopback:replace:cycle: startAudioMixing} is more than 100 ms.
   */
  AgoraAudioMixingErrorTypeTooFrequentlyCall = 702,
  /**
   * 703: The audio mixing file playback is interrupted.
   */
  AgoraAudioMixingErrorTypeInterruptedEOF = 703,
  /**
   * 0: No error.
   */
  AgoraAudioMixingErrorTypeOk = 0,
};

typedef NS_ENUM (NSInteger, AgoraMetadataType) {
  /** -1: The metadata type is unknown.
   */
  AgoraMetadataTypeUnknown = -1,
  /** 0: The metadata type is video.
   */
  AgoraMetadataTypeVideo = 0
};

/**
 * The maximum metadata size.
 */
typedef NS_ENUM (NSInteger, AgoraMaxMetadataSizeType) {
  AgoraMaxMetadataSizeTypeInvalid = -1,
  AgoraMaxMetadataSizeTypeDefault = 512,
  AgoraMaxMetadataSizeTypeMax = 1024
};

/**
 * Supported logging severities of SDK.
 */
typedef NS_ENUM (NSInteger, AgoraLogLevel) {
  /**
   * Do not output any log file.
   */
  AgoraLogLevelNone = 0x0000,
  /**
   * (Recommended) Output log files of the Info level.
   */
  AgoraLogLevelInfo = 0x0001,
  /**
   * Output log files of the Warning level.
   */
  AgoraLogLevelWarn = 0x0002,
  /**
   * Output log files of the Error level.
   */
  AgoraLogLevelError = 0x0004,
  /**
   * Output log files of the Critical level.
   */
  AgoraLogLevelFatal = 0x0008
};

/** Areas for geofencing.
 */
typedef NS_ENUM(NSUInteger, AgoraAreaCodeType) {
  /**
   * Mainland China.
   */
  AgoraAreaCodeTypeCN = 0x1,
  /**
   * North America.
   */
  AgoraAreaCodeTypeNA = 0x2,
  /**
   * Europe.
   */
  AgoraAreaCodeTypeEUR = 0x4,
  /**
   * Asia, excluding Mainland China.
   */
  AgoraAreaCodeTypeAS = 0x8,
  /**
   * (Default) Global.
   */
  AgoraAreaCodeTypeGlobal = 0xFFFFFFFF
};

/**
 * States of the RTMP streaming.
 */
typedef NS_ENUM(NSUInteger, AgoraRtmpStreamPublishState) {
  /**
   * 0: The RTMP streaming has not started or has ended.
   *
   * This state is also reported after you remove
   * an RTMP address from the CDN by calling `removePublishStreamUrl`.
   */
  RTMP_STREAM_PUBLISH_STATE_IDLE = 0,
  /**
   * 1: The SDK is connecting to the streaming server and the RTMP server.
   *
   * This state is reported after you call `addPublishStreamUrl`.
   */
  RTMP_STREAM_PUBLISH_STATE_CONNECTING = 1,
  /**
   * 2: The RTMP streaming publishes. The SDK successfully publishes the RTMP streaming and returns
   * this state.
   */
  RTMP_STREAM_PUBLISH_STATE_RUNNING = 2,
  /**
   * 3: The RTMP streaming is recovering. When exceptions occur to the CDN, or the streaming is
   * interrupted, the SDK tries to resume RTMP streaming and reports this state.
   *
   * - If the SDK successfully resumes the streaming, `RTMP_STREAM_PUBLISH_STATE_RUNNING(2)` is reported.
   * - If the streaming does not resume within 60 seconds or server errors occur,
   * `RTMP_STREAM_PUBLISH_STATE_FAILURE(4)` is reported. You can also reconnect to the server by calling
   * `removePublishStreamUrl` and `addPublishStreamUrl`.
   */
  RTMP_STREAM_PUBLISH_STATE_RECOVERING = 3,
  /**
   * 4: The RTMP streaming fails. See the `errCode` parameter for the detailed error information. You
   * can also call `addPublishStreamUrl` to publish the RTMP streaming again.
   */
  RTMP_STREAM_PUBLISH_STATE_FAILURE = 4
};

/**
 * Error codes of the RTMP streaming.
 */
typedef NS_ENUM(NSInteger, AgoraRtmpStreamPublishError) {
  /**
   * -1: The RTMP streaming fails.
   */
  RTMP_STREAM_PUBLISH_ERROR_FAILED = -1,
  /**
   * 0: The RTMP streaming publishes successfully.
   */
  RTMP_STREAM_PUBLISH_ERROR_OK = 0,
  /**
   * 1: Invalid argument. If, for example, you did not call `setLiveTranscoding` to configure the
   * LiveTranscoding parameters before calling `addPublishStreamUrl`, the SDK reports this error.
   * Check whether you set the parameters in `LiveTranscoding` properly.
   */
  RTMP_STREAM_PUBLISH_ERROR_INVALID_ARGUMENT = 1,
  /**
   * 2: The RTMP streaming is encrypted and cannot be published.
   */
  RTMP_STREAM_PUBLISH_ERROR_ENCRYPTED_STREAM_NOT_ALLOWED = 2,
  /**
   * 3: A timeout occurs with the RTMP streaming. Call `addPublishStreamUrl`
   * to publish the streaming again.
   */
  RTMP_STREAM_PUBLISH_ERROR_CONNECTION_TIMEOUT = 3,
  /**
   * 4: An error occurs in the streaming server. Call `addPublishStreamUrl` to publish
   * the stream again.
   */
  RTMP_STREAM_PUBLISH_ERROR_INTERNAL_SERVER_ERROR = 4,
  /**
   * 5: An error occurs in the RTMP server.
   */
  RTMP_STREAM_PUBLISH_ERROR_RTMP_SERVER_ERROR = 5,
  /**
   * 6: The RTMP streaming publishes too frequently.
   */
  RTMP_STREAM_PUBLISH_ERROR_TOO_OFTEN = 6,
  /**
   * 7: The host publishes more than 10 URLs. Delete the unnecessary URLs before adding new ones.
   */
  RTMP_STREAM_PUBLISH_ERROR_REACH_LIMIT = 7,
  /**
   * 8: The host manipulates other hosts' URLs. Check your app logic.
   */
  RTMP_STREAM_PUBLISH_ERROR_NOT_AUTHORIZED = 8,
  /**
   * 9: The Agora server fails to find the RTMP streaming.
   */
  RTMP_STREAM_PUBLISH_ERROR_STREAM_NOT_FOUND = 9,
  /**
   * 10: The format of the RTMP streaming URL is not supported. Check whether the URL format is correct.
   */
  RTMP_STREAM_PUBLISH_ERROR_FORMAT_NOT_SUPPORTED = 10,
  /**
   * 11: CDN related errors. Remove the original URL address and add a new one by calling
   * `removePublishStreamUrl` and `addPublishStreamUrl`.
   */
  RTMP_STREAM_PUBLISH_ERROR_CDN_ERROR = 11,
  /**
   * 12: Resources are occupied and cannot be reused.
   */
  RTMP_STREAM_PUBLISH_ERROR_ALREADY_IN_USE = 12
};

/**
 * Encryption error type.
 */
typedef NS_ENUM(NSInteger, AgoraEncryptionErrorType) {
  ENCRYPTION_ERROR_INTERNAL_FAILURE = 0,
  ENCRYPTION_ERROR_DECRYPTION_FAILURE = 1,
  ENCRYPTION_ERROR_ENCRYPTION_FAILURE = 2
};

