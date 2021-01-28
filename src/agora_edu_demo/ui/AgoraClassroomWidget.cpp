#include "AgoraClassroomWidget.h"
#include "util.h"

AgoraClassroomWidget::AgoraClassroomWidget(agora::edu::EduUser user_info,
                                           QWidget* parent)
    : user_info_(user_info), QWidget(parent) {
  ui.setupUi(this);
  setWindowFlags(Qt::FramelessWindowHint);

  std::string str = user_info_.user_name;
  ui.ClassroomLabel->setText(str2qstr(str));
}
