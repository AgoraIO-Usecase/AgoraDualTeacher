//
//  EduUserService.h
//
//  Created by LC on 2020/11/18.
//  Copyright © 2020 agora. All rights reserved.
//
#pragma once
#include <unordered_map>
#include "IAgoraVideoSourceEngine.h"
#include "base/facilities/tools/rtc_callback.h"
#include "edu_collection_impl.h"
#include "edu_service_restful_api.h"
#include "edu_user_service_base.h"
#include "interface/base/EduStream.h"
#include "interface/edu_sdk/IEduUserService.h"
#include "rtc_conn_manager.h"

namespace agora {
namespace edu {

class EduUserService : public IEduUserService,
                       public IEduReceiveClassRoomManagerEventHandler {
  friend class EduTeacherService;
  friend class EduAssistantService;

 public:
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

  EduError UpdateStream(const EduStream& stream);

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

  // set custom render
  // sdk can't render video frame, usage EduRenderConfig to set custom_render
  // custom_resder will callback IEduVideoFrame data.
  virtual EduError SetCustomRender(bool enabled);
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

  EduUserService(const EduUserServiceConfig& config);

  ~EduUserService() { Destory(); }

 protected:
  EduError _CreateLocalStream(EduStreamConfig config, EduStream& stream,
                              bool notify_exist);

 private:
  agora_refptr<StreamInfoCollection> local_streams_;
  EduLocalUser local_user_;
  RtcInfo rtc_info_;
  std::unique_ptr<RtcConnManager> rtc_manager_;
  std::shared_ptr<ServiceRestfulHelper> rest_api_helper_;
  bool auto_publish_;
  bool auto_subscribe_;
  utils::RtcAsyncCallback<IEduUserEventHandler>::Type user_event_handler_;
};
}  // namespace edu
}  // namespace agora
