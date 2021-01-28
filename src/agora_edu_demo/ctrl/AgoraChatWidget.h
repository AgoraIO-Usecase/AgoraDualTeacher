#pragma once

#include <QKeyEvent>
#include <QWidget>
#include <memory>
#include "ui_AgoraChatWidget.h"
class AgoraChatWidget : public QWidget {
  Q_OBJECT

 public:
  AgoraChatWidget(QWidget* parent = Q_NULLPTR);
  virtual ~AgoraChatWidget();

  void AddMessage(const QString& title, const QString& msg,
                  Qt::Alignment = Qt::AlignLeft);
  void SetUserName(std::string user_name) { user_name_ = user_name; }
  void SetLineEditEnable(bool enable) { ui.lineEdit->setEnabled(enable); }

 protected:
  void keyPressEvent(QKeyEvent* event);
  void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

 private slots:
  void OnSendMessage();
  void OnExitPushButtonClicked();

 signals:
  void ChatMessageSendSig(ChatMessage message);
  void ChatWidgetExitSig(bool isChecked = false);

 private:
  Ui::AgoraChatWidget ui;

  bool is_drag_ = false;
  QPoint mouse_start_point_;
  QPoint window_top_left_point_;

  std::string user_name_;
};
