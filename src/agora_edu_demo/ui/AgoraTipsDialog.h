#pragma once

#include <QDialog>
#include "ui_AgoraTipsWidget.h"

class AgoraTipsDialog : public QDialog {
  Q_OBJECT

 public:
  static int ExecTipsDialog(const QString& title, const QString& tips);

 private:
  AgoraTipsDialog(QWidget* parent = Q_NULLPTR);
  ~AgoraTipsDialog();

 protected:
  void paintEvent(QPaintEvent* event);

 private slots:
  void OnCancelPushButtonClicked();
  void OnConfirmPushButtonClicked();

 private:
  Ui::BorderWidget ui;
};
