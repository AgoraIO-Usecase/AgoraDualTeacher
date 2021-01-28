//
//  edu_assistant_service_impl.cpp
//
//  Created by LC on 2020/11/27.
//  Copyright © 2020 agora. All rights reserved.
//
#include "edu_assistant_service_impl.h"
#include "edu_user_service_base.h"
#include "refcountedobject.h"
namespace agora {
namespace edu {
EduAssistantService::EduAssistantService(const EduUserServiceConfig& config) {
  base = new RefCountedObject<EduUserService>(config);
}
EduAssistantService::~EduAssistantService() {}

agora_refptr<IStreamInfoCollection> EduAssistantService::GetLocalStreams() {
  return base->GetLocalStreams();
}

EduLocalUser EduAssistantService::GetLocalUserInfo() {
  return base->GetLocalUserInfo();
}
/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduAssistantService::SetVideoConfig(const EduStream& stream,
                                             const EduVideoConfig& config) {
  return base->SetVideoConfig(stream, config);
}

// media
/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduAssistantService::CreateLocalStream(EduStreamConfig config,
                                                EduStream& stream) {
  return base->CreateLocalStream(config, stream);
}

/* code:message
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduAssistantService::SwitchCamera(const EduStream& stream,
                                           const char* device_id) {
  return base->SwitchCamera(stream, device_id);
}

// stream
/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduAssistantService::SubscribeStream(
    const EduStream& stream, const EduSubscribeOptions& options) {
  return base->SubscribeStream(stream, options);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 */
EduError EduAssistantService::UnsubscribeStream(
    const EduStream& stream, const EduSubscribeOptions& options) {
  return base->UnsubscribeStream(stream, options);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::PublishStream(const EduStream& stream) {
  return base->PublishStream(stream);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::UnpublishStream(const EduStream& stream) {
  return base->UnpublishStream(stream);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::MuteStream(const EduStream& stream) {
  return base->MuteStream(stream);
}

/* code:message
 * 1:parameter XXX is invalid
 * 201:media error:code，透传rtc错误code或者message。
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::UnmuteStream(const EduStream& stream) {
  return base->UnmuteStream(stream);
}

// message
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::SendRoomCustomMessage(const char* text) {
  return base->SendRoomCustomMessage(text);
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::SendUserCustomMessage(
    const char* text, const EduUser& remote_user) {
  return base->SendUserCustomMessage(text, remote_user);
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::SendRoomChatMessage(const char* text) {
  return base->SendRoomChatMessage(text);
}

/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::SendUserChatMessage(const char* text,
                                                  const EduUser& remote_user) {
  return base->SendUserChatMessage(text, remote_user);
}

// process action
// 一期教育SDK没有这个方法，只是给娱乐使用
EduError EduAssistantService::StartActionWithConfig(
    const EduStartActionConfig& config) {
  return base->StartActionWithConfig(config);
}
EduError EduAssistantService::StopActionWithConfig(
    const EduStopActionConfig& config) {
  return base->StopActionWithConfig(config);
}

// property
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::SetRoomProperty(Property property,
                                              const char* custom_cause) {
  return base->SetRoomProperty(property, custom_cause);
}
/* code:message
 * 1:parameter XXX is invalid
 * 301:network error，透传后台错误msg字段
 */
EduError EduAssistantService::SetUserProperty(Property property,
                                              const char* custom_cause,
                                              EduUser target_user) {
  return base->SetUserProperty(property, custom_cause, target_user);
}

EduError EduAssistantService::SetCustomRender(bool enabled) {
  return base->SetCustomRender(enabled);
}

// render
/* code:message
 * 1:parameter XXX is invalid
 */
EduError EduAssistantService::SetStreamView(EduStream stream, View* view) {
  return base->SetStreamView(stream, view);
}
EduError EduAssistantService::SetStreamView(EduStream stream, View* view,
                                            const EduRenderConfig& config) {
  return base->SetStreamView(stream, view, config);
}

void EduAssistantService::RegisterEventHandler(
    IEduUserEventHandler* event_handler) {
  return base->RegisterEventHandler(event_handler);
}
void EduAssistantService::UnregisterEventHandler(
    IEduUserEventHandler* event_hadler) {
  return base->UnregisterEventHandler(event_hadler);
}
void EduAssistantService::RegisterOperationEventHandler(
    IEduUserOperationEventHandler* handler) {
  return base->UnregisterOperationEventHandler(handler);
}
void EduAssistantService::UnregisterOperationEventHandler(
    IEduUserOperationEventHandler* handler) {
  return base->UnregisterOperationEventHandler(handler);
}
void EduAssistantService::Destory() { base->Destory(); }

// Teacher Stream
EduError EduAssistantService::CreateOrUpdateTeacherStream(
    EduStream remote_stream) {
  int err = base->rest_api_helper_->CreateOrUpdateTeacherStream(remote_stream);
  if (err) {
    return EDU_ERROR_NETWORK(err, "net:CreateOrUpdateTeacherStream falied");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}
// Student Stream
EduError EduAssistantService::CreateOrUpdateStudentStream(
    EduStream remote_stream) {
  int err = base->rest_api_helper_->CreateOrUpdateStudentStreamWithAssistant(
      remote_stream);
  if (err) {
    return EDU_ERROR_NETWORK(
        err, "net:CreateOrUpdateStudentStreamWithAssistant falied");
  }
  return EDU_ERROR_DEFAULT(ERR_OK, "");
}

void EduAssistantService::RegisterOperationEventHandler(
    IEduAssistantOperationEventHandler* handler) {
  base->RegisterOperationEventHandler(handler);
}

void EduAssistantService::UnregisterOperationEventHandler(
    IEduAssistantOperationEventHandler* handler) {
  base->UnregisterOperationEventHandler(handler);
}

void EduAssistantService::OnLocalUserStateUpdated(EduUserEvent user_event,
                                                  EduUserStateChangeType type) {
  base->OnLocalUserStateUpdated(user_event, type);
}

void EduAssistantService::OnLocalUserPropertyUpdated(EduUser user,
                                                     const char* cause) {
  base->OnLocalUserPropertyUpdated(user, cause);
}

void EduAssistantService::OnLocalStreamChanged(EduStreamEvent stream_event,
                                               MediaStreamState state) {
  base->OnLocalStreamChanged(stream_event, state);
}

void EduAssistantService::OnRemoteUserStateUpdated(
    EduUserEvent user_event, EduUserStateChangeType type) {
  base->OnRemoteUserStateUpdated(user_event, type);
}

void EduAssistantService::OnRemoteUserPropertyUpdated(EduUser user,
                                                      const char* cause) {
  base->OnRemoteUserPropertyUpdated(user, cause);
}

void EduAssistantService::OnRemoteStreamChanged(
    agora_refptr<IStreamEventCollection> stream_event_collection,
    MediaStreamState state) {
  base->OnRemoteStreamChanged(stream_event_collection, state);
}

void EduAssistantService::OnRemoteStreamAdded(
    agora_refptr<IStreamInfoCollection> stream_event_collection,
    MediaStreamState state) {
  base->OnRemoteStreamAdded(stream_event_collection, state);
}

#if 0
void test() {
  agora_refptr<EduAssistantService> ptr =
      new RefCountedObject<EduAssistantService>();
}
#endif

}  // namespace edu
}  // namespace agora