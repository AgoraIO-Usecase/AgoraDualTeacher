//
//  AgoraLivePublisher.h
//  AgoraLiveKit
//
//  Created by Sting Feng on 2015-8-11.
//  Copyright (c) 2015 Agora. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AgoraObjects.h"

/** 用于管理实况广播的基类。 除发布和订阅外，AgoraLiveKit还管理频道和所有操作。
 */
@class AgoraLiveKit;

/** 管理与实时发布相关的操作的类。
 */
@class AgoraLivePublisher;

/** 提供带回调的AgoraLivePublisher类的协议。
 */
@protocol AgoraLivePublisherDelegate <NSObject>
@optional
-(void)publisher:(AgoraLivePublisher *_Nonnull)publisher streamPublishedWithUrl:(NSString *_Nonnull)url error:(AgoraErrorCode)error;
-(void)publisher:(AgoraLivePublisher *_Nonnull)publisher streamUnpublishedWithUrl:(NSString *_Nonnull)url;
-(void)publisherTranscodingUpdated: (AgoraLivePublisher *_Nonnull)publisher;

-(void)publisher:(AgoraLivePublisher *_Nonnull)publisher streamInjectedStatusOfUrl:(NSString *_Nonnull)url uid:(NSUInteger)uid status:(AgoraInjectStreamStatus)status;
@end


__attribute__((visibility("default"))) @interface AgoraLivePublisher: NSObject

-(void)setDelegate:(_Nullable id<AgoraLivePublisherDelegate>)delegate;

-(instancetype _Nonnull)initWithLiveKit:(AgoraLiveKit *_Nonnull)kit;

/** @deprecated
 手动设置视频属性
 
 @param resolution 您想要设置的视频的分辨率。 最高值是1280 x 720。
 @param frameRate 您要设置的视频的帧速率。 最高值为30.您可以将其设置为5,10,15,24,30等等。
 @param bitrate 您想要设置的视频的比特率。 您需要根据宽度，高度和帧速率手动计算帧速率。 在宽度和高度相同的情况下，比特率随帧速率的变化而变化：

* 如果帧频为5 fps，请将建议的比特率除以2。
* 如果帧频为15 fps，请使用建议的比特率。
* 如果帧频为30 fps，则将建议的比特率乘以1.5。
* 如果您选择其他帧速率，请按比例计算比特率。

如果您设置的比特率超出了适当的范围，则SDK会自动将其调整到该范围内的值。
 */
- (void)setVideoResolution:(CGSize)resolution andFrameRate:(NSInteger)frameRate bitrate:(NSInteger)bitrate;

/** 设置直播转码
 
该方法用于 CDN 推流的视图布局及音频设置等。
 
 Note: 该功能正在开发阶段，预计会在一个月后上线。

 @param transcoding 一个 AgoraLiveTranscoding 的对象。
 */
-(void)setLiveTranscoding:(AgoraLiveTranscoding *_Nullable)transcoding;


/** 添加本地视频水印
 
 该方法将一张 PNG 图片作为水印添加到本地发布的直播视频流上，同一直播频道中的观众，旁路直播观众，甚至录制设备都能看到或采集到该水印图片。 如果你仅仅希望在旁路直播推流中添加水印，请参考 设置直播转码 [setLiveTranscoding](setLiveTranscoding:) 中描述的用法。

 @param watermark AgoraImage
 @return Name of the watermark image.
 */
-(int)addVideoWatermark:(AgoraImage * _Nonnull)watermark  NS_SWIFT_NAME(addVideoWatermark(_:));

/** 删除已添加的视频水印
 */
-(void)clearVideoWatermarks;

/** 设置媒体类型。

 @param mediaType AgoraMediaType
 */
-(void)setMediaType:(AgoraMediaType)mediaType;

/** 增加推流地址
 
 该方法用于 CDN 推流，用于添加推流地址。
 
 Note:
 
 * 请确保在成功加入房间后才能调用该接口。
 * 该方法每次只能增加一路 CDN 推流地址。若需推送多路流，则需多次调用该方法。
 * url 不支持中文等特殊字符。
 
 @param url 推流地址
 @param transcodingEnabled * True: 转码
 * False: 不转码

 */
-(void)addStreamUrl:(NSString *_Nullable)url transcodingEnabled:(BOOL)transcodingEnabled;

/** 删除推流地址
 
 该方法用于 CDN 推流，用于删除推流地址。

 @param url 推流地址
 */
-(void)removeStreamUrl:(NSString *_Nullable)url;

/** 发布推流。
 */
-(void)publish;

/** 停止推流发布
 */
-(void)unpublish;

/**
 切换前置/后置摄像头
 
 该方法用于在前置/后置摄像头间切换。
 */
-(void)switchCamera;

/** 导入外部视频流 URL

 @param url 添加到直播中的视频流 url 地址， 支持 RTMP， HLS， FLV 协议传输， 也支持 mp3 / mp4 流。
 @param config AgoraLiveInjectStreamConfig
 */
- (void)addInjectStreamUrl:(NSString *_Nonnull)url config:(AgoraLiveInjectStreamConfig * _Nonnull)config;

/** 删除导入的外部视频源
 
 该方法删除导入的外部视频源。
 
 @param url URL address of the added stream to be removed.
 */
- (void)removeInjectStreamUrl:(NSString *_Nonnull)url;

@end
