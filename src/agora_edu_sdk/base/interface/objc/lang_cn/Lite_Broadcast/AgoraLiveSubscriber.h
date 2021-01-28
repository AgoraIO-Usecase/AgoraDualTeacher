//
//  AgoraLiveSubscriber.h
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

/** 用于管理与实时订阅相关的操作的类。
 */
@class AgoraLiveSubscriber;

/** 提供带回调的AgoraLiveSubscriber类的协议。
 */
@protocol AgoraLiveSubscriberDelegate <NSObject>
@optional

// 订户
- (void)subscriber: (AgoraLiveSubscriber *_Nonnull)subscriber publishedByHostUid:(NSUInteger)uid streamType:(AgoraMediaType) type;

- (void)subscriber: (AgoraLiveSubscriber *_Nonnull)subscriber streamTypeChangedTo:(AgoraMediaType) type byHostUid:(NSUInteger)uid;

// 取消静音，离线
- (void)subscriber: (AgoraLiveSubscriber *_Nonnull)subscriber unpublishedByHostUid:(NSUInteger)uid;

// 视频
/**
 *  返回屏幕上远程用户呈现的第一帧。
 *
 *  @param subscriber     活用户。
 *  @param uid     远程用户ID。
 *  @param size    视频流的大小。
 *  @param elapsed 从会话开始时间过去的时间（毫秒）
 */
- (void)subscriber:(AgoraLiveSubscriber *_Nonnull)subscriber firstRemoteVideoDecodedOfHostUid:(NSUInteger)uid size:(CGSize)size elapsed:(NSInteger)elapsed;

/**
 *  返回为本地或远程用户更改的视频大小
 *
 *  @param subscriber     活用户。
 *  @param uid     用户ID
 *  @param size    新的视频大小
 *  @param rotation  新的视频旋转
 */
- (void)subscriber:(AgoraLiveSubscriber *_Nonnull)subscriber videoSizeChangedOfHostUid:(NSUInteger)uid size:(CGSize)size rotation:(NSInteger)rotation;
@end


__attribute__((visibility("default"))) @interface AgoraLiveSubscriber: NSObject // AgoraLiveSubscriber

-(instancetype _Nonnull)initWithLiveKit:(AgoraLiveKit * _Nonnull)kit;

-(void)setDelegate:(_Nullable id<AgoraLiveSubscriberDelegate>)delegate;

- (void)subscribeToHostUid:(NSUInteger)uid
             mediaType:(AgoraMediaType)mediaType
                  view:(VIEW_CLASS *_Nullable)view
            renderMode:(AgoraVideoRenderMode)mode
             videoType:(AgoraVideoStreamType)videoType;

-(void)unsubscribeToHostUid:(NSUInteger)uid;

@end
