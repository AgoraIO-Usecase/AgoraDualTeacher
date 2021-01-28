#pragma once
#include <QObject>
#include <QtNetwork/QtNetwork>

#include "AgoraSettingDialog.h"
#include "IEduClassroomManager.h"
#include "IEduManager.h"

using namespace agora::edu;

class AgoraLoginWidget;
class TeacherWidgetManager;
class StudentWidgetManager;
class LoginWidgetManager
    : public QObject,
      public IEduManagerEventHandler,
      public std::enable_shared_from_this<LoginWidgetManager> {
  Q_OBJECT

 public:
  explicit LoginWidgetManager();
  ~LoginWidgetManager();

  void ShowLoginDialog();

  void SetSettingConfig(SettingConfig config);

  int InitializeEduManager();

  void CreateClassroomManager(const std::string& room_name,
                              const std::string& user_name,
                              const EduRoleType& role_type);

  void ExitToLoginWidget();
  void ExitLoginWidget();

 private:
  void CreateClassroom(const std::string& room_uuid,
                       const std::string& room_name);

  // IEduManagerEventHandler
  void OnInitializeSuccess() override;
  void OnInitializeFailed() override;

  void OnDebugItemUploadSuccess(const char* upload_serial_number) override;
  void OnDebugItemUploadFailure(EduError err) override;

  void OnPeerMessageReceived(agora_refptr<IAgoraEduMessage> text_message,
                             EduBaseUser from_user) override;
  void OnPeerCustomMessageReceived(agora_refptr<IAgoraEduMessage> text_message,
                                   EduBaseUser from_user) override;

  // 一期教育SDK没有这个方法，只是给娱乐使用
  void OnUserActionMessageReceived(EduActionMessage action_message) override {}

 signals:
  void InitializeResultSig(bool is_success);

 private slots:
  void replyFinish(QNetworkReply*);  //用于处理响应返回的数据
  void OnJoinClassRoomResult(int results);

 private:
  IEduManager* edu_manager_ = nullptr;
  std::unique_ptr<AgoraLoginWidget> login_widget_;
  SettingConfig setting_config_;
  EduClassroomConfig classroom_config_;
  EduClassroomJoinOptions options_;

  std::shared_ptr<TeacherWidgetManager> teacher_widget_manager_;
  std::shared_ptr<StudentWidgetManager> student_widget_manager_;
};