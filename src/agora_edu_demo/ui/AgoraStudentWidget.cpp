#include <QtPlatformHeaders/QWindowsWindowFunctions>

#include "AgoraStudentWidget.h"
#include "AgoraChatWidget.h"
#include "AgoraTipsDialog.h"

AgoraStudentWidget::AgoraStudentWidget(
    QString classroom_name, std::string user_name,
    StudentWidgetManager* student_widget_manager, QWidget* parent)
    : student_widget_manager_(student_widget_manager), QWidget(parent) {
  ui.setupUi(this);

  ui.ClassroomNameLabel->setText(classroom_name);
  setWindowFlags(Qt::FramelessWindowHint);

  video_widget_[0] = new AgoraVideoWidget(this);
  video_widget_[1] = new AgoraVideoWidget(this);

  video_list_widget_ = new AgoraVideoListWidget(this);

  chat_widget_.reset(new AgoraChatWidget);
  chat_widget_->SetUserName(user_name);

  if (control_widget_ == nullptr) {
    control_widget_ = new QWidget(this);
    control_widget_->setAutoFillBackground(true);
  }

  chat_pushbutton_ = new QPushButton(this);
  QObject::connect(chat_widget_.get(), SIGNAL(ChatWidgetExitSig(bool)),
                   chat_pushbutton_, SLOT(setChecked(bool)));

  LayoutVideoWidget(false);

  QObject::connect(chat_pushbutton_, SIGNAL(clicked()), this,
                   SLOT(OnChatPushButtonClicked()));
  QObject::connect(chat_widget_.get(), SIGNAL(ChatMessageSendSig(ChatMessage)),
                   student_widget_manager,
                   SLOT(OnChatMessageSend(ChatMessage)));
  QObject::connect(ui.ChangeSizePushButton, SIGNAL(clicked()), this,
                   SLOT(OnChangeSizePushButtonClicked()));
  QObject::connect(ui.ExitPushButton, SIGNAL(clicked()), this,
                   SLOT(OnExitPushButtonClicked()));
}

AgoraStudentWidget::~AgoraStudentWidget() {
  if (video_list_widget_) {
    delete video_list_widget_;
    video_list_widget_ = nullptr;
  }
}

void AgoraStudentWidget::customEvent(QEvent* event) {
  if (event->type() == AGORA_EVENT) {
    auto agora_event = static_cast<AgoraEvent*>(event);
    if (agora_event) {
      agora_event->RunTask();
    }
  }
}

void AgoraStudentWidget::closeEvent(QCloseEvent* event) {
  if (student_widget_manager_) {
    student_widget_manager_->LeaveClassroom();
  }
}

void AgoraStudentWidget::resizeEvent(QResizeEvent* event) {
  video_list_widget_->setMaximumHeight(frameGeometry().height() / 4);
  int video_height = ui.widget->frameGeometry().height() / 4;
  int video_width = video_height / 16 * 29;

  video_widget_[1]->setGeometry(
      ui.widget->frameGeometry().width() - video_width - 20,
      ui.widget->frameGeometry().y(), video_width, video_height);
  video_widget_[1]->raise();
}

void AgoraStudentWidget::mousePressEvent(QMouseEvent* event) {
  window_top_left_point_ = frameGeometry().topLeft();
  auto current_pos = event->globalPos();
  auto exit_button_pos =
      mapToGlobal(ui.ExitPushButton->geometry().bottomRight());
  auto rect = QRect(window_top_left_point_, exit_button_pos);

  if (event->button() == Qt::LeftButton && rect.contains(current_pos)) {
    is_drag_ = true;
    //获得鼠标的初始位置
    mouse_start_point_ = event->globalPos();
    setCursor(Qt::OpenHandCursor);
  }

  QWidget::mousePressEvent(event);
}

void AgoraStudentWidget ::mouseMoveEvent(QMouseEvent* event) {
  if (is_drag_) {
    //获得鼠标移动的距离
    QPoint distance = event->globalPos() - mouse_start_point_;
    //改变窗口的位置
    this->move(window_top_left_point_ + distance);
  }

  QWidget::mouseMoveEvent(event);
}

void AgoraStudentWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    is_drag_ = false;
    setCursor(Qt::ArrowCursor);
  }

  QWidget::mouseReleaseEvent(event);
}

void AgoraStudentWidget::AddStreamWidget(const EduStream& stream) {
  EduRenderConfig render_config;
  render_config.render_mode = EDU_RENDER_MODE_FIT;
  if (stream.user_info.role == EDU_ROLE_TYPE_TEACHER) {
    if (video_widget_[0]->GetEduStream() == nullptr) {
      SetWidgetView(video_widget_[0], stream);
    } else if (video_widget_[1]->GetEduStream() == nullptr) {
      SetWidgetView(video_widget_[1], stream);
    } else if (video_list_widget_->GetUuidVideoListWidgetsCount() > 0) {
      auto widget = video_list_widget_->GetUuidVideoListWidget(0);
      SetWidgetView(widget, stream);
    }
  } else if (stream.user_info.role == EDU_ROLE_TYPE_STUDENT) {
    if (auto mini_stream = video_widget_[1]->GetEduStream()) {
      auto info = video_list_widget_->AddVideoWidget(*mini_stream);
      render_config.custom_render = info.delegate_func;
      student_widget_manager_->SetWidgetView(*mini_stream, info.view,
                                             render_config);
      switch_stream = *mini_stream;
    } else {
      switch_stream = EduStream();
      video_list_widget_->AddVideoWidget(switch_stream);
    }

    SetWidgetView(video_widget_[1], stream);
    LayoutVideoWidget(true);
  }
}

void AgoraStudentWidget::RemoveStreamWidget(const EduStream& stream) {
  AgoraVideoWidget* widget = nullptr;
  if (stream.user_info.role == EDU_ROLE_TYPE_TEACHER) {
    if (video_widget_[0]->GetEduStream() &&
        strncmp(video_widget_[0]->GetEduStream()->stream_uuid,
                stream.stream_uuid, kMaxStreamUuidSize) == 0) {
      widget = video_widget_[0];
    } else if (video_widget_[1]->GetEduStream() &&
               strncmp(video_widget_[1]->GetEduStream()->stream_uuid,
                       stream.stream_uuid, kMaxStreamUuidSize) == 0) {
      widget = video_widget_[1];
    } else if (video_list_widget_->GetUuidVideoListWidgetsCount() > 0) {
      widget = video_list_widget_->GetUuidVideoListWidget(0);
    }
  } else if (stream.user_info.role == EDU_ROLE_TYPE_STUDENT) {
    LayoutVideoWidget(false);
    widget = video_list_widget_->RemoveVideoWidget(switch_stream.stream_uuid);

    if (!widget) {
      return;
    }
    auto teacher_stream = widget->GetEduStream();

    AgoraVideoWidget* recover_widget = nullptr;
    auto temp = video_widget_[0]->GetEduStream();
    if (temp && temp->user_info.role == EDU_ROLE_TYPE_STUDENT) {
      recover_widget = video_widget_[0];
    } else {
      recover_widget = video_widget_[1];
    }
    if (teacher_stream && *teacher_stream->stream_uuid) {
      SetWidgetView(recover_widget, *teacher_stream);
    } else {
      ResetStreamWidget(stream, recover_widget);
    }
    switch_stream = EduStream();
  }

  if (widget) {
    ResetStreamWidget(stream, widget);
    if (stream.user_info.role == EDU_ROLE_TYPE_STUDENT) {
      delete widget;
    }
  }
}

void AgoraStudentWidget::ResetStreamWidget(const agora::edu::EduStream& stream,
                                           AgoraVideoWidget* widget) {
  EduRenderConfig render_config;
  render_config.render_mode = EDU_RENDER_MODE_FIT;
  render_config.custom_render = nullptr;
  widget->SetEduStream(nullptr);
  student_widget_manager_->SetWidgetView(stream, (View*)widget->winId(),
                                         render_config);
}

void AgoraStudentWidget::ReceiveMessage(const std::string message,
                                        const EduBaseUser& user) {
  chat_widget_->AddMessage(user.user_name, QString::fromStdString(message));
}

void AgoraStudentWidget::SetChatButtonEnable(bool enable) {
  chat_widget_->SetLineEditEnable(enable);
}

void AgoraStudentWidget::showEvent(QShowEvent* event) {
  QWidget::showEvent(event);
  QWindowsWindowFunctions::setHasBorderInFullScreen(windowHandle(), true);
}

void AgoraStudentWidget::ShowScreen() {
  setWindowIcon(QIcon(":/image/resource/logo-dual-teacher@2x.png"));
  setWindowTitle("student main screen");
  resize(1366, 768);
  show();

  control_widget_->setLayout(new QHBoxLayout);
  chat_pushbutton_->setMaximumSize(38, 38);
  chat_pushbutton_->setMinimumSize(38, 38);
  chat_pushbutton_->setCheckable(true);
  chat_pushbutton_->setStyleSheet(STR(
	QPushButton{
    border-image : url(":/image/resource/icon-message@2x.png");
    background-color : rgba(255, 255, 255, 1);
	}
	QPushButton:hover{
    border-image : url(":/image/resource/icon-message-hover@2x.png");
    background-color : rgba(255, 255, 255, 1);
	}
    QPushButton:checked {
      border-image : url(":/image/resource/icon-message-active@2x.png");
      background-color : rgba(255, 255, 255, 1);
    }
    QPushButton:checked:hover {
      border-image : url(":/image/resource/icon-message active-hover@2x.png");
      background-color : rgba(255, 255, 255, 1);
    }));

  control_widget_->layout()->addWidget(chat_pushbutton_);
  control_widget_->layout()->setContentsMargins(6, 6, 6, 6);

  control_widget_->setStyleSheet("background-color:rgba(255,255,255,1);");
  control_widget_->move(20, 316);
  control_widget_->resize(50, 50);
  control_widget_->show();
}

void AgoraStudentWidget::HideScreen() {
  if (chat_widget_) {
    chat_widget_->hide();
  }

  hide();
}

void AgoraStudentWidget::LayoutVideoWidget(bool is_raise) {
  if (is_raise) {
    ui.gridLayout->addWidget(video_list_widget_, 0, 0, 1, 2);
    ui.gridLayout->addWidget(video_widget_[0], 1, 0, 3, 2);
    video_list_widget_->show();
  } else {
    ui.gridLayout->removeWidget(video_list_widget_);
    ui.gridLayout->addWidget(video_widget_[0], 0, 0);
    video_list_widget_->hide();
  }
}

void AgoraStudentWidget::SetWidgetView(AgoraVideoWidget* widget,
                                       const EduStream& stream_info) {
  EduRenderConfig render_config;
  render_config.render_mode = EDU_RENDER_MODE_FIT;

  render_config.custom_render = widget->GetReceiver();
  widget->SetEduStream(&stream_info);
  student_widget_manager_->SetWidgetView(stream_info, (View*)widget->winId(),
                                         render_config);
}

void AgoraStudentWidget::OnChatPushButtonClicked() {
  if (chat_pushbutton_->isChecked()) {
    chat_widget_->setGeometry(frameGeometry().x() + 85,
                              frameGeometry().y() + 357, 293, 390);
    chat_widget_->show();
  } else {
    chat_widget_->hide();
  }
}

void AgoraStudentWidget::OnChangeSizePushButtonClicked() {
  if (ui.ChangeSizePushButton->isChecked()) {
    showFullScreen();
  } else {
    showNormal();
  }
}

void AgoraStudentWidget::OnExitPushButtonClicked() {
  if (AgoraTipsDialog::ExecTipsDialog(QString::fromLocal8Bit("退出教室？")) ==
      QDialog::Accepted) {
    student_widget_manager_->LeaveClassroom();
  }
}
