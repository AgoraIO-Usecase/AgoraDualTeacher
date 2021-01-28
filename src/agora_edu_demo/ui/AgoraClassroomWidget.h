#pragma once
#include <QWidget>

#include "EduUser.h"

#include "ui_AgoraClassroomWidget.h"

class AgoraClassroomWidget : public QWidget {
  Q_OBJECT

  friend class AgoraClassroomListWidget;

 public:
  AgoraClassroomWidget(agora::edu::EduUser user_info, QWidget* parent = Q_NULLPTR);
  agora::edu::EduUser GetUserInfo() { return user_info_; }

 private:
  static unsigned int current_count;

  Ui::AgoraClassroomWidget ui;

  agora::edu::EduUser user_info_;
};
