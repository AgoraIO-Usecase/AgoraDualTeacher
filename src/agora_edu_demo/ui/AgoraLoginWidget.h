#pragma once

#include <QWidget>

#include "ui_AgoraLoginWidget.h"

#include <memory>

class LoginWidgetManager;
class AgoraSettingDialog;

class AgoraLoginWidget : public QWidget {
  Q_OBJECT

 public:
  AgoraLoginWidget(std::shared_ptr<LoginWidgetManager>,
                   QWidget *parent = Q_NULLPTR);

  void SetTipsLabelContent(QString content);

 protected:
  void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

 private slots:
  void OnLoginPushButtonClicked();
  void OnExitPushButtonClicked();
  void OnInitializeResult(bool is_success);
  void OnSettingPushButtonClicked();
  void OnRoomNameLineEditTextChanged(const QString &);

 private:
  Ui::AgoraLoginWidget ui;

  bool is_drag_ = false;
  QPoint mouse_start_point_;
  QPoint window_top_left_point_;

  std::unique_ptr<AgoraSettingDialog> setting_dlg_;
  std::shared_ptr<LoginWidgetManager> login_widget_manager_;
};
