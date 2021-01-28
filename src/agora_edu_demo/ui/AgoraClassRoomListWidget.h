#pragma once
#include <unordered_map>

#include <QWidget>

#include "EduUser.h"

#include "ui_AgoraClassroomListWidget.h"

using namespace agora::edu;

class TeacherWidgetManager;
class AgoraClassroomWidget;
class AgoraClassroomListWidget : public QWidget {
  Q_OBJECT

  friend class AgoraClassroomWidget;
  static const unsigned int MAX_CLASSROOM_WIDGETS = 4;
 
 public:
  enum PushButtonType {
    RAISE_PUSHBUTTON,
  };

  AgoraClassroomListWidget(TeacherWidgetManager* teacher_widget_manager,
                           QWidget* parent = Q_NULLPTR);
  bool AddClassroomWidget(const EduUser& user_info);
  bool RemoveClassroomWidget(const std::string& user_uuid);
  bool MuteClassroomAudio(const std::string& user_uuid, bool is_muted);
  bool MuteClassroomChat(const std::string& user_uuid, bool is_muted);
  bool RaiseClassroom(const std::string& user_uuid, bool is_raise);
  void EnablePushButton(PushButtonType type, bool enable);

 private:
  Ui::AgoraClassroomListWidget ui;
  TeacherWidgetManager* teacher_widget_manager_ = nullptr;

  std::unordered_map<std::string, AgoraClassroomWidget*> classroom_widgets_map_;
};
