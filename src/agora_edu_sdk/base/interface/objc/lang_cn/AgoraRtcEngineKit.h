//
//  AgoraRtcEngineKit.h
//  AgoraRtcEngineKit
//
//  Copyright (c) 2018 Agora. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "AgoraConstants.h"
#import "AgoraObjects.h"

@class AgoraRtcEngineKit;

/** AgoraRtcEngineDelegate 接口类采用 Delegate 方法用于向应用程序发送回调通知。

 从 1.1 版本开始，SDK 使用 Delegate 代替原有的部分 Block 回调。下文也包括了 Block 回调内容。原有的 Block 回调被标为废弃，目前仍然可以使用，但是建议用相应的 Delegate 方法代替。

 SDK 会通过代理方法 AgoraRtcEngineDelegate 向应用程序上报一些运行时事件。从 1.1 版本开始，SDK 使用 Delegate 代替原有的部分 Block 回调。 原有的 Block 回调被标为废弃，目前仍然可以使用，但是建议用相应的 Delegate 方法代替。如果同一个回调 Block 和 Delegate 方法都有定义，则 SDK 只回调 Block 方法。
 */
@protocol AgoraRtcEngineDelegate <NSObject>
@optional

#pragma mark Core Delegates

/**-----------------------------------------------------------------------------
 * @name Core Delegates
 * -----------------------------------------------------------------------------
 */

/** 发生警告回调

该回调方法表示 SDK 运行时出现了（网络或媒体相关的）警告。通常情况下，SDK 上报的警告信息应用程序可以忽略，SDK 会自动恢复。比如和服务器失去连接时，SDK 可能会上报 AgoraRtc_Error_OpenChannelTimeout(106) 警告，同时自动尝试重连。

 Note: 部分 Warning Code 在上报给客户之前，会经过映射，映射后的 Warning Code 与原来的 Warning Code 意义相同，但错误类别更清晰，方便客户判断。详细的错误代码及映射表，见 AgoraWarningCode。

 @param engine      AgoraRtcEngineKit 对象
 @param warningCode AgoraWarningCode
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didOccurWarning:(AgoraWarningCode)warningCode;

/** 发生错误回调

 该回调方法表示 SDK 运行时出现了（网络或媒体相关的）错误。通常情况下，SDK 上报的错误意味着 SDK 无法自动恢复，需要应用程序干预或提示用户。 比如启动通话失败时，SDK 会上报 AgoraErrorCodeStartCall = 1002 错误。应用程序可以提示用户启动通话失败，并调用 [leaveChannel]([AgoraRtcEngineKit leaveChannel:])  退出频道。

 Npte: 部分 Error Code 在上报给客户之前，会经过映射，映射后的 Error Code 与原来的 Error Code 意义相同，但错误类别更清晰，方便客户判断。详细的错误代码及映射表，见 AgoraErrorCode。

 @param engine    AgoraRtcEngineKit 对象
 @param errorCode AgoraErrorCode
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didOccurError:(AgoraErrorCode)errorCode;

/** 返回媒体引擎的成功加载。

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineMediaEngineDidLoaded:(AgoraRtcEngineKit * _Nonnull)engine;

/** 返回成功的媒体引擎启动。

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineMediaEngineDidStartCall:(AgoraRtcEngineKit * _Nonnull)engine;

/** Token 过期回调

 在调用 [joinChannelByToken](AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:]) 时如果指定了 Token，由于 Token 具有一定的时效，在通话过程中SDK可能由于网络原因和服务器失去连接，重连时可能需要新的 Token。 该回调通知APP需要生成新的 Token，并需调用 [renewToken]([AgoraRtcEngineKit renewToken:]) 为 SDK 指定新的 Token。
 之前的处理方法是在 [didOccurError]([AgoraRtcEngineDelegate rtcEngine:didOccurError:]) 回调报告 AgoraErrorCodeTokenExpired = 109,
 AgoraErrorCodeInvalidToken = 110.  时，APP 需要生成新的 Token。 在新版本中，原来的处理仍然有效，但建议把相关逻辑放进该回调里。

  @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineRequestToken:(AgoraRtcEngineKit * _Nonnull)engine;

- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine tokenPrivilegeWillExpire:(NSString *_Nonnull)token;

/** 网络连接中断回调

 该回调方法表示 SDK 和服务器失去了网络连接。
 与 [rtcEngineConnectionDidLost]([AgoraRtcEngineDelegate rtcEngineConnectionDidLost:]) 回调的区别是: [rtcEngineConnectionDidInterrupted]([AgoraRtcEngineDelegate rtcEngineConnectionDidInterrupted:]) 回调在 SDK 刚失去和服务器连接时触发，[rtcEngineConnectionDidLost]([AgoraRtcEngineDelegate rtcEngineConnectionDidLost:]) 在失去连接且尝试自动重连失败后才触发。 失去连接后，除非 APP 主动调用 [leaveChannel]([AgoraRtcEngineKit leaveChannel:])，不然 SDK 会一直自动重连。

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineConnectionDidInterrupted:(AgoraRtcEngineKit * _Nonnull)engine;

/** 网络连接丢失回调

 在 SDK 和服务器失去了网络连接后，会触发 [rtcEngineConnectionDidInterrupted]([AgoraRtcEngineDelegate rtcEngineConnectionDidInterrupted:]) 回调，并自动重连。如果重连一定时间内（默认 10 秒）后仍未连上，会触发该回调。 如果 SDK 在调用 [joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:]) 后 10 秒内没有成功加入频道，也会触发该回调。该回调触发后，SDK 仍然会尝试重连，重连成功后会触发 [didRejoinChannel]([AgoraRtcEngineDelegate rtcEngine:didRejoinChannel:withUid:elapsed:]) 回调。

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineConnectionDidLost:(AgoraRtcEngineKit * _Nonnull)engine;

/** 连接已被禁止回调

当你被服务端禁掉连接的权限时，会触发该回调。意外掉线之后，SDK 会自动进行重连，重连多次都失败之后，该回调会被触发，判定为连接不可用。

  @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineConnectionDidBanned:(AgoraRtcEngineKit * _Nonnull)engine;

/** SDK与服务器网络连接状态及原因回调

 * @param state 详见#AgoraConnectionState.
 * @param reason 详见#AgoraConnectionChangedReason
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine connectionStateChanged:(AgoraConnectionState)state reason:(AgoraConnectionChangedReason)reason;

/** Rtc Engine统计数据回调

 * 同 rtcStatsBlock。该回调定期上报 Rtc Engine 的运行时的状态，每两秒触发一次。

 * @param engine AgoraRtcEngineKit 对象
 * @param stats  AgoraChannelStats，详见 已离开频道回调 [didLeaveChannelWithStats]([AgoraRtcEngineDelegate rtcEngine:didLeaveChannelWithStats:]) 中的描述
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine reportRtcStats:(AgoraChannelStats * _Nonnull)stats;

/** 网络质量回调

 报告本地用户的网络质量。当你调用 [startLastmileProbeTest]([AgoraRtcEngineKit startLastmileProbeTest]) 之后，该回调函数在 2 秒左右触发一次。

 @param engine  AgoraRtcEngineKit 对象
 @param quality 网络质量评分: AgoraNetworkQuality
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine lastmileQuality:(AgoraNetworkQuality)quality;

/** 大小流已切换

 该回调表示视频的大小流类型是否已切换。详见 设置视频大小流 [setRemoteVideoStream]([AgoraRtcEngineKit setRemoteVideoStream:type:]) 中的相关描述。

 @param engine AgoraRtcEngineKit 对象
 @param error  AgoraErrorCode
 @param api    API 说明

 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didApiCallExecute:(NSInteger)error api:(NSString * _Nonnull)api result:(NSString * _Nonnull)result;

/** 在成功执行 [refreshRecordingServiceStatus]([AgoraRtcEngineKit refreshRecordingServiceStatus]）方法后返回状态码。

 @param engine AgoraRtcEngineKit 对象
 @param status 0：录制已停止。 1：正在录制中。

 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didRefreshRecordingServiceStatus:(NSInteger)status;

#if (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC))

/** 返回是否添加或删除设备。

 NOTE: 该回调仅适用于 macOS 平台。

 @param engine AgoraRtcEngineKit 对象
 @param deviceId   设备ID
 @param deviceType AgoraMediaDeviceType
 @param state      设备的状态：

 * 0: 已添加。
 * 1: 删除。

 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine device:(NSString * _Nonnull) deviceId type:(AgoraMediaDeviceType) deviceType stateChanged:(NSInteger) state;

#endif

#pragma mark Local User Core Delegates

/**-----------------------------------------------------------------------------
 * @name Local User Core Delegates
 * -----------------------------------------------------------------------------
 */

/** 加入频道回调

 该回调方法表示该客户端成功加入了指定的频道。同[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:]) API 的 joinSuccessBlock 回调。

 @param engine  AgoraRtcEngineKit 对象
 @param channel 频道名称
 @param uid 用户ID. 如果在[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])方法中指定了uid，它会返回指定的ID; 如果没有，它将返回由Agora服务器自动分配的ID。
@param elapsed 从调用[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])过去的时间（ms)，直到发生此事件。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didJoinChannel:(NSString * _Nonnull)channel withUid:(NSUInteger)uid elapsed:(NSInteger) elapsed;

/** 重新加入频道回调

有时候由于网络原因，客户端可能会和服务器失去连接，SDK 会进行自动重连，自动重连成功后触发此回调方法。

 @param engine  AgoraRtcEngineKit 对象
 @param channel 频道名称
 @param uid     用户ID. 如果在[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])方法中指定了uid, 它会返回指定的ID; 如果没有，它将返回由Agora服务器自动分配的ID。
 @param elapsed  从调用[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])过去的时间（毫秒），直到发生此事件。

 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didRejoinChannel:(NSString * _Nonnull)channel withUid:(NSUInteger)uid elapsed:(NSInteger) elapsed;

/** 上下麦回调

 直播场景下，当用户上下麦时会触发此回调，即主播切换为观众时，或观众切换为主播时。

 @param engine AgoraRtcEngineKit 对象.
 @param oldRole 切换前的角色
 @param newRole 切换后的角色
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didClientRoleChanged:(AgoraClientRole)oldRole newRole:(AgoraClientRole)newRole;

/** 已离开频道回调

 当用户调用 [leaveChannel]([AgoraRtcEngineKit leaveChannel:]) 离开频道后，SDK 会触发该回调。

 @param engine AgoraRtcEngineKit 对象
 @param stats   通话相关的统计信息：AgoraChannelStats
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didLeaveChannelWithStats:(AgoraChannelStats * _Nonnull)stats;

/** 频道内网络质量报告回调

 该回调每 2 秒触发，向APP报告频道内所有用户当前的上行、下行网络质量。

 @param engine  AgoraRtcEngineKit 对象
 @param uid     用户 ID。表示该回调报告的是持有该ID的用户的网络质量 。当 uid 为 0 时，返回的是本地用户的网络质量
 @param txQuality 该用户的上行网络质量: AgoraNetworkQuality
 @param rxQuality 该用户的下行网络质量: AgoraNetworkQuality
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine networkQuality:(NSUInteger)uid txQuality:(AgoraNetworkQuality)txQuality rxQuality:(AgoraNetworkQuality)rxQuality;

#pragma mark Local User Audio Delegates

/**-----------------------------------------------------------------------------
 * @name Local User Audio Delegate Methods
 * -----------------------------------------------------------------------------
 */

/** 发送的第一个音频帧回调

 @param engine  AgoraRtcEngineKit 对象
 @param elapsed 从调用[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])过去的时间（ms)，直到发生此事件。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine firstLocalAudioFramePublished:(NSInteger)elapsed;

/** 语音路由已发生变化回调

 当语音路由发生变化时，SDK 会触发此回调。

 @param engine AgoraRtcEngineKit 对象
 @param routing 设置语音路由: AgoraAudioOutputRouting
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didAudioRouteChanged:(AgoraAudioOutputRouting)routing;

/** 本地伴奏播放已结束回调

 当调用 startAudioMixing 播放伴奏音乐结束后，会触发该回调。如果调用 startAudioMixing 失败，会在 [didOccurError]([AgoraRtcEngineDelegate rtcEngine:didOccurError:])  回调里，返回错误码 WARN_AUDIO_MIXING_OPEN_ERROR。

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineLocalAudioMixingDidFinish:(AgoraRtcEngineKit * _Nonnull)engine;

/** 本地音效播放已结束回调

 @param engine AgoraRtcEngineKit 对象.
 @param soundId 指定音效的 ID。每个音效均有唯一的 ID
 */
- (void)rtcEngineDidAudioEffectFinish:(AgoraRtcEngineKit * _Nonnull)engine soundId:(NSInteger)soundId;


#pragma mark Local User Video Delegates

/**-----------------------------------------------------------------------------
 * @name Local User Video Delegate Methods
 * -----------------------------------------------------------------------------
 */

/** 摄像头启用回调

 同 cameraReadyBlock。提示已成功打开摄像头，可以开始捕获视频。

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineCameraDidReady:(AgoraRtcEngineKit * _Nonnull)engine;

#if TARGET_OS_IPHONE

/** 相机对焦区域已改变回调

 NOTE: 该回调仅适用于 iOS 平台。

 该回调表示相机的对焦区域发生了改变。

 @param engine AgoraRtcEngineKit 对象
 @param rect   镜头内表示对焦区域的长方形
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine cameraFocusDidChangedToRect:(CGRect)rect;
#endif

/** 视频功能停止回调

 提示视频功能已停止。应用程序如需在停止视频后对 view 做其他处理（比如显示其他画面），可以在这个回调中进行。

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineVideoDidStop:(AgoraRtcEngineKit * _Nonnull)engine;

/** 本地首帧视频显示回调

 @param engine  AgoraRtcEngineKit 对象.
 @param size    视频流尺寸（宽度和高度）
 @param elapsed 从调用[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])过去的时间（毫秒），直到发生此事件。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine firstLocalVideoFrameWithSize:(CGSize)size elapsed:(NSInteger)elapsed;

/** 本地视频统计回调

 同 localVideoStatBlock。报告更新本地视频统计信息，该回调方法每两秒触发一次。

 @param engine AgoraRtcEngineKit 对象.
 @param stats 本地视频的统计信息: AgoraRtcLocalVideoStats
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine localVideoStats:(AgoraRtcLocalVideoStats * _Nonnull)stats;

/** 本地发布的媒体流已回退为音频流

  Note: * 该功能正在开发阶段，预计会在一个月后上线。
 * 如果本地推流已回退为音频流，远端的 App 上会收到 [didVideoMuted]([AgoraRtcEngineDelegate rtcEngine:didVideoMuted:byUid:])  的回调事件。

 如果你调用了设置本地推流回退选项 [setLocalPublishFallbackOption]([AgoraRtcEngineKit setLocalPublishFallbackOption:]) 接口并将 option 设置为 2，当上行网络环境不理想、本地发布的媒体流回退为音频流时，或当上行网络改善、媒体流恢复为音视频流时，会触发该回调。

 @param engine AgoraRtcEngineKit 对象.
 @param isFallbackOrRecover * True: 由于网络环境不理想，本地发布的媒体流已回退为音频流
 * False: 由于网络环境改善，发布的音频流已恢复为音视频流
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didLocalPublishFallbackToAudioOnly:(BOOL)isFallbackOrRecover;

#pragma mark Remote User Core Delegates

/**-----------------------------------------------------------------------------
 * @name Remote User Core Delegates
 * -----------------------------------------------------------------------------
 */

/** 主播加入回调

 同 [userJoinedBlock]([AgoraRtcEngineKit userJoinedBlock:])。该回调提示有主播加入了频道，并返回该主播的 ID。如果在加入之前，已经有主播在频道中了，新加入的用户也会收到已有主播加入频道的回调。Agora 建议连麦主播不超过 17 人。

 Note: 直播场景下:

 * 主播间能相互收到新主播加入频道的回调，并能获得该主播的 uid
 * 观众也能收到新主播加入频道的回调，并能获得该主播的 uid
 * 当 Web 端加入直播频道时，只要 Web 端有推流，SDK 会默认该 Web 端为主播，并触发该回调

 @param engine  AgoraRtcEngineKit 对象.
 @param uid     主播 ID。如果 [joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:]) 中指定了 uid，则此处返回该 ID；否则使用 Agora 服务器自动分配的 ID。
 @param elapsed 从调用 [joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:]) 过去的时间（毫秒），直到发生此事件。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didJoinedOfUid:(NSUInteger)uid elapsed:(NSInteger)elapsed;

/** 本地音频统计回调.
 同 localAudioStatBlock([AgoraRtcEngineKit localAudioStatBlock:]).。报告更新本地视频统计信息，该回调方法每两秒触发一次。

 @param engine AgoraRtcEngineKit 对象.
 @param stats  本地音频的统计信息:AgoraRtcLocalAudioStats
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine localAudioStats:(AgoraRtcLocalAudioStats * _Nonnull)stats;

/** 本地音频流变化及原因

 * @param engine AgoraRtcEngineKit 对象
 * @param state  本地音频状态:
 
 * Stopped: 本地音频默认初始状态
 * Recording: 本地音频录制设备启动成功
 * Encoding: 本地音频首帧编码成功
 * Failed: 本地音频启动失败
 
 * @param error 本地音频出错原因
 
 * OK: 本地音频状态正常
 * Failure: 本地音频出错原因不明确
 * DeviceNoPermission: 没有权限启动本地音频录制设备
 * DeviceBusy: 本地音频录制设备已经在使用中
 * RecordFailure: 本地音频录制失败，建议你检查录制设备是否正常工作
 * EncodeFailure: 本地音频编码失败
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine localAudioStateChanged:(AgoraAudioLocalState)state error:(AgoraAudioLocalError)error;

/**  主播离线回调

 同 [userOfflineBlock]([AgoraRtcEngineKit userOfflineBlock:])。提示有主播离开了频道（或掉线）。SDK 判断用户离开频道（或掉线）的依据是超时: 在一定时间内（15 秒）没有收到对方的任何数据包，判定为对方掉线。 在网络较差的情况下，可能会有误报。建议可靠的掉线检测应该由信令来做。

 @param engine AgoraRtcEngineKit 对象
 @param uid    主播 ID。如果 (joinChannelByToken)([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:]) 中指定了 uid，则此处返回该 ID；否则使用 Agora 服务器自动分配的 ID。
 @param reason  AgoraUserOfflineReason
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didOfflineOfUid:(NSUInteger)uid reason:(AgoraUserOfflineReason)reason;

/** 接收到对方数据流消息的回调

 提示本地用户已在5秒内收到对方发送的数据包。

 @param engine AgoraRtcEngineKit 对象
 @param uid    用户ID
 @param streamId 数据流 ID
 @param data   接收到的数据
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine receiveStreamMessageFromUid:(NSUInteger)uid streamId:(NSInteger)streamId data:(NSData * _Nonnull)data;

/** 接收对方数据流消息错误的回调

 提示本地用户没有在5秒内收到对方发送的数据包。

 @param engine AgoraRtcEngineKit 对象
 @param uid    用户 ID
 @param streamId 数据流 ID
 @param error     错误代码: AgoraErrorCode
 @param missed   丢失的消息数量
 @param cached   数据流中断时，后面缓存的消息数量
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didOccurStreamMessageErrorFromUid:(NSUInteger)uid streamId:(NSInteger)streamId error:(NSInteger)error missed:(NSInteger)missed cached:(NSInteger)cached;

#pragma mark Remote User Audio Delegates

/**-----------------------------------------------------------------------------
 * @name Remote User Audio Delegate Methods
 * -----------------------------------------------------------------------------
 */

/** 接收到远端用户第一帧音频帧的回调

 @param engine  AgoraRtcEngineKit 对象.
 @param uid     远端用户 UID。
 @param elapsed 从调用[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])过去的时间（毫秒），直到发生此事件。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine firstRemoteAudioFrameOfUid:(NSUInteger)uid elapsed:(NSInteger)elapsed;

/** 远端音频统计回调.
 * 同 remoteAudioStatBlock([AgoraRtcEngineKit remoteAudioStatBlock:]).。报告更新远端视频统计信息，该回调方法每两秒触发一次。

 @param engine AgoraRtcEngineKit 对象.
 @param stats  远端音频的统计信息:AgoraRtcRemoteAudioStats
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine remoteAudioStats:(AgoraRtcRemoteAudioStats * _Nonnull)stats;

/** 远端用户音频静音回调

 @param engine AgoraRtcEngineKit 对象
 @param muted  * True: 静音
 * False: 取消静音
 @param uid    远端用户ID
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didAudioMuted:(BOOL)muted byUid:(NSUInteger)uid;

/**  音量提示的回调。与 [audioVolumeIndicationBlock]([AgoraRtcEngineKit audioVolumeIndicationBlock:]) 相同。

 该回调提示频道内谁在说话以及说话者的音量。默认禁用。可通过 [enableAudioVolumeIndication]([AgoraRtcEngineKit enableAudioVolumeIndication:smooth:]) 方法开启；开启后，无论频道内是否有人说话，都会按方法中设置的时间间隔返回提示音量。

 @param engine      AgoraRtcEngineKit 对象.
 @param speakers     AgoraRtcAudioVolumeInfo
 @param totalVolume （混音后的）总音量（0~255）

 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine reportAudioVolumeIndicationOfSpeakers:(NSArray<AgoraRtcAudioVolumeInfo *> * _Nonnull)speakers totalVolume:(NSInteger)totalVolume;

/** 监测到活跃用户的回调

 @param engine      AgoraRtcEngineKit 对象.
 @param speakerUid  当前时间段声音最大的用户的 uid。如果返回的 uid 为 0，则默认为本地用户
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine activeSpeaker:(NSUInteger)speakerUid;

/** 远端伴奏播放已开始回调

 当远端有用户调用 startAudioMixing 播放伴奏音乐，会触发该回调。在合唱应用中可以利用这个回调作为本端歌词播放的触发条件。

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineRemoteAudioMixingDidStart:(AgoraRtcEngineKit * _Nonnull)engine;

/** 本地音效播放已结束回调

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineRemoteAudioMixingDidFinish:(AgoraRtcEngineKit * _Nonnull)engine;

/** 语音质量回调

同 [audioQualityBlock]([AgoraRtcEngineKit audioQualityBlock:])。在通话中，该回调方法每两秒触发一次，报告当前通话的（嘴到耳）音频质量。

 @param engine  AgoraRtcEngineKit 对象。
 @param uid     用户 ID 。每次回调上报一个用户当前的语音质量（不包含自己），不同用户在不同的回调中上报。
 @param quality  AgoraNetworkQuality
 @param delay   延迟（毫秒）
 @param lost    丢包率（百分比）
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine audioQualityOfUid:(NSUInteger)uid quality:(AgoraNetworkQuality)quality delay:(NSUInteger)delay lost:(NSUInteger)lost;

/**  远端音频传输状态回调

在通话中，当收到远端用户发送的音频数据包后，会周期性地发生该回调上报。回调频率约为 2 秒 1 次。

 @param engine AgoraRtcEngineKit 对象.
 @param uid                发送音频数据包的远端用户 UID
 @param delay              远端到本地客户端的延迟，单位为毫秒
 @param lost               丢包率（百分比）
 @param rxKBitRate         音频包的接收码率（Kbps）
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine audioTransportStatsOfUid:(NSUInteger)uid delay:(NSUInteger)delay lost:(NSUInteger)lost rxKBitRate:(NSUInteger)rxKBitRate;

#pragma mark Remote User Video Delegates

/**-----------------------------------------------------------------------------
 * @name Remote User Video Delegates
 * -----------------------------------------------------------------------------
 */

/** 远端首帧视频接收解码回调

 同 [firstRemoteVideoDecodedBlock](firstRemoteVideoDecodedBlock:)。提示已收到第一帧远程视频流并解码。

 @param engine  AgoraRtcEngineKit 对象.
 @param uid     远端用户的 uid。
 @param size    视频流尺寸（宽度和高度）
 @param elapsed 从调用 [joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:]) 过去的时间（毫秒），直至发生此事件。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine firstRemoteVideoDecodedOfUid:(NSUInteger)uid size:(CGSize)size elapsed:(NSInteger)elapsed;

/** 远端首帧视频显示回调

 同 [firstRemoteVideoFrameBlock](firstRemoteVideoFrameBlock:).

 @param engine  AgoraRtcEngineKit 对象.
 @param uid     远端用户 UID。
 @param size    Size of the video (width and height).
 @param elapsed 从调用[joinChannelByToken]([AgoraRtcEngineKit joinChannelByToken:channelId:info:uid:joinSuccess:])过去的时间（毫秒），直到发生此事件。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine firstRemoteVideoFrameOfUid:(NSUInteger)uid size:(CGSize)size elapsed:(NSInteger)elapsed;

/** 特定用户视频尺寸发生改变回调

 @param engine  AgoraRtcEngineKit 对象
 @param uid     用户ID
 @param size    新的视频尺寸
 @param rotation  新的视频旋转
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine videoSizeChangedOfUid:(NSUInteger)uid size:(CGSize)size rotation:(NSInteger)rotation;

/** 远端视频流状态发生改变回调

 该回调方法表示远端的视频流状态发生了改变。

 @param engine  AgoraRtcEngineKit 对象
 @param uid     发生视频流状态改变的远端用户的 ID
 @param state    远端视频流状态：

 * Running: 远端视频正常播放
 * Frozen: 远端视频卡住，可能因为网络连接问题导致
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine remoteVideoStateChangedOfUid:(NSUInteger)uid state:(AgoraVideoRemoteState)state;

/** 远端音频流状态发生改变回调

 该回调方法表示远端的音频流状态发生了改变。

 @param engine  AgoraRtcEngineKit 对象
 @param uid     发生音频流状态改变的远端用户的 ID
 @param state   远端音频流状态：

 * Stopped: 远端音频流默认初始状态。在 REMOTE_AUDIO_REASON_LOCAL_MUTED (3)、 REMOTE_AUDIO_REASON_REMOTE_MUTED (5) 或 REMOTE_AUDIO_REASON_REMOTE_OFFLINE (7) 的情况下，会报告该状态
 * Starting: 本地用户已接收远端音频首包
 * Decoding: 远端音频流正在解码，正常播放。在 REMOTE_AUDIO_REASON_NETWORK_RECOVERY (2)、 REMOTE_AUDIO_REASON_LOCAL_UNMUTED (4) 或 REMOTE_AUDIO_REASON_REMOTE_UNMUTED (6) 的情况下，会报告该状态
 * Frozen: 远端音频流卡顿。在 REMOTE_AUDIO_REASON_NETWORK_CONGESTION (1) 的情况下，会报告该状态
 * Failed: 远端音频流播放失败。在 REMOTE_AUDIO_REASON_INTERNAL (0) 的情况下，会报告该状态

 @param reason  远端音频流状态改变的具体原因：

 * Interal: 内部原因
 * NetworkCongestion: 网络阻塞
 * NetworkRecovery: 网络恢复正常
 * LocalMuted: 本地用户停止接收远端音频流或本地用户禁用音频模块
 * LocalUnmuted: 本地用户恢复接收远端音频流或本地用户启用音频模块
 * RemoteMuted: 远端用户停止发送音频流或远端用户禁用音频模块
 * RemoteUnmuted: 远端用户恢复发送音频流或远端用户启用音频模块
 * Offline: 远端用户离开频道

 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine remoteAudioStateChangedOfUid:(NSUInteger)uid state:(AgoraAudioRemoteState)state  reason: (AgoraAudioRemoteReason) elapsed:(NSInteger)elapsed;

/** 用户停止/重新发送视频回调

 同 [userMuteVideoBlock](userMuteVideoBlock:)。

 Note:  现阶段，当频道内用户数超过 20 人，该回调会失效。后续会改进。

 @param engine AgoraRtcEngineKit 对象.
 @param muted   Paused or resumed:

 * Yes: 该用户已暂停发送其视频流
 * No: 该用户已恢复发送其视频流

 @param uid    远端用户 UID.
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didVideoMuted:(BOOL)muted byUid:(NSUInteger)uid;

/**  用户启用/关闭视频回调

 提示有其他用户启用/关闭了视频功能。关闭视频功能是指该用户只能进行语音直播，不能显示、发送自己的视频，也不能接收、显示别人的视频。

 Note: 现阶段，当频道内用户数超过 20 人，该回调会失效。后续会改进。

 @param engine AgoraRtcEngineKit 对象.
 @param enabled   是否启用了视频功能。

 * Yes: 该用户已启用了视频功能
 * No: 该用户已关闭了视频功能

 @param uid    远端用户 UID。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didVideoEnabled:(BOOL)enabled byUid:(NSUInteger)uid;

/** 远端用户的本地视频已启用／关闭回调

 @param engine AgoraRtcEngineKit 对象.
 @param enabled   是否启用了视频功能。

 * Yes: 该用户已启用了视频功能
 * No: 该用户已关闭了视频功能

 @param uid    远端用户 UID。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didLocalVideoEnabled:(BOOL)enabled byUid:(NSUInteger)uid;

/** 远端视频统计回调

 报告更新远端视频统计信息，该回调方法每两秒触发一次。

 @param engine            AgoraRtcEngineKit 对象
 @param stats            AgoraRtcRemoteVideoStats
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine remoteVideoStats:(AgoraRtcRemoteVideoStats * _Nonnull)stats;

/** 远端订阅流已回退为音频流

 如果你调用了 [setRemoteSubscribeFallbackOption]([AgoraRtcEngineKit setRemoteSubscribeFallbackOption:]),
 接口并将 option 设置为 2，当下行网络环境不理想、仅接收远端音频流时，或当下行网络改善、恢复订阅音视频流时，会触发该回调。

  Note: 远端订阅流因弱网环境不能同时满足音视频而回退为小流时，你可以使用 remoteVideoStats 方法来监控远端视频大小流的切换。

 @param engine AgoraRtcEngineKit 对象.
 @param  isFallbackOrRecover

 * True: 由于网络环境不理想，远端订阅流已回退为音频流
 * False: 由于网络环境改善，订阅的音频流已恢复为音视频流

 @param uid    远端用户ID
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine didRemoteSubscribeFallbackToAudioOnly:(BOOL)isFallbackOrRecover byUid:(NSUInteger)uid;

/**  远端视频传输状态回调

 @param engine AgoraRtcEngineKit 对象.
 @param uid                 远端用户 UID
 @param delay               远端到本地客户端的延迟，单位为毫秒
 @param lost                丢包率 (%)
 @param rxKBitRate          视频包的接收码率（Kbps）
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine videoTransportStatsOfUid:(NSUInteger)uid delay:(NSUInteger)delay lost:(NSUInteger)lost rxKBitRate:(NSUInteger)rxKBitRate;

#pragma mark Stream Publish Delegates

/**-----------------------------------------------------------------------------
 * @name Stream Publish Delegates
 * -----------------------------------------------------------------------------
 */

 /** 已推流回调

  @param engine AgoraRtcEngineKit 对象.
  @param url 推流地址。
  @param errorCode AgoraErrorCode
  */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine streamPublishedWithUrl:(NSString * _Nonnull)url errorCode:(AgoraErrorCode)errorCode;

/** 推流已结束回调

 @param engine AgoraRtcEngineKit 对象.
 @param url  主播停止推流的地址。
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine streamUnpublishedWithUrl:(NSString * _Nonnull)url;

/** 推流转码已更新回调

 @param engine AgoraRtcEngineKit 对象
 */
- (void)rtcEngineTranscodingUpdated:(AgoraRtcEngineKit * _Nonnull)engine;

/** 导入外部视频流状态回调

@param engine AgoraRtcEngineKit 对象
@param url  导入进直播的外部视频源 url 地址
@param uid 用户 ID
@param status  AgoraInjectStreamStatus
 */
- (void)rtcEngine:(AgoraRtcEngineKit * _Nonnull)engine streamInjectedStatusOfUrl:(NSString * _Nonnull)url uid:(NSUInteger)uid status:(AgoraInjectStreamStatus)status;
@end

#pragma mark - AgoraRtcEngineKit

/** 提供所有可供 app 调用的方法。

 声网通过全球部署的虚拟网络专为 WebRTC 以及 移动端到移动端的 App 进行过优化。可以为全世界的音视频通信提供质量保证的体验（QoE）。

 AgoraRtcEngineKit 是 Agora SDK 的入口类。它为 App 提供了快速搭建音视频通信的 API。
*/
__attribute__((visibility("default"))) @interface AgoraRtcEngineKit : NSObject

#pragma mark Core Methods

/**-----------------------------------------------------------------------------
 * @name Core Methods
 * -----------------------------------------------------------------------------
 */

/** 设置／获取 AgoraRtcEngineDelegate
 */
@property (nonatomic, weak) id<AgoraRtcEngineDelegate> _Nullable delegate;
/** 加入频道

 该方法让用户加入通话频道，在同一个频道内的用户可以互相通话，多个用户加入同一个频道，可以群聊。 使用不同 App ID 的应用程序是不能互通的。如果已在通话中，用户必须调用 [leaveChannel](leaveChannel:) 退出当前通话，才能进入下一个频道。 SDK 在通话中使用 iOS 系统的 AVAudioSession 共享对象进行录音和播放，应用程序对该对象的操作可能会影响 SDK 的音频相关功能。

 调用该 API 后会触发回调，如果同时实现 [rejoinChannelSuccessBlock]([AgoraRtcEngineKit rejoinChannelSuccessBlock:]) 和 [didJoinChannel]([AgoraRtcEngineDelegate rtcEngine:didJoinChannel:withUid:elapsed:]) 会出现冲突，[rejoinChannelSuccessBlock]([rejoinChannelSuccessBlock:]) 优先级高，2 个同时存在时，[didJoinChannel]([AgoraRtcEngineDelegate rtcEngine:didJoinChannel:withUid:elapsed:]) 会被忽略。 需要有 [didJoinChannel]([AgoraRtcEngineDelegate rtcEngine:didJoinChannel:withUid:elapsed:]) 回调时，请将 [rejoinChannelSuccessBlock]([rejoinChannelSuccessBlock:]) 设置为 nil 。

Note: 频道内每个用户的 UID 必须是唯一的。如果将 UID 设为 0，系统将自动分配一个 UID。

 @param token 动态秘钥

  * 安全要求不高: 将值设为 null
  * 安全要求高: 将值设置为 Token。如果你已经启用了 App Certificate, 请务必使用 Token。

 @param channelId 标识通话的频道名称，长度在 64 字节以内的字符串。以下为支持的字符集范围（共 89 个字符）: a-z,A-Z,0-9,space,! #$%&,()+, -,:;<=.#$%&,()+,-,:;<=.,>?@[],^_,{|},~
 @param info (非必选项) 开发者需加入的任何附加信息。一般可设置为空字符串，或频道相关信息。该信息不会传递给频道内的其他用户。

 Note: 在 joinChannel() 时，SDK 调用 setCategoryAVAudioSessionCategoryPlayAndRecord 将 AVAudioSession 设置到 PlayAndRecord 模式，应用程序不应将其设置到其他模式。设置该模式时，正在播放的声音会被打断（比如正在播放的响铃声）。

 @param uid (Optional) (非必选项) 用户 ID，32 位无符号整数。建议设置范围：1到 (2^32-1)，并保证唯一性。如果不指定（即设为0），SDK 会自动分配一个，并在 onJoinChannelSuccess 回调方法中返回，App 层必须记住该返回值并维护，SDK 不对该返回值进行维护。uid 在 SDK 内部用 32 位无符号整数表示，由于 Java 不支持无符号整数，uid 被当成 32 位有符号整数处理，对于过大的整数，Java 会表示为负数，如有需要可以用 (uid&0xffffffffL) 转换成 64 位整数。
 @param joinSuccessBlock 成功加入频道回调。同 [leaveChannel]([AgoraRtcEngineKit leaveChannel:]) 。如果为 nil， [didJoinChannel]([AgoraRtcEngineDelegate rtcEngine:didJoinChannel:withUid:elapsed:]) 将工作。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)joinChannelByToken:(NSString * _Nullable)token
            channelId:(NSString * _Nonnull)channelId
                   info:(NSString * _Nullable)info
                    uid:(NSUInteger)uid
            joinSuccess:(void(^ _Nullable)(NSString * _Nonnull channel, NSUInteger uid, NSInteger elapsed))joinSuccessBlock;

/** 离开频道

 离开频道，即挂断或退出通话。

 当调用 joinChannelByToken() 方法后，必须调用 [leaveChannel]([AgoraRtcEngineKit leaveChannel:]) 结束通话，否则无法开始下一次通话。 不管当前是否在通话中，都可以调用 leaveChannel()，没有副作用。该方法会把会话相关的所有资源释放掉。该方法是异步操作，调用返回时并没有真正退出频道。在真正退出频道后，SDK 会触发 [didLeaveChannelWithStats]([AgoraRtcEngineDelegate rtcEngine:didLeaveChannelWithStats:]) 回调。

 Note: 如果你调用了本方法后立即调用 [destroy](destroy) 方法，SDK 将无法触发 [didLeaveChannelWithStats]([AgoraRtcEngineDelegate rtcEngine:didLeaveChannelWithStats:]) 回调。

 @param leaveChannelBlock 成功离开频道的回调

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)leaveChannel:(void(^ _Nullable)(AgoraChannelStats * _Nonnull stat))leaveChannelBlock;

/** 设置频道属性

Note:

* 同一频道内只能同时设置一种模式。
* 该方法必须在加入频道前调用和进行设置，进入频道后无法再设置。

 @param profile 频道模式。

 * AgoraChannelProfileCommunication = 0: 通信模式 (默认)
 * AgoraChannelProfileBroadcasting = 1: 直播模式

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setChannelProfile:(AgoraChannelProfile)profile;

/** 设置用户角色

在加入频道前，用户需要通过本方法设置观众(默认)或主播模式。在加入频道后，用户可以通过本方法切换用户模式。

 @param role 直播场景里的用户角色

 * AgoraClientRoleBroadcaster = 1; 主播
 * AgoraClientRoleAudience = 2; 观众(默认)

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setClientRole:(AgoraClientRole)role;

/** 更新 Token

 该方法用于更新 Token。如果启用了 Token 机制，过一段时间后使用的 Token 会失效。当：

 * 发生 [tokenPrivilegeWillExpire]([AgoraRtcEngineDelegate rtcEngine:tokenPrivilegeWillExpire:]) 回调时，或发生
 * [didOccurError]([AgoraRtcEngineDelegate rtcEngine:didOccurError:]) 回调报告 ERR_TOKEN_EXPIRED(109) 时，或发生
 * [rtcEngineRequestToken]([AgoraRtcEngineDelegate rtcEngineRequestToken:]) 报告 ERR_TOKEN_EXPIRED(109) 时

 应用程序应重新获取 Token，然后调用该 API 更新 Token，否则 SDK 无法和服务器建立连接。

 @param token 新的 Token

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)renewToken:(NSString * _Nonnull)token;

/** 打开与 Web SDK 的互通

 @param enabled 是否已打开与 Agora Web SDK 的互通:

 * True: 互通已打开
 * False: 互通已关闭

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)enableWebSdkInteroperability:(BOOL)enabled;


/** 初始化 一个 AgoraRtcEngineKit 对象

 该方法初始化一个 AgoraRtcEngineKit 单例。使用 AgoraRtcEngineKit，必须先调用该接口进行初始化。 Agora Native SDK 通过指定的 delegate 通知应用程序引擎运行时的事件。Delegate 中定义的所有方法都是可选实现的。


 @param appId Agora 为应用程序开发者签发的 App ID。每个项目都应该有一个独一无二的 App ID。如果你的开发包里没有 App ID，请从声网申请一个新的 App ID。在你调用 joinChannelByToken:channelId:info:uid:joinSuccess: 加入声网的全球网络实现一对一或一对多直播通信时需要：

 * 用 App ID 标示你的项目和所属组织以及
 * 用一个独一无二的 channel name

 @param delegate AgoraRtcEngineDelegate

 @return 一个 AgoraRtcEngineKit 实例对象。
 */
+ (instancetype _Nonnull)sharedEngineWithAppId:(NSString * _Nonnull)appId
                                      delegate:(id<AgoraRtcEngineDelegate> _Nullable)delegate;
/** 销毁 RtcEngine 实例

 该方法用于释放 Agora SDK 使用的所有对象资源。帮助偶尔使用音视频通话的应用程序在无需通话时释放资源。一旦应用程序调用了 destroy() 接口销毁创建的 AgoraRtcEngineKit 实例，将无法调用 SDK 内的任何方法也不再会有任何回调产生。如需重启通话，请调用初始化方法 sharedEngineWithAppId:delegate: 创建一个新的 AgoraRtcEngineKit 实例。

 NOTE: 该方法为同步调用。应用程序不得在 SDK 生成的回调中调用该方法，不然 SDK 只能等候该回调返回才能重新获取相应的对象资源造成死锁。
 */
+ (void)destroy;

#pragma mark Core Audio

/**-----------------------------------------------------------------------------
 * @name Core Audio
 * -----------------------------------------------------------------------------
 */

/** 打开音频

 打开音频(默认为开启状态)。

 NOTE: 该方法是设置内部引擎使能状态，在leave channel后仍然有效。
 
 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)enableAudio;

/** 关闭音频
 
 NOTE: 该方法是设置内部引擎使能状态，在leave channel后仍然有效。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)disableAudio;

/** 打开外放

 NOTE: 请确保在调用此方法前已调用过 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 方法。
 SDK 会调用 setCategory AVAudioSessionCategoryPlayAndRecord 并配置耳麦或者外放，所以调用该方法时任何声音都会被干扰。

 @param enableSpeaker * True: 切换到外放.
 * False: 切换到耳麦.

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setEnableSpeakerphone:(BOOL)enableSpeaker;

/** 是否是扬声器状态

 @return BOOL * True: 输出到扬声器
 * False: 输出到非扬声器(听筒，耳机等)

 */
- (BOOL)isSpeakerphoneEnabled;

/** 设置默认的语音路径

 该方法设置接收到的语音从听筒或扬声器出声。如果用户不调用本方法，语音默认从听筒出声。

 Note: * 该方法只在纯音频模式下工作，在有视频的模式下不工作。
 * 该方法需要在 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 前设置，否则不生效。

 @param defaultToSpeaker * True: 从扬声器出声
 * False: 从听筒出声

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setDefaultAudioRouteToSpeakerphone:(BOOL)defaultToSpeaker;

/** 设置音质

 Note:

 * 该方法需要在 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 之前设置好，[joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 之后设置不生效
 * 通信模式下，该方法设置 profile 生效，设置 scenario 不生效
 * 通信和直播模式下，音质（码率）会有网络自适应的调整，通过该方法设置的是一个最高码率

 @param profile AgoraAudioProfile
 @param scenario AgoraAudioScenario

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setAudioProfile:(AgoraAudioProfile)profile
              scenario:(AgoraAudioScenario)scenario;

/** 调节录音信号音量

 @param volume 录音信号音量可在 0~400 范围内进行调节:

 * 0: 静音
 * 100: 原始音量
 * 400: 最大可为原始音量的 4 倍(自带溢出保护)

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)adjustRecordingSignalVolume:(NSInteger)volume;

/** 调节播放信号音量

 @param volume 播放信号音量可在 0~400 范围内进行调节:

 * 0: 静音
 * 100: 原始音量
 * 400: 最大可为原始音量的 4 倍(自带溢出保护)

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)adjustPlaybackSignalVolume:(NSInteger)volume;

/** 静音录制信号。

 @param muted * True: 静音录制信号。
 * False: 取消静音录制信号。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
-(int)muteRecordingSignal:(BOOL)muted;

/**
 @deprecated

 */
- (int)setSpeakerphoneVolume:(NSUInteger)volume __deprecated;

/** 启用说话者音量提示

 该方法允许 SDK 定期向应用程序反馈当前谁在说话以及说话者的音量。

 @param interval 指定音量提示的时间间隔：

 * <= 0: 禁用音量提示功能
 * \> 0: 提示间隔，单位为毫秒。建议设置到大于 200 毫秒。最小不得少于 10 毫秒。启用该方法后，无论频道内是否有人说话，都会在 [reportAudioVolumeIndicationOfSpeakers]([AgoraRtcEngineDelegate rtcEngine:reportAudioVolumeIndicationOfSpeakers:totalVolume:]) 及 [audioVolumeIndicationBlock](audioVolumeIndicationBlock:) 回调中按设置的时间间隔返回音量提示。

 @param smooth   平滑系数。默认可以设置为 3

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)enableAudioVolumeIndication:(NSInteger)interval
                            smooth:(NSInteger)smooth;

/** Mutes/Unmutes the local audio.

 @param mute * True: 静音本地音频
 * False: 不静音本地音频

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)muteLocalAudioStream:(BOOL)mute;

/** 静音指定用户音频

 Note: 如果之前有调用过 muteAllRemoteAudioStreams (true) 对所有远端音频进行静音，在调用本 API 之前请确保你已调用 muteAllRemoteAudioStreams (false) 。 muteAllRemoteAudioStreams 是全局控制，muteRemoteAudioStream 是精细控制。

 @param uid 指定用户的用户 ID
 @param mute * True: 暂停播放指定音频流
 * False: 允许播放指定音频流

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)muteRemoteAudioStream:(NSUInteger)uid mute:(BOOL)mute;

/** 静音所有远端音频

 @param mute * True: 停止播放所有远端音频流
 * False: 允许播放所有远端音频流

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)muteAllRemoteAudioStreams:(BOOL)mute;

/** 设置默认静音选项

 @param mute * True: 默认静音（不订阅）所有接收到的远端音频
 * False: 默认订阅（不静音）所有接收到的远端音频

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setDefaultMuteAllRemoteAudioStreams:(BOOL)mute;

#if (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC))

/** Starts loopback recording.

 该方法仅用于 macOS。

 @param enabled Enable recording.
 @param deviceName Device name of the recorder.
 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
-(int)enableLoopbackRecording:(BOOL)enabled
                   deviceName:(NSString * _Nullable)deviceName;
#endif



#pragma mark Core Video

/**-----------------------------------------------------------------------------
 * @name Core Video
 * -----------------------------------------------------------------------------
 */

/** 打开视频模式

该方法用于打开视频模式。可以在加入频道前或者通话中调用，在加入频道前调用，则自动开启视频模式，在通话中调用则由音频模式切换为视频模式。调用 disableVideo() 方法可关闭视频模式。

  NOTE: 该方法是设置内部引擎使能状态，在leave channel后仍然有效。
 
 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)enableVideo;

/** 关闭视频模式

该方法用于关闭视频。可以在加入频道前或者通话中调用，在加入频道前调用，则自动开启纯音频模式，在通话中调用则由视频模式切换为纯音频频模式。调用 enableVideo() 方法可开启视频模式。
 
 NOTE: 该方法是设置内部引擎使能状态，在leave channel后仍然有效。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)disableVideo;

/** 启用本地视频

 禁用/启用本地视频功能。该方法用于只看不发的视频场景。该方法不需要本地有摄像头。
 
  NOTE: 该方法是设置内部引擎使能状态，在leave channel后仍然有效。

 @param enabled 是否启用本地视频:

 * True: 开启本地视频采集和渲染（默认）
 * False: 关闭使用本地摄像头设备

 @return * 0: 方法调用成功 
 * < 0: 方法调用失败
 */
- (int)enableLocalVideo:(BOOL)enabled;

/** 设置视频编码属性

 该方法设置视频编码属性。每个属性对应一套视频参数，如分辨率、帧率、码率、视频方向等。 所有设置的参数均为理想情况下的最大值。当视频引擎因网络环境等原因无法达到设置的分辨率、帧率或码率的最大值时，会取最接近最大值的那个值。

 Note: 该功能正在开发阶段，预计会在一个月后上线。在 v2.3.0 版本中，如下接口已废弃，Agora 不再推荐使用:

 * [setVideoProfile](setVideoProfile:swapWidthAndHeight:)
 * [setVideoResolution](setVideoResolution:andFrameRate:bitrate:)

 @param config AgoraVideoEncoderConfiguration
 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setVideoEncoderConfiguration:(AgoraVideoEncoderConfiguration * _Nonnull)config;

/**
- (int)enableLocalVideoCapture: (bool)enabled;
- (int)enableLocalVideoRender: (bool)enabled;
- (int)enableLocalVideoSend: (bool)enabled;
*/

/**  Configures the video display settings on the local machine.

 该方法设置本地视频显示信息。应用程序通过调用此接口绑定本地视频流的显示视窗(view)，并设置视频显示模式。 在应用程序开发中，通常在初始化后调用该方法进行本地视频设置，然后再加入频道。退出频道后，绑定仍然有效，如果需要解除绑定，可以指定空(NULL)View 调用 setupLocalVideo() 。

 @param local AgoraRtcVideoCanvas

 * view 视频显示视窗
 * renderMode: 视频显示模式。AgoraVideoRenderModeHidden (1)：视频尺寸等比缩放。优先保证视窗被填满。因视频尺寸与显示视窗尺寸不一致而多出的视频将被截掉。AgoraVideoRenderModeFit (2)：视频尺寸等比缩放。优先保证视频内容全部显示。因视频尺寸与显示视窗尺寸不一致造成的视窗未被填满的区域填充黑色。

 @return * 0: 本地用户 ID，与 joinChannel 中的 uid 保持一致
 * < 0: 方法调用失败
 */
- (int)setupLocalVideo:(AgoraRtcVideoCanvas * _Nullable)local;

/** 设置本地视频显示模式

 该方法设置本地视频显示模式。应用程序可以多次调用此方法更改显示模式。

 @param mode AgoraVideoRenderMode

 * AgoraVideoRenderModeHidden (1)：视频尺寸等比缩放。优先保证视窗被填满。因视频尺寸与显示视窗尺寸不一致而多出的视频将被截掉。
 * AgoraVideoRenderModeFit (2)：视频尺寸等比缩放。优先保证视频内容全部显示。因视频尺寸与显示视窗尺寸不一致造成的视窗未被填满的区域填充黑色。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setLocalRenderMode:(AgoraVideoRenderMode) mode;

/** 设置本地视频镜像

 该方法设置本地视频镜像，须在开启本地预览前设置。如果在开启预览后设置，需要重新开启预览才能生效。

 @param mode AgoraVideoMirrorMode

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setLocalVideoMirrorMode:(AgoraVideoMirrorMode) mode;

/** 开启视频预览

该方法用于在进入频道前启动本地视频预览。调用该 API 前，必须

* 调用 setupLocalVideo 设置预览窗口及属性
* 调用 enableVideo() 开启视频功能。

启用了本地视频预览后，如果调用 [leaveChannel](leaveChannel:) 退出频道，本地预览依然处于启动状态，如需要关闭本地预览，需要调用 [stopPreview](stopPreview) 。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)startPreview;

/** 停止本地视频预览

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)stopPreview;


/**  设置远端视频显示属性

该方法绑定远程用户和显示视图，即设定 uid 指定的用户用哪个视图显示。调用该接口时需要指定远程视频的 uid，一般可以在进频道前提前设置好。

如果应用程序不能事先知道对方的 uid，可以在 APP 收到 [didJoinedOfUid]([AgoraRtcEngineDelegate rtcEngine:didJoinedOfUid:elapsed:]) 事件时设置。如果启用了视频录制功能，视频录制服务会做为一个哑客户端加入频道，因此其他客户端也会收到它的 [didJoinedOfUid]([AgoraRtcEngineDelegate rtcEngine:didJoinedOfUid:elapsed:]) 事件，APP 不应给它绑定视图（因为它不会发送视频流），如果 APP 不能识别哑客户端，可以在 [firstRemoteVideoDecodedOfUid]([AgoraRtcEngineDelegate rtcEngine:firstRemoteVideoDecodedOfUid:size:elapsed:]) 事件时再绑定视图。解除某个用户的绑定视图可以把 view 设置为空。退出频道后，SDK 会把远程用户的绑定关系清除掉。

 @param remote AgoraRtcVideoCanvas

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setupRemoteVideo:(AgoraRtcVideoCanvas * _Nonnull)remote;


/** 设置远端视频显示模式

 该方法设置远端视频显示模式。应用程序可以多次调用此方法更改显示模式。

 @param uid  远端用户 UID
 @param mode AgoraVideoRenderMode 视频显示模式

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setRemoteRenderMode:(NSUInteger)uid
                      mode:(AgoraVideoRenderMode) mode;

/** 暂停本地视频流

Note: 该方法不影响本地视频流获取，没有禁用摄像头。

 @param mute * True: 不发送本地视频流
 * False: 不发送本地视频流

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)muteLocalVideoStream:(BOOL)mute;

/** 暂停所有远端视频流

该方法用于允许/禁止播放所有人的视频流。

 @param mute

 * True: 停止播放接收到的所有远端视频流
 * False: 允许播放接收到的所有远端视频流

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)muteAllRemoteVideoStreams:(BOOL)mute;

/** 设置是否默认播放所有远端用户视频流的默认播放

 @param mute * True: 默认不播放
    * False: 默认播放

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setDefaultMuteAllRemoteVideoStreams:(BOOL)mute;

/** 是否播放指定远端视频流

Note: 如果之前有调用过 muteAllRemoteVideoStreams (true) 对所有远端音频进行静音，在调用本 API 之前请确保你已调用 muteAllRemoteVideoStreams (false) 。 muteAllRemoteVideoStreams 是全局控制，muteRemoteVideoStream 是精细控制。

 @param uid  远端用户ID
 @param mute * True: 停止播放指定用户的视频流
 * False: 允许播放指定用户的视频流

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)muteRemoteVideoStream:(NSUInteger)uid
                        mute:(BOOL)mute;

#pragma mark Audio Effect

/**-----------------------------------------------------------------------------
 * @name Audio Effect
 * -----------------------------------------------------------------------------
 */

/** 设置本地语音音调

 该方法改变本地说话人声音的音调。

 @param pitch 语音频率可以 [0.5, 2.0] 范围内设置。默认值为 1.0

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setLocalVoicePitch:(double)pitch;

/** 设置本地语音音效均衡

 @param bandFrequency 取值范围是 [0-9]，分别代表音效的 10 个 band 的中心频率 [31，62，125，250，500，1k，2k，4k，8k，16k]Hz

 @param gain 每个 band 的增益，单位是 dB，每一个值的范围是 [-15，15]
 */
- (int)setLocalVoiceEqualizationOfBandFrequency:(AgoraAudioEqualizationBandFrequency)bandFrequency withGain:(NSInteger)gain;

/** 设置本地音效混响

 @param reverbType 混响音效类型
 @param value AgoraAudioReverbType
 */
- (int)setLocalVoiceReverbOfType:(AgoraAudioReverbType)reverbType withValue:(NSInteger)value;

#pragma mark Audio Effect Playback

/**-----------------------------------------------------------------------------
 * @name Audio Effect Playback
 * -----------------------------------------------------------------------------
 */

/** 获取音效音量，范围为 [0.0, 100.0]。
 */
- (double)getEffectsVolume;

/** 设置音效音量

 @param volume 取值范围为 [0.0, 100.0]。 100.0 为默认值

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setEffectsVolume:(double)volume;

/** 实时调整音效音量，范围为 [0.0, 100.0]。

 @param soundId 指定音效的 ID。每个音效均有唯一的 ID
 @param volume 取值范围为 [0.0, 100.0]。 100.0 为默认值

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setVolumeOfEffect:(int)soundId
              withVolume:(double)volume;

/** 播放指定音效

 @param soundId 指定音效的 ID。每个音效均有唯一的 ID
 如果你已通过 [preloadEffect](preloadEffect:filePath:) 将音效加载至内存，确保这里设置的 soundId 与 [preloadEffect](preloadEffect:filePath:) 设置的 soundId 相同。
 @param filePath 音效文件的绝对路径
 @param loopCount 设置音效循环播放的次数：

 * 0: 播放音效一次
 * 1: 循环播放音效两次
 * -1: 无限循环播放音效，直至调用 [stopEffect](stopEffect:) 或 stopAllEffects 后停止

 @param pitch 设置音效的音调 取值范围为 [0.5, 2]。默认值为 1.0，表示不需要修改音调。取值越小，则音调越低
 @param pan 设置是否改变音效的空间位置。取值范围为 [-1.0, 1.0]：

 * 0.0: 音效出现在正前方
 * 1.0: 音效出现在左边
 * -1.0: 音效出现在右边

 @param gain 设置是否改变单个音效的音量。取值范围为 [0.0, 100.0]。默认值为 100.0。取值越小，则音效的音量越低
 @param publish 设置是否将音效传到远端

 * True: 音效在本地播放的同时，会发布到 Agora 云上，因此远端用户也能听到该音效
 * False: 音效不会发布到 Agora 云上，因此只能在本地听到该音效

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)playEffect:(int)soundId
         filePath:(NSString * _Nullable)filePath
        loopCount:(int)loopCount
            pitch:(double)pitch
              pan:(double)pan
             gain:(double)gain
          publish:(BOOL)publish;

/** 停止播放指定音效

 @param soundId 指定音效的 ID。每个音效均有唯一的 ID

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)stopEffect:(int)soundId;

/** 停止播放所有音效

 */
- (int)stopAllEffects;

/** 将指定音效文件(压缩的语音文件)预加载至内存

 @param soundId 指定音效的 ID。每个音效均有唯一的 ID
 @param filePath 音效文件的绝对路径

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)preloadEffect:(int)soundId
            filePath:(NSString * _Nullable) filePath;

/** 从内存释放某个预加载的音效

 @param soundId 指定音效的 ID。每个音效均有唯一的 ID

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)unloadEffect:(int)soundId;

/** 暂停音效播放

 @param soundId 指定音效的 ID。每个音效均有唯一的 ID

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)pauseEffect:(int)soundId;

/** 暂停所有音效播放

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)pauseAllEffects;

/** Resumes playing a specific audio effect.

 @param soundId ID of the audio effect. Each audio effect has a unique ID.

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)resumeEffect:(int)soundId;

/** 恢复播放指定音效

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)resumeAllEffects;


#pragma mark Audio Mixing

/**-----------------------------------------------------------------------------
 * @name Audio Mixing
 * -----------------------------------------------------------------------------
 */

/** 开始播放伴奏

 指定本地音频文件来和麦克风采集的音频流进行混音和替换(用音频文件替换麦克风采集的音频流)， 可以通过参数选择是否让对方听到本地播放的音频和指定循环播放的次数。该 API 也支持播放在线音乐。

 Note:

 * 使用本 API 前请确保你的 iOS 设备版本不低于 8.0。
 * 请在频道内调用该方法，如果在频道外调用该方法可能会出现问题。

 @param filePath 指定需要混音的本地音频文件名和文件路径名。支持以下音频格式: mp3，aac，m4a, 3gp, wav
 @param loopback * True: 只有本地可以听到混音或替换后的音频流
 * False: 本地和对方都可以听到混音或替换后的音频流

 @param replace * True: 音频文件内容将会替换本地录音的音频流
 * False: 音频文件内容将会和麦克风采集的音频流进行混音

 @param cycle 指定音频文件循环播放的次数:

 * 正整数: 循环的次数
 * -1：无限循环

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)startAudioMixing:(NSString *  _Nonnull)filePath
               loopback:(BOOL)loopback
                replace:(BOOL)replace
                  cycle:(NSInteger)cycle;

/** 停止播放伴奏

 请在频道内调用该方法。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)stopAudioMixing;

/** 暂停播放伴奏

 请在频道内调用该方法。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)pauseAudioMixing;

/** 恢复播放伴奏

 请在频道内调用该方法。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)resumeAudioMixing;

/** 调节伴奏音量

 请在频道内调用该方法。

 @param volume 伴奏音量范围为 0~100。默认 100 为原始文件音量
 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)adjustAudioMixingVolume:(NSInteger)volume;

/** 获取伴奏时长

 请在频道内调用该方法。如果返回 0，则代表该方法调用失败。

 @return * 伴奏时长
 * < 0: 方法调用失败
 */
- (int)getAudioMixingDuration;

/** 获取伴奏播放进度，单位为毫秒。

 请在频道内调用该方法。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)getAudioMixingCurrentPosition;

/** 拖动语音进度条到理想位置而非从头到尾播放一个文件

 @param pos 整数。进度条位置，单位为毫秒
 */
- (int)setAudioMixingPosition:(NSInteger)pos;

#pragma mark Local audio recording

/**-----------------------------------------------------------------------------
 * @name Local Audio Recording
 * -----------------------------------------------------------------------------
 */

/** 开始客户端录音

 Agora SDK 支持通话过程中在客户端进行录音。该方法录制频道内所有用户的音频，并生成一个包含所有用户声音的录音文件，录音文件格式可以为:

 * .wav: 文件大，音质保真度高
 * .aac: 文件小，有一定的音质保真度损失

 请确保应用程序里指定的目录存在且可写。该接口需在 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 之后调用。如果调用 [leaveChannel](leaveChannel:) 时还在录音，录音会自动停止。

 @param filePath Full 录音文件的本地保存路径，需精确到文件名及格式，例如：/dir1/dir2/dir3/audio.aac。文件名字符串应为 UTF-8 格式。
 @param quality  AgoraAudioRecordingQuality

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)startAudioRecording:(NSString * _Nonnull)filePath
                   quality:(AgoraAudioRecordingQuality)quality;

/** 停止客户端录音

 Note: 停止录音。该接口需要在 [leaveChannel](leaveChannel:) 之前调用，不然会在调用 [leaveChannel](leaveChannel:) 时自动停止。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)stopAudioRecording;

#pragma mark Audio Echo Test

/**-----------------------------------------------------------------------------
 * @name Audio Echo Test
 * -----------------------------------------------------------------------------
 */

/** 开始语音通话测试

 该方法启动语音通话测试，目的是测试系统的音频设备（耳麦、扬声器等）和网络连接是否正常。 在测试过程中，用户先说一段话，在 10 秒后，声音会回放出来。如果 10 秒后用户能正常听到自己刚才说的话，就表示系统音频设备和网络连接都是正常的。

 Note: 调用 startEchoTest 后必须调用 stopEchoTest 以结束测试，否则不能进行下一次回声测试，或者调用 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 进行通话。

 @param successBlock 成功开始语音通话测试回调。详见 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 中的 JoinSuccessBlock。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败。如 ERR_REFUSED (-5)：无法启动测试，可能没有成功初始化
 */
- (int)startEchoTest:(void(^ _Nullable)(NSString * _Nonnull channel, NSUInteger uid, NSInteger elapsed))successBlock;

/** 停止语音通话测试

 @return * 0: 方法调用成功
 * < 0: 方法调用失败。如 ERR_REFUSED (-5)：无法启动测试，可能没有成功初始化
 */
- (int)stopEchoTest;

#pragma mark Miscellaneous Audio Control

/**-----------------------------------------------------------------------------
 * @name Miscellaneous Audio Control
 * -----------------------------------------------------------------------------
 */

#if TARGET_OS_IPHONE

/** 开启耳返功能

 NOTE: 该方法仅适用于 iOS 平台。

 @param enabled * YES: 开启耳返功能 * NO: 关闭耳返功能
 @param inludeAudioFilter * YES: 使用音频滤波* NO: 不使用音频滤波
 @return * 0: 方法调用成功 * <0: 方法调用失败
 */
- (int)enableInEarMonitoring:(BOOL)enabled;
- (int)enableInEarMonitoring:(BOOL)enabled inludeAudioFilter:(BOOL)inludeAudioFilter;

/** 设置耳返音量

 NOTE: 该方法仅适用于 iOS 平台。

 @param volume 设置耳返音量，取值范围在 [0.100]。默认值为 100

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setInEarMonitoringVolume:(NSInteger)volume;

/** 设置 Audio Session 控制权限

 NOTE: 该方法仅适用于 iOS 平台。

 该方法限制 Agora SDK 对 Audio Session 的操作权限。在默认情况下，SDK 和 App 对 Audio Session 都有控制权，但某些场景下，App 会希望限制 Agora SDK 对 Audio Session 的控制权限，而使用其他应用或第三方组件对 Audio Session 进行操控。调用该方法可以实现该功能。

 Note: 一旦调用该方法限制了 Agora SDK 对 Audio Session 的控制权限， SDK 将无法对 Audio Session 进行相关设置，而需要用户使用其他程序或第三方组件才能设置。
 */
- (void)setAudioSessionOperationRestriction:(AgoraAudioSessionOperationRestriction)restriction;
#endif

#pragma mark Dual Video Mode

/**-----------------------------------------------------------------------------
 * @name Dual Video Mode
 * -----------------------------------------------------------------------------
 */

/** Sets the stream mode (only applicable to live broadcast) to single- (default) or dual-video stream mode.

 @param enabled The mode is in a single-video stream or dual-video stream:

 * True: Dual-video stream
 * False: Single-video stream

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)enableDualStreamMode:(BOOL)enabled;

/** 使用双流/单流模式

 该方法设置单流（默认）或者双流直播模式，仅在直播模式有效。

 * 如果用户已调用 [enableDualStreamMode](enableDualStreamMode:) 启用了双流模式，默认接收小流。如需使用大流，可调用本方法进行切换。
 * 如果用户未启用双流模式，默认接收大流。

 The result after calling this method will be returned in [didApiCallExecute]([AgoraRtcEngineDelegate rtcEngine:didApiCallExecute:api:result:]). The Agora SDK receives the high-video stream by default to save the bandwidth. If needed, users can switch to the low-video stream using this method.

 The resolution of the high-video stream is 1.1, 4.3, or 16.9. By default, the aspect ratio of the low-video stream is the 同 that of the high-video stream. Once the resolution of the high-video stream is set, the system automatically sets the aspect ratio for the low-video stream.

 @param uid 用户 ID
 @param streamType AgoraVideoStreamType

 @return * 0: 方法调用成功 
 * < 0: 方法调用失败
 */
- (int)setRemoteVideoStream:(NSUInteger)uid
                       type:(AgoraVideoStreamType)streamType;


- (int)setRemoteDefaultVideoStreamType:(AgoraVideoStreamType)streamType;

#pragma mark Stream Fallback

/**-----------------------------------------------------------------------------
 * @name Stream Fallback
 * -----------------------------------------------------------------------------
 */

/** 设置本地推流回退选项

 网络不理想的环境下，直播音视频的质量都会下降。使用该接口并将 option 设置为 2 后，SDK 会在上行弱网且音视频质量严重受影响时，自动关断视频流，从而保证或提高音频质量。同时 SDK 会持续监控网络质量，并在网络质量改善时恢复音视频流。 当本地推流回退为音频流时，或由音频流恢复为音视频流时，SDK 会触发 本地发布的媒体流已回退为音频流 [didLocalPublishFallbackToAudioOnly](didLocalPublishFallbackToAudioOnly:) 回调。

 Caution: CDN 推流场景下，设置本地推流回退为 Audio Only 可能会导致远端的 CDN 用户听到声音的时间有所延迟。因此在有 CDN 推流的场景下，Agora 建议不开启该功能。

 @param option AgoraStreamFallbackOptions
 @return * 0: 方法调用成功 
 * < 0: 方法调用失败
 */
- (int)setLocalPublishFallbackOption:(AgoraStreamFallbackOptions)option;

/** 在弱网条件下启动订阅流回退选项

 @param option AgoraStreamFallbackOptions
 @return * 0: 方法调用成功 
 * < 0: 方法调用失败
 */
- (int)setRemoteSubscribeFallbackOption:(AgoraStreamFallbackOptions)option;

#pragma mark Video Quality Control

/**-----------------------------------------------------------------------------
 * @name Video Quality Control
 * -----------------------------------------------------------------------------
 */

/** 设置视频优化选项

 @param preferFrameRateOverImageQuality 画质和流畅度里，是否优先保障流畅度：

 * True: 画质和流畅度里，优先保证流畅度
 * False: 画质和流畅度里，优先保证画质（默认）

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setVideoQualityParameters:(BOOL)preferFrameRateOverImageQuality;

#pragma mark Advanced Video Source and Sink Module

/**-----------------------------------------------------------------------------
 * @name Advanced Video Source and Sink Module
 * -----------------------------------------------------------------------------
 */

/**  设置视频源

 该方法将自定义的视频源加入到 SDK 中
 */
- (void)setVideoSource:(id<AgoraVideoSourceProtocol> _Nullable)videoSource;
/** videoSource
 */
- (id<AgoraVideoSourceProtocol> _Nullable)videoSource;

/** 设置本地视频渲染器

 该方法将视频渲染器加入到 SDK 中。
 */
- (void)setLocalVideoRenderer:(id<AgoraVideoSinkProtocol> _Nullable)videoRenderer;
/** localVideoRenderer
 */
- (id<AgoraVideoSinkProtocol> _Nullable)localVideoRenderer;

/** 设置远端视频渲染器

 该方法将视频渲染器加入到 SDK 中。
 */
- (void)setRemoteVideoRenderer:(id<AgoraVideoSinkProtocol> _Nullable)videoRenderer forUserId:(NSUInteger)userId;
/** remoteVideoRendererOfUserId:
 */
- (id<AgoraVideoSinkProtocol> _Nullable)remoteVideoRendererOfUserId:(NSUInteger)userId;

#pragma mark External Media Source

/**-----------------------------------------------------------------------------
 * @name External Media Source
 * -----------------------------------------------------------------------------
 */

/** 配置外部视频源

 NOTE: 如果使用了外部视频源，请在调用 [enableVideo](enableVideo) 或 [startPreview](startPreview) 之前调用此 API。

 @param enable 是否使用外部视频源：

 * True: 使用外部视频源
 * False: 不适用外部视频源

 目前 Agora SDK 不支持频道内动态切换视频源。如果您启用了外部视频源，且已经在频道内，如果想切换成外部视频源，须先退出频道，调用本方法将参数 enable 设为 FALSE 后，再重新加入频道。
 @param useTexture 是否使用 Texture 作为输入：

 * True: 使用 Texture 作为输入
 * False: 使用 Texture 作为输入

 @param pushMode 是否外部视频源需要调用 [pushExternalVideoFrame](pushExternalVideoFrame:) 将视频帧主动推送给 Agora SDK：

 * True: 使用推送 (push) 模式
 * False: 使用拉 (pull) 模式（暂不支持）

 */
- (void)setExternalVideoSource:(BOOL)enable useTexture:(BOOL)useTexture pushMode:(BOOL)pushMode;

/** 推送外部视频帧

 该方法主动将视频帧数据用 AgoraVideoFrame 类封装后传递给 SDK。该方法主动向 Agora SDK 推送一个视频帧。请确保在你调用本方法前已调用 [setExternalVideoSource](setExternalVideoSource:useTexture:pushMode:)，并将参数 pushMode 设为 TRUE，不然调用本方法后会一直报错。

 @param frame 该视频帧包含待 Agora SDK 编码的视频数据。
 @return BOOL * True: 该帧推送成功
 * False: 该帧推送不成功

 */
- (BOOL)pushExternalVideoFrame:(AgoraVideoFrame * _Nonnull)frame;

/** 开启外部音频采集

 @param sampleRate 外部音频源的采样率
 @param channelsPerFrame 外部音频源的通道数（最多支持两个声道）
 */
- (void)enableExternalAudioSourceWithSampleRate:(NSUInteger)sampleRate
                               channelsPerFrame:(NSUInteger)channelsPerFrame;

/**
 关闭外部音频采集
 */
- (void)disableExternalAudioSource;
/**
 推送外部音频帧

 @param data 外部音频数据
 @param samples 每次调用推入采样点
 @param timestamp 外部音频帧的时间戳，用于和外部视频源同步
 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (BOOL)pushExternalAudioFrameRawData:(void * _Nonnull)data
                              samples:(NSUInteger)samples
                            timestamp:(NSTimeInterval)timestamp;

/**
 pushExternalAudioFrameSampleBuffer:
 */
- (BOOL)pushExternalAudioFrameSampleBuffer:(CMSampleBufferRef _Nonnull)sampleBuffer;
/**
 pushExternalAudioFrameSampleBuffer:channel:mode:samplesPerCall:
 */
- (int)setRecordingAudioFrameParametersWithSampleRate:(NSInteger)sampleRate
                                              channel:(NSInteger)channel
                                                 mode:(AgoraAudioRawFrameOperationMode)mode
                                       samplesPerCall:(NSInteger)samplesPerCall;

/**
 setPlaybackAudioFrameParametersWithSampleRate:channel:mode:samplesPerCall:
 */
- (int)setPlaybackAudioFrameParametersWithSampleRate:(NSInteger)sampleRate
                                             channel:(NSInteger)channel
                                                mode:(AgoraAudioRawFrameOperationMode)mode
                                      samplesPerCall:(NSInteger)samplesPerCall;

/**
 pushExternalAudioFrameSampleBuffer:samplesPerCall:
 */
- (int)setMixedAudioFrameParametersWithSampleRate:(NSInteger)sampleRate
                                   samplesPerCall:(NSInteger)samplesPerCall;

#pragma mark Built-in Encryption

/**-----------------------------------------------------------------------------
 * @name Built-in Encryption
 * -----------------------------------------------------------------------------
 */

/** 设置内置的加密方案

 Agora Native SDK 支持内置加密功能，默认使用 AES-128-XTS 加密方式。如需使用其他加密方式，可以调用该 API 设置。同一频道内的所有用户必须设置相同的加密方式和 secret 才能进行通话。关于这几种加密方式的区别，请参考 AES 加密算法的相关资料。

 在调用本方法前，请先调用 [setEncryptionSecret](setEncryptionSecret:) 启用内置加密功能。

 Note: 请不要在 CDN 推流时调用此方法。

 @param encryptionMode AgoraEncryptionMode

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setEncryptionMode:(NSString * _Nullable)encryptionMode;

/** 启用内置的加密密码

 在加入频道之前，应用程序需调用 setEncryptionSecret() 指定 secret 来启用内置的加密功能，同一频道内的所有用户应设置相同的 secret。 当用户离开频道时，该频道的 secret 会自动清除。如果未指定 secret 或将 secret 设置为空，则无法激活加密功能。

 Note: 请不要在 CDN 推流时调用此方法。

 @param secret 加密密码
 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setEncryptionSecret:(NSString * _Nullable)secret;

#pragma mark Data Stream

/**-----------------------------------------------------------------------------
 * @name Data Stream
 * -----------------------------------------------------------------------------
 */

/** 创建数据流

 该方法用于创建数据流。频道内每人最多只能创建 5 个数据流。频道内数据通道最多允许数据延迟 5 秒，若超过 5 秒接收方尚未收到数据流，则数据通道会向应用程序报错。 目前 Agora Native SDK 支持 99% 可靠和 100% 有序的数据传输。

 Note: 请将 reliable 和 ordered 同时设置为 True 或 False, 暂不支持交叉设置。

 @param streamId 数据流 ID
 @param reliable * True: 接收方 5 秒内会收到发送方所发送的数据，否则连接中断，数据通道会向应用程序报错。
 * False: 接收方不保证收到，就算数据丢失也不会报错。

 @param ordered * True: 接收方 5 秒内会按照发送方发送的顺序收到数据包。
 * False: 接收方不保证按照发送方发送的顺序收到数据包

 @return * <0: 创建数据流失败的错误码。返回的错误码是负数，对应错误代码和警告代码里的正整数。例如返回的错误码为-2，则对应错误代码和警告代码里的 2: ERR_INVALID_ARGUMENT 。
 * >0: 数据流 ID
 */
- (int)createDataStream:(NSInteger * _Nonnull)streamId
               reliable:(BOOL)reliable
                ordered:(BOOL)ordered;

/** 发送数据流

 该方法发送数据流消息到频道内所有用户。频道内每秒最多能发送 30 个包，且每个包最大为 1 KB。 API 须对数据通道的传送速率进行控制: 每个客户端每秒最多能发送 6 KB 数据。频道内每人最多能同时有 5 个数据通道。

 @param streamId    数据流 ID.
 @param message    需要发送的数据
 @return * 0: 方法调用成功
 * <0: 失败，返回错误码: ERR_SIZE_TOO_LARGE/ERR_TOO_OFTEN/ERR_BITRATE_LIMIT等
 */
- (int)sendStreamMessage:(NSInteger)streamId
                    data:(NSData * _Nonnull)data;

#pragma mark Stream Publish

/**-----------------------------------------------------------------------------
 * @name Stream Publish
 * -----------------------------------------------------------------------------
 */

/** 增加 CDN 推流地址

 Note:

 * 请确保在成功加入房间后才能调用该接口。
 * 该方法每次只能增加一路 CDN 推流地址。若需推送多路流，则需多次调用该方法。
 * url 不支持中文等特殊字符。

 @param url    推流地址
 @param transcodingEnabled * True: 转码
 * False: 不转码

 */
- (int)addPublishStreamUrl:(NSString * _Nonnull)url transcodingEnabled:(BOOL) transcodingEnabled;

/** 删除 CDN 推流地址

 Note:

 * 该方法每次只能删除一路 CDN 推流地址。若需删除多路流，则需多次调用该方法。
 * url 不支持中文等特殊字符。

 @param url    推流地址
 */
- (int)removePublishStreamUrl:(NSString * _Nonnull)url;

/** 设置直播转码

 @param transcoding    AgoraLiveTranscoding
 */
- (int)setLiveTranscoding:(AgoraLiveTranscoding *_Nullable) transcoding;

/** 导入外部视频流

 该方法将正在播放的视频作为视频源导入到正在进行的直播中。可主要应用于赛事直播、多人看视频互动等直播场景。如果导入成功，该音视频流会出现在频道中，其 uid 为 666。

 @param url    添加到直播中的视频流 url 地址， 支持 RTMP， HLS， FLV 协议传输。
 @param config    AgoraLiveInjectStreamConfig
 */
- (int)addInjectStreamUrl:(NSString * _Nonnull) url config:(AgoraLiveInjectStreamConfig * _Nonnull)config;

/** 删除导入的外部视频源

 @param url    已导入、待删除的外部视频流 url 地址
 */
- (int)removeInjectStreamUrl:(NSString * _Nonnull) url;

#pragma mark Deprecated CDN Publisher

/**-----------------------------------------------------------------------------
 * @name Deprecated CDN Publisher
 * -----------------------------------------------------------------------------
 */

/** @deprecated
 配置旁路推流

 该方法已被废弃。虽然你仍然可以调用该接口，声网推荐您使用以下接口：

 * [addPublishStreamUrl](addInjectStreamUrl:config:)
 * [removePublishStreamUrl](removeInjectStreamUrl:)
 * [setLiveTranscoding](setLiveTranscoding:)
 */
- (int)configPublisher:(AgoraPublisherConfiguration * _Nonnull)config __deprecated;

/** @deprecated
 设置画中画布局

 该方法已被废弃。声网不建议您使用。如果您希望设置旁路推流布局，请调用 [setLiveTranscoding](setLiveTranscoding:) 方法。

 该方法用于在旁路直播中设置画中画布局，仅适用于从声网服务器推流的情况。当您从声网服务器推流时，您需要：

 1. 设置一张画布、及其宽高（视频分辨率）、背景色以及希望显示的视频流的数目。
 2. 定义画布上每条视频流的位置和尺寸以及是否裁剪或者缩放适配。

 推流的 APP 在生成 H.264 视频流并通过 RTMP 协议将其推送到 CDN 供应商时，会将定制布局的格式信息以 *JSON* 形式打包进每个关键帧辅助增强信息 SEI。

 Note: * 加入频道后再调用该方法。
 * 在同一频道内 APP 只能允许一个用户调用该方法。如果多个用户调用了该方法，其他用户必须调用 [clearVideoCompositingLayout]([AgoraRtcEngineKit clearVideoCompositingLayout]) 方法移除相关设置。
 */
- (int)setVideoCompositingLayout:(AgoraRtcVideoCompositingLayout * _Nonnull)layout __deprecated;

/** @deprecated
 移除画中画布局设置

 该方法用于在调用 [setVideoCompositingLayout]([AgoraRtcEngineKit setVideoCompositingLayout:]) 方法后移除相关设置。
 */
- (int)clearVideoCompositingLayout __deprecated;

#if (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC))
#pragma mark Screen capture

/**-----------------------------------------------------------------------------
 * @name Screen Capture
 * -----------------------------------------------------------------------------
 */

/** 开始屏幕共享

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)startScreenCapture:(NSUInteger)windowId
          withCaptureFreq:(NSInteger)captureFreq
                  bitRate:(NSInteger)bitRate
                  andRect:(CGRect)rect;

/** 停止屏幕共享

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)stopScreenCapture;

/** 更新屏幕共享区域

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)updateScreenCaptureRegion:(CGRect)rect;

#if TARGET_OS_IPHONE
#pragma mark Video Camera Control

/**-----------------------------------------------------------------------------
 * @name Video Camera Control
 * -----------------------------------------------------------------------------
 */

/** 检测设备是否支持相机缩放功能

 NOTE: 本方法仅适用于 iOS 平台，不适用于 macOS。

 @return * True: 设备支持相机缩放功能
 * False: 设备不支持相机缩放功能

 */
- (BOOL)isCameraZoomSupported;

/** 设置相机缩放因子

 NOTE: 本方法仅适用于 iOS 平台，不适用于 macOS。

 @param zoomFactor 相机缩放因子，有效范围从 1.0 到最大缩放
 */
- (CGFloat)setCameraZoomFactor:(CGFloat)zoomFactor;

/** 检测设备是否支持手动对焦功能

 NOTE: 本方法仅适用于 iOS 平台，不适用于 macOS。

 @return * True: 设备支持手动对焦功能
 * False: 设备不支持手动对焦功能

 */
- (BOOL)isCameraFocusPositionInPreviewSupported;

/** 设置手动对焦位置，并触发对焦

 NOTE: 本方法仅适用于 iOS 平台，不适用于 macOS。

 @param position * positionX:    触摸点相对于视图的横坐标
 * positionY:    触摸点相对于视图的纵坐标

 */
- (BOOL)setCameraFocusPositionInPreview:(CGPoint)position;

/** 检测设备是否支持闪光灯常开

 Note:

 * 本方法仅适用于 iOS 平台，不适用于 macOS。
 * 一般情况下，App 默认开启前置摄像头，因此如果你的前置摄像头不支持闪光灯常开，直接使用该方法会返回 false。如果需要检查后置摄像头是否支持闪光灯常开，需要先使用 switchCamera 切换摄像头，再使用该方法。

 @return BOOL * True: 设备支持闪光灯常开
 * False: 设备不支持闪光灯常开

 */
- (BOOL)isCameraTorchSupported;

/** 设置是否打开闪光灯

 NOTE: 本方法仅适用于 iOS 平台，不适用于 macOS。

 @param isOn * True: 打开
 * False: 打开

 */
- (BOOL)setCameraTorchOn:(BOOL)isOn;

/** 检测设备是否支持人脸对焦功能

 NOTE: 本方法仅适用于 iOS 平台，不适用于 macOS。

 @return BOOL * True: 设备支持人脸对焦功能
 * False: 设备不支持人脸对焦功能

 */
- (BOOL)isCameraAutoFocusFaceModeSupported;

/** 设置是否开启人脸对焦功能

 NOTE: 本方法仅适用于 iOS 平台，不适用于 macOS。

 @param enable * True: 开启
 * False: 关闭

 */
- (BOOL)setCameraAutoFocusFaceModeEnabled:(BOOL)enable;

/** 切换前置/后置摄像头

 NOTE: 本方法仅适用于 iOS 平台，不适用于 macOS。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)switchCamera;

#endif

#if (!(TARGET_OS_IPHONE) && (TARGET_OS_MAC))
#pragma mark Device test
/**-----------------------------------------------------------------------------
 * @name Device Test
 * -----------------------------------------------------------------------------
 */

/** 监控设备改变

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 */
- (void)monitorDeviceChange:(BOOL)enabled;

/** 枚举设备

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 */
- (NSArray<AgoraRtcDeviceInfo *> * _Nullable)enumerateDevices:(AgoraMediaDeviceType)type;

/** 获取设备 ID

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 */
- (NSString * _Nullable)getDeviceId:(AgoraMediaDeviceType)type;


/** 设置设备

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 */
- (int)setDevice:(AgoraMediaDeviceType)type deviceId:(NSString * _Nonnull)deviceId;

/** 获取设备音量

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 */
- (int)getDeviceVolume:(AgoraMediaDeviceType)type;

/** 设置设备音量

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 */
- (int)setDeviceVolume:(AgoraMediaDeviceType)type volume:(int)volume;
/**
 @deprecated

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。

 */
- (int)startRecordingDeviceTest:(int)indicationInterval;
/**
 @deprecated

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。
 */
- (int)stopRecordingDeviceTest;
/** 开始回放设备测试

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。
 */
- (int)startPlaybackDeviceTest:(NSString * _Nonnull)audioFileName;
/** 停止回放设备测试

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。
 */
- (int)stopPlaybackDeviceTest;
/** 开始采集设备测试

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。
 */
- (int)startCaptureDeviceTest:(NSView * _Nonnull)view;
/** 开始采集设备测试

 NOTE: 该方法仅支持 macOS 平台，不支持 iOS 平台。
 */
- (int)stopCaptureDeviceTest;
#endif

#pragma mark Server Recording

/**-----------------------------------------------------------------------------
 * @name Server Recording
 * -----------------------------------------------------------------------------
 */

/** 开始录制服务
 */
- (int)startRecordingService:(NSString * _Nonnull)recordingKey;
/** 停止录制服务
 */
- (int)stopRecordingService:(NSString * _Nonnull)recordingKey;
/** Refreshes the server recording service status.

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)refreshRecordingServiceStatus;

#pragma mark Watermark

/**-----------------------------------------------------------------------------
 * @name Watermark
 * -----------------------------------------------------------------------------
 */

/** 添加本地视频水印

 该方法将一张 PNG 图片作为水印添加到本地发布的直播视频流上，同一直播频道中的观众，旁路直播观众，甚至录制设备都能看到或采集到该水印图片。 如果你仅仅希望在旁路直播推流中添加水印，请参考 设置直播转码 [setLiveTranscoding](setLiveTranscoding:) 中描述的用法。

 NOTE:

 * 在本地直播和旁路直播中，url 的定义不同。本地直播中，url 指本地直播视频上水印图片的本地绝对路径；旁路直播中，url 指旁路直播视频上水印图片的 url 地址，并且该 url 只支持 HTTP，不支持 HTTPS
 * 水印图片的源文件格式必须是 PNG
 * Agora 当前只支持在直播视频流中添加一个水印，后添加的水印会替换掉之前添加的水印
 * 如果待添加的 PNG 图片的尺寸与你该方法中设置的尺寸不一致，SDK 会对 PNG 图片进行裁剪，以与设置相符

 @param watermark AgoraImage

 * url: 直播视频上图片的 url 地址
 * x: 图片在视频帧左上角的横轴坐标
 * y: 图片在视频帧左上角的纵轴坐标
 * width: 图片在视频帧上的宽度
 * height: 图片在视频帧上的高度

 */
- (int)addVideoWatermark:(AgoraImage * _Nonnull)watermark NS_SWIFT_NAME(addVideoWatermark(_:));

/** 删除已添加的视频水印
 */
- (void)clearVideoWatermarks;

#pragma mark Miscellaneous methods

/**-----------------------------------------------------------------------------
 * @name Miscellaneous methods
 * -----------------------------------------------------------------------------
 */

/** 查询 SDK 版本号

 @return string, SDK 版本号
 */
+ (NSString * _Nonnull)getSdkVersion;
/** 获取 Native SDK Engine Handler。

 该方法获取 native SDK engine 的 C++ handler，用于包括注册音视频帧观测器在内的特殊场景。

 */
- (void * _Nullable)getNativeHandle;

/** 设置日志文件

 设置 SDK 的输出 log 文件。SDK 运行时产生的所有 log 将写入该文件。应用程序必须保证指定的目录存在而且可写。

 Note: iOS 平台下日志文件的默认地址为：Library/caches/agorasdk.log。

 @param filePath 日志文件的完整路径。该日志文件为 UTF-8 编码。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setLogFile:(NSString * _Nonnull)filePath;

/** 设置日志过滤器

 @param filter AgoraLogFilter.

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)setLogFilter:(NSUInteger)filter;

/** 获取通话 ID

 客户端在每次 [joinChannelByToken](joinChannelByToken:channelId:info:uid:joinSuccess:) 后会生成一个对应的 CallId，标识该客户端的此次通话。有些方法如rate, complain需要在通话结束后调用，向SDK提交反馈，这些方法必须指定 CallId 参数。使用这些反馈方法，需要在通话过程中调用 getCallId 方法获取 CallId，在通话结束后在反馈方法中作为参数传入

 @return 当前通话 ID
 */
- (NSString * _Nullable)getCallId;

/** 给通话评分

 该方法能够让用户为通话评分，一般在通话结束后调用。

 @param callId      通话 getCallId 函数获取的通话 ID
 @param rating      给通话的评分，最低 1 分，最高 10 分
 @param description (非必选项) 给通话的描述，可选，长度应小于 800 字节

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 * ERR_INVALID_ARGUMENT (-2)：传入的参数无效，例如 callId 无效
 * ERR_NOT_READY (-3)：SDK 不在正确的状态，可能是因为没有成功初始化

 */
- (int)rate:(NSString * _Nonnull)callId
     rating:(NSInteger)rating
description:(NSString * _Nullable)description;

/** 投诉通话质量

 该方法让用户就通话质量进行投诉。一般在通话结束后调用。

 @param callId      通过 getCallId 函数获取的通话 ID
 @param description (非必选项) 给通话的描述，可选，长度应小于 800 字节

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 * ERR_INVALID_ARGUMENT (-2)：传入的参数无效，例如 callId 无效
 * ERR_NOT_READY (-3)：SDK 不在正确的状态，可能是因为没有成功初始化

 */
- (int)complain:(NSString * _Nonnull)callId
    description:(NSString * _Nullable)description;

/** 分发／不分发回调至主队列

 如被禁用，应用程序应将 UI 操作分发到主队列。

 @param enabled * True: 分发回调方法到主队列。
 * False: 不分发回调方法到主队列。

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)enableMainQueueDispatch:(BOOL)enabled;
/** 通过 JSON 配置 SDK 提供技术预览或特别定制功能

 JSON 选项默认不公开。声网工程师正在努力寻求以标准化方式公开 JSON 选项。

 @param options JSON 格式的 SDK 选项
 */
- (int)setParameters:(NSString * _Nonnull)options;

 /**
  该方法未公开，请联系声网支持 support@agora.io 获取详情。
  */
- (NSString * _Nullable)getParameter:(NSString * _Nonnull)parameter
                                args:(NSString * _Nullable)args;

#pragma mark Deprecated methods

/**-----------------------------------------------------------------------------
 * @name Deprecated methods
 * -----------------------------------------------------------------------------
 */

/**
 @deprecated
 */
- (void)audioVolumeIndicationBlock:(void(^ _Nullable)(NSArray * _Nonnull speakers, NSInteger totalVolume))audioVolumeIndicationBlock __deprecated;
/**
 @deprecated
 */
- (void)firstLocalVideoFrameBlock:(void(^ _Nullable)(NSInteger width, NSInteger height, NSInteger elapsed))firstLocalVideoFrameBlock __deprecated;
/**
 @deprecated
 */
- (void)firstRemoteVideoDecodedBlock:(void(^ _Nullable)(NSUInteger uid, NSInteger width, NSInteger height, NSInteger elapsed))firstRemoteVideoDecodedBlock __deprecated;
/**
 @deprecated
 */
- (void)firstRemoteVideoFrameBlock:(void(^ _Nullable)(NSUInteger uid, NSInteger width, NSInteger height, NSInteger elapsed))firstRemoteVideoFrameBlock __deprecated;
/**
 @deprecated
 */
- (void)userJoinedBlock:(void(^ _Nullable)(NSUInteger uid, NSInteger elapsed))userJoinedBlock __deprecated;
/**
 @deprecated
 */
- (void)userOfflineBlock:(void(^ _Nullable)(NSUInteger uid))userOfflineBlock __deprecated;
/**
 @deprecated
 */
- (void)userMuteAudioBlock:(void(^ _Nullable)(NSUInteger uid, BOOL muted))userMuteAudioBlock __deprecated;
/**
 @deprecated
 */
- (void)userMuteVideoBlock:(void(^ _Nullable)(NSUInteger uid, BOOL muted))userMuteVideoBlock __deprecated;
/**
 @deprecated
 */
- (void)localVideoStatBlock:(void(^ _Nullable)(NSInteger sentBitrate, NSInteger sentFrameRate))localVideoStatBlock __deprecated;
/**
 @deprecated
 */
- (void)remoteVideoStatBlock:(void(^ _Nullable)(NSUInteger uid, NSInteger delay, NSInteger receivedBitrate, NSInteger receivedFrameRate))remoteVideoStatBlock __deprecated;
/**
 @deprecated
 */
- (void)cameraReadyBlock:(void(^ _Nullable)(void))cameraReadyBlock __deprecated;
/**
 @deprecated
 */
- (void)connectionLostBlock:(void(^ _Nullable)(void))connectionLostBlock __deprecated;
/**
 @deprecated
 */
- (void)rejoinChannelSuccessBlock:(void(^ _Nullable)(NSString * _Nonnull channel, NSUInteger uid, NSInteger elapsed))rejoinChannelSuccessBlock __deprecated;
/**
 @deprecated
 */
- (void)rtcStatsBlock:(void(^ _Nullable)(AgoraChannelStats * _Nonnull stat))rtcStatsBlock __deprecated;
/**
 @deprecated
 */
- (void)leaveChannelBlock:(void(^ _Nullable)(AgoraChannelStats * _Nonnull stat))leaveChannelBlock __deprecated;
/**
 @deprecated
 */
- (void)audioQualityBlock:(void(^ _Nullable)(NSUInteger uid, AgoraNetworkQuality quality, NSUInteger delay, NSUInteger lost))audioQualityBlock __deprecated;
/**
 @deprecated
 */
- (void)networkQualityBlock:(void(^ _Nullable)(NSUInteger uid, AgoraNetworkQuality txQuality, AgoraNetworkQuality rxQuality))networkQualityBlock __deprecated;
/**
 @deprecated
 */
- (void)lastmileQualityBlock:(void(^ _Nullable)(AgoraNetworkQuality quality))lastmileQualityBlock __deprecated;
/**
 @deprecated
 */
- (void)mediaEngineEventBlock:(void(^ _Nullable)(NSInteger code))mediaEngineEventBlock __deprecated;

/**
 @deprecated
 */
- (int)playEffect:(int)soundId
         filePath:(NSString * _Nullable)filePath
        loopCount:(int)loopCount
            pitch:(double)pitch
              pan:(double)pan
             gain:(double)gain __deprecated;
/**
 @deprecated
 */
- (int)switchView:(NSUInteger)uid1
       andAnother:(NSUInteger)uid2 __deprecated;
/** 设置本地视频属性

 @deprecated
 */
- (int)setVideoProfile:(AgoraVideoProfile)profile
    swapWidthAndHeight:(BOOL)swapWidthAndHeight __deprecated_msg("use setVideoEncoderConfiguration: insted");
/**
 @deprecated

 已被 [sharedEngineWithAppId]([AgoraRtcEngineKit sharedEngineWithAppId:delegate:]) 方法替代。
 */
+ (instancetype _Nonnull)sharedEngineWithAppId:(NSString * _Nonnull)AppId error:(void(^ _Nullable)(AgoraErrorCode errorCode))errorBlock __deprecated;

/**
 @deprecated

 获取媒体引擎版本号

 @return string, 媒体引擎版本号
 */
+ (NSString * _Nonnull)getMediaEngineVersion;
/** 手动设置视频的宽、高、帧率和码率

 @deprecated From v2.3

 @param size 想要设置的视频尺寸，最高不超过 1280 x 720.
 @param frameRate 想要设置的视频帧率，最高值不超过 30. 如 5、 10、 15、 24、 30 等。
 @param bitrate 你想要设置的视频码率，需要开发者根据想要设置的视频的宽、高和帧率，并结合上表，手动推算出合适值。宽和高固定的情况下，码率随帧率的变化而变化：

 * 若选取的帧率为 5 fps，则推算码率为上表推荐码率除以 2
 * 若选取的帧率为 15 fps，则推算码率为上表推荐码率
 * 若选取的帧率为 30 fps，则推算码率为上表码率乘以 1.5
 * 若选取其余帧率，等比例推算即可

 假设分辨率为 320 x 240，根据上表，帧率为 15 fps 时推荐码率为 200，则：

 * 若帧率为 5 FPS，则推算码率为 200 除以 2，即 100
 * 若帧率为 30 FPS，则推算码率为 200 乘以 1.5，即 300
 * 若设置的码率超过合理范围， SDK 将自动将其调整到合理区间。

 */
- (int)setVideoResolution: (CGSize)size andFrameRate: (NSInteger)frameRate bitrate: (NSInteger) bitrate __deprecated_msg("use setVideoEncoderConfiguration: insted");
/** 关闭音频
 @deprecated

 Replaced with [disableAudio]([AgoraRtcEngineKit disableAudio]) from v2.3

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)pauseAudio;

/** 打开音频
 @deprecated

 Replaced with [enableAudio]([AgoraRtcEngineKit enableAudio]) from v2.3.

 @return * 0: 方法调用成功
 * < 0: 方法调用失败
 */
- (int)resumeAudio;
/**
 @deprecated
 请使用 setAudioProfile:scenario:
 */
- (int)setHighQualityAudioParametersWithFullband:(BOOL)fullband
                                          stereo:(BOOL)stereo
                                     fullBitrate:(BOOL)fullBitrate __deprecated;
@end
