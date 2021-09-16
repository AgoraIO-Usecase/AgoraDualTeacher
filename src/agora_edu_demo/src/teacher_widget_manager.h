#pragma once
#include <QObject>

#include <unordered_map>

#include "AgoraInnerChatContent.h"
#include "util.h"

#include "AgoraSettingDialog.h"
#include "IEduClassroomManager.h"
#include "IEduManager.h"
#include "IEduTeacherService.h"

using namespace agora::edu;

class LoginWidgetManager;
class AgoraTeacherWidget;
class TeacherWidgetManager
    : public QObject,
      public IEduClassroomEventHandler,
      public IEduUserEventHandler,
      public IEduTeacherOperationEventHandler,
      public std::enable_shared_from_this<TeacherWidgetManager> {
  Q_OBJECT

 public:
  TeacherWidgetManager(std::shared_ptr<LoginWidgetManager> login_widget_manager,
                       agora_refptr<IEduClassroomManager> classroom_manager,
                       const EduClassroomJoinOptions& options,
                       const EduClassroomConfig& config,
                       const SettingConfig& setting_config);
  ~TeacherWidgetManager();

  void JoinClassroom();
  void LeaveClassroom();

  void SubScribeStream(const EduStream& stream, EduSubscribeOptions options);
  void StartScreenShare(const EduShareScreenConfig& config);
  void StopScreenShare();
  void SetWidgetView(const EduStream& stream, View* view,
                     const EduRenderConfig& config) {
    user_service_->SetStreamView(stream, view, config);
  }
  void SetRaiseUserUuid(const std::string& user_uuid) {
    raise_user_uuid_ = user_uuid;
  }
  void SendRoomCustomMessage(QString str);
  void NotifyStudentWidgets();

 signals:
  void JoinClassroomSig(int);

 private slots:
  void OnMuteAllClassroomClicked();
  void OnChatMessageSend(ChatMessage);
  void OnMutePushButtonClicked();
  void OnRaisePushButtonClicked();
  void OnChatPushButtonClicked();
  void OnKickPushButtonClicked();

 protected:
  void customEvent(QEvent* event);

 private:
  void ShowTeacherWidget();

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
  void OnCourseStateUpdated(EduCourseState current_state,
                            EduError err) override;

  void OnAllStudentChaAllowed(bool current_enable, EduError err) override;
  void OnStudentChatAllowed(bool current_enable, EduError err) override;

  void OnCreateOrUpdateStudentStreamCompleted(EduStream stream,
                                              EduError err) override;

 private:
  const EduClassroomJoinOptions options_;
  const EduClassroomConfig config_;
  EduStream edu_master_stream_;
  EduStream edu_slave_stream;
  int cnt_screen_;
  EduBaseUser user_info_;
  std::string raise_user_uuid_;
  std::string ready_raise_user_uuid_;

  EduShareScreenConfig share_screen_config_;
  std::shared_ptr<LoginWidgetManager> login_widget_manager_;
  agora_refptr<IEduClassroomManager> classroom_manager_;
  agora_refptr<IEduTeacherService> user_service_;
  SettingConfig setting_config_;
  std::unique_ptr<AgoraTeacherWidget> teacher_screen_widget_;
};
