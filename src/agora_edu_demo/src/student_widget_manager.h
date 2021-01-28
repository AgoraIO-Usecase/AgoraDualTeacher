#pragma once
#include <QObject>

#include <set>

#include "util.h"

#include "AgoraInnerChatContent.h"
#include "AgoraSettingDialog.h"
#include "IEduClassroomManager.h"
#include "IEduManager.h"
#include "IEduStudentService.h"

class AgoraStudentWidget;
class LoginWidgetManager;
class StudentWidgetManager
    : public QObject,
      public IEduClassroomEventHandler,
      public IEduUserEventHandler,
      public IEduStudentOperationEventHandler,
      public std::enable_shared_from_this<StudentWidgetManager> {
  Q_OBJECT

 public:
  StudentWidgetManager(std::shared_ptr<LoginWidgetManager> login_widget_manager,
                       agora_refptr<IEduClassroomManager> classroom_manager,
                       const EduClassroomJoinOptions& options,
                       const EduClassroomConfig& config,
                       const SettingConfig& setting_config);
  ~StudentWidgetManager();

  void JoinClassroom();
  void LeaveClassroom();
  void SetWidgetView(const EduStream& stream, View* view,
                     const EduRenderConfig& config) {
    if (view == nullptr) {
      return;
    }
    user_service_->SetStreamView(stream, view, config);
  }

 protected:
  void customEvent(QEvent* event);

 signals:
  void JoinClassroomSig(int);

 private slots:
  void OnChatMessageSend(ChatMessage);

 private:
  // IEduClassroomEventHandler
  void OnRemoteUsersJoined(
      agora_refptr<IUserInfoCollection> user_info_collection,
      EduClassroom from_classroom) override;
  void OnRemoteUsersLeft(
      agora_refptr<IUserEventCollection> user_event_collection,
      EduClassroom from_classroom) override;
  void OnRemoteUserStateUpdated(EduUserEvent user_event,
                                EduUserStateChangeType type,
                                EduClassroom from_classroom) override;

  // message
  void OnRoomChatMessageReceived(agora_refptr<IAgoraEduMessage> text_message,
                                 EduBaseUser from_user,
                                 EduClassroom from_classroom) override;
  void OnRoomCustomMessageReceived(agora_refptr<IAgoraEduMessage> text_message,
                                   EduBaseUser from_user,
                                   EduClassroom from_classroom) override;

  // stream
  void OnRemoteStreamsAdded(
      agora_refptr<IStreamInfoCollection> stream_info_collection,
      EduClassroom from_classroom) override;
  void OnRemoteStreamsRemoved(
      agora_refptr<IStreamEventCollection> stream_event_collection,
      EduClassroom from_classroom) override;
  void OnRemoteStreamUpdated(
      agora_refptr<IStreamEventCollection> stream_event_collection,
      EduClassroom from_classroom) override;

  // class room
  void OnClassroomStateUpdated(EduClassroomChangeType type,
                               EduUser operator_user,
                               EduClassroom from_classroom) override;
  void OnNetworkQualityChanged(NetworkQuality quality, EduUser user,
                               EduClassroom from_classroom) override;

  // property
  void OnClassroomPropertyUpdated(EduClassroom from_classroom,
                                  const char* cause) override;
  void OnRemoteUserPropertyUpdated(EduUser user, EduClassroom from_classroom,
                                   const char* cause) override;

  // connection
  void OnConnectionStateChanged(ConnectionState state,
                                EduClassroom from_classroom) override;
  void OnSyncProgress(uint8_t progress) override;

  // IEduUserEventHandler
  void OnLocalUserStateUpdated(EduUserEvent user_event,
                               EduUserStateChangeType type) override;
  void OnLocalUserPropertyUpdated(EduUser user, const char* cause) override;
  void OnLocalStreamChanged(EduStreamEvent stream_event,
                            MediaStreamState state) override;

  // IEduTeacherOperationEventHandler
  void OnLocalStreamCreated(EduStream stream_info, EduError err) override;
  void OnStreamPublished(EduStream stream_info, EduError err) override;
  void OnStreamUnpublished(EduStream stream_info, EduError err) override;

  void OnRemoteStreamSubscribed(EduStream stream_info, EduError err) override;
  void OnRemoteStreamUnsubscribed(EduStream stream_info, EduError err) override;

  void OnStreamMuted(EduStream stream_info, EduError err) override;
  void OnStreamUnmuted(EduStream stream_info, EduError err) override;

  void OnRoomCustomMessageSended(const char* text, EduError err) override;
  void OnUserCustomMessageSended(const char* text, EduUser remote_user,
                                 EduError err) override;
  void OnRoomChatMessageSended(const char* text, EduError err) override;
  void OnUserChatMessageSended(const char* text, EduUser remote_user,
                               EduError err) override;
  void OnSetRoomPropertyCompleted(Property property, const char* custom_cause,
                                  EduError err) override;
  void OnSetUserPropertyCompleted(Property property, const char* custom_cause,
                                  EduUser target_user, EduError err) override;

 private:
  const EduClassroomJoinOptions options_;
  const EduClassroomConfig config_;
  EduStream camera_stream_;

  EduBaseUser user_info_;

  std::shared_ptr<LoginWidgetManager> login_widget_manager_;
  agora_refptr<IEduClassroomManager> classroom_manager_;
  SettingConfig setting_config_;
  agora_refptr<IEduStudentService> user_service_;
  std::set<std::string> raise_stream_uuid_;

  std::unique_ptr<AgoraStudentWidget> student_screen_widget_;
};
