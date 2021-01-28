//
//  edu_student_service_impl.h
//
//  Created by LC on 2020/11/20.
//  Copyright © 2020 agora. All rights reserved.
//

#pragma once
#include "edu_service_restful_api.h"
#include "edu_user_service_impl.h"
#include "facilities/tools/rtc_callback.h"
#include "interface/base/AgoraRefPtr.h"
#include "interface/edu_sdk/IEduStudentService.h"
#include "rtc_conn_manager.h"
namespace agora {
namespace edu {

class EduStudentService : public IEduStudentService,
                          public IEduReceiveClassRoomManagerEventHandler {
 private:
  agora_refptr<EduUserService> base;

 public:
  EduStudentService(const EduUserServiceConfig& config);
  ~EduStudentService();

  agora_refptr<IStreamInfoCollection> GetLocalStreams() override;
  EduLocalUser GetLocalUserInfo() override;

  virtual EduError SetVideoConfig(const EduStream& stream,
                                  const EduVideoConfig& config) override;
  virtual EduError CreateLocalStream(EduStreamConfig config,
                                            EduStream& stream) override;
  virtual EduError SwitchCamera(const EduStream& stream,
                                const char* device_id = "") override;
  virtual EduError SubscribeStream(const EduStream& stream,
                                   const EduSubscribeOptions& options) override;
  virtual EduError UnsubscribeStream(
      const EduStream& stream, const EduSubscribeOptions& options) override;
  virtual EduError PublishStream(const EduStream& stream) override;
  virtual EduError UnpublishStream(const EduStream& stream) override;
  virtual EduError MuteStream(const EduStream& stream) override;
  virtual EduError UnmuteStream(const EduStream& stream) override;
  virtual EduError SendRoomCustomMessage(const char* text) override;
  virtual EduError SendUserCustomMessage(const char* text,
                                   const EduUser& remote_user) override;
  virtual EduError SendRoomChatMessage(const char* text) override;
  virtual EduError SendUserChatMessage(const char* text,
                                       const EduUser& remote_user) override;
  virtual EduError StartActionWithConfig(
      const EduStartActionConfig& config) override;
  virtual EduError StopActionWithConfig(
      const EduStopActionConfig& config) override;
  virtual EduError SetRoomProperty(Property property,
                                   const char* custom_cause) override;
  virtual EduError SetUserProperty(Property property, const char* custom_cause,
                                   EduUser target_user) override;
  virtual EduError SetCustomRender(bool enabled) ;
  virtual EduError SetStreamView(EduStream stream, View* view) override;
  virtual EduError SetStreamView(EduStream stream, View* view,
                                 const EduRenderConfig& config) override;

 public:
  virtual void RegisterEventHandler(
      IEduUserEventHandler* event_handler) override;
  virtual void UnregisterEventHandler(
      IEduUserEventHandler* event_hadler) override;

  virtual void RegisterOperationEventHandler(
      IEduUserOperationEventHandler* handler) override;
  virtual void UnregisterOperationEventHandler(
      IEduUserOperationEventHandler* handler) override;

  virtual void RegisterOperationEventHandler(
      IEduStudentOperationEventHandler* handler) override;
  virtual void UnregisterOperationEventHandler(
      IEduStudentOperationEventHandler* handler) override;

  virtual void Destory() override;

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

 private:
  ServiceRestfulHelper rest_api_helper_;
};

}  // namespace edu
}  // namespace agora
