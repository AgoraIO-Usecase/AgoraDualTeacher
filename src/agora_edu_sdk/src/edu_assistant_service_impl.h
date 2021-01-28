//
//  edu_assistant_service_impl.h
//
//  Created by LC on 2020/11/27.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once
#include "edu_user_service_impl.h"
#include "interface/edu_sdk/IEduAssistantService.h"

namespace agora {
namespace edu {

class EduAssistantService : public IEduAssistantService,
                            public IEduReceiveClassRoomManagerEventHandler {
 private:
  agora_refptr<EduUserService> base;

 public:
  EduAssistantService(const EduUserServiceConfig& config);
  ~EduAssistantService();

  EduLocalUser GetLocalUserInfo() override;

  agora_refptr<IStreamInfoCollection> GetLocalStreams() override;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError SetVideoConfig(const EduStream& stream,
                                  const EduVideoConfig& config) override;

  // media
  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError CreateLocalStream(EduStreamConfig config,
                                            EduStream& stream) override;

  /* code:message
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError SwitchCamera(const EduStream& stream,
                                const char* device_id = "") override;

  // stream
  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError SubscribeStream(const EduStream& stream,
                                   const EduSubscribeOptions& options) override;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   */
  virtual EduError UnsubscribeStream(
      const EduStream& stream, const EduSubscribeOptions& options) override;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError PublishStream(const EduStream& stream) override;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError UnpublishStream(const EduStream& stream) override;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError MuteStream(const EduStream& stream) override;

  /* code:message
   * 1:parameter XXX is invalid
   * 201:media error:code，透传rtc错误code或者message。
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError UnmuteStream(const EduStream& stream) override;

  // message
  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SendRoomCustomMessage(const char* text) override;

  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SendUserCustomMessage(const char* text,
                                   const EduUser& remote_user) override;

  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SendRoomChatMessage(const char* text) override;

  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SendUserChatMessage(const char* text,
                                       const EduUser& remote_user) override;

  // process action
  // 一期教育SDK没有这个方法，只是给娱乐使用
  virtual EduError StartActionWithConfig(
      const EduStartActionConfig& config) override;
  virtual EduError StopActionWithConfig(
      const EduStopActionConfig& config) override;

  // property
  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SetRoomProperty(Property property,
                                   const char* custom_cause) override;
  /* code:message
   * 1:parameter XXX is invalid
   * 301:network error，透传后台错误msg字段
   */
  virtual EduError SetUserProperty(Property property, const char* custom_cause,
                                   EduUser target_user) override;

  virtual EduError SetCustomRender(bool enabled) ;
  // render
  /* code:message
   * 1:parameter XXX is invalid
   */
  virtual EduError SetStreamView(EduStream stream, View* view) override;
  virtual EduError SetStreamView(EduStream stream, View* view,
                                 const EduRenderConfig& config) override;
  virtual void RegisterEventHandler(
      IEduUserEventHandler* event_handler) override;
  virtual void UnregisterEventHandler(
      IEduUserEventHandler* event_hadler) override;
  virtual void RegisterOperationEventHandler(
      IEduUserOperationEventHandler* handler) override;
  virtual void UnregisterOperationEventHandler(
      IEduUserOperationEventHandler* handler) override;
  virtual void Destory() override;
  // Teacher Stream
  virtual EduError CreateOrUpdateTeacherStream(
      EduStream remote_stream) override;
  // Student Stream
  virtual EduError CreateOrUpdateStudentStream(
      EduStream remote_stream) override;
  virtual void RegisterOperationEventHandler(
      IEduAssistantOperationEventHandler* handler) override;
  virtual void UnregisterOperationEventHandler(
      IEduAssistantOperationEventHandler* handler) override;

 public:
  virtual void OnLocalUserStateUpdated(EduUserEvent user_event,
                                       EduUserStateChangeType type) override;
  virtual void OnLocalUserPropertyUpdated(EduUser user,
                                          const char* cause) override;

  virtual void OnLocalStreamChanged(EduStreamEvent stream_event,
                                    MediaStreamState state) override;

  virtual void OnRemoteUserStateUpdated(EduUserEvent user_event,
                                        EduUserStateChangeType type) override;
  virtual void OnRemoteUserPropertyUpdated(EduUser user,
                                           const char* cause) override;

  virtual void OnRemoteStreamChanged(
      agora_refptr<IStreamEventCollection> stream_event_collection,
      MediaStreamState state) override;

  virtual void OnRemoteStreamAdded(
      agora_refptr<IStreamInfoCollection> stream_event_collection,
      MediaStreamState state) override;
};
}  // namespace edu
}  // namespace agora