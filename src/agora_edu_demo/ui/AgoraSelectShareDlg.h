#ifndef AGORASELECTSHAREDLG_H
#define AGORASELECTSHAREDLG_H

#include <QtCore/QSet>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QScrollArea>
#include <QPushButton>
#include "ui_AgoraSelectShareDlg.h"

struct CaptureInfo {
  bool isDesktop;
  QRect rcDesktop;
  QPixmap pixmap;
  QString title;
  int width;
  int height;
  HWND hwnd;
};

class ShareItemButton : public QPushButton {
  Q_OBJECT
 public:
  ShareItemButton(QWidget* parent = 0);
  ~ShareItemButton();
  CaptureInfo info;

 protected:
  virtual void paintEvent(QPaintEvent*) Q_DECL_OVERRIDE;
  QFont font;
  QFontMetrics fontMetrics;
 public slots:
  void sharetoggled(bool checked);
 signals:
  void notifyChecked(bool checked, ShareItemButton* pButton);
};

class AgoraSelectShareDlg : public QDialog {
  Q_OBJECT

 public:
  AgoraSelectShareDlg(QWidget* parent = 0);
  ~AgoraSelectShareDlg();

  void getAvailableScreenId();

  CaptureInfo selectedInfo;
  bool bCapture;

 private:
  Ui::AgoraSelectShareDlg ui;
  std::vector<CaptureInfo> wndsInfo;
  std::vector<ShareItemButton*> shareButtons;
  ShareItemButton* selectedButton;
 private slots:
  void onNotify(bool checked, ShareItemButton* pButton);
  void on_applyButton_clicked();
  void on_ExitPushButton_clicked();
};

#endif  // AGORASELECTSHAREDLG_H
