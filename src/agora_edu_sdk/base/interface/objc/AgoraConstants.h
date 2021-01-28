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

/** The standard bitrate in [setVideoEncoderConfiguration]([AgoraRtcEngineKit setVideoEncoderConfiguration:]).

 (Recommended) In a live broadcast, Agora recommends setting a larger bitrate to improve the video quality. When you choose AgoraVideoBitrateStandard, the bitrate value doubles in a live broadcast mode, and remains the same as in AgoraVideoProfile in a communication mode.
 */
extern NSInteger const AgoraVideoBitrateStandard;

/** The compatible bitrate in [setVideoEncoderConfiguration]([AgoraRtcEngineKit setVideoEncoderConfiguration:]).

 The bitrate in both the live broadcast and communication modes remain the same as in AgoraVideoProfile.
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
/** 960 x 720 (Depends on the hardware)
 */
extern CGSize const AgoraVideoDimension960x720;
/** 1280 x 720 (Depends on the hardware)
 */
extern CGSize const AgoraVideoDimension1280x720;
#if TARGET_OS_MAC
/** 1920 x 1080 (Depends on the hardware, macOS only)
 */
extern CGSize const AgoraVideoDimension1920x1080;
/** 25400 x 1440 (Depends on the hardware, macOS only)
 */
extern CGSize const AgoraVideoDimension2540x1440;
/** 3840 x 2160 (Depends on the hardware, macOS only)
 */
extern CGSize const AgoraVideoDimension3840x2160;
#endif
