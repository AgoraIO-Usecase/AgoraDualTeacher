#include "util.h"

#include "login_widget_manager.h"
#include "student_widget_manager.h"
#include "teacher_widget_manager.h"
#include "ui/AgoraLoginWidget.h"
#include "AgoraTipsDialog.h"

LoginWidgetManager::LoginWidgetManager() {}

LoginWidgetManager::~LoginWidgetManager() { ExitLoginWidget(); }

void LoginWidgetManager::ShowLoginDialog() {
  if (strcmp(APP_ID, "") == 0) {
    AgoraTipsDialog::ExecTipsDialog(QString::fromLocal8Bit("请添加appid."));
    ExitProcess(1);
  }
  if (!login_widget_) {
    login_widget_.reset(new AgoraLoginWidget(shared_from_this()));
    QObject::connect(this, SIGNAL(InitializeResultSig(bool)),
                     login_widget_.get(), SLOT(OnInitializeResult(bool)));
    login_widget_->setWindowIcon(
        QIcon(":/image/resource/logo-dual-teacher@2x.png"));
    login_widget_->setWindowTitle("Login");
  }

  InitializeEduManager();

  login_widget_->show();
}

void LoginWidgetManager::SetSettingConfig(SettingConfig config) {
  setting_config_ = config;
}

int LoginWidgetManager::InitializeEduManager() {
  if (!edu_manager_) {
    edu_manager_ = createAgoraEduManager();
  }

  SetInitStreamUuidFlag(false);

  std::string rand_uuid;
  SET_RANDOM_STR(rand_uuid);

  EduConfiguration config;
  config.app_id = APP_ID;
  config.customer_id = CUSTOMER_ID;
  config.customer_certificate = CUSTOMER_CERTIFICATE;
  strncpy(config.user_uuid, rand_uuid.c_str(), kMaxUserUuidSize);

  auto res = edu_manager_->Initialize(config);
  if (res == ERR_OK) {
    edu_manager_->RegisterEventHandler(this);
  }

  return res;
}

void LoginWidgetManager::CreateClassroom(const std::string& room_uuid,
                                         const std::string& room_name) {
  QNetworkAccessManager* manager = new QNetworkAccessManager(this);
  QNetworkRequest request;

  QString url(QString("http://api.agora.io/scene/apps/%1/"
                      "v1/rooms/%2/config")
                  .arg(APP_ID, room_uuid.c_str()));

  request.setUrl(QUrl(url));
  request.setRawHeader("Content-Type", "application/json");
  QString raw_auth = CUSTOMER_ID;
  raw_auth += ":";
  raw_auth += CUSTOMER_CERTIFICATE;
  
  QString encoded_auth = "Basic ";
  encoded_auth += raw_auth.toLocal8Bit().toBase64();
  request.setRawHeader("Authorization", encoded_auth.toLocal8Bit());

  QString json_str =
      QString(
          R"({"roomName" : "%1","roleConfig" : {"host" : {"limit" : 1}, "audience" : {"limit" : 4}}})")
          .arg(room_name.c_str());

  QNetworkReply* reply = manager->post(request, json_str.toLocal8Bit());

  connect(manager, SIGNAL(finished(QNetworkReply*)), this,
          SLOT(replyFinish(QNetworkReply*)));
}

void LoginWidgetManager::replyFinish(QNetworkReply* reply) {
  QNetworkReply::NetworkError result = reply->error();
  QByteArray data = reply->readAll();
  QString s = QString::fromStdString(data.toStdString());
  if ((reply && result == QNetworkReply::NoError) ||
      (reply && result == QNetworkReply::ContentConflictError)) {
    if (s.contains(R"("code":0)") || s.contains(R"("msg":"Room conflict!")")) {
      auto classroom_manager =
          edu_manager_->CreateClassroomManager(classroom_config_);
      if (options_.role_type == EDU_ROLE_TYPE_TEACHER) {
        teacher_widget_manager_ = std::make_shared<TeacherWidgetManager>(
            shared_from_this(), classroom_manager, options_, classroom_config_,
            setting_config_);
        QObject::connect(teacher_widget_manager_.get(),
                         SIGNAL(JoinClassroomSig(int)), this,
                         SLOT(OnJoinClassRoomResult(int)));
        teacher_widget_manager_->JoinClassroom();
      } else if (options_.role_type == EDU_ROLE_TYPE_STUDENT) {
        student_widget_manager_ = std::make_shared<StudentWidgetManager>(
            shared_from_this(), classroom_manager, options_, classroom_config_,
            setting_config_);
        QObject::connect(student_widget_manager_.get(),
                         SIGNAL(JoinClassroomSig(int)), this,
                         SLOT(OnJoinClassRoomResult(int)));
        student_widget_manager_->JoinClassroom();
      }
      login_widget_->hide();
    }
  }

  reply->close();
}

void LoginWidgetManager::OnJoinClassRoomResult(int results) {
  if (results == CONNECTION_STATE_FULL_ROLE_ABORTED) {
    login_widget_->SetTipsLabelContent(QString::fromLocal8Bit(STR(
        <h4><font color = red> 房间该角色人数已满，无法加入！</ font></ h4>)));
  }
}

void LoginWidgetManager::CreateClassroomManager(const std::string& room_name,
                                                const std::string& user_name,
                                                const EduRoleType& role_type) {
  std::string room_uuid = "room_uuid" + room_name;
  strncpy(classroom_config_.room_uuid, room_uuid.c_str(), kMaxRoomUuidSize);
  strncpy(classroom_config_.room_name, room_name.c_str(), kMaxRoomUuidSize);
  classroom_config_.class_type = EDU_CLASSROOM_TYPE_SMALL;

  strncpy(options_.user_name, user_name.c_str(), kMaxUserUuidSize);
  options_.role_type = role_type;
  options_.custom_render = true;

  CreateClassroom(room_uuid, room_name);
}

void LoginWidgetManager::ExitToLoginWidget() {
  if (teacher_widget_manager_) {
    teacher_widget_manager_.reset();
  }

  if (student_widget_manager_) {
    student_widget_manager_.reset();
  }

  ShowLoginDialog();
}

void LoginWidgetManager::ExitLoginWidget() {
  if (teacher_widget_manager_) {
    teacher_widget_manager_.reset();
  }

  if (student_widget_manager_) {
    student_widget_manager_.reset();
  }

  login_widget_.reset();

  if (edu_manager_) {
    edu_manager_->Release();
    edu_manager_ = nullptr;
  }
}

void LoginWidgetManager::OnInitializeSuccess() {
  emit InitializeResultSig(true);
}
void LoginWidgetManager::OnInitializeFailed() {
  emit InitializeResultSig(false);
}

void LoginWidgetManager::OnDebugItemUploadSuccess(
    const char* upload_serial_number) {}

void LoginWidgetManager::OnDebugItemUploadFailure(EduError err) {}

void LoginWidgetManager::OnPeerMessageReceived(
    agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user) {}

void LoginWidgetManager::OnPeerCustomMessageReceived(
    agora_refptr<IAgoraEduMessage> text_message, EduBaseUser from_user) {
  auto msg = text_message->GetEduMessage();
  QByteArray json_byte_array(msg, strlen(msg));

  QJsonParseError err;
  auto doc = QJsonDocument::fromJson(json_byte_array, &err);

  if (doc.isNull() || (err.error != QJsonParseError::NoError)) {
    return;
  }

  if (!doc.isObject()) {
    return;
  }

  QJsonObject object = doc.object();
  if (!object.contains("type")) {
    return;
  }

  QJsonValue value = object.value("type");
  if (!value.isDouble()) {
    return;
  }
  switch (value.toInt()) {
    case KICK_MSG:
      if (student_widget_manager_) {
        AgoraEvent::PostAgoraEvent(
            new AgoraEvent, student_widget_manager_.get(),
            [=] { student_widget_manager_->LeaveClassroom(); });
      }
    case ROOM_ENTER_READY:
      if (teacher_widget_manager_) {
        AgoraEvent::PostAgoraEvent(
            new AgoraEvent, teacher_widget_manager_.get(),
            [=] { teacher_widget_manager_->NotifyStudentWidgets(); });
      }
    default:
      break;
  };
}
