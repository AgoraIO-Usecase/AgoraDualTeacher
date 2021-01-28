//
// AgoraEnumerates.h
// AgoraRtcEngineKit
//
// Copyright (c) 2018 Agora. All rights reserved.
//

#import <Foundation/Foundation.h>

 /** 警告代码 */
typedef NS_ENUM(NSInteger, AgoraWarningCode) {
    /** 指定的 view 无效，使用视频功能时需要指定 view，如果 view 尚未指定，则返回该警告。*/
    AgoraWarningCodeInvalidView = 8,
    /** 初始化视频功能失败。*/
    AgoraWarningCodeInitVideo = 16,
     /** 请求处于待定状态。一般是由于某个模块还没准备好，请求被延迟处理。*/
    AgoraWarningCodePending = 20,
    /** 没有可用的频道资源。可能是因为服务端没法分配频道资源。 */
    AgoraWarningCodeNoAvailableChannel = 103,
    /** 查找频道超时。在加入频道时 SDK 先要查找指定的频道，出现该警告一般是因为网络太差，连接不到服务器。*/
    AgoraWarningCodeLookupChannelTimeout = 104,
    /** 查找频道请求被服务器拒绝。服务器可能没有办法处理这个请求或请求是非法的。 */
    AgoraWarningCodeLookupChannelRejected = 105,
    /** 打开频道超时。查找到指定频道后，SDK 接着打开该频道，超时一般是因为网络太差，连接不到服务器。*/
    AgoraWarningCodeOpenChannelTimeout = 106,
    /** 打开频道请求被服务器拒绝。服务器可能没有办法处理该请求或该请求是非法的。 */
    AgoraWarningCodeOpenChannelRejected = 107,
    /** 切换直播视频超时。 */
    AgoraWarningCodeSwitchLiveVideoTimeout = 111,
    /** 直播模式下设置用户模式超时。*/
    AgoraWarningCodeSetClientRoleTimeout = 118,
    /** 直播模式下用户模式未授权。 */
    AgoraWarningCodeSetClientRoleNotAuthorized = 119,
    /** TICKET 非法，打开频道失败。 */
    AgoraWarningCodeOpenChannelInvalidTicket = 121,
    /** 尝试打开另一个服务器。 */
    AgoraWarningCodeOpenChannelTryNextVos = 122,
    /** 打开伴奏出错。 */
    AgoraWarningCodeAudioMixingOpenError = 701,
    /** 音频设备模块：运行时播放设备出现警告。 */
    AgoraWarningCodeAdmRuntimePlayoutWarning = 1014,
    /** 音频设备模块：运行时录音设备出现警告。 */
    AgoraWarningCodeAdmRuntimeRecordingWarning = 1016,
    /** 音频设备模块：没有采集到有效的声音数据。 */
    AgoraWarningCodeAdmRecordAudioSilence = 1019,
    /** 音频设备模块：播放設備故障。 */
    AgoraWarningCodeAdmPlaybackMalfunction = 1020,
    /** 音频设备模块：錄音設備故障。 */
    AgoraWarningCodeAdmRecordMalfunction = 1021,
    /** TBD */
    AgoraWarningCodeAdmInterruption = 1025,
    /** 音频设备模块：录到的声音太低。 */
    AgoraWarningCodeAdmRecordAudioLowlevel = 1031,
    /** 音频设备模块：播放的声音太低。 */
    AgoraWarningCodeAdmPlayoutAudioLowlevel = 1032,
    /** 音频设备模块：录音声音异常。 */
    AgoraWarningCodeApmHowling = 1051,
    /** 音频设备模块：音频播放会卡顿。 */
    AgoraWarningCodeAdmGlitchState = 1052,
    /** 音频设备模块：音频底层设置被修改。 */
    AgoraWarningCodeAdmImproperSettings = 1053,
};

 /** 错误代码 */
typedef NS_ENUM(NSInteger, AgoraErrorCode) {
    /** 没有错误。 */
    AgoraErrorCodeNoError = 0,
    /** 一般性的错误（没有明确归类的错误原因。 */
    AgoraErrorCodeFailed = 1,
    /** API 调用了无效的参数。例如指定的频道名含有非法字符。 */
    AgoraErrorCodeInvalidArgument = 2,
    /** RTC 初始化失败。处理方法：
     
     * 检查音频设备状态
     * 检查程序集完整性
     * 尝试重新初始化 AgoraRtcEngineKit
     
     */
    AgoraErrorCodeNotReady = 3,
    /** RTC 当前状态禁止此操作，因此不能进行此操作。 */
    AgoraErrorCodeNotSupported = 4,
    /** 调用被拒绝。仅供 SDK 内部使用，不通过 API 或者回调事件返回给应用程序。 */
    AgoraErrorCodeRefused = 5,
    /** 传入的缓冲区大小不足以存放返回的数据。 */
    AgoraErrorCodeBufferTooSmall = 6,
    /** SDK 尚未初始化，就调用其 API。请确认在调用 API 之前已创建 AgoraRtcEngineKit 对象并完成初始化。 */
    AgoraErrorCodeNotInitialized = 7,
    /** 没有操作权限，仅供 SDK 内部使用，不通过 API 或者回调事件返回给应用程序。 */
    AgoraErrorCodeNoPermission = 9,
    /** API 调用超时。有些 API 调用需要 SDK 返回结果，如果 SDK 处理事件过程，会出现此错误。 */
    AgoraErrorCodeTimedOut = 10,
    /** 请求被取消。仅供 SDK 内部使用，不通过 API 或者回调事件返回给应用程序。 */
    AgoraErrorCodeCanceled = 11,
    /** 调用频率太高。仅供 SDK 内部使用，不通过 API 或者回调事件返回给应用程序。 */
    AgoraErrorCodeTooOften = 12,
    /** SDK 内部绑定到网络 Socket 失败。仅供 SDK 内部使用，不通过 API 或者回调事件返回给应用程序。 */
    AgoraErrorCodeBindSocket = 13,
    /** 网络不可用。仅供 SDK 内部使用，不通过 API 或者回调事件返回给应用程序。 */
    AgoraErrorCodeNetDown = 14,
    /** 没有网络缓冲区可用。仅供 SDK 内部使用，不通过 API 或者回调事件返回给应用程序。 */
    AgoraErrorCodeNoBufs = 15,
    /** 加入频道被拒绝。一般有以下原因：
     
     * 用户已进入频道，再次调用加入频道的 API，例如 [joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])，会返回此错误。停止调用该 API 即可
     * 用户在做 Echo 测试时尝试加入频道。等待 Echo test 结束后再加入频道即可
     
     */
    AgoraErrorCodeJoinChannelRejected = 17,
    /** 离开频道失败。一般有以下原因：
     
     * 用户已离开频道，再次调用退出频道的 API，例如 leaveChannel，会返回此错误。停止调用该 API 即可
     * 用户尚未加入频道，就调用退出频道的 API。这种情况下无需额外操作
     
     */
    AgoraErrorCodeLeaveChannelRejected = 18,
    /** 资源已被占用，不能重复使用。 */
    AgoraErrorCodeAlreadyInUse = 19,
    /** SDK 放弃请求，可能由于请求次数太多。仅供 SDK 内部使用，不通过 API 或者回调时间返回给应用程序。 */
    AgoraErrorCodeAbort = 20,
    /** Windows 下特定的防火墙设置导致 SDK 初始化失败然后崩溃。 */
    AgoraErrorCodeInitNetEngine = 21,
    /** Resources are limited. */
    AgoraErrorCodeResourceLimited = 22,

    /** 不是有效的 App ID。请更换有效的 App ID 重新加入频道。 */
    AgoraErrorCodeInvalidAppId = 101,
    /** 不是有效的频道名。请更换有效的频道名重新加入频道。 */
    AgoraErrorCodeInvalidChannelId = 102,
    /** 当前使用的 Token 过期，不再有效。一般有以下原因：
     
     * Token 授权时间戳无效：Token 授权时间戳为 Token 生成时的时间戳，自 1970 年 1 月 1 日开始到当前时间的描述。授权该 Token 在生成后的 24 小时内可以访问 Agora 服务。如果 24 小时内没有访问，则该 Token 无法再使用。需要重新在服务端申请生成 Token。
     * Token 服务到期时间戳已过期：用户设置的服务到期时间戳小于当前时间戳，无法继续使用 Agora 服务（比如正在进行的通话会被强制终止）；设置服务到期时间并不意味着 Token 失效，而仅仅用于限制用户使用当前服务的时间。需要重新在服务端申请生成 Token。
     
     */
    AgoraErrorCodeTokenExpired = 109,
    /** 生成的 Token 无效，一般有以下原因：
     
     * 用户在 Dashboard 上启用了 App Certificate，但仍旧在代码里仅使用了 App ID。当启用了 App Certificate，必须使用 Token。
     * 字段 uid 为生成 Token 的必须字段，用户在调用 [joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:]) 加入频道时必须设置相同的 uid。
     
    */
    AgoraErrorCodeInvalidToken = 110,
    /** 网络连接中断。仅适用于 Agora Web SDK。 */
    AgoraErrorCodeConnectionInterrupted = 111,
    /** 网络连接丢失。仅适用于 Agora Web SDK。 */
    AgoraErrorCodeConnectionLost = 112,
    /** 用户不在频道内。 */
    AgoraErrorCodeNotInChannel = 113,
    /** 数据太大。 */
    AgoraErrorCodeSizeTooLarge = 114,
    /** 码率受限。 */
    AgoraErrorCodeBitrateLimit = 115,
    /** 数据流/通道过多。 */
    AgoraErrorCodeTooManyDataStreams = 116,
    /** 解密失败，可能是用户加入频道用了不同的密码。请检查加入频道时的设置，或尝试重新加入频道。 */
    AgoraErrorCodeDecryptionFailed = 120,
/** 水印文件参数错误。 */
    AgoraErrorCodeWatermarkParam = 124,
    /** 水印文件路径错误。 */
    AgoraErrorCodeWatermarkPath = 125,
    /** 水印文件格式错误。 */
    AgoraErrorCodeWatermarkPng = 126,
    /** 水印文件信息错误。 */
    AgoraErrorCodeWatermarkInfo = 127,
    /** 水印文件数据格式错误。 */
    AgoraErrorCodeWatermarkAGRB = 128,
    /** 水印文件读取错误。 */
    AgoraErrorCodeWatermarkRead = 129,
    /** 加载媒体引擎失败。 */
    AgoraErrorCodeLoadMediaEngine = 1001,
    /** 启动媒体引擎开始通话失败。请尝试重新进入频道。 */
    AgoraErrorCodeStartCall = 1002,
    /** 启动摄像头失败，请检查摄像头是否被其他应用占用，或者尝试重新进入频道。 */
    AgoraErrorCodeStartCamera = 1003,
    /** 启动视频渲染模块失败。 */
    AgoraErrorCodeStartVideoRender = 1004,
    /** 音频设备模块：音频初始化失败。请检查音频设备是否被其他应用占用，或者尝试重新进入频道。 */
    AgoraErrorCodeAdmGeneralError = 1005,
    /** 音频设备模块：使用 java 资源出现错误。 */
    AgoraErrorCodeAdmJavaResource = 1006,
    /** 音频设备模块：设置的采样频率出现错误。 */
    AgoraErrorCodeAdmSampleRate = 1007,
    /** 音频设备模块：初始化播放设备出现错误。请检查播放设备是否被其他应用占用，或者尝试重新进入频道。 */
    AgoraErrorCodeAdmInitPlayout = 1008,
    /** 音频设备模块：启动播放设备出现错误。请检查播放设备是否正常，或者尝试重新进入频道。 */
    AgoraErrorCodeAdmStartPlayout = 1009,
    /** 音频设备模块：停止播放设备出现错误。 */
    AgoraErrorCodeAdmStopPlayout = 1010,
    /** 音频设备模块：初始化录音设备时出现错误。请检查录音设备是否正常，或者尝试重新进入频道。 */
    AgoraErrorCodeAdmInitRecording = 1011,
    /** 音频设备模块：启动录音设备出现错误。请检查录音设备是否正常，或者尝试重新进入频道。 */
    AgoraErrorCodeAdmStartRecording = 1012,
    /** 音频设备模块：停止录音设备出现错误。 */
    AgoraErrorCodeAdmStopRecording = 1013,
    /** 音频设备模块：运行时播放出现错误。请检查录音设备是否正常，或者尝试重新进入频道。 */
    AgoraErrorCodeAdmRuntimePlayoutError = 1015,
    /** 音频设备模块：运行时录音错误。请检查录音设备是否正常，或者尝试重新进入频道。 */
    AgoraErrorCodeAdmRuntimeRecordingError = 1017,
    /** 音频设备模块：录音失败。 */
    AgoraErrorCodeAdmRecordAudioFailed = 1018,
    /** 回放频率异常。 */
    AgoraErrorCodeAdmPlayAbnormalFrequency = 1020,
    /** 录制频率异常。 */
    AgoraErrorCodeAdmRecordAbnormalFrequency = 1021,
    /** 音频设备模块：初始化 Loopback 设备错误。*/
    AgoraErrorCodeAdmInitLoopback  = 1022,
    /** 音频设备模块：启动 Loopback 设备错误。 */
    AgoraErrorCodeAdmStartLoopback = 1023,
    /** 音频设备模块：无录制设备。请检查是否有可用的录放音设备或者录放音设备是否已经被其他应用占用。 */
    AgoraErrorCodeAdmNoRecordingDevice = 1359,
    /** 音频设备模块：无播放设备。 */
    AgoraErrorCodeAdmNoPlayoutDevice = 1360,
    /** 视频设备模块：没有摄像头使用权限。请检查是否已经打开摄像头权限。 */
    AgoraErrorCodeVdmCameraNotAuthorized = 1501,
    /** 视频设备模块：未知错误。 */
    AgoraErrorCodeVcmUnknownError = 1600,
    /** 视频设备模块：视频 Codec 初始化错误。该错误为严重错误，请尝试重新加入频道。 */
    AgoraErrorCodeVcmEncoderInitError = 1601,
    /** 视频设备模块：视频 Codec 错误。该错误为严重错误，请尝试重新加入频道。 */
    AgoraErrorCodeVcmEncoderEncodeError = 1602,
    /**
      @deprecated
     
     视频设备模块：视频 Codec 设置错误。 */
    AgoraErrorCodeVcmEncoderSetError = 1603,
};

/** 视频档案 */
typedef NS_ENUM(NSInteger, AgoraVideoProfile) {
    /** 160 x 120  @ 15 fps */
    AgoraVideoProfileInvalid = -1,
    /** 120 x 120 @ 15 fps */
    AgoraVideoProfileLandscape120P = 0,
    /** 320 x 180 @ 15 fps */
    AgoraVideoProfileLandscape120P_3 = 2, // iOS
    /** 180 x 180  @ 15 fps */
    AgoraVideoProfileLandscape180P = 10, // iOS
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
    AgoraVideoProfilePortrait480P_3 = 1042,    // iOS
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

/** 視頻幀率 */
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

/** 视频输出方向模式 */
typedef NS_ENUM(NSInteger, AgoraVideoOutputOrientationMode) {
    /** 自适应布局 */
    AgoraVideoOutputOrientationModeAdaptative = 0,
    /** 横屏布局 */
    AgoraVideoOutputOrientationModeFixedLandscape = 1,
     /** 竖屏布局 */
    AgoraVideoOutputOrientationModeFixedPortrait = 2,
};

 /** 频道模式 */
typedef NS_ENUM(NSInteger, AgoraChannelProfile) {
    /**  通信模式 */
    AgoraChannelProfileCommunication = 0,
    /**  直播模式 (默认) */
    AgoraChannelProfileLiveBroadcasting = 1,
    /** 游戏语音模式 */
    AgoraChannelProfileGame = 2,
};

/** 直播场景里的用户角色 */
typedef NS_ENUM(NSInteger, AgoraClientRole) {
    /** 主播 */
    AgoraClientRoleBroadcaster = 1,
    /** 观众(默认) */
    AgoraClientRoleAudience = 2,
};


/** 媒体类型 */
typedef NS_ENUM(NSInteger, AgoraMediaType) {
    /** 沒有 */
    AgoraMediaTypeNone = 0,
    /** 仅音频 */
    AgoraMediaTypeAudioOnly = 1,
    /** 仅视频 */
    AgoraMediaTypeVideoOnly = 2,
    /** 音频和视频 */
    AgoraMediaTypeAudioAndVideo = 3,
};


  /** 加密方式 */
typedef NS_ENUM(NSInteger, AgoraEncryptionMode) {
    /** 设置为空字符串时，使用默认加密方式 “aes-128-xts”*/
    /** 128 位 AES 加密， XTS 模式 */
    AgoraEncryptionModeAES128XTS = 1,
    /**  128 位 AES 加密， ECB 模式 */
    AgoraEncryptionModeAES256XTS = 2,
    /** 256 位 AES 加密，XTS 模式 */
    AgoraEncryptionModeAES128ECB = 3,
};

/** 离线原因 */
typedef NS_ENUM(NSUInteger, AgoraUserOfflineReason) {
    /** 用户主动离开 */
    AgoraUserOfflineReasonQuit = 0,
    /** 因过长时间收不到对方数据包，超时掉线。注意：由于 SDK 使用的是不可靠通道，也有可能对方主动离开本方没收到对方离开消息而误判为超时掉线*/
    AgoraUserOfflineReasonDropped = 1,
    /** 用户身份从主播切换为观众时触发 */
    AgoraUserOfflineReasonBecomeAudience = 2,
};

/** 导入的外部视频源状态 */
typedef NS_ENUM(NSUInteger, AgoraInjectStreamStatus) {
    /** 外部视频流导入成功 */
    AgoraInjectStreamStatusStartSuccess = 0,
    /** 外部视频流已存在 */
    AgoraInjectStreamStatusStartAlreadyExists = 1,
    /** 外部视频流导入未经授权 */
    AgoraInjectStreamStatusStartUnauthorized = 2,
    /** 导入外部视频流超时 */
    AgoraInjectStreamStatusStartTimedout = 3,
    /** 外部视频流导入失败 */
    AgoraInjectStreamStatusStartFailed = 4,
    /** 外部视频流停止导入成功 */
    AgoraInjectStreamStatusStopSuccess = 5,
    /** 未找到要停止导入的外部视频流 */
    AgoraInjectStreamStatusStopNotFound = 6,
    /** 要停止导入的外部视频流未经授权 */
    AgoraInjectStreamStatusStopUnauthorized = 7,
    /** 停止导入外部视频流超时 */
    AgoraInjectStreamStatusStopTimedout = 8,
    /** 停止导入外部视频流失败 */
    AgoraInjectStreamStatusStopFailed = 9,
    /** 外部视频流被破坏 */
    AgoraInjectStreamStatusBroken = 10,
};

/** 设置过滤器等级 */
typedef NS_ENUM(NSUInteger, AgoraLogFilter) {
    /** 不输出日志信息 */
    AgoraLogFilterOff = 0,
    /** 输出所有 API 日志信息 */
    AgoraLogFilterDebug = 0x080f,
    /** 输出 CRITICAL、ERROR、WARNING 和 INFO 级别的日志信息 */
    AgoraLogFilterInfo = 0x000f,
    /** 输出 CRITICAL、ERROR 和 WARNING 级别的日志信息 */
    AgoraLogFilterWarning = 0x000e,
    /** 输出 CRITICAL 和 ERROR 级别的日志信息 */
    AgoraLogFilterError = 0x000c,
    /** 输出 CRITICAL 级别的日志信息 */
    AgoraLogFilterCritical = 0x0008,
};

/** 录音音质 */
typedef NS_ENUM(NSInteger, AgoraAudioRecordingQuality) {
   /** 低音质。录制 10 分钟的文件大小为 1.2 M 左右 */
    AgoraAudioRecordingQualityLow = 0,
    /** 中音质。录制 10 分钟的文件大小为 2 M 左右 */
    AgoraAudioRecordingQualityMedium = 1,
    /** 高音质。录制 10 分钟的文件大小为 3.75 M 左右 */
    AgoraAudioRecordingQualityHigh = 2
};

/** RTMP流周期 */
typedef NS_ENUM(NSInteger, AgoraRtmpStreamLifeCycle) {
    AgoraRtmpStreamLifeCycleBindToChannel = 1,
    AgoraRtmpStreamLifeCycleBindToOwnner = 2,
};

/** 网络质量 */
typedef NS_ENUM(NSUInteger, AgoraNetworkQuality) {
    /** 网络质量未知 */
    AgoraNetworkQualityUnknown = 0,
    /**  网络质量极好 */
    AgoraNetworkQualityExcellent = 1,
    /** 用户主观感觉和 excellent 差不多，但码率可能略低于 excellent */
    AgoraNetworkQualityGood = 2,
    /** 用户主观感受有瑕疵但不影响沟通 */
    AgoraNetworkQualityPoor = 3,
    /** 勉强能沟通但不顺畅 */
    AgoraNetworkQualityBad = 4,
     /** 网络质量非常差，基本不能沟通 */
    AgoraNetworkQualityVBad = 5,
     /** 完全无法沟通 */
    AgoraNetworkQualityDown = 6,
};

/** 设置视频流大小 */
typedef NS_ENUM(NSInteger, AgoraVideoStreamType) {
    /** 视频大流 */
    AgoraVideoStreamTypeHigh = 0,
    /** 视频小流 */
    AgoraVideoStreamTypeLow = 1,
};

/** 视频显示模式 */
typedef NS_ENUM(NSUInteger, AgoraVideoRenderMode) {
    /**
     Hidden(1): 如果视频尺寸与显示视窗尺寸不一致，则视频流会按照显示视窗的比例进行周边裁剪或图像拉伸后填满视窗
     */
    AgoraVideoRenderModeHidden = 1,

    /**
     Fit(2): 如果视频尺寸与显示视窗尺寸不一致，在保持长宽比的前提下，将视频进行缩放后填满视窗，缩放后的视频四周会有一圈黑边
     */
    AgoraVideoRenderModeFit = 2,

    /**
     Adaptive(3)：该模式已废弃，不推荐使用
     */
    AgoraVideoRenderModeAdaptive __deprecated_enum_msg("AgoraVideoRenderModeAdaptive is deprecated.") = 3,
};

/** 视频的编解码类型 */
typedef NS_ENUM(NSInteger, AgoraVideoCodecProfileType) {
    /** 最低视频编码率 */
    AgoraVideoCodecProfileTypeBaseLine = 66,
    AgoraVideoCodecProfileTypeMain = 77,
    /**  默认值，最高视频编码率 */
    AgoraVideoCodecProfileTypeHigh = 100,
};

/** 视频镜像模式 */
typedef NS_ENUM(NSUInteger, AgoraVideoMirrorMode) {
    /** 默认镜像模式，即由 SDK 决定镜像模式 */
    AgoraVideoMirrorModeAuto = 0,
    /** 启用镜像模式 */
    AgoraVideoMirrorModeEnabled = 1,
    /** 关闭镜像模式 */
    AgoraVideoMirrorModeDisabled = 2,
};

/** 远端视频流状态 */
typedef NS_ENUM(NSUInteger, AgoraVideoRemoteState) {
    /** 远端视频停止播放  */
    AgoraVideoRemoteStateStopped = 0,
    /** 远端视频正常 */
    AgoraVideoRemoteStateRunning = 1,
    /** 远端视频卡住 */
    AgoraVideoRemoteStateFrozen = 2,
};

/** 本地音频流类型. */
typedef NS_ENUM(NSUInteger, AgoraAudioLocalState) {
    /** 本地音频默认初始状态 */
    AgoraAudioLocalStateStopped = 0,
    /** 本地音频录制设备启动成功 */
    AgoraAudioLocalStateRecording = 1,
    /** 本地音频首帧编码成功 */
    AgoraAudioLocalStateEncoding = 2,
    /** 本地音频启动失败 */
    AgoraAudioLocalStateFailed = 3,
};

/** 远端音频流状态 */
typedef NS_ENUM(NSUInteger, AgoraAudioRemoteState) {
    /**
     * 远端音频流停止, 默认状态, 表示音频可能刚刚开始或者远端用户禁止/静音了音频流，在以下的情况下，会报告该状态
     * #AgoraAudioRemoteReasonLocalMuted (3),
     * #AgoraAudioRemoteReasonRemoteMuted (5)
     * #AgoraAudioRemoteReasonRemoteOffline (7).
     */
    AgoraAudioRemoteStateStopped = 0,
    /** 第一帧音频流已被接收到 */
    AgoraAudioRemoteStateStarting = 1,
    /**
     * 远端音频流已经解码并正常播放，在以下的情况下，会报告该状态
     * #AgoraAudioRemoteReasonNetworkRecovery (2),
     * #AgoraAudioRemoteReasonLocalUnmuted (4)或
     * #AgoraAudioRemoteReasonRemoteUnmuted (6).
     */
    AgoraAudioRemoteStateDecoding = 2,
    /** 远端音频流卡顿, 在AgoraAudioRemoteReasonNetworkCongestion (1)的情况下，会报告该状态 */
    AgoraAudioRemoteStateFrozen = 3,
    /** 远端音频流播放失败，在AgoraAudioRemoteReasonInteral (0)的情况下，会报告该状态 */
    AgoraAudioRemoteStateFailed = 4,
};

/** 产生远端视频流不同状态（AgoraAudioRemoteState）的原因 */
typedef NS_ENUM(NSUInteger, AgoraAudioRemoteReason) {
      /** SDK内部错误 */
      AgoraAudioRemoteReasonInteral = 0,
      /** 网络拥堵 */
      AgoraAudioRemoteReasonNetworkCongestion = 1,
      /** 网络重连.*/
      AgoraAudioRemoteReasonNetworkRecovery = 2,
      /** 本地用户停止接收远端音频流 或 禁止了音频模块 */
      AgoraAudioRemoteReasonLocalMuted = 3,
      /** 本地用户重新恢复继续接收远端音频流 或 开启了音频模块 */
      AgoraAudioRemoteReasonLocalUnmuted = 4,
      /** 远端用户停止了发送音频流 或 禁止了音频模块 */
      AgoraAudioRemoteReasonRemoteMuted = 5,
      /** 远端用户重新恢复发送音频流 或 开启了音频模块 */
      AgoraAudioRemoteReasonRemoteUnmuted = 6,
      /** The remote user leaves the channel.*/
      AgoraAudioRemoteReasonRemoteOffline = 7,
};

/** 推流回退处理选项 */
typedef NS_ENUM(NSInteger, AgoraStreamFallbackOptions) {
    /** （默认）上行网络较弱时，不对音视频流作回退处理，但不能保证音视频流的质量。 */
    AgoraStreamFallbackOptionDisabled = 0,
    /** 在網絡條件較差的情況下，SDK將發送或接收AgoraVideoStreamTypeLow。 */
    AgoraStreamFallbackOptionVideoStreamLow = 1,
    /** 上行网络较弱时只发布音频流。 */
    AgoraStreamFallbackOptionAudioOnly = 2,
};

/** 音频的采样率 */
typedef NS_ENUM(NSInteger, AgoraAudioSampleRateType) {
    /** 32 kHz */
    AgoraAudioSampleRateType32000 = 32000,
    /** 44.1 kHz */
    AgoraAudioSampleRateType44100 = 44100,
    /** 48 kHz */
    AgoraAudioSampleRateType48000 = 48000,
};

/** 音频档案 */
typedef NS_ENUM(NSInteger, AgoraAudioProfile) {
    /**
     * 默认设置。通信模式下为 1，直播模式下为 2
     */
    AgoraAudioProfileDefault = 0,
    /**
     * 指定 32 KHz采样率，语音编码, 单声道，编码码率约 18 kbps
     */
    AgoraAudioProfileSpeechStandard = 1,
    /**
     * 指定 48 KHz采样率，音乐编码, 单声道，编码码率约 48 kbps
     */
    AgoraAudioProfileMusicStandard = 2,
    /**
     * 指定 48 KHz采样率，音乐编码, 双声道，编码码率约 56 kbps
     */
    AgoraAudioProfileMusicStandardStereo = 3,
    /**
     * 指定 48 KHz 采样率，音乐编码, 单声道，编码码率约 128 kbps
     */
    AgoraAudioProfileMusicHighQuality = 4,
    /**
     * 指定 48 KHz采样率，音乐编码, 双声道，编码码率约 192 kbps
     */
    AgoraAudioProfileMusicHighQualityStereo = 5,
};

/** 音频应用场景 */
typedef NS_ENUM(NSInteger, AgoraAudioScenario) {
    /** 默认设置 */
    AgoraAudioScenarioDefault = 0,
    /** 娱乐应用，需要频繁上下麦的场景 */
    AgoraAudioScenarioChatRoomEntertainment = 1,
    /** 教育应用，流畅度和稳定性优先 */
    AgoraAudioScenarioEducation = 2,
    /** 游戏直播应用，需要外放游戏音效也直播出去的场景 */
    AgoraAudioScenarioGameStreaming = 3,
    /** 秀场应用，音质优先和更好的专业外设支持 */
    AgoraAudioScenarioShowRoom = 4,
    /** 游戏开黑 */
    AgoraAudioScenarioChatRoomGaming = 5
};

/** 语音路由 */
typedef NS_ENUM(NSInteger, AgoraAudioOutputRouting)
{
    /** 使用默认的语音路由 */
    AgoraAudioOutputRoutingDefault = -1,
    /** 使用耳机为语音路由 */
    AgoraAudioOutputRoutingHeadset = 0,
    /** 使用听筒为语音路由 */
    AgoraAudioOutputRoutingEarpiece = 1,
    /** 使用不带麦的耳机为语音路由 */
    AgoraAudioOutputRoutingHeadsetNoMic = 2,
    /** 使用手机的扬声器为语音路由 */
    AgoraAudioOutputRoutingSpeakerphone = 3,
    /** 使用外接的扬声器为语音路由 */
    AgoraAudioOutputRoutingLoudspeaker = 4,
    /** 使用蓝牙耳机为语音路由 */
    AgoraAudioOutputRoutingHeadsetBluetooth = 5
};

/** 指定onRecordAudioFrame的使用模式 */
typedef NS_ENUM(NSInteger, AgoraAudioRawFrameOperationMode) {
    /** 只读模式，用户仅从 AudioFrame 获取原始数据。例如：若用户通过 Agora SDK 采集数据，自己进行 RTMP 推流，则可以选择该模式。 */
    AgoraAudioRawFrameOperationModeReadOnly = 0,
    /** 只写模式，用户替换 AudioFrame 中的数据以供 Agora SDK 编码传输。例如：若用户自行采集数据，可选择该模式。y */
    AgoraAudioRawFrameOperationModeWriteOnly = 1,
    /** 读写模式，用户从 AudioFrame 获取并修改数据，并返回给 Aogra SDK 进行编码传输。例如：若用户自己有音效处理模块，且想要根据实际需要对数据进行前处理 (例如变声)，则可以选择该模式。 */
    AgoraAudioRawFrameOperationModeReadWrite = 2,
};

/** 音频均衡频段 */
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

/** 音频混响类型 */
typedef NS_ENUM(NSInteger, AgoraAudioReverbType) {
    /** (dB, -20 to 10), 原始声音效果，即所谓的 dry signal*/
    AgoraAudioReverbDryLevel = 0,
    /** (dB, -20 to 10), 早期反射信号效果，即所谓的 wet signal */
    AgoraAudioReverbWetLevel = 1,
    /** (0 to 100), 所需混响效果的房间尺寸*/
    AgoraAudioReverbRoomSize = 2,
    /** (ms, 0 to 200), wet signal 的初始延迟长度，以毫秒为单位 */
    AgoraAudioReverbWetDelay = 3,
     /** (0 to 100), wet signal 的初始延迟长度，以毫秒为单位 */
    AgoraAudioReverbStrength = 4,
};

/** 音频会话限制 */
typedef NS_OPTIONS(NSUInteger, AgoraAudioSessionOperationRestriction) {
    /** 没有限制，SDK可以完全控制音频会话操作。 */
    AgoraAudioSessionOperationRestrictionNone              = 0,
    /** SDK不会更改音频会话类别。 */
    AgoraAudioSessionOperationRestrictionSetCategory       = 1,
    /** SDK不会更改音频会话的任何设置（类别，模式，categoryOptions)。 */
    AgoraAudioSessionOperationRestrictionConfigureSession  = 1 << 1,
    /** 离开某个频道时，SDK会保持音频会话处于活动状态。 */
    AgoraAudioSessionOperationRestrictionDeactivateSession = 1 << 2,
    /** SDK将不再配置音频会话。 */
    AgoraAudioSessionOperationRestrictionAll               = 1 << 7
};

/** 媒体设备类型 */
typedef NS_ENUM(NSInteger, AgoraMediaDeviceType) {
    /** 未知 */
    AgoraMediaDeviceTypeAudioUnknown = -1,
    /** 声音录制 */
    AgoraMediaDeviceTypeAudioRecording = 0,
    /** 音频播放 */
    AgoraMediaDeviceTypeAudioPlayout = 1,
    /** 视频呈现 */
    AgoraMediaDeviceTypeVideoRender = 2,
    /** 视频截取 */
    AgoraMediaDeviceTypeVideoCapture = 3,
};

/** Connection State */
typedef NS_ENUM(NSInteger, AgoraConnectionState) {
  /** 1: The SDK is disconnected from Agora's edge server.

   - This is the initial state before calling the `joinChannel` method.
   - The SDK also enters this state when the application calls the "leaveChannel" method.
   */
  AgoraConnectionStateDisconnected = 1,
  /** 2: The SDK is connecting to Agora's edge server.

   - When the application calls the `joinChannel` method, the SDK starts to establish a connection to the specified channel, triggers the "onConnectionStateChanged" callback, and switches to the #CONNECTION_STATE_CONNECTING state.
   - When the SDK successfully joins the channel, it triggers the "onConnectionStateChanged" callback and switches to the #CONNECTION_STATE_CONNECTED state.
   - After the SDK joins the channel and when it finishes initializing the media engine, the SDK triggers the "onJoinChannelSuccess" callback.
   */
  AgoraConnectionStateConnecting = 2,
  /** 3: The SDK is connected to Agora's edge server and has joined a channel. You can now publish or subscribe to a media stream in the channel.

   If the connection to the channel is lost because, for example, the network is down or switched, the SDK automatically tries to reconnect and triggers:
   - The "onConnectionInterrupted" callback (deprecated).
   - The "onConnectionStateChanged" callback and switches to the #CONNECTION_STATE_RECONNECTING state.
   */
  AgoraConnectionStateConnected = 3,
  /** 4: The SDK keeps rejoining the channel after being disconnected from a joined channel because of network issues.

   - If the SDK cannot rejoin the channel within 10 seconds after being disconnected from Agora's edge server, the SDK triggers the "onConnectionLost" callback, stays in the #CONNECTION_STATE_RECONNECTING state, and keeps rejoining the channel.
   - If the SDK fails to rejoin the channel 20 minutes after being disconnected from Agora's edge server, the SDK triggers the "onConnectionStateChanged" callback, switches to the #CONNECTION_STATE_FAILED state, and stops rejoining the channel.
   */
  AgoraConnectionStateReconnecting = 4,
  /** 5: The SDK fails to connect to Agora's edge server or join the channel.

   You must call the `leaveChannel` method to leave this state, and call the `joinChannel` method again to rejoin the channel.

   If the SDK is banned from joining the channel by Agora's edge server (through the RESTful API), the SDK triggers the "onConnectionBanned" (deprecated) and "onConnectionStateChanged" callbacks.
   */
  AgoraConnectionStateFailed = 5,
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
}
