//
//  AgoraLiveKit.h
//  AgoraLiveKit
//
//  Created by Junhao Wang
//  Copyright (c) 2017 Agora. All rights reserved.
//

#import <Foundation/Foundation.h>

#import "AgoraLivePublisher.h"
#import "AgoraLiveSubscriber.h"
#import "AgoraRtcEngineKit.h"

/** 用于启用或禁用视频。
 */
__attribute__((visibility("default"))) @interface AgoraLiveChannelConfig: NSObject
@property (assign, nonatomic) BOOL videoEnabled;

+(AgoraLiveChannelConfig *_Nonnull) defaultConfig;
@end

/** 用于管理实况广播的基类。
 
 除发布和订阅外，AgoraLiveKit还管理频道和所有操作。
 */
@class AgoraLiveKit;

/** 提供带回调的AgoraLiveKit类的协议。
 */
@protocol AgoraLiveDelegate <NSObject>
@optional

/** 发生警告回调
 
 该回调方法表示 SDK 运行时出现了（网络或媒体相关的）警告。通常情况下，SDK 上报的警告信息应用程序可以忽略，SDK 会自动恢复。比如和服务器失去连接时，SDK 可能会上报 AgoraRtc_Error_OpenChannelTimeout(106) 警告，同时自动尝试重连。
 
 Note: 部分 Warning Code 在上报给客户之前，会经过映射，映射后的 Warning Code 与原来的 Warning Code 意义相同，但错误类别更清晰，方便客户判断。详细的错误代码及映射表，见 错误代码及映射。

 @param kit         AgoraRtcEngineKit 对象
 @param warningCode 警告代码，详见AgoraWarningCode
 */
- (void)liveKit:(AgoraLiveKit *_Nonnull)kit didOccurWarning:(AgoraWarningCode)warningCode;

/** 发生错误回调
 
 该回调方法表示 SDK 运行时出现了（网络或媒体相关的）错误。通常情况下，SDK 上报的错误意味着 SDK 无法自动恢复，需要应用程序干预或提示用户。 比如启动通话失败时，SDK 会上报 AgoraRtc_Error_StartCall(1002) 错误。应用程序可以提示用户启动通话失败，并调用 leaveChannel() 退出频道。
 
 Note: 部分 Error Code 在上报给客户之前，会经过映射，映射后的 Error Code 与原来的 Error Code 意义相同，但错误类别更清晰，方便客户判断。详细的错误代码及映射表，见 错误代码及映射 。

 @param kit       AgoraRtcEngineKit 对象
 @param errorCode 错误代码, 详见AgoraErrorCode
 */
- (void)liveKit:(AgoraLiveKit *_Nonnull)kit didOccurError:(AgoraErrorCode)errorCode;

/** 加入频道回调
 
 该回调方法表示该客户端成功加入了指定的频道。同 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) API 的 joinSuccessBlock 回调。

 @param kit     AgoraRtcEngineKit 对象
 @param channel 频道名称
 @param uid     本地用户ID
 @param elapsed 从会话开始时间过去的时间（毫秒）
 */
- (void)liveKit:(AgoraLiveKit *_Nonnull)kit didJoinChannel:(NSString *_Nonnull)channel withUid:(NSUInteger)uid elapsed:(NSInteger) elapsed;

/** 离开频道时的通话统计回调

 @param kit    AgoraRtcEngineKit 对象
 */
- (void)liveKitDidLeaveChannel:(AgoraLiveKit *_Nonnull)kit;

/** 重新加入频道回调

 @param kit     AgoraRtcEngineKit 对象
 @param channel 频道名称
 @param uid     用户ID
 @param elapsed 从会话开始时间过去的时间（毫秒）
 */
- (void)liveKit:(AgoraLiveKit *_Nonnull)kit didRejoinChannel:(NSString *_Nonnull)channel withUid:(NSUInteger)uid elapsed:(NSInteger) elapsed;

/** Token 过期回调
 
 在调用 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 时如果指定了 Token，由于 Token 具有一定的时效，在通话过程中SDK可能由于网络原因和服务器失去连接，重连时可能需要新的 Token。 该回调通知APP需要生成新的 Token，并需调用 [renewToken](renewToken:) 为 SDK 指定新的 Token。
 之前的处理方法是在 [didOccurError][(AgoraLiveDelegate liveKit:didOccurError:]) 回调报告 ERR_TOKEN_EXPIRED(109)、ERR_INVALID_TOKEN(110) 时，APP 需要生成新的 Token。 在新版本中，原来的处理仍然有效，但建议把相关逻辑放进该回调里。
 
 
 @param kit AgoraRtcEngineKit 对象
 */
- (void)liveKitRequestToken:(AgoraLiveKit *_Nonnull)kit;

- (void)liveKit:(AgoraLiveKit * _Nonnull)kit tokenPrivilegeWillExpire:(NSString * _Nonnull)token;

// statistics

/** 每两秒返回一次RTC对象状态的统计信息。

 @param kit    AgoraRtcEngineKit 对象
 @param stats  RTC状态的统计信息，包括持续时间，发送字节数和接收字节数
 */
- (void)liveKit:(AgoraLiveKit *_Nonnull)kit reportLiveStats:(AgoraChannelStats *_Nonnull)stats;
// network

/** 网络连接中断回调
 
 该回调方法表示 SDK 和服务器失去了网络连接。
 与 [rtcEngineConnectionDidLost]([AgoraRtcEngineDelegate rtcEngineConnectionDidLost:]) 回调的区别是: liveKitEngineConnectionDidInterrupted 回调在 SDK 刚失去和服务器连接时触发，[rtcEngineConnectionDidLost]([AgoraRtcEngineDelegate rtcEngineConnectionDidLost:]) 在失去连接且尝试自动重连失败后才触发。 失去连接后，除非 APP 主动调用 [leaveChannel](leaveChannel)，不然 SDK 会一直自动重连。

 @param kit    AgoraRtcEngineKit 对象
 */
- (void)liveKitConnectionDidInterrupted:(AgoraLiveKit *_Nonnull)kit;

/** 网络连接丢失回调
 
 在 SDK 和服务器失去了网络连接后，会触发 [ConnectionDidInterrupted]([AgoraRtcEngineDelegate rtcEngineConnectionDidInterrupted:]) 回调，并自动重连。如果重连一定时间内（默认 10 秒）后仍未连上，会触发该回调。 如果 SDK 在调用 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 后 10 秒内没有成功加入频道，也会触发该回调。 该回调触发后，SDK 仍然会尝试重连，重连成功后会触发 [rtcEngine:didRejoinChannel]([AgoraRtcEngineDelegate rtcEngine:didRejoinChannel:withUid:elapsed:]) 回调。
 

@param kit    AgoraRtcEngineKit 对象
 */
- (void)liveKitConnectionDidLost:(AgoraLiveKit *_Nonnull)kit;

/** 频道内网络质量报告回调

 @param kit     AgoraRtcEngineKit 对象
 @param uid     用户ID
 @param txQuality 网络传输质量
 @param rxQuality 网络接收质量
 */
- (void)liveKit:(AgoraLiveKit *_Nonnull)kit networkQuality:(NSUInteger)uid txQuality:(AgoraNetworkQuality)txQuality rxQuality:(AgoraNetworkQuality)rxQuality;
@end

__attribute__((visibility("default"))) @interface AgoraLiveKit : NSObject

@property (weak, nonatomic) _Nullable id<AgoraLiveDelegate> delegate;

/** 查询 SDK 版本号
 
 该方法返回 SDK 版本号字符串。

@return string, SDK 版本号
*/
+ (NSString *_Nonnull)getSdkVersion;

/** 返回SDK引擎的本机处理程序
 */
- (AgoraRtcEngineKit *_Nonnull)getRtcEngineKit;

/** 初始化AgoraLiveKit对象。

 @param appId appId由Agora发布给应用程序开发人员。

 @return AgoraLiveKit类的一个对象
 */
+ (instancetype _Nonnull)sharedLiveKitWithAppId:(NSString *_Nonnull)appId;

+ (void)destroy;

/** 核心方法 */

/**  加入频道
 
 该方法让用户加入通话频道，在同一个频道内的用户可以互相通话，多个用户加入同一个频道，可以群聊。 使用不同 App ID 的应用程序是不能互通的。如果已在通话中，用户必须调用 leaveChannel() 退出当前通话，才能进入下一个频道。 SDK 在通话中使用 iOS 系统的 AVAudioSession 共享对象进行录音和播放，应用程序对该对象的操作可能会影响 SDK 的音频相关功能。
 调用该 API 后会触发回调，如果同时实现 joinChannelSuccessBlock 和 [didJoinChannel]([AgoraRtcEngineDelegate rtcEngine:didJoinChannel:withUid:elapsed:]) 会出现冲突，joinChannelSuccessBlock 优先级高，2 个同时存在时，[didJoinChannel]([AgoraRtcEngineDelegate rtcEngine:didJoinChannel:withUid:elapsed:]) 会被忽略。 需要有 [didJoinChannel]([AgoraRtcEngineDelegate rtcEngine:didJoinChannel:withUid:elapsed:]) 回调时，请将 joinChannelSuccessBlock 设置为 nil。
 
 Note: 同一个频道里不能出现两个相同的 UID。如果你的 App 支持多设备同时登录，即同一个用户账号可以在不同的设备上同时登录(例如微信支持在 PC 端和移动端同时登录)，请保证传入的 UID 不相同。 例如你之前都是用同一个用户标识作为 UID, 建议从现在开始加上设备 ID, 以保证传入的 UID 不相同 。如果你的 App 不支持多设备同时登录，例如在电脑上登录时，手机上会自动退出，这种情况下就不需要在 UID 上添加设备 ID。
 
 @param token * 安全要求不高: 将值设为 null
 *安全要求高: 将值设置为 Token。如果你已经启用了 App Certificate, 请务必使用 Token。关于如何获取 Token.
 
 @param channelId 标识通话的频道名称，长度在 64 字节以内的字符串。以下为支持的字符集范围（共 89 个字符）: a-z,A-Z,0-9,space,! #$%&,()+, -,:;<=.#$%&,()+,-,:;<=.,>?@[],^_,{|},~
 @param config AgoraLiveChannelConfig
 @param uid (非必选项) 用户 ID，32 位无符号整数。建议设置范围：1到 (2^32-1)，并保证唯一性。如果不指定（即设为0），SDK 会自动分配一个，并在 onJoinChannelSuccess 回调方法中返回，App 层必须记住该返回值并维护，SDK 不对该返回值进行维护。
 uid 在 SDK 内部用 32 位无符号整数表示，由于 Java 不支持无符号整数，uid 被当成 32 位有符号整数处理，对于过大的整数，Java 会表示为负数，如有需要可以用 (uid&0xffffffffL) 转换成 64 位整数。

 @return * 0: 方法调用成功  * < 0: 方法调用失败
 */
- (int)joinChannelByToken:(NSString *_Nullable)token
               channelId:(NSString *_Nonnull)channelId
            config:(AgoraLiveChannelConfig *_Nonnull)channelConfig
               uid:(NSUInteger)uid;

/** 离开频道
 
 离开频道，即挂断或退出通话。
 当调用 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:)  API 方法后，必须调用 [leaveChannel](leaveChannel) 结束通话，否则无法开始下一次通话。 不管当前是否在通话中，都可以调用 [leaveChannel](leaveChannel)，没有副作用。该方法会把会话相关的所有资源释放掉。该方法是异步操作，调用返回时并没有真正退出频道。在真正退出频道后，SDK 会触发 [didLeaveChannelWithStats]([AgoraRtcEngineDelegate rtcEngine:didLeaveChannelWithStats:)] 回调。
 
 @return * 0: 方法调用成功  * < 0: 方法调用失败
 */
- (int)leaveChannel;


/** 更新 Token
 
 该方法用于更新 Token。如果启用了 Token 机制，过一段时间后使用的 Token 会失效。
 当：
 
 * 发生 rtcEngine:tokenPrivilegeWillExpire: 回调时，
 * rtcEngine:didOccurError: 回调报告 ERR_TOKEN_EXPIRED(109) 时，
 * rtcEngineRequestToken: 回调报告 ERR_TOKEN_EXPIRED(109) 时，
 
 应用程序应重新获取 Token，然后调用该 API 更新 Token，否则 SDK 无法和服务器建立连接。
 
 @param token 指定要更新的 Token
 
 @return * 0: 方法调用成功  * < 0: 方法调用失败
 */
- (int)renewToken:(NSString*_Nonnull)token;

/** 开启视频预览
 
 该方法用于启动本地视频预览。在开启预览前，必须先调用 setupLocalVideo:local: 设置预览窗口及属性，且必须调用 enableVideo 开启视频功能。 如果在调用 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 进入频道之前调用了 startPreview 启动本地视频预览，在调用 [leaveChannel](leaveChannel) 退出频道之后本地预览仍然处于启动状态，如需要关闭本地预览，需要调用 stopPreview。
 
 @param renderMode AgoraVideoRenderMode
 @return * 0: 方法调用成功  * < 0: 方法调用失败
 */
- (int)startPreview:(VIEW_CLASS *_Nonnull)view
         renderMode:(AgoraVideoRenderMode)mode;

/** 停止视频预览
 
 该方法用于停止本地视频预览。
 
 @return * 0: 方法调用成功  * < 0: 方法调用失败
 */
- (int)stopPreview;

@end
