//
//  edu_student_service_impl.cpp
//
//  Created by LC on 2020/11/20.
//  Copyright © 2020 agora. All rights reserved.
//

#include "edu_student_service_impl.h"
#include "refcountedobject.h"
namespace agora {
namespace edu {
EduStudentService::EduStudentService(const EduUserServiceConfig& config) {
  base = new RefCountedObject<EduUserService>(config);
}

EduStudentService::~EduStudentService() {}

agora_refptr<IStreamInfoCollection> EduStudentService::GetLocalStreams() {
  return base->GetLocalStreams();
}

EduLocalUser EduStudentService::GetLocalUserInfo() {
  return base->GetLocalUserInfo();
}

EduError EduStudentService::SetVideoConfig(const EduStream& stream,
                                           const EduVideoConfig& config) {
  return base->SetVideoConfig(stream, config);
}
EduError EduStudentService::CreateLocalStream(EduStreamConfig config,
                                              EduStream& stream) {
  return base->CreateLocalStream(config, stream);
}
EduError EduStudentService::SwitchCamera(const EduStream& stream,
                                         const char* device_id) {
  return base->SwitchCamera(stream, device_id);
}
EduError EduStudentService::SubscribeStream(
    const EduStream& stream, const EduSubscribeOptions& options) {
  return base->SubscribeStream(stream, options);
}
EduError EduStudentService::UnsubscribeStream(
    const EduStream& stream, const EduSubscribeOptions& options) {
  return base->UnsubscribeStream(stream, options);
}
EduError EduStudentService::PublishStream(const EduStream& stream) {
  return base->PublishStream(stream);
}
EduError EduStudentService::UnpublishStream(const EduStream& stream) {
  return base->UnpublishStream(stream);
}
EduError EduStudentService::MuteStream(const EduStream& stream) {
  return base->MuteStream(stream);
}
EduError EduStudentService::UnmuteStream(const EduStream& stream) {
  return base->UnmuteStream(stream);
}
EduError EduStudentService::SendRoomCustomMessage(const char* text) {
  return base->SendRoomChatMessage(text);
}
EduError EduStudentService::SendUserCustomMessage(const char* text,
                                                  const EduUser& remote_user) {
  return base->SendUserCustomMessage(text, remote_user);
}
EduError EduStudentService::SendRoomChatMessage(const char* text) {
  return base->SendRoomChatMessage(text);
}
EduError EduStudentService::SendUserChatMessage(const char* text,
                                                const EduUser& remote_user) {
  return base->SendUserChatMessage(text, remote_user);
}
EduError EduStudentService::StartActionWithConfig(
    const EduStartActionConfig& config) {
  return base->StartActionWithConfig(config);
}
EduError EduStudentService::StopActionWithConfig(
    const EduStopActionConfig& config) {
  return base->StopActionWithConfig(config);
}
EduError EduStudentService::SetRoomProperty(Property property,
                                            const char* custom_cause) {
  return base->SetRoomProperty(property, custom_cause);
}
EduError EduStudentService::SetUserProperty(Property property,
                                            const char* custom_cause,
                                            EduUser target_user) {
  return base->SetUserProperty(property, custom_cause, target_user);
}
EduError EduStudentService::SetCustomRender(bool enabled) {
  return base->SetCustomRender(enabled);
}
EduError EduStudentService::SetStreamView(EduStream stream, View* view) {
  return base->SetStreamView(stream, view);
}
EduError EduStudentService::SetStreamView(EduStream stream, View* view,
                                          const EduRenderConfig& config) {
  return base->SetStreamView(stream, view, config);
}

void EduStudentService::RegisterEventHandler(
    IEduUserEventHandler* event_handler) {
  base->RegisterEventHandler(event_handler);
}

void EduStudentService::UnregisterEventHandler(
    IEduUserEventHandler* event_hadler) {
  base->UnregisterEventHandler(event_hadler);
}

void EduStudentService::RegisterOperationEventHandler(
    IEduUserOperationEventHandler* handler) {
  base->RegisterOperationEventHandler(handler);
}

void EduStudentService::UnregisterOperationEventHandler(
    IEduUserOperationEventHandler* handler) {
  base->UnregisterOperationEventHandler(handler);
}

void EduStudentService::RegisterOperationEventHandler(
    IEduStudentOperationEventHandler* handler) {
  base->RegisterOperationEventHandler(handler);
}
void EduStudentService::UnregisterOperationEventHandler(
    IEduStudentOperationEventHandler* handler) {
  base->UnregisterOperationEventHandler(handler);
}

void EduStudentService::Destory() { base->Destory(); }

void EduStudentService::OnLocalUserStateUpdated(EduUserEvent user_event,
                                                EduUserStateChangeType type) {
  base->OnLocalUserStateUpdated(user_event, type);
}

void EduStudentService::OnLocalUserPropertyUpdated(EduUser user,
                                                   const char* cause) {
  base->OnLocalUserPropertyUpdated(user, cause);
}

void EduStudentService::OnLocalStreamChanged(EduStreamEvent stream_event,
                                             MediaStreamState state) {
  base->OnLocalStreamChanged(stream_event, state);
}

void EduStudentService::OnRemoteUserStateUpdated(EduUserEvent user_event,
                                                 EduUserStateChangeType type) {
  base->OnRemoteUserStateUpdated(user_event, type);
}

void EduStudentService::OnRemoteUserPropertyUpdated(EduUser user,
                                                    const char* cause) {
  base->OnRemoteUserPropertyUpdated(user, cause);
}

void EduStudentService::OnRemoteStreamChanged(
    agora_refptr<IStreamEventCollection> stream_event_collection,
    MediaStreamState state) {
  base->OnRemoteStreamChanged(stream_event_collection, state);
}

void EduStudentService::OnRemoteStreamAdded(
    agora_refptr<IStreamInfoCollection> stream_event_collection,
    MediaStreamState state) {
  base->OnRemoteStreamAdded(stream_event_collection, state);
}

#if 0
void test() {
  agora_refptr<EduStudentService> ptr = new edu::RefCountedObject<EduStudentService>();
}
#endif

}  // namespace edu
}  // namespace agora
