//
//  AgoraConstants.h
//  AgoraRtcEngineKit
//
//  Copyright (c) 2018 Agora. All rights reserved.
//
#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#elif TARGET_OS_MAC
#import <AppKit/AppKit.h>
#endif

/**
 [setVideoEncoderConfiguration]([AgoraRtcEngineKit setVideoEncoderConfiguration:]) 中的标准码率。
 
 (推荐）声网建议在互动直播场景下设定较高的码率以提高图像质量。当你选择了 AgoraVideoBitrateStandard 时，直播模式下码率翻倍而通信模式下维持码率不变。
 */
extern NSInteger const AgoraVideoBitrateStandard;

/**

[setVideoEncoderConfiguration]([AgoraRtcEngineKit setVideoEncoderConfiguration:]) 中的兼容码率
 
 直播模式和通信模式的码率和在 AgoraVideoProfile 标注的标定值保持一致。 
 */
extern NSInteger const AgoraVideoBitrateCompatible;
/** 120 x 120
 */
extern CGSize const AgoraVideoDimension120x120;
/** 160 x 120
 */
extern CGSize const AgoraVideoDimension160x120;
/** 180 x 180
 */
extern CGSize const AgoraVideoDimension180x180;
/** 240 x 180
 */
extern CGSize const AgoraVideoDimension240x180;
/** 320 x 180
 */
extern CGSize const AgoraVideoDimension320x180;
/** 240 x 240
 */
extern CGSize const AgoraVideoDimension240x240;
/** 320 x 240
 */
extern CGSize const AgoraVideoDimension320x240;
/** 424 x 240
 */
extern CGSize const AgoraVideoDimension424x240;
/** 360 x 360
 */
extern CGSize const AgoraVideoDimension360x360;
/** 480 x 360
 */
extern CGSize const AgoraVideoDimension480x360;
/** 640 x 360
 */
extern CGSize const AgoraVideoDimension640x360;
/** 480 x 480
 */
extern CGSize const AgoraVideoDimension480x480;
/** 640 x 480
 */
extern CGSize const AgoraVideoDimension640x480;
/** 840 x 480
 */
extern CGSize const AgoraVideoDimension840x480;
/** 960 x 720 (取决于硬件能力)
 */
extern CGSize const AgoraVideoDimension960x720;
/** 1280 x 720 (取决于硬件能力)
 */
extern CGSize const AgoraVideoDimension1280x720;
#if TARGET_OS_MAC
/** 1920 x 1080 (取决于硬件能力，仅适用于 macOS 平台)
 */
extern CGSize const AgoraVideoDimension1920x1080;
/** 25400 x 1440 (取决于硬件能力，仅适用于 macOS 平台)
 */
extern CGSize const AgoraVideoDimension2540x1440;
/** 3840 x 2160 (取决于硬件能力，仅适用于 macOS 平台)
 */
extern CGSize const AgoraVideoDimension3840x2160;
#endif
