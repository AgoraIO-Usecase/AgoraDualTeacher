#pragma once

#include <QWidget>
#include <mutex>
#include "EduUser.h"
#include "IEduClassroomManager.h"

#include "teacher_widget_manager.h"
#include "ui_AgoraTeacherWidget.h"


using namespace agora::edu;

class TeacherWidgetManager;
class AgoraClassroomListWidget;
class AgoraVideoWidget;
class AgoraChatWidget;
class AgoraVideoListWidget;
class AgoraTeacherWidget : public QWidget {
  Q_OBJECT

 public:
  AgoraTeacherWidget(QString classroom_name, std::string user_name,
                     TeacherWidgetManager* teacher_widget_manager,
                     QWidget* parent = Q_NULLPTR);
  ~AgoraTeacherWidget();

 public:
  void RemoteClassroomJoin(
      agora_refptr<IUserInfoCollection> user_info_collection);
  void RemoteClassroomLeave(
      agora_refptr<IUserEventCollection> user_event_collection);
  void UpdateVideoWidgetUi(std::string user_uuid,
                           bool is_enable);
  void UpdateClassroomStateUi(std::string user_uuid,
                              ClassroomStateChangeType type, bool is_enable);
  void SetStreamUUidWithUserUUid(const std::string& user_uuid,const std::string &stream_uuid);
  void RaiseClassroom(std::string user_uuid,
                      bool is_enable);
  void AddFreeLocalStream(const EduStream& stream_info);
  void AddSpecifiedLocalStream(const size_t& index,
                               const EduStream& stream_info);
  void SetVideoWidget(const EduStream & stream,AgoraVideoWidget * widget);
  void RemoveLocalStream(const EduStream& stream_info);
  void AddRemoteStream(const EduStream& stream_info);
  void RemoveRemoteStream(const EduStream& stream_info);
  void AutoSubscribeDualStream();
  void ResetStreamWidget(const agora::edu::EduStream& stream,
                         AgoraVideoWidget* widget);
  void ReceiveMessage(const std::string message, const EduBaseUser& user);
  void ShowScreen(bool enable_share_screen = false);
  void HideScreen();
  void EnableClassroomListRaisePushbutton(bool enable);
  std::vector<AgoraVideoWidget*> GetVideoWidgetList();
  void SwapAgoraVideoWidget(AgoraVideoWidget* widget_1,AgoraVideoWidget * widget_2);

 protected:
  void customEvent(QEvent* event) Q_DECL_OVERRIDE;
  void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;
 private:
  bool CheckNeedDualScreen();
  bool AddClassroomVideoWidget(const std::string& user_uuid);
  bool RemoveClassroomVideoWidget(const std::string& user_uuid);
  void LayoutClassroomVideoWidget(const size_t& video_count);
  void ShowControlWidget(bool enable_share_screen = false);

 public slots:
  void OnClassPushButtonClicked();
  void OnChangeSizePushButtonClicked();
  void OnExitPushButtonClicked();
  void OnChatPushButtonClicked();
  void OnShareScreenPushButtonClicked();
  void OnOpenGLWidgetTouched();

 private:
  enum DisplayMode { SINGLE_SCREEN, DUAL_SCREEN };

  DisplayMode display_mode_ = SINGLE_SCREEN;
  Ui::AgoraTeacherWidget main_ui_;

  bool is_drag_ = false;
  QPoint mouse_start_point_;
  QPoint window_top_left_point_;

  std::unique_ptr<QWidget> extend_widget_;
  AgoraVideoListWidget* video_list_widget_ = nullptr;
  AgoraClassroomListWidget* classroom_list_widget_ = nullptr;

  AgoraVideoWidget* video_widget_[2];
  bool subscribe_big_stream_;
  bool raise_;
  std::string raise_user_uuid_ = "";
  AgoraVideoWidget* raise_video_widget_ = nullptr;
  QWidget* control_widget_ = nullptr;
  QPushButton* class_pushbutton_ = nullptr;
  QPushButton* chat_pushbutton_ = nullptr;
  QPushButton* share_screen_pushbutton_ = nullptr;
  std::unique_ptr<AgoraChatWidget> chat_widget_;

  std::vector<UuidWidgetPair> classroom_video_widgets_;
  std::vector<AgoraVideoWidget*> video_widget_list_;
  std::unordered_map<std::string, std::string> map_uuid_streamid_;
  std::unordered_map<std::string, AgoraVideoWidget*> map_student_uuid_video_widgets_;
  std::unordered_map<int, AgoraVideoWidget*> map_teacher_video_widgets_;
  std::mutex mutex_;
  std::unordered_map<std::string, std::string> map_streamid_uuid_;
  TeacherWidgetManager* teacher_widget_manager_ = nullptr;
};
