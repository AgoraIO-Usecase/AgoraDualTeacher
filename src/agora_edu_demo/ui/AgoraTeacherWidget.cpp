#include <QDesktopWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVBoxLayout>
#include <QtPlatformHeaders/QWindowsWindowFunctions>

#include "AgoraTeacherWidget.h"
#include "util.h"

#include "AgoraChatWidget.h"
#include "AgoraClassroomListWidget.h"
#include "AgoraSelectShareDlg.h"
#include "AgoraTipsDialog.h"
#include "AgoraVideoListWidget.h"
#include "AgoraVideoWidget.h"
#include "teacher_widget_manager.h"

AgoraTeacherWidget::AgoraTeacherWidget(
    QString classroom_name, std::string user_name,
    TeacherWidgetManager* teacher_widget_manager, QWidget* parent)
    : teacher_widget_manager_(teacher_widget_manager), QWidget(parent) {
  main_ui_.setupUi(this);
  main_ui_.ClassroomNameLabel->setText(classroom_name);
  subscribe_big_stream_ = true;
  raise_ = false;
  CheckNeedDualScreen();
  setMouseTracking(true);
  setWindowFlags(Qt::FramelessWindowHint);

  chat_widget_.reset(new AgoraChatWidget);
  chat_widget_->SetUserName(user_name);

  QGridLayout* layout = new QGridLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  main_ui_.widget->setLayout(layout);

  if (display_mode_ == DUAL_SCREEN) {
    video_widget_[0] = new AgoraVideoWidget(this);
    video_widget_[1] = new AgoraVideoWidget(this);
    main_ui_.widget->layout()->addWidget(video_widget_[0]);
    class_pushbutton_ = new QPushButton(video_widget_[0]);
    classroom_list_widget_ =
        new AgoraClassroomListWidget(teacher_widget_manager_, video_widget_[0]);
  } else {
    video_widget_[0] = new AgoraVideoWidget(this);
    video_widget_[1] = new AgoraVideoWidget(this);
    video_list_widget_ = new AgoraVideoListWidget(this);
    class_pushbutton_ = new QPushButton(this);
    classroom_list_widget_ =
        new AgoraClassroomListWidget(teacher_widget_manager_, this);
    LayoutClassroomVideoWidget(0);
  }
  map_teacher_video_widgets_[0] = video_widget_[0];
  map_teacher_video_widgets_[1] = video_widget_[1];

  video_widget_list_.push_back(video_widget_[0]);
  video_widget_list_.push_back(video_widget_[1]);

  classroom_list_widget_->setStyleSheet(
      "background-color : rgba(255, 255, 255, 0.7); border-radius:6px;");
  class_pushbutton_->setStyleSheet(STR(
	  QPushButton{
		   border-image: url(":/image/resource/icon-classroom@2x.png");
           border-radius:6px;
		   background-color:rgba(255,255,255, 0.7);
	  }
  QPushButton:hover{
	  border-image: url(":/image/resource/icon-classroom-hover@2x.png");
	  border-radius:6px;
	  background-color:rgba(255,255,255, 0.7);
	  }));

  QObject::connect(main_ui_.ExitPushButton, SIGNAL(clicked()), this,
                   SLOT(OnExitPushButtonClicked()));
  QObject::connect(class_pushbutton_, SIGNAL(clicked()), this,
                   SLOT(OnClassPushButtonClicked()));
  QObject::connect(chat_widget_.get(), SIGNAL(ChatMessageSendSig(ChatMessage)),
                   teacher_widget_manager_,
                   SLOT(OnChatMessageSend(ChatMessage)));
  QObject::connect(main_ui_.ChangeSizePushButton, SIGNAL(clicked()), this,
                   SLOT(OnChangeSizePushButtonClicked()));
  QObject::connect(video_widget_[0], SIGNAL(touchedSig()), this,
                   SLOT(OnOpenGLWidgetTouched()));
  QObject::connect(video_widget_[1], SIGNAL(touchedSig()), this,
                   SLOT(OnOpenGLWidgetTouched()));
}

AgoraTeacherWidget::~AgoraTeacherWidget() {
  if (video_list_widget_) {
    delete video_list_widget_;
    video_list_widget_ = nullptr;
  }
}

void AgoraTeacherWidget::customEvent(QEvent* event) {
  if (event->type() == AGORA_EVENT) {
    auto agora_event = static_cast<AgoraEvent*>(event);
    if (agora_event) {
      agora_event->RunTask();
    }
  }
}

void AgoraTeacherWidget::closeEvent(QCloseEvent* event) {
  if (teacher_widget_manager_) {
    teacher_widget_manager_->LeaveClassroom();
  }
}

void AgoraTeacherWidget::resizeEvent(QResizeEvent* event) {
  class_pushbutton_->move(frameGeometry().width() - 38 - 20, 368);
  classroom_list_widget_->move(
      frameGeometry().width() - 310,
      main_ui_.widget->frameGeometry().y() + 20 + 160 + 20);
  if (display_mode_ == DUAL_SCREEN) {
    video_widget_[1]->move(frameGeometry().width() - 310,
                           main_ui_.widget->frameGeometry().y() + 20);
  }
  if (video_list_widget_) {
    video_list_widget_->setMaximumHeight(frameGeometry().height() / 4);
  }
}

void AgoraTeacherWidget::RemoteClassroomJoin(
    agora_refptr<IUserInfoCollection> user_info_collection) {
  for (size_t i = 0; i < user_info_collection->NumberOfUserInfo(); ++i) {
    EduUser user_info;
    user_info_collection->GetUserInfo(i, user_info);
    classroom_list_widget_->AddClassroomWidget(user_info);
    map_uuid_streamid_[user_info.user_uuid] = "";
    map_student_uuid_video_widgets_[user_info.user_uuid] = nullptr;
    AddClassroomVideoWidget(user_info.user_uuid);
  }
}

void AgoraTeacherWidget::RemoteClassroomLeave(
    agora_refptr<IUserEventCollection> user_event_collection) {
  for (size_t i = 0; i < user_event_collection->NumberOfUserEvent(); ++i) {
    EduUserEvent user_event;
    user_event_collection->GetUserEvent(i, user_event);
    if (raise_ &&
        strcmp(user_event.modified_user.user_uuid, raise_user_uuid_.c_str()) == 0) {
      UpdateVideoWidgetUi(raise_user_uuid_, false);
    }
    classroom_list_widget_->RemoveClassroomWidget(
        user_event.modified_user.user_uuid);
    RemoveClassroomVideoWidget(user_event.modified_user.user_uuid);
    map_student_uuid_video_widgets_[user_event.modified_user.user_uuid] =
        nullptr;
  }
}

void AgoraTeacherWidget::UpdateVideoWidgetUi(std::string user_uuid,
                                             bool is_enabled) {
  if (raise_ == is_enabled) return;
  raise_ = is_enabled;
  if (is_enabled) {
    map_teacher_video_widgets_[0] = video_widget_[0];
    map_teacher_video_widgets_[1] = video_widget_[1];
    raise_user_uuid_ = user_uuid;
  }
  SwapAgoraVideoWidget(map_teacher_video_widgets_[1],
                       map_student_uuid_video_widgets_[user_uuid]);
  auto temp = map_teacher_video_widgets_[1];
  map_teacher_video_widgets_[1] = map_student_uuid_video_widgets_[user_uuid];
  map_student_uuid_video_widgets_[user_uuid] = temp;
}

void AgoraTeacherWidget::UpdateClassroomStateUi(std::string user_uuid,
                                                ClassroomStateChangeType type,
                                                bool is_enable) {
  if (type == CLASSROOM_STATE_CHANGE_TYPE_MUTE) {
    classroom_list_widget_->MuteClassroomAudio(user_uuid, is_enable);
  } else if (type == CLASSROOM_STATE_CHANGE_TYPE_CHAT) {
    classroom_list_widget_->MuteClassroomChat(user_uuid, is_enable);
  } else if (type == CLASSROOM_STATE_CHANGE_TYPE_RAISE) {
    classroom_list_widget_->RaiseClassroom(user_uuid, is_enable);

    RaiseClassroom(user_uuid, is_enable);
  }
}

void AgoraTeacherWidget::SetStreamUUidWithUserUUid(
    const std::string& user_uuid, const std::string& stream_uuid) {
  map_uuid_streamid_[user_uuid] = stream_uuid;
  map_streamid_uuid_[stream_uuid] = user_uuid;
}

void AgoraTeacherWidget::RaiseClassroom(std::string user_uuid, bool is_enable) {
  UpdateVideoWidgetUi(user_uuid, is_enable);
}

void AgoraTeacherWidget::AddFreeLocalStream(const EduStream& stream_info) {
  EduRenderConfig render_config;
  render_config.render_mode = EDU_RENDER_MODE_FIT;
  if (stream_info.user_info.role == EDU_ROLE_TYPE_TEACHER) {
    if (map_teacher_video_widgets_[0]->GetEduStream() == nullptr) {
      render_config.custom_render =
          map_teacher_video_widgets_[0]->GetReceiver();
      map_teacher_video_widgets_[0]->SetEduStream(&stream_info);
      teacher_widget_manager_->SetWidgetView(
          stream_info, (View*)map_teacher_video_widgets_[0]->winId(),
          render_config);
    } else if (map_teacher_video_widgets_[1]->GetEduStream() == nullptr) {
      render_config.custom_render =
          map_teacher_video_widgets_[1]->GetReceiver();
      map_teacher_video_widgets_[1]->SetEduStream(&stream_info);
      teacher_widget_manager_->SetWidgetView(
          stream_info, (View*)map_teacher_video_widgets_[1]->winId(),
          render_config);
    }
  }
  video_widget_[1]->raise();
}

void AgoraTeacherWidget::AddSpecifiedLocalStream(const size_t& index,
                                                 const EduStream& stream_info) {
  EduRenderConfig render_config;
  render_config.render_mode = EDU_RENDER_MODE_FIT;
  render_config.custom_render = video_widget_[index]->GetReceiver();
  video_widget_[index]->SetEduStream(&stream_info);
  teacher_widget_manager_->SetWidgetView(
      stream_info, (View*)video_widget_[index]->winId(), render_config);
  video_widget_[1]->raise();
}

void AgoraTeacherWidget::SetVideoWidget(const EduStream& stream,
                                        AgoraVideoWidget* widget) {
  if (!widget) return;
  if (stream.stream_uuid[0] == 0) return;
  EduRenderConfig render_config;
  render_config.render_mode = EDU_RENDER_MODE_FIT;
  render_config.custom_render = widget->GetReceiver();
  widget->SetEduStream(&stream);
  teacher_widget_manager_->SetWidgetView(stream, (View*)widget->winId(),
                                         render_config);
  video_widget_[1]->raise();
}

void AgoraTeacherWidget::RemoveLocalStream(const EduStream& stream_info) {
  if (auto stream0 = video_widget_[0]->GetEduStream()) {
    if (strncmp(stream0->stream_uuid, stream_info.stream_uuid,
                kMaxStreamUuidSize) == 0) {
      ResetStreamWidget(stream_info, video_widget_[0]);
      return;
    }
  }

  if (auto stream1 = map_teacher_video_widgets_[1]->GetEduStream()) {
    if (strncmp(stream1->stream_uuid, stream_info.stream_uuid,
                kMaxStreamUuidSize) == 0) {
      ResetStreamWidget(stream_info, map_teacher_video_widgets_[1]);
      return;
    }
  }
}

void AgoraTeacherWidget::AddRemoteStream(const EduStream& stream_info) {
  auto find_it = std::find_if(
      classroom_video_widgets_.begin(), classroom_video_widgets_.end(),
      [&](const UuidWidgetPair& widget_pair) {
        return widget_pair.first.compare(stream_info.user_info.user_uuid) == 0;
      });

  if (find_it == classroom_video_widgets_.end()) {
    return;
  }

  EduRenderConfig render_config;
  render_config.render_mode = EDU_RENDER_MODE_FIT;
  render_config.custom_render = find_it->second->GetReceiver();
  find_it->second->SetEduStream(&stream_info);

  teacher_widget_manager_->SetWidgetView(
      stream_info, (View*)find_it->second->winId(), render_config);
  AutoSubscribeDualStream();
}

void AgoraTeacherWidget::RemoveRemoteStream(const EduStream& stream_info) {
  auto find_it = std::find_if(
      classroom_video_widgets_.begin(), classroom_video_widgets_.end(),
      [&](const UuidWidgetPair& widget_pair) {
        return widget_pair.first.compare(stream_info.user_info.user_uuid) == 0;
      });

  if (find_it == classroom_video_widgets_.end()) {
    return;
  }
  ResetStreamWidget(stream_info, find_it->second);
  AutoSubscribeDualStream();
}

void AgoraTeacherWidget::AutoSubscribeDualStream() {
  int count_streams = 0;
  for (auto& video_widget : video_widget_)
    if (video_widget->GetEduStream()) ++count_streams;
  EduSubscribeOptions options;
  if (count_streams <= 2 && !subscribe_big_stream_) {
    for (auto& video_widget : video_widget_) {
      auto stream = video_widget->GetEduStream();
      options.video_stream_type = EDU_VIDEO_STREAM_TYPE_HIGH;
      if (stream) {
        teacher_widget_manager_->SubScribeStream(*stream, options);
      }
    }
  }

  if (count_streams > 2 && subscribe_big_stream_) {
    for (auto& video_widget : video_widget_) {
      auto stream = video_widget->GetEduStream();
      options.video_stream_type = EDU_VIDEO_STREAM_TYPE_LOW;
      if (stream) {
        teacher_widget_manager_->SubScribeStream(*stream, options);
      }
    }
  }
}

void AgoraTeacherWidget::ResetStreamWidget(const agora::edu::EduStream& stream,
                                           AgoraVideoWidget* widget) {
  EduRenderConfig render_config;
  render_config.render_mode = EDU_RENDER_MODE_FIT;
  render_config.custom_render = nullptr;
  widget->SetEduStream(nullptr);
  teacher_widget_manager_->SetWidgetView(stream, (View*)widget->winId(),
                                         render_config);
}

void AgoraTeacherWidget::ReceiveMessage(const std::string message,
                                        const EduBaseUser& user) {
  chat_widget_->AddMessage(user.user_name, QString::fromStdString(message));
}

void AgoraTeacherWidget::ShowScreen(bool enable_share_screen) {
  setWindowIcon(QIcon(":/image/resource/logo-dual-teacher@2x.png"));
  setWindowTitle("teacher main screen");
  resize(1366, 768);
  show();

  if (extend_widget_) {
    extend_widget_->setWindowTitle("extend Screen");
    extend_widget_->showFullScreen();
  }

  class_pushbutton_->move(frameGeometry().width() - 38 - 20, 368);
  class_pushbutton_->resize(38, 38);
  class_pushbutton_->setContentsMargins(6, 6, 6, 6);

  video_widget_[1]->setGeometry(frameGeometry().width() - 310,
                                main_ui_.widget->frameGeometry().y() + 20, 290,
                                160);
  video_widget_[1]->raise();
  video_widget_[1]->show();

  classroom_list_widget_->move(
      frameGeometry().width() - 310,
      main_ui_.widget->frameGeometry().y() + 20 + 160 + 20);
  classroom_list_widget_->resize(290, 300);

  ShowControlWidget(enable_share_screen);
}

void AgoraTeacherWidget::HideScreen() {
  if (extend_widget_) {
    extend_widget_->hide();
  }

  if (chat_widget_) {
    chat_widget_->hide();
  }

  hide();
}

void AgoraTeacherWidget::EnableClassroomListRaisePushbutton(bool enable) {
  classroom_list_widget_->EnablePushButton(
      AgoraClassroomListWidget::RAISE_PUSHBUTTON, enable);
}

std::vector<AgoraVideoWidget*> AgoraTeacherWidget::GetVideoWidgetList() {
  video_widget_list_[0] = video_widget_[0];
  video_widget_list_[1] = video_widget_[1];
  return video_widget_list_;
}

void AgoraTeacherWidget::SwapAgoraVideoWidget(AgoraVideoWidget* widget_1,
                                              AgoraVideoWidget* widget_2) {
  if (!widget_1 || !widget_2) return;
  auto edu_stream0 = widget_1->GetEduStream();
  auto edu_stream1 = widget_2->GetEduStream();

  EduStream copy_stream0;
  if (edu_stream0) {
    copy_stream0 = EduStream(*edu_stream0);
  }
  EduStream copy_stream1;
  if (edu_stream1) {
    copy_stream1 = EduStream(*edu_stream1);
  }
  ResetStreamWidget(copy_stream0, widget_1);
  ResetStreamWidget(copy_stream1, widget_2);
  SetVideoWidget(copy_stream0, widget_2);
  SetVideoWidget(copy_stream1, widget_1);
}

void AgoraTeacherWidget::mousePressEvent(QMouseEvent* event) {
  window_top_left_point_ = frameGeometry().topLeft();
  auto current_pos = event->globalPos();
  auto exit_button_pos =
      mapToGlobal(main_ui_.ExitPushButton->geometry().bottomRight());
  auto rect = QRect(window_top_left_point_, exit_button_pos);

  if (event->button() == Qt::LeftButton && rect.contains(current_pos)) {
    is_drag_ = true;
    mouse_start_point_ = event->globalPos();
    setCursor(Qt::OpenHandCursor);
  }

  QWidget::mousePressEvent(event);
}

void AgoraTeacherWidget ::mouseMoveEvent(QMouseEvent* event) {
  if (is_drag_) {
    QPoint distance = event->globalPos() - mouse_start_point_;
    this->move(window_top_left_point_ + distance);
  }

  QWidget::mouseMoveEvent(event);
}

void AgoraTeacherWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    is_drag_ = false;
    setCursor(Qt::ArrowCursor);
  }

  QWidget::mouseReleaseEvent(event);
}

void AgoraTeacherWidget::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  QWindowsWindowFunctions ::setHasBorderInFullScreen(windowHandle(),true);
}

bool AgoraTeacherWidget::CheckNeedDualScreen() {
  auto desk_widget = QApplication::desktop();
  int screen_count = desk_widget->screenCount();
  if (screen_count >= 2) {
    if (extend_widget_ == nullptr) {
      extend_widget_.reset(new QWidget);
      extend_widget_->setLayout(new QGridLayout);
      extend_widget_->layout()->setContentsMargins(0, 0, 0, 0);
    }

    int cur = desk_widget->screenNumber(this);
    extend_widget_->setGeometry(
        desk_widget->screenGeometry(screen_count - cur - 1));
    extend_widget_->setWindowFlags(Qt::FramelessWindowHint);
    display_mode_ = DUAL_SCREEN;
    return true;
  } else {
    display_mode_ = SINGLE_SCREEN;
    return false;
  }
}

bool AgoraTeacherWidget::AddClassroomVideoWidget(const std::string& user_uuid) {
  size_t count = classroom_video_widgets_.size();
  if (count >= 4) {
    return false;
  }
  map_student_uuid_video_widgets_[user_uuid] = new AgoraVideoWidget(this);
  classroom_video_widgets_.push_back(
      {user_uuid, map_student_uuid_video_widgets_[user_uuid]});
  LayoutClassroomVideoWidget(++count);

  if (display_mode_ == SINGLE_SCREEN) {
    for (size_t i = 0; i < count; ++i) {
      video_list_widget_->AddVideoWidgetDirectly(
          classroom_video_widgets_[i].second);
    }
  }
  return true;
}

bool AgoraTeacherWidget::RemoveClassroomVideoWidget(
    const std::string& user_uuid) {
  size_t count = classroom_video_widgets_.size();
  if (count == 0) {
    return false;
  }

  QGridLayout* layout = nullptr;

  if (display_mode_ == DUAL_SCREEN) {
    layout = static_cast<QGridLayout*>(extend_widget_->layout());
  } else {
    layout = static_cast<QGridLayout*>(main_ui_.widget->layout());
  }

  auto find_it = std::find_if(
      classroom_video_widgets_.begin(), classroom_video_widgets_.end(),
      [&](const UuidWidgetPair& widget_pair) {
        return widget_pair.first.compare(user_uuid) == 0;
      });

  if (find_it != classroom_video_widgets_.end()) {
    if (auto edu_stream = find_it->second->GetEduStream()) {
      RemoveRemoteStream(*edu_stream);
      layout->removeWidget(find_it->second);
      delete find_it->second;
      classroom_video_widgets_.erase(find_it);
      LayoutClassroomVideoWidget(--count);
    }

    return true;
  }

  return false;
}

void AgoraTeacherWidget::LayoutClassroomVideoWidget(const size_t& video_count) {
  if (display_mode_ == DUAL_SCREEN) {
    if (!extend_widget_) {
      return;
    }
    auto layout = static_cast<QGridLayout*>(extend_widget_->layout());

    int rowspan = 1;
    int columnspan = 1;
    switch (video_count) {
      case 1: {
        rowspan *= 2;
        columnspan *= 2;
      } break;
      case 2: {
        rowspan *= 2;
      }
      default:
        break;
    }

    for (size_t i = 0; i < video_count; ++i) {
      int row = i / 2;
      int column = i % 2;
      layout->addWidget(classroom_video_widgets_[i].second, row, column,
                        rowspan, columnspan);
    }
  } else if (display_mode_ == SINGLE_SCREEN) {
    if (!video_list_widget_) {
      return;
    }
    auto layout = static_cast<QGridLayout*>(main_ui_.widget->layout());
    if (video_count > 0) {
      layout->addWidget(video_list_widget_, 0, 0, 1, 2);
      layout->addWidget(video_widget_[0], 1, 0, 3, 1);
      layout->addWidget(video_widget_[1], 1, 1, 3, 1);
      video_list_widget_->show();
    } else {
      layout->removeWidget(video_list_widget_);
      video_list_widget_->hide();
      layout->addWidget(video_widget_[0], 0, 0);
      layout->addWidget(video_widget_[1], 0, 1);
    }
  }
}

void AgoraTeacherWidget::ShowControlWidget(bool enable_share_screen) {
  if (control_widget_ == nullptr) {
    control_widget_ = new QWidget(video_widget_[0]);
    share_screen_pushbutton_ = new QPushButton(control_widget_);
    share_screen_pushbutton_->setMaximumSize(38, 38);
    share_screen_pushbutton_->setMinimumSize(38, 38);
    share_screen_pushbutton_->setCheckable(true);

    share_screen_pushbutton_->setStyleSheet(
		STR(
        QPushButton{
        border-image : url(":/image/resource/icon-share@2x.png");
	    border-radius:6px;
        background-color : rgba(255, 255, 255, 0.7);
        }
        QPushButton:hover{
        border-image : url(":/image/resource/icon-share-hover@2x.png");
		border-radius:6px;
        background-color : rgba(255, 255, 255, 0.7);
	    }
        QPushButton:checked {
        border-image : url(":/image/resource/icon-share-active@2x.png");
		border-radius:6px;
        background-color : rgba(255, 255, 255, 0.7);
        }
	  QPushButton:checked:hover {
        border-image : url(":/image/resource/icon-share-active-hover@2x.png");
		border-radius:6px;
        background-color : rgba(255, 255, 255, 0.7);
      }
	  ));

    share_screen_pushbutton_->setEnabled(enable_share_screen);

    chat_pushbutton_ = new QPushButton(control_widget_);
    chat_pushbutton_->setMaximumSize(38, 38);
    chat_pushbutton_->setMinimumSize(38, 38);
    chat_pushbutton_->setCheckable(true);

    chat_pushbutton_->setStyleSheet(
        STR(
		QPushButton{
		  border-image: url(":/image/resource/icon-message@2x.png");
		  border-radius:6px;
		  background-color : rgba(255, 255, 255, 0.7);
		}
		QPushButton:hover{
		  border-image: url(":/image/resource/icon-message-hover@2x.png");
		  border-radius:6px;
		  background-color : rgba(255, 255, 255, 0.7);
		}
		QPushButton:checked{
		  border-image: url(":/image/resource/icon-message-active@2x.png");
		  border-radius:6px;
		  background-color : rgba(255, 255, 255, 0.7);
		}
		QPushButton:checked:hover{
		  border-image: url(":/image/resource/icon-message active-hover@2x.png");
		  border-radius:6px;
		  background-color : rgba(255, 255, 255, 0.7);
		}));

    control_widget_->move(20, 316);
    control_widget_->resize(50, 90);
    control_widget_->setContentsMargins(6, 6, 6, 6);

    QVBoxLayout* v_layout = new QVBoxLayout(this);
    v_layout->addWidget(share_screen_pushbutton_);
    v_layout->addWidget(chat_pushbutton_);
    v_layout->setAlignment(Qt::AlignCenter);
    v_layout->setContentsMargins(0, 0, 0, 0);

    control_widget_->setLayout(v_layout);
    QObject::connect(share_screen_pushbutton_, SIGNAL(clicked()), this,
                     SLOT(OnShareScreenPushButtonClicked()));
    QObject::connect(chat_pushbutton_, SIGNAL(clicked()), this,
                     SLOT(OnChatPushButtonClicked()));
    QObject::connect(chat_widget_.get(), SIGNAL(ChatWidgetExitSig(bool)),
                     chat_pushbutton_, SLOT(setChecked(bool)));
  }

  control_widget_->setStyleSheet("background-color:rgba(255,255,255,0.7);");
  control_widget_->show();
}

void AgoraTeacherWidget::OnClassPushButtonClicked() {
  classroom_list_widget_->show();
}

void AgoraTeacherWidget::OnChangeSizePushButtonClicked() {
  if (main_ui_.ChangeSizePushButton->isChecked()) {
    showFullScreen();
  } else {
    showNormal();
  }
}

void AgoraTeacherWidget::OnExitPushButtonClicked() {
  if (AgoraTipsDialog::ExecTipsDialog(QString::fromLocal8Bit("ÍË³ö½ÌÊÒ£¿")) ==
      QDialog::Accepted) {
    teacher_widget_manager_->LeaveClassroom();
  }
}

void AgoraTeacherWidget::OnChatPushButtonClicked() {
  if (chat_pushbutton_->isChecked()) {
    chat_widget_->setGeometry(frameGeometry().x() + 85,
                              frameGeometry().y() + 357, 293, 390);
    chat_widget_->show();
  } else {
    chat_widget_->hide();
  }
}

void AgoraTeacherWidget::OnShareScreenPushButtonClicked() {
  if (share_screen_pushbutton_->isChecked()) {
    AgoraSelectShareDlg dlg;
    dlg.getAvailableScreenId();
    static bool bShareScreen = false;
    if (QDialog::Accepted == dlg.exec() && dlg.bCapture) {
      bShareScreen = true;
      CaptureInfo captureinfo = dlg.selectedInfo;
      EduShareScreenConfig share_config;
      if (captureinfo.isDesktop) {
        share_config.enableRect = false;
        share_config.hwnd = GetDesktopWindow();
      } else {
        RECT rc;
        rc.left = 0;
        rc.top = 0;
        rc.right = captureinfo.width;
        rc.bottom = captureinfo.height;
        share_config.hwnd = captureinfo.hwnd;
      }

      teacher_widget_manager_->StartScreenShare(share_config);

    } else {
      share_screen_pushbutton_->setChecked(false);
    }
  } else {
    teacher_widget_manager_->StopScreenShare();
    share_screen_pushbutton_->setChecked(false);
  }
}

void AgoraTeacherWidget::OnOpenGLWidgetTouched() {
  std::lock_guard<std::mutex> _(mutex_);
  auto edu_stream0 = video_widget_[0]->GetEduStream();
  auto edu_stream1 = video_widget_[1]->GetEduStream();

  EduStream copy_stream0;
  if (edu_stream0) {
    copy_stream0 = EduStream(*edu_stream0);
  }
  EduStream copy_stream1;
  if (edu_stream1) {
    copy_stream1 = EduStream(*edu_stream1);
  }
  if (raise_) {
    if (map_teacher_video_widgets_[0] == video_widget_[0]) {
      map_teacher_video_widgets_[0] = video_widget_[1];
      map_student_uuid_video_widgets_[raise_user_uuid_] = video_widget_[0];
    } else {
      map_teacher_video_widgets_[0] = video_widget_[0];
      map_student_uuid_video_widgets_[raise_user_uuid_] = video_widget_[1];
    }
  }
  ResetStreamWidget(copy_stream0, video_widget_[0]);
  ResetStreamWidget(copy_stream1, video_widget_[1]);
  if (edu_stream0) {
    AddSpecifiedLocalStream(1, copy_stream0);
  }
  if (edu_stream1) {
    AddSpecifiedLocalStream(0, copy_stream1);
  }
  QJsonObject obj;
  obj.insert("type", SWITCH_WIDGET);
  obj.insert("widget0", copy_stream1.stream_uuid);
  obj.insert("widget1", copy_stream0.stream_uuid);

  QJsonDocument doc;
  doc.setObject(obj);
  teacher_widget_manager_->SendRoomCustomMessage(
      doc.toJson(QJsonDocument::Compact));
}
