//
//  AgoraMediaIO.h
//  AgoraRtcEngineKit
//
//  Copyright (c) 2018 Agora. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import "AgoraEnumerates.h"
#import "AgoraObjects.h"


/** 视频像素格式
 
 关于 YVU 图像格式的描述，请参考如下链接：
 http://www.fourcc.org/yuv.php and
 https://msdn.microsoft.com/en-us/library/windows/desktop/dd206750(v=vs.85).aspx */
typedef NS_ENUM(NSUInteger, AgoraVideoPixelFormat) {
    /** I420 */
    AgoraVideoPixelFormatI420   = 1,
    /** BGRA */
    AgoraVideoPixelFormatBGRA   = 2,
    /** NV12 */
    AgoraVideoPixelFormatNV12   = 8,
};

/** 视频的顺时针旋转角度
 
 如果设置为其他数字，系统会自动忽略
 */
typedef NS_ENUM(NSInteger, AgoraVideoRotation) {
    /** 顺时针旋转 0 度 */
    AgoraVideoRotationNone      = 0,
    /** 顺时针旋转 90 度 */
    AgoraVideoRotation90        = 1,
    /** 顺时针旋转 180 度 */
    AgoraVideoRotation180       = 2,
    /** 顺时针旋转 270 度 */
    AgoraVideoRotation270       = 3,
};

 /** Buffer 类型*/
typedef NS_ENUM(NSInteger, AgoraVideoBufferType) {
   /** 使用 Pixel Buffer 类型的 Buffer */
    AgoraVideoBufferTypePixelBuffer = 1,
    /** 使用 Raw Data 类型的 Buffer */
    AgoraVideoBufferTypeRawData     = 2,
};

/** AgoraVideoFrameConsumer 支持接收两种 Buffer 类型的视频帧数据：PixelBuffer 和裸数据。 自定义视频源时，开发者需要通过 [bufferType]([AgoraVideoSinkProtocol bufferType]) 来指定一种 Buffer 类型，并在自定义视频源中只使用与其对应的方法来传递视频帧数据。 各类型包含的视频帧信
  */
@protocol AgoraVideoFrameConsumer <NSObject>

/** PixelBuffer 类型

 @param pixelBuffer PixelBuffer 类型的视频 Buffer
 @param timestamp 传入的视频帧的时间戳，以毫秒为单位。
 @param rotation AgoraVideoRotation
 */
- (void)consumePixelBuffer:(CVPixelBufferRef _Nonnull)pixelBuffer
             withTimestamp:(CMTime)timestamp
                  rotation:(AgoraVideoRotation)rotation;
/** RawData 类型

 @param rawData RawData 类型的视频 Buffer
 @param timestamp 传入的视频帧的时间戳，以毫秒为单位。
 @param format AgoraVideoPixelFormat
 @param size 视频裸数据的尺寸
 @param rotation AgoraVideoRotation
 */
- (void)consumeRawData:(void * _Nonnull)rawData
         withTimestamp:(CMTime)timestamp
                format:(AgoraVideoPixelFormat)format
                  size:(CGSize)size
              rotation:(AgoraVideoRotation)rotation;
@end

/** [AgoraVideoSourceProtocol]([AgoraVideoSourceProtocol]) 定义了一套协议，开发者通过实现该接口，来创建自定义的视频源，并设置给 Agora 底层的 Media Engine
 
 实时通讯过程中，Agora SDK 通常会启动默认的视频输入设备，即内置的摄像头，进行视频推流。 使用  [AgoraVideoSourceProtocol]([AgoraVideoSourceProtocol]) 接口可以自定义视频源。通过调用 设置视频源 [setVideoSource]([AgoraRtcEngineKit setVideoSource:]) 接口，可以改变并控制默认的视频输入设备，再将自定义的视频源发送给 Agora Media Engine，让 Media Engine 进行其它视频处理，如过滤视频、将视频发布到 RTC 链接等。
 
 [AgoraVideoSourceProtocol]([AgoraVideoSourceProtocol])由以下方法组成:

* 初始化视频源 ([shouldInitialize](shouldInitialize))
* 启动视频源 ([shouldStart](shouldStart))
* 停止视频源 ([shouldStop](shouldStop))
* 释放视频源 ([shouldDispose](shouldDispose))
* 获取 Buffer 类型 (AgoraVideoBufferType)
 
 */
@protocol AgoraVideoSourceProtocol <NSObject>
@required
@property (strong) id<AgoraVideoFrameConsumer> _Nullable consumer;
/** 初始化视频源
 
 Media Engine 在初始化视频源的时候会回调此方法。开发者可以在这个方法中做一些准备工作，例如打开 Camera，或者初始化视频源，并通过返回值告诉 Media Engine，自定义的视频源是否已经准备好。
 
 Note: 初始化视频源过程中，开发者需要在 [bufferType]([AgoraVideoSinkProtocol bufferType]) 中指定一种 Buffer 类型，并在自定义视频源中只使用与其对应的方法来传递视频帧数据。
 
 在初始化视频源过程中，Media Engine 会传递给开发者的一个 AgoraVideoFrameConsumer 对象。开发者需要保存该对象，并在视频源启动后，通过这个对象把视频帧输入给 Media Engine。

 @return BOOL 开发者需要手动输入返回值，以告诉 Media Engine 自定义视频源是否已准备好：
 
 * True: 自定义的视频源已经完成了初始化工作
 * False: 自定义的视频源设备没准备好或者初始化失败，Media Engine 会停下来并上报错误

 */
- (BOOL)shouldInitialize;

/** 启动视频源
 
 Media Engine 在启动视频源时会回调这个方法。开发者可以在该方法中启动视频帧捕捉。开发者需要通过返回值告诉告知 Media Engine 自定义的视频源是否开启成功。
 
 开发者需要手动输入返回值，以告诉 Media Engine 自定义视频源是否开启：
 
 * True：自定义的视频源已成功开启，接下来会打开 [AgoraVideoFrameConsumer]([AgoraVideoFrameConsumer]) 的开关，接收开发者传输的视频帧
 * False：自定义的视频源设备启动失败，Media Engine 会停下来并上报错误
 
 */
- (void)shouldStart;
/** 停止视频源
 
 Media Engine 在停止视频源的时候会回调这个方法。开发者可以在这个方法中停止视频的采集。Media Engine 通过这个回调通知开发者，[AgoraVideoFrameConsumer]([AgoraVideoFrameConsumer]) 的帧输入开关即将关闭，之后输入的视频帧都会被丢弃。
 */
- (void)shouldStop;

/** 释放视频源
 
Media Engine 通知开发者视频源即将失效，开发者可以在这个方法中关闭视频源设备。引擎会销毁 [AgoraVideoFrameConsumer]([AgoraVideoFrameConsumer]) 对象，开发者需要确保在此回调之后不再使用它。
 */
- (void)shouldDispose;
/** 获取 Buffer 类型
 
 Media Engine 在初始化的时候，会调用这个方法来查询该视频源所使用的 Buffer 类型。开发者必须指定且只能指定一种 Buffer 类型并通过返回值告诉 Media Engine
 
 @return AgoraVideoBufferType

 */
- (AgoraVideoBufferType)bufferType;
@end

/** [AgoraVideoSinkProtocol]([AgoraVideoSinkProtocol])  定义了一套协议，开发者通过实现该接口，来创建自定义的视频渲染器，并设置给 Agora 底层的 Media Engine。
 
 实时通讯过程中，Agora SDK 通常会启动默认的视频渲染器进行视频渲染。 [AgoraVideoSinkProtocol]([AgoraVideoSinkProtocol])  可以自定义视频渲染器，再通过调用 设置本地视频渲染器 [setLocalVideoRenderer]([AgoraRtcEngineKit setLocalVideoRenderer:]) 和 设置远端视频渲染器 [setRemoteVideoRenderer]([AgoraRtcEngineKit setRemoteVideoRenderer:forUserId:]) 接口，改变并控制默认的视频渲染器。
 
 [AgoraVideoSinkProtocol]([AgoraVideoSinkProtocol])由以下方法组成:

 * 初始化渲染器([shouldInitialize](shouldInitialize))
 * 启动渲染器 ([shouldStart](shouldStart))
 * 停止渲染器 ([shouldStop](shouldStop))
 * 释放渲染器 ([shouldDispose](shouldDispose))
 * Gets the Buffer Type ([AgoraVideoBufferType](AgoraVideoBufferType))
 * Gets the Pixel Format ([AgoraVideoPixelFormat](AgoraVideoPixelFormat))
 * (Optional) Outputs the Video in the Pixel Buffer ([renderPixelBuffer](renderPixelBuffer:rotation:))
 * (Optional) Outputs the Video in the Raw Data ([renderRawData](renderRawData:size:rotation:))

 Note: All methods defined in [AgoraVideoSinkProtocol]([AgoraVideoSinkProtocol]) are callback methods. The media engine uses these methods to inform the customized renderer of its internal changes.
 An example is shown in the following steps to customize the video sink:

 1. Call bufferType and AgoraVideoPixelFormat to set the buffer type and pixel format of the video frame.
 2. Implement [shouldInitialize](shouldInitialize), [shouldStart](shouldStart), [shouldStop](shouldStop), and [shouldDispose](shouldDispose) to manage the customized video sink.
 3. Implement the buffer type and pixel format as specified in [AgoraVideoFrameConsumer]([AgoraVideoFrameConsumer]).
 4. Create the customized video sink object.
 5. Call the [setLocalVideoRenderer]([AgoraRtcEngineKit setLocalVideoRenderer:]) and [setRemoteVideoRenderer]([AgoraRtcEngineKit setRemoteVideoRenderer:forUserId:]) methods to set the local and remote renderers.
 6. The media engine will call functions in [AgoraVideoSinkProtocol]([AgoraVideoSinkProtocol]) according to its internal state.
 
 */
@protocol AgoraVideoSinkProtocol <NSObject>
@required
/** 初始化渲染器
 
 Media Engine 初始化渲染器的时候调用这个方法。开发者可以在这个方法中做渲染器的初始化工作。如果是耗时操作，也可以提前初始化好，然后在这个方法中通过返回值告知 Media Engine 自定义渲染器已初始化好。
 该方法需要开发者输入返回值，告知 Media Engine 自定义渲染器的状态：
 

 @return BOOL * True: Media Engine 会认为自定义的渲染器已经初始化好
 * False: Media Engine 会认为自定义的渲染器初始化失败，不继续往下运行
 
 */
- (BOOL)shouldInitialize;
/** 启动渲染器
 
 Media Engine 在开启渲染功能的时候会回调这个方法。开发者可以在这个方法中启动渲染器。
 该方法需要开发者输入返回值，Media Engine 会根据返回值做对应的动作：
 
 * True：Media Engine 继续进行渲染
 * False：Media Engine 认为出错而停止渲染器的功能

 */
- (void)shouldStart;
/** 停止渲染器
 
Media Engine 在停止渲染功能的时候会回调这个方法。开发者可以在这个方法中停止渲染。
 */
- (void)shouldStop;
/** 释放渲染器
 
  Media Engine 通知开发者渲染器即将被废弃。在 [shouldDispose](shouldDispose) 返回之后，开发者就可以释放掉资源了。
 */
- (void)shouldDispose;
/** 获取 Buffer 类型
 
 用于在自定义渲染器的时候，需要指定 Buffer 类型，通过返回值告知引擎。Media Engine 会调用这个方法并检查返回值类型。

 @return bufferType AgoraVideoBufferType
 */
- (AgoraVideoBufferType)bufferType;
/** 获取像素格式
 
 用于自定义渲染器的时候，还需要指定视频数据的像素格式。

 @return pixelFormat AgoraVideoPixelFormat
 */
- (AgoraVideoPixelFormat)pixelFormat;
@optional
/** （可选）输出视频像素 Buffer
 
 该方法输出 Buffer 格式的视频数据。

 @param pixelBuffer Buffer 格式的视频像素
 @param rotation 视频像素的顺时针旋转角度, AgoraVideoRotation
 */
- (void)renderPixelBuffer:(CVPixelBufferRef _Nonnull)pixelBuffer
                 rotation:(AgoraVideoRotation)rotation;
/** 输出视频裸数据

 @param rawData RawData 格式的视频像素
 @param size 视频像素的尺寸
 @param rotation 视频像素的顺时针旋转角度, AgoraVideoRotation
 */
- (void)renderRawData:(void * _Nonnull)rawData
                 size:(CGSize)size
             rotation:(AgoraVideoRotation)rotation;
@end


#pragma mark - Agora default media io
    /** 默认相机位置 */
typedef NS_ENUM(NSInteger, AgoraRtcDefaultCameraPosition) {
/** 前置摄像头  */
    AgoraRtcDefaultCameraPositionFront = 0,
    /** 后置摄像头  */
    AgoraRtcDefaultCameraPositionBack = 1,
};

__attribute__((visibility("default"))) @interface AgoraRtcDefaultCamera: NSObject<AgoraVideoSourceProtocol>
#if TARGET_OS_IPHONE
@property (nonatomic, assign) AgoraRtcDefaultCameraPosition position;
- (instancetype _Nonnull)initWithPosition:(AgoraRtcDefaultCameraPosition)position;
#endif
@end

#if (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC))
__attribute__((visibility("default"))) @interface AgoraRtcScreenCapture: NSObject<AgoraVideoSourceProtocol>
@property (nonatomic, readonly) NSUInteger windowId;
+ (instancetype _Nonnull)fullScreenCaptureWithFrequency:(NSInteger)captureFrequency
                                                bitRate:(NSInteger)bitRate;
+ (instancetype _Nonnull)windowCaptureWithId:(CGWindowID)windowId
                            captureFrequency:(NSInteger)captureFrequency
                                     bitRate:(NSInteger)bitRate
                                        rect:(CGRect)rect;
@end
#endif

__attribute__((visibility("default"))) @interface AgoraRtcDefaultRenderer: NSObject<AgoraVideoSinkProtocol>
@property (nonatomic, strong, readonly) VIEW_CLASS * _Nonnull view;
@property (nonatomic, assign) AgoraVideoRenderMode mode;
- (instancetype _Nonnull)initWithView:(VIEW_CLASS * _Nonnull)view
                           renderMode:(AgoraVideoRenderMode)mode;
@end
