#include <QGridLayout>

#include "AgoraClassroomListWidget.h"
#include "AgoraClassroomWidget.h"
#include "teacher_widget_manager.h"

AgoraClassroomListWidget::AgoraClassroomListWidget(
    TeacherWidgetManager* teacher_widget_manager, QWidget* parent)
    : teacher_widget_manager_(teacher_widget_manager), QWidget(parent) {
  ui.setupUi(this);
  setAttribute(Qt::WA_DeleteOnClose);
  setAutoFillBackground(true);
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

  auto layout = new QVBoxLayout(this);
  layout->setAlignment(Qt::AlignTop);
  layout->setSpacing(4);
  layout->setContentsMargins(0, 0, 0, 0);
  ui.ClassroomListScrollArea->widget()->setLayout(layout);

  QObject::connect(ui.ExitPushButton, SIGNAL(clicked()), this, SLOT(hide()));
  QObject::connect(ui.MuteAllClassroomPushButton, SIGNAL(clicked()),
                   teacher_widget_manager_, SLOT(OnMuteAllClassroomClicked()));
}

bool AgoraClassroomListWidget::AddClassroomWidget(const EduUser& user_info) {
  if (classroom_widgets_map_.size() >= 4) {
    return false;
  }

  auto classroom_widget = new AgoraClassroomWidget(user_info);
  QObject::connect(classroom_widget->ui.MutePushButton, SIGNAL(clicked()),
                   teacher_widget_manager_, SLOT(OnMutePushButtonClicked()));
  QObject::connect(classroom_widget->ui.RaisePushButton, SIGNAL(clicked()),
                   teacher_widget_manager_, SLOT(OnRaisePushButtonClicked()));
  QObject::connect(classroom_widget->ui.ChatPushButton, SIGNAL(clicked()),
                   teacher_widget_manager_, SLOT(OnChatPushButtonClicked()));
  QObject::connect(classroom_widget->ui.KickPushButton, SIGNAL(clicked()),
                   teacher_widget_manager_, SLOT(OnKickPushButtonClicked()));

  classroom_widget->setMinimumSize(290, 44);
  classroom_widget->setMaximumSize(290, 44);
  classroom_widget->setAutoFillBackground(true);
  classroom_widget->setStyleSheet(STR(background - color
                                      : rgba(255, 255, 255, 1);));
  ui.ClassroomListScrollArea->widget()->layout()->addWidget(classroom_widget);
  classroom_widgets_map_.insert({user_info.user_uuid, classroom_widget});

  if (!user_info.is_chat_allowed) {
    MuteClassroomChat(user_info.user_uuid, true);
  }

  for (size_t i = 0; i < user_info.properties->NumberOfProperties(); ++i) {
    Property property;
    user_info.properties->GetProperty(i, property);
    if (strncmp(property.key, RAISE_KEY, kMaxKeySize) != 0) {
      continue;
    }

    if (strncmp(property.value, RAISE_TRUE, kMaxValSize) == 0) {
      RaiseClassroom(user_info.user_uuid, true);
      teacher_widget_manager_->SetRaiseUserUuid(user_info.user_uuid);
      break;
    }
  }

  return true;
}

bool AgoraClassroomListWidget::RemoveClassroomWidget(
    const std::string& user_uuid) {
  if (classroom_widgets_map_.size() == 0) {
    return false;
  }

  auto class_widget_it = classroom_widgets_map_.find(user_uuid);
  if (class_widget_it == classroom_widgets_map_.end()) {
    return false;
  };

  ui.ClassroomListScrollArea->widget()->layout()->removeWidget(
      class_widget_it->second);

  delete class_widget_it->second;
  classroom_widgets_map_.erase(class_widget_it);

  return true;
}

bool AgoraClassroomListWidget::MuteClassroomAudio(const std::string& user_uuid,
                                                  bool is_muted) {
  if (classroom_widgets_map_.size() == 0) {
    return false;
  }

  auto class_widget_it = classroom_widgets_map_.find(user_uuid);
  if (class_widget_it == classroom_widgets_map_.end()) {
    return false;
  };
  if (is_muted) {
    class_widget_it->second->ui.MutePushButton->setStyleSheet(STR(
		QPushButton {
          border-image:url(":/image/resource/icon-microphone-off@2x.png");
		  background-color:rgba(0xff,0xff,0xff,1);
        }
        QPushButton:hover{
          border-image:url(":/image/resource/icon-microphone-off-hover@2x.png");
		  background-color:rgba(0xff,0xff,0xff,1);
        }
	));
  } else {
    class_widget_it->second->ui.MutePushButton->setStyleSheet(STR(
		QPushButton {
          border-image:url(":/image/resource/icon-microphone-on@2x.png");
        }
        QPushButton:hover{
          border-image:url(":/image/resource/icon-microphone-on-hover@2x.png");
        }
	));
  }
  return true;
}

bool AgoraClassroomListWidget::MuteClassroomChat(const std::string& user_uuid,
                                                 bool is_muted) {
  if (classroom_widgets_map_.size() == 0) {
    return false;
  }

  auto class_widget_it = classroom_widgets_map_.find(user_uuid);
  if (class_widget_it == classroom_widgets_map_.end()) {
    return false;
  };

  if (is_muted) {
    class_widget_it->second->ui.ChatPushButton->setStyleSheet(
        STR(QPushButton{
          border-image:url(":/image/resource/icon-message-off@2x.png");
        } 
		QPushButton:hover{
          border-image: url(":/image/resource/icon-message off-hover@2x.png");
        }));
  } else {
    class_widget_it->second->ui.ChatPushButton->setStyleSheet(
        STR(
		QPushButton{
          border-image:url(":/image/resource/icon-message@2x.png");
        }
		QPushButton:hover{
          border-image:url(":/image/resource/icon-message-hover@2x.png");
        }));
  }
  return true;
}

bool AgoraClassroomListWidget::RaiseClassroom(const std::string& user_uuid,
                                              bool is_raise) {
  if (classroom_widgets_map_.size() == 0) {
    return false;
  }

  auto class_widget_it = classroom_widgets_map_.find(user_uuid);
  if (class_widget_it == classroom_widgets_map_.end()) {
    return false;
  };

  if (is_raise) {
    class_widget_it->second->ui.RaisePushButton->setStyleSheet(QString::fromLocal8Bit(STR(
		QPushButton{
			border-image:url(":/image/resource/icon-已上台@2x.png");
		}
		QPushButton:hover{
			border-image : url(":/image/resource/icon-已上台-hover@2x.png");
		}))
		);
  } else {
    class_widget_it->second->ui.RaisePushButton->setStyleSheet(QString::fromLocal8Bit(
		STR(
		QPushButton{
			border-image:url(":/image/resource/icon-上台@2x.png");
		}
		QPushButton:hover{
			border-image: url(":/image/resource/icon-上台-hover@2x.png");
		})));
  }
  return true;
}

void AgoraClassroomListWidget::EnablePushButton(PushButtonType type,
                                                bool enable) {
  for (const auto& pair : classroom_widgets_map_) {
    switch (type) {
      case AgoraClassroomListWidget::RAISE_PUSHBUTTON:
        pair.second->ui.RaisePushButton->setEnabled(enable);
        break;
      default:
        break;
    }
  }
}