#ifndef AGORACOURSE_H
#define AGORACOURSE_H

#include<QDialog>
#include "ui_agoracourse.h"
#include "QRoundCornerDialog.h"
#include "DlgSettings.h"
class DlgVideoRoom;
class DlgExtend;
class AgoraCourse : public QRoundCornerDialog
{
	Q_OBJECT

public:
	AgoraCourse(QWidget *parent = 0);
	~AgoraCourse();
	virtual void resizeEvent(QResizeEvent* event);
	virtual void showEvent(QShowEvent* event);
	//virtual void changeEvent(QEvent* event)override;
private:
	void InitDlg();
	Ui::AgoraCourse ui;
	DlgVideoRoom* dlgRoom = nullptr;
	DlgExtend* dlgExtend = nullptr;
	DlgSettings* dlgSettings = nullptr;
	bool bMax = true;
	//RoomInfo info;
	int titleHeight = 80;
	int titleFontSize = 58;
	int titleTopMargin = 40;
	int titleBottomMargin = 115;
	int titleWidth = 500;

	int edtFonSize = 18;
	int edtPaddingLeft = 26;
	int lineHeight = 50;
	int edtLayoutSpacing = 60;
	int edtHeight = 90;
	int edtWidth = 610;

	int loginButtonTopMargin = 100;
	int loginButtonWidth = 240;
	int loginButtonHeight = 90;
	
	int loginBtnFontSize = 38;
	int loginBtnLineHeight = 53;
	
	void setTopButtonLayout();
	void setTitleLayout();
	void setEditLayout();
	void setLoginButton();
	void setBottomLabel();
	void maxButtonChange(bool max);

	unsigned int  randomUid(); // 1:teacher 2:student
	void showClassRoomDlg();
private slots :
	void on_parentMax_slot(bool childMax);
	void on_settingsButton_clicked();
	void on_minButton_clicked();
	void on_maxButton_clicked();
	void on_closeButton_clicked();
	void on_loginButton_clicked();
	void onJoinChannelSuccess(const char* channel, agora::rtc::uid_t uid, int elapsed);
	
	void onCloseRoom(bool bExtend);
	void onLeaveChannel();
};

#endif // AGORACOURSE_H
