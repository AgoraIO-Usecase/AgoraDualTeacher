#pragma once

#include <QDialog>
#include "ui_AgoraTipsWidget.h"

class AgoraTipsDialog : public QDialog {
  Q_OBJECT

 public:
  static int ExecTipsDialog(QString tips);

 private:
  AgoraTipsDialog(QWidget *parent = Q_NULLPTR);
  ~AgoraTipsDialog();


 private slots:
  void OnCancelPushButtonClicked();
  void OnConfirmPushButtonClicked();

 private:
  Ui::QTipsWidget ui;
};
