#pragma once

#include <QWidget>

#include "AgoraVideoListWidget.h"
#include "student_widget_manager.h"

#include "ui_AgoraStudentWidget.h"

class AgoraChatWidget;
class AgoraStudentWidget : public QWidget {
  Q_OBJECT

 public:
  AgoraStudentWidget(QString classroom_name, std::string user_name,
                     StudentWidgetManager* student_widget_manager,
                     QWidget* parent = Q_NULLPTR);
  ~AgoraStudentWidget();

  void AddStreamWidget(const EduStream& stream);
  void RemoveStreamWidget(const EduStream& stream);
  void ResetStreamWidget(const agora::edu::EduStream& stream,
                         AgoraVideoWidget* widget);
  void ReceiveMessage(const std::string message, const EduBaseUser& user);
  void SetChatButtonEnable(bool enable);
  void ShowScreen();
  void HideScreen();
  AgoraVideoWidget* GetVideoWidget(const size_t& index) {
    return video_widget_[index];
  }
  void SetWidgetView(AgoraVideoWidget* widget, const EduStream& stream_info);

 protected:
  void customEvent(QEvent* event) Q_DECL_OVERRIDE;
  void closeEvent(QCloseEvent* event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void showEvent(QShowEvent* event) Q_DECL_OVERRIDE;

 private:
  void LayoutVideoWidget(bool is_raise);

 private slots:
  void OnChangeSizePushButtonClicked();
  void OnExitPushButtonClicked();
  void OnChatPushButtonClicked();

 private:
  Ui::AgoraStudentWidget ui;

  bool is_drag_ = false;
  QPoint mouse_start_point_;
  QPoint window_top_left_point_;

  AgoraVideoWidget* video_widget_[2];
  QWidget* control_widget_ = nullptr;
  QPushButton* chat_pushbutton_ = nullptr;
  EduStream switch_stream;
  std::unique_ptr<AgoraChatWidget> chat_widget_;

  AgoraVideoListWidget* video_list_widget_ = nullptr;
  StudentWidgetManager* student_widget_manager_ = nullptr;
};
