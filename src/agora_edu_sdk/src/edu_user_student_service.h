#pragma once
#include "facilities/tools/rtc_callback.h"
#include "interface/base/AgoraRefPtr.h"
#include "interface/edu_sdk/IEduStudentService.h"
#include "rtc_conn_manager.h"

namespace agora {
namespace edu {

class EduStudentService : public IEduStudentService {
 private:
  utils::RtcAsyncCallback<IEduStudentOperationEventHandler>::Type
      event_handlers_;

 public:
  EduStudentService();
  ~EduStudentService();

  virtual EduError SetVideoConfig(const EduStream & stream,const EduVideoConfig& config) override;
  virtual EduError StartOrUpdateLocalStream(const EduStreamConfig& config,
                                            EduStream& stream) override;
  virtual EduError SwitchCamera(EduStream& stream,
                                const char* device_id = "") override;
  virtual EduError SubscribeStream(const EduStream& stream,
                                   const EduSubscribeOptions& options) override;
  virtual EduError UnsubscribeStream(
      const EduStream& stream, const EduSubscribeOptions& options) override;
  virtual EduError PublishStream(const EduStream& stream) override;
  virtual EduError UnpublishStream(const EduStream& stream) override;
  virtual EduError MuteStream(const EduStream& stream) override;
  virtual EduError UnmuteStream(const EduStream& stream) override;
  virtual EduError SendRoomMessage(const char* text) override;
  virtual EduError SendUserMessage(const char* text,
                                   const EduUser& remote_user) override;
  virtual EduError SendRoomChatMessage(const char* text) override;
  virtual EduError SendUserChatMessage(const char* text,
                                       const EduUser& remote_user) override;
  virtual EduError StartActionWithConfig(
      const EduStartActionConfig& config) override;
  virtual EduError StopActionWithConfig(
      const EduStopActionConfig& config) override;
  virtual EduError SetRoomProperty(Property property, char* custom_cause) override;
  virtual EduError SetUserProperty(Property property, char* custom_cause,
                                   EduUser target_user) override;
  virtual EduError SetStreamView(EduStream stream, View* view) override;
  virtual EduError SetStreamView(EduStream stream, View* view,
                                 const EduRenderConfig& config) override;

 public:
  virtual void RegisterOperationEventHandler(
      IEduStudentOperationEventHandler* handler) override;
  virtual void UnregisterOperationEventHandler(
      IEduStudentOperationEventHandler* handler) override;

 private:
  RtcConnManager rtc_manager_;
};

}  // namespace edu
}  // namespace agora
