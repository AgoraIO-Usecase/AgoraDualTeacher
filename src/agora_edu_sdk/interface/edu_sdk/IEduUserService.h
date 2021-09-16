//
//  EduUserService.h
//
//  Created by SRS on 2020/6/28.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once

#include <functional>
#include "AgoraRefPtr.h"
#include "EduBaseTypes.h"
#include "EduStream.h"
#include "EduUser.h"
#include "EduVideoEncoderConfiguration.h"

namespace agora {
namespace edu {

enum MediaStreamState { STREAM_ADDED, STREAM_UPDATED, STREAM_REMOVED };

class IEduUserEventHandler {
 public:
  virtual void OnLocalUserStateUpdated(EduUserEvent user_event,
                                       EduUserStateChangeType type) = 0;
  virtual void OnLocalUserPropertyUpdated(EduUser user, const char* cause) = 0;
  virtual void OnLocalStreamChanged(EduStreamEvent stream_event,
                                    MediaStreamState state) = 0;
};

class IEduUserOperationEventHandler {
 public:
  virtual void OnLocalStreamCreated(EduStream stream_info, EduError err) = 0;
  virtual void OnStreamPublished(EduStream stream_info, EduError err) = 0;
  virtual void OnStreamUnpublished(EduStream stream_info, EduError err) = 0;

  virtual void OnRemoteStreamSubscribed(EduStream stream_info,
                                        EduError err) = 0;
  virtual void OnRemoteStreamUnsubscribed(EduStream stream_info,
                                          EduError err) = 0;

  virtual void OnStreamMuted(EduStream stream_info, EduError err) = 0;
  virtual void OnStreamUnmuted(EduStream stream_info, EduError err) = 0;

  virtual void OnRoomCustomMessageSended(const char* text, EduError err) = 0;
  virtual void OnUserCustomMessageSended(const char* text, EduUser remote_user,
                                   EduError err) = 0;
  virtual void OnRoomChatMessageSended(const char* text, EduError err) = 0;
  virtual void OnUserChatMessageSended(const char* text, EduUser remote_user,
                                       EduError err) = 0;
  virtual void OnSetRoomPropertyCompleted(Property property,
                                          const char* custom_cause,
                                          EduError err) = 0;
  virtual void OnSetUserPropertyCompleted(Property property,
                                          const char* custom_cause,
                                          EduUser target_user,
                                          EduError err) = 0;
};

enum EduRenderMode {
  /** Hidden(1): Uniformly scale the video until it fills the visible boundaries
     (cropped). One dimension of the video may have clipped contents. */
  EDU_RENDER_MODE_HIDDEN,
  /** Fit(2): Uniformly scale the video until one of its dimension fits the
     boundary (zoomed to fit). Areas that are not filled due to the disparity in
     the aspect ratio are filled with black. */
  EDU_RENDER_MODE_FIT
};

enum EduVideoStreamType {
  EDU_VIDEO_STREAM_TYPE_HIGH,
  EDU_VIDEO_STREAM_TYPE_LOW
};

struct EduStartActionConfig {
  const char* process_uuid;
  EduActionType action;
  EduUser to_user;
  long timeout;

  Property* payload;
  size_t payload_count;
};

struct EduStopActionConfig {
  const char* process_uuid;
  EduActionType action;

  Property* payload;
  size_t payload_count;
};

class IEduVideoFrame;
using delegate_render = std::function<void(IEduVideoFrame*)>;

struct EduRenderConfig {
  EduRenderMode render_mode;
  delegate_render custom_render;
};

struct EduSubscribeOptions {
  bool subscribe_audio;
  bool subscribe_video;
  EduVideoStreamType video_stream_type;

  EduSubscribeOptions()
      : subscribe_audio(true),
        subscribe_video(true),
        video_stream_type(EDU_VIDEO_STREAM_TYPE_HIGH) {}
};

struct EduVideoConfig {
  uint32_t video_dimension_width;
  uint32_t video_dimension_height;
  uint32_t frame_rate;
  uint32_t bitrate;
  EduVideoOutputOrientationMode orientation_mode;
  EduDegradationPreference degradation_preference;

  EduVideoConfig()
      : video_dimension_width(360),
        video_dimension_height(360),
        frame_rate(15),
        bitrate(0),
        orientation_mode(EDU_VIDEO_OUTPUT_ORIENTATION_MODE_FIXED_LANDSCAPE),
        degradation_preference(EDU_DEGRADATION_MAINTAIN_QUALITY) {}
};

struct EduStreamConfig {
  int stream_uuid;
  char stream_name[kMaxStreamUuidSize];
  bool enable_camera;
  bool enable_microphone;
  EduVideoSourceType video_soruce_type;
  EduStreamConfig()
      : stream_uuid{0},
        stream_name{0},
        enable_camera(false),
        enable_microphone(false) {}
};

class IEduUserService : public RefCountInterface {
 public:
  /* code:message
   * 1:you haven't joined the room
   */
  virtual EduLocalUser GetLocalUserInfo() = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual agora_refptr<IStreamInfoCollection> GetLocalStreams() = 0;

  virtual EduError SetVideoConfig(const EduStream& stream,
                                  const EduVideoConfig& config) = 0;

  // media
  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError CreateLocalStream(EduStreamConfig config,EduStream& stream) = 0;

  /* code:message
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError SwitchCamera(const EduStream& stream,
                                const char* device_id = "") = 0;

  // stream
  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError SubscribeStream(const EduStream& stream,
                                   const EduSubscribeOptions& options) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError UnsubscribeStream(const EduStream& stream,
                                     const EduSubscribeOptions& options) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError PublishStream(const EduStream& stream) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError UnpublishStream(const EduStream& stream) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError MuteStream(const EduStream& stream) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError UnmuteStream(const EduStream& stream) = 0;

  // message
  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SendRoomCustomMessage(const char* text) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SendUserCustomMessage(const char* text,
                                   const EduUser& remote_user) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SendRoomChatMessage(const char* text) = 0;

  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SendUserChatMessage(const char* text,
                                       const EduUser& remote_user) = 0;

  // process action
  // 一期教育SDK没有这个方法，只是给娱乐使用
  virtual EduError StartActionWithConfig(
      const EduStartActionConfig& config) = 0;
  virtual EduError StopActionWithConfig(const EduStopActionConfig& config) = 0;


  // property
  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SetRoomProperty(Property property,
                                   const char* custom_cause) = 0;
  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SetUserProperty(Property property, const char* custom_cause,
                                   EduUser target_user) = 0;

  // render
  /* code:message
   * 1:parameter XXX is invalid
   */
  virtual EduError SetStreamView(EduStream stream, View* view) = 0;
  virtual EduError SetStreamView(EduStream stream, View* view,
                                 const EduRenderConfig& config) = 0;

  virtual void RegisterEventHandler(IEduUserEventHandler* event_handler) = 0;
  virtual void UnregisterEventHandler(IEduUserEventHandler* event_hadler) = 0;
  virtual void RegisterOperationEventHandler(
      IEduUserOperationEventHandler* handler) = 0;
  virtual void UnregisterOperationEventHandler(
      IEduUserOperationEventHandler* handler) = 0;
  virtual void Destory() = 0;
};

}  // namespace edu
}  // namespace agora