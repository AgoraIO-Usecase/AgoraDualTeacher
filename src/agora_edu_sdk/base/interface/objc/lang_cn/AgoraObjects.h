//
//  AgoraObjects.h
//  AgoraRtcEngineKit
//
//
//  Copyright (c) 2017 Agora. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <CoreMedia/CoreMedia.h>
#import "AgoraEnumerates.h"

#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
typedef UIView VIEW_CLASS;
typedef UIColor COLOR_CLASS;
#elif TARGET_OS_MAC
#import <AppKit/AppKit.h>
typedef NSView VIEW_CLASS;
typedef NSColor COLOR_CLASS;
#endif

/** 视频画布对象的属性。
 */
__attribute__((visibility("default"))) @interface AgoraRtcVideoCanvas : NSObject
/** 视频显示视窗

 */
@property (strong, nonatomic) VIEW_CLASS* _Nullable view;
/**
 视频显示模式, AgoraVideoRenderMode
 */
@property (assign, nonatomic) AgoraVideoRenderMode renderMode;
/**
本地用户 ID，与 joinChannel 中的 uid 保持一致
 */
@property (assign, nonatomic) NSUInteger uid;
@end

/** 本地视频统计回调
 */
__attribute__((visibility("default"))) @interface AgoraRtcLocalVideoStats : NSObject
/**
 （上次统计后）发送的字节数
 */
@property (assign, nonatomic) NSUInteger sentBitrate;
/**
 （上次统计后）发送的帧数
 */
@property (assign, nonatomic) NSUInteger sentFrameRate;
@end

/** 远端视频统计回调
 */
__attribute__((visibility("default"))) @interface AgoraRtcRemoteVideoStats : NSObject
/**
 用户 ID，指定远程视频来自哪个用户
 */
@property (assign, nonatomic) NSUInteger uid;
/**
 延时(毫秒)
 */
@property (assign, nonatomic) NSUInteger delay __deprecated;
/**
 视频流宽（像素）
 */
@property (assign, nonatomic) NSUInteger width;
/**
 视频流高（像素）
 */
@property (assign, nonatomic) NSUInteger height;
/**
 （上次统计后）接收到的码率(kbps)
 */
@property (assign, nonatomic) NSUInteger receivedBitrate;
/**
（上次统计后）接收帧率(fps)
 */
@property (assign, nonatomic) NSUInteger receivedFrameRate;
/**
 视频流类型，大流或小流: AgoraVideoStreamType
 */
@property (assign, nonatomic) AgoraVideoStreamType rxStreamType;
@end

/** 音量信息的属性。
 */
__attribute__((visibility("default"))) @interface AgoraRtcAudioVolumeInfo : NSObject
/**
说话者的用户 ID。如果返回的 uid 为 0，则默认为本地用户
 */
@property (assign, nonatomic) NSUInteger uid;
/**
 说话者的音量（0 - 255）
 */
@property (assign, nonatomic) NSUInteger volume;
@end

/** 通道的统计数据。
 */
__attribute__((visibility("default"))) @interface AgoraChannelStats: NSObject
/**
 通话时长，单位为秒，累计值
 */
@property (assign, nonatomic) NSInteger duration;
/**
 发送字节数 (bytes)，累计值
 */
@property (assign, nonatomic) NSInteger txBytes;
/**
  接收字节数 (bytes)，累计值
 */
@property (assign, nonatomic) NSInteger rxBytes;
/**
 音频发送码率 (kbps)，瞬时值
 */
@property (assign, nonatomic) NSInteger txAudioKBitrate;
/**
 音频接收码率 (kbps)，瞬时值
 */
@property (assign, nonatomic) NSInteger rxAudioKBitrate;
/**
 视频发送码率 (kbps)，瞬时值
 */
@property (assign, nonatomic) NSInteger txVideoKBitrate;
/**
视频接收码率 (kbps)，瞬时值
 */
@property (assign, nonatomic) NSInteger rxVideoKBitrate;
/**
客户端到 vos 服务器的延迟（毫秒）
 */
@property (assign, nonatomic) NSInteger lastmileDelay;
/**
 当前频道内的用户人数
 */
@property (assign, nonatomic) NSInteger userCount;
/**
 当前系统的 CPU 使用率 (%)
 */
@property (assign, nonatomic) double cpuAppUsage;
/**
当前应用程序的 CPU 使用率 (%)
 */
@property (assign, nonatomic) double cpuTotalUsage;
@end

/** 视频编码器配置的属性。
 */
__attribute__((visibility("default"))) @interface AgoraVideoEncoderConfiguration: NSObject
/** 视频编码的分辨率，由宽 x 高组成，用户可以自行对宽和高设值，也可以根据下面的枚举来设置：
 
 * VD_120x120
 * VD_160x120
 * VD_180x180
 * VD_240x180
 * VD_320x180
 * VD_240x240
 * VD_320x240
 * VD_424x240
 * VD_360x360
 * VD_480x360
 * VD_640x360
 * VD_480x480
 * VD_640x480
 * VD_840x480
 * VD_960x720
 * VD_1280x720
 
 Note: 视频能否达到 720P 的分辨率取决于设备的性能，在性能配备较低的设备上有可能无法实现。如果采用 720P 分辨率而设备性能跟不上，则有可能出现帧率过低的情况。如设备有特别需求，请联系 mailto:support@agora.io。
 */
@property (assign, nonatomic) CGSize dimensions;
/** 视频编码的帧率。具体可设置如下帧率：

 * FRAME_RATE_FPS_1(1): 每秒钟 1 帧
 * FRAME_RATE_FPS_7(7): 每秒钟 7 帧
 * FRAME_RATE_FPS_10(10): 每秒钟 10 帧
 * FRAME_RATE_FPS_15(15): 每秒钟 15 帧
 * FRAME_RATE_FPS_24(24): 每秒钟 24 帧
 * FRAME_RATE_FPS_30(30): 每秒钟 30 帧

 */
@property (assign, nonatomic) AgoraVideoFrameRate frameRate;
/** 视频编码的码率。你可以根据场景需要，参考 AgoraVideoProfile，手动设置你想要的码率。若设置的视频码率超出合理范围，SDK 会自动按照合理区间处理码率。如果觉得手动设置比较繁琐，也可以直接选择
 

 * STANDARD_BITRATE(0): （推荐）在直播场景下，Agora 推荐你使用较大码率来提升视频质量。当使用 STANDARD_BITRATE 时，码率比参考表中的码率翻倍。在通讯模式下，仍然以参考码率编码
 * COMPATIBLE_BITRATE(-1): 使用 COMPATIBLE_BITRATE 时，直播和通讯场景中仍然使用接近参考表中的码率值

 */
@property (assign, nonatomic) NSInteger bitrate;
/** 视频编码的方向模式:

 * ORIENTATION_MODE_ADAPTIVE(0): （默认）该模式下 SDK 输出的视频方向与采集到的视频方向一致。接收端会根据收到的视频旋转信息对视频进行旋转。该模式适用于接收端可以调整视频方向的场景:
 
    * 如果采集的视频是横屏模式，则输出的视频也是横屏模式。
    * 如果采集的视频是竖屏模式，则输出的视频也是竖屏模式。

* ORIENTATION_MODE_FIXED_LANDSCAPE(1): 该模式下 SDK 固定输出风景（横屏）模式的视频。如果采集到的视频是竖屏模式，则视频编码器会对其进行裁剪。该模式适用于当接收端无法调整视频方向时，如使用 CDN 推流场景下。
* ORIENTATION_MODE_FIXED_PORTRAIT(2): 该模式下 SDK 固定输出人像（竖屏）模式的视频，如果采集到的视频是横屏模式，则视频编码器会对其进行裁剪。该模式适用于当接收端无法调整视频方向时，如使用 CDN 推流场景下。

 */
@property (assign, nonatomic) AgoraVideoOutputOrientationMode orientationMode;

- (instancetype _Nonnull)initWithSize:(CGSize)size
                            frameRate:(AgoraVideoFrameRate)frameRate
                              bitrate:(NSInteger)bitrate
                      orientationMode:(AgoraVideoOutputOrientationMode)orientationMode;

- (instancetype _Nonnull)initWithWidth:(NSInteger)width
                                height:(NSInteger)height
                             frameRate:(AgoraVideoFrameRate)frameRate
                               bitrate:(NSInteger)bitrate
                       orientationMode:(AgoraVideoOutputOrientationMode)orientationMode;
@end

/** 提供用户特定音频/视频转码设置的类。
 */
__attribute__((visibility("default"))) @interface AgoraLiveTranscodingUser: NSObject
/**
 用户 ID
 */
@property (assign, nonatomic) NSUInteger uid;
/**
 用于视频控制
 */
@property (assign, nonatomic) CGRect rect;
/**
视频帧图层编号。取值范围为 [1,100] 中的整型。1 表示该区域图像位于最下层，而 100 表示该区域图像位于最上层

 */
@property (assign, nonatomic) NSInteger zOrder; // Optional, [0, 100] // 0 (Default): Minimum, 100: Maximum
/** 视频帧的透明度。取值范围为 [0,1]。0 表示该区域图像完全透明，而1表示该区域图像完全不透明。默认值为 1。

 */
@property (assign, nonatomic) double alpha; // Optional, [0, 1.0] // 0: Transparent, 1.0: Opaque

/**
 音频所在声道。取值范围为 [0, 5]，默认值为 ：
 
 * 0:(推荐) 默认混音设置，最多支持双声道，与主播端上行音频相关
 * 1: 对应主播的音频，推流中位于FL声道。如果主播上行为双声道，则仅取左声道用于推流
 * 2: 对应主播的音频，推流中位于FC声道。如果主播上行为双声道，则仅取左声道用于推流
 * 3: 对应主播的音频，推流中位于FR声道。如果主播上行为双声道，则仅取左声道用于推流
 * 4: 对应主播的音频，推流中位于BL声道。如果主播上行为双声道，则仅取左声道用于推流
 * 5: 对应主播的音频，推流中位于BR声道。如果主播上行为双声道，则仅取左声道用于推流
 
 选项不为 0 时，需要特殊的播放器支持。
 
 */

@property (assign, nonatomic) NSInteger audioChannel;
@end

/** 水印图像属性。

 */
__attribute__((visibility("default"))) @interface AgoraImage: NSObject
/**
直播视频上图片的 url 地址
 */
@property (strong, nonatomic) NSURL *_Nonnull url;
/**
 图片在视频帧上的位置和大小，类型为 CGRect
 */
@property (assign, nonatomic) CGRect rect;
@end

/** 管理CDN转码的类。
 */
__attribute__((visibility("default"))) @interface AgoraLiveTranscoding: NSObject
/**
用于旁路直播的输出视频的总尺寸（宽与高）
 */
@property (assign, nonatomic) CGSize size;
/**
 用于旁路直播的输出视频的码率。单位为 Kbps。400 Kbps 为默认值
 */
@property (assign, nonatomic) NSInteger videoBitrate;
/**
 用于旁路直播的输出视频的帧率。单位为帧每秒。15 fps 为默认值
 */
@property (assign, nonatomic) NSInteger videoFramerate;
/** 低延时模式 * True: 低延时，不保证画质
* False:（默认值）高延时，保证画质

 */
@property (assign, nonatomic) BOOL lowLatency;
/**
用于旁路直播的输出视频的 GOP。单位为帧
 */
@property (assign, nonatomic) NSInteger videoGop;
/** 用于旁路直播的输出视频的编解码类型: AgoraVideoCodecProfileType
 */
@property (assign, nonatomic) AgoraVideoCodecProfileType videoCodecProfile;

/** 用于管理参与旁路直播的视频转码合图的用户。最多支持 17 人同时参与转码合图，更多详情请见以下 AgoraLiveTranscodingUser
 */
@property (copy, nonatomic) NSDictionary<NSNumber *, AgoraLiveTranscodingUser *> *_Nullable transcodingUsers; // <uid, transcodingUser>
/**
 预留参数：用户自定义的发送到 CDN 客户端的信息
 */
@property (copy, nonatomic) NSString *_Nullable transcodingExtraInfo;
/**
 用于旁路直播的输出视频上水印图片的 HTTP (非 HTTPS）url 地址。添加后所有旁路直播的观众都可以看到水印。水印图片的定义详见 AgoraImage 定义
 */
@property (strong, nonatomic) AgoraImage *_Nullable watermark;
/**
用于旁路直播的输出视频上背景图片的 HTTP（非 HTTPS）url 地址。添加后所有旁路直播的观众都可以看到背景图片。背景图片的定义详见 AgoraImage 定义
 */
@property (strong, nonatomic) AgoraImage *_Nullable backgroundImage;
/**
用于旁路直播的输出视频的背景色。格式为 RGB 定义下的六位数字符号
 */
@property (strong, nonatomic) COLOR_CLASS *_Nullable backgroundColor;

/** 用于旁路直播的输出音频的采样率： AgoraAudioSampleRateType
 */
@property (assign, nonatomic) AgoraAudioSampleRateType audioSampleRate;
/**
 于旁路直播的输出音频的码率。单位为 Kbps，默认值为 48，最大值为 128
 */
@property (assign, nonatomic) NSInteger audioBitrate;  // kbit/s
/**
用于旁路直播的输出音频的声道数，默认值为 1。取值范围为 [1, 5] 中的整型，建议取 1 或 2，3、4、5需要特殊播放器支持：

 * 1: 单声道
 * 2: 双声道
 * 3: 三声道
 * 4: 四声道
 * 5: 五声道

 */
@property (assign, nonatomic) NSInteger audioChannels;

+(AgoraLiveTranscoding *_Nonnull) defaultTranscoding;
@end

/** 实况直播注入流配置。
 */
__attribute__((visibility("default"))) @interface AgoraLiveInjectStreamConfig: NSObject
/**
添加进入直播的外部视频源尺寸。默认值为 0，即保留视频导入前的尺寸
 */
@property (assign, nonatomic) CGSize size;
/**
 添加进入直播的外部视频源的 GOP。默认值为 30
 */
@property (assign, nonatomic) NSInteger videoGop;
/**
 添加进入直播的外部视频源的帧率。默认值为 15 fps
 */
@property (assign, nonatomic) NSInteger videoFramerate;
/**
 添加进入直播的外部视频源的码率。默认值为 400 kbit/s.
 */
@property (assign, nonatomic) NSInteger videoBitrate;

/**
 添加进入直播的外部音频采样率。默认值为 48000
 */
@property (assign, nonatomic) AgoraAudioSampleRateType audioSampleRate;
/**
 添加进入直播的外部音频码率。默认值为 48
 */
@property (assign, nonatomic) NSInteger audioBitrate;  // kbit/s
/**
 添加进入直播的外部音频频道。默认值为 1
 */
@property (assign, nonatomic) NSInteger audioChannels;

+(AgoraLiveInjectStreamConfig *_Nonnull) defaultConfig;
@end

__deprecated
__attribute__((visibility("default"))) @interface AgoraRtcVideoCompositingRegion : NSObject
/**
要在该区域中显示的视频的用户ID。
 */
@property (assign, nonatomic) NSUInteger uid;
/**
屏幕上区域的水平位置（0.0到1.0)。
 */
@property (assign, nonatomic) CGFloat x;
/**
 屏幕上区域的垂直位置（0.0到1.0)。
 */
@property (assign, nonatomic) CGFloat y;
/**
 该区域的实际宽度（0.0到1.0)。
 */
@property (assign, nonatomic) CGFloat width;
/**
该区域的实际高度（0.0至1.0）。
 */
@property (assign, nonatomic) CGFloat height;
/**
 0表示该区域在底部，而100表示该区域在顶部。
 */
@property (assign, nonatomic) NSInteger zOrder; // Optional, [0, 100] // 0 (default): Minimum, 100: Maximum
/**
0表示该区域是透明的，1表示该区域不透明。 默认值是1.0。
 */
@property (assign, nonatomic) CGFloat alpha; // Optional, [0, 1.0] // 0: Transparent, 1.0: Opaque
/**
 AgoraVideoRenderMode
 */
@property (assign, nonatomic) AgoraVideoRenderMode renderMode;
@end

__deprecated
__attribute__((visibility("default"))) @interface AgoraRtcVideoCompositingLayout : NSObject
/**
 整个画布的宽度（显示窗口或屏幕)。
 */
@property (assign, nonatomic) NSInteger canvasWidth;
/**
 整个画布的高度（显示窗口或屏幕)。
 */
@property (assign, nonatomic) NSInteger canvasHeight;
/**
 输入RGB中定义的任何6位符号。
 */
@property (copy, nonatomic) NSString * _Nullable backgroundColor;// For example, "#c0c0c0"
/**
 AgoraRtcVideoCompositingRegion
 */
@property (copy, nonatomic) NSArray<AgoraRtcVideoCompositingRegion *> * _Nullable regions;
/**
应用定义的数据。
 */
@property (copy, nonatomic) NSString * _Nullable appData;// App defined data
@end

/**
 @deprecated
 */
__deprecated
__attribute__((visibility("default"))) @interface AgoraPublisherConfiguration : NSObject
@property (assign, nonatomic) BOOL owner;

/**
为CDN Live设置的输出数据流的宽度。360是默认值。
 */
@property (assign, nonatomic) NSInteger width;
/**
 为CDN Live设置的输出数据流的高度。640是默认值。
 */
@property (assign, nonatomic) NSInteger height;
/**
 为CDN Live设置的输出数据流的帧速率。 15 fps是默认值。
 */
@property (assign, nonatomic) NSInteger framerate;
/**
 为CDN Live设置的输出数据流的比特率。 500 kbit/s是默认值。
 */
@property (assign, nonatomic) NSInteger bitrate;
/**
 为CDN Live设置的输出数据流的音频采样率。 默认值是48000。
 */
@property (assign, nonatomic) NSInteger audiosamplerate;
/**
 为CDN Live设置的输出数据流的音频比特率。 默认值是48。
 */
@property (assign, nonatomic) NSInteger audiobitrate;
/**
 为CDN Live设置的输出数据流的音频通道。 默认值是1。
 */
@property (assign, nonatomic) NSInteger audiochannels;
/**

* 0: 水平平铺
* 1: 分层Windows
* 2: 垂直平铺

 */
@property (assign, nonatomic) NSInteger defaultLayout;
/**
 AgoraRtmpStreamLifeCycle
 */
@property (assign, nonatomic) AgoraRtmpStreamLifeCycle lifeCycle;
@property (assign, nonatomic) NSInteger injectStreamWidth;
@property (assign, nonatomic) NSInteger injectStreamHeight;
@property (copy, nonatomic) NSString * _Nullable injectStreamUrl;
@property (copy, nonatomic) NSString * _Nullable publishUrl;
@property (copy, nonatomic) NSString * _Nullable rawStreamUrl;
@property (copy, nonatomic) NSString * _Nullable extraInfo;
-(BOOL) validate;
@end

#if (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC))
__attribute__((visibility("default"))) @interface AgoraRtcDeviceInfo : NSObject
@property (assign, nonatomic) int index __deprecated;
@property (assign, nonatomic) AgoraMediaDeviceType type;
@property (copy, nonatomic) NSString * _Nullable deviceId;
@property (copy, nonatomic) NSString * _Nullable deviceName;
@end
#endif


__attribute__((visibility("default"))) @interface AgoraVideoFrame : NSObject
/**视频格式:

 * 1: I420
 * 2: BGRA
 * 3: NV21
 * 4: RGBA
 * 5: IMC2
 * 7: ARGB
 * 8: NV12
 * 12: iOS纹理(CVPixelBufferRef)

 */
@property (assign, nonatomic) NSInteger format;
@property (assign, nonatomic) CMTime time; // Time for this frame.
@property (assign, nonatomic) int stride DEPRECATED_MSG_ATTRIBUTE("use strideInPixels instead");
@property (assign, nonatomic) int strideInPixels; // Number of pixels between two consecutive rows. Note: in pixel, not byte.
// Not used for iOS textures.
@property (assign, nonatomic) int height; // Number of rows of pixels. Not used for iOS textures.

@property (assign, nonatomic) CVPixelBufferRef _Nullable textureBuf;

@property (strong, nonatomic) NSData * _Nullable dataBuf;  // Raw data buffer. Not used for iOS textures.
@property (assign, nonatomic) int cropLeft;   // Number of pixels to crop on the left boundary.
@property (assign, nonatomic) int cropTop;    // Number of pixels to crop on the top boundary.
@property (assign, nonatomic) int cropRight;  // Number of pixels to crop on the right boundary.
@property (assign, nonatomic) int cropBottom; // Number of pixels to crop on the bottom boundary.
@property (assign, nonatomic) int rotation;   // 0, 90, 180, 270. See document for rotation calculation.
/* Note
 * 1. strideInPixels
 *    Stride is in pixels, not bytes
 * 2. About the frame width and height
 *    No field is defined for the width. However, it can be deduced by:
 *       croppedWidth = (strideInPixels - cropLeft - cropRight)
 *    And
 *       croppedHeight = (height - cropTop - cropBottom)
 * 3. About crop
 *    _________________________________________________________________.....
 *    |                        ^                                      |  ^
 *    |                        |                                      |  |
 *    |                     cropTop                                   |  |
 *    |                        |                                      |  |
 *    |                        v                                      |  |
 *    |                ________________________________               |  |
 *    |                |                              |               |  |
 *    |                |                              |               |  |
 *    |<-- cropLeft -->|          valid region        |<- cropRight ->|
 *    |                |                              |               | height
 *    |                |                              |               |
 *    |                |_____________________________ |               |  |
 *    |                        ^                                      |  |
 *    |                        |                                      |  |
 *    |                     cropBottom                                |  |
 *    |                        |                                      |  |
 *    |                        v                                      |  v
 *    _________________________________________________________________......
 *    |                                                               |
 *    |<---------------- strideInPixels ----------------------------->|
 *
 *    If your buffer contains garbage data, you can crop them. For example, the frame size is
 *    360 x 640, often the buffer stride is 368, that is, there extra 8 pixels on the
 *    right are for padding, and should be removed. In this case, you can set:
 *    strideInPixels = 368;
 *    height = 640;
 *    cropRight = 8;
 *    // cropLeft, cropTop, cropBottom are set to a default of 0
 */

@end
