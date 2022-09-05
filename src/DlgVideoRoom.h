#ifndef DlgVideoRoom_H
#define DlgVideoRoom_H

#include<QDialog>
#include "ui_DlgVideoRoom.h"
#include "QRoundCornerDialog.h"
#include <QVector>
#include "SettingsData.h"
#include <unordered_set>
#define VIDEO_COUNT 4
class VideoWidget;
class AgoraCourse;
class DlgSettings;
class DlgVideoRoom : public QRoundCornerDialog
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	DlgVideoRoom(QWidget *parent = 0);
	~DlgVideoRoom();
	bool IsMax() { return bMax; }
	void SetRate(float rate, float rate2);
protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
private:
	Ui::DlgVideoRoom ui;
	QWidget* agoraCourse = nullptr;
	DlgSettings* dlgSettings = nullptr;
	bool bMax = true;
	//title
	int titleHeight = 67;
	int titleFontSize = 48;
	int titleTopMargin = 48;
	int titleBottomMargin = 20;
	int labTitleW = 192;
	int labTitleH = 67;

	//video widget
	int videoW = 710;
	int videoH = 400;
	int videoSpace = 36;
	int videoMargin = 73;
	int pageBtnW = 52;
	int pageBtnH = 52;
	int midMargin = 98;
	int bottomTopMargin = 18;
	
	void setTopButtonLayout();
	void setTitleLayout();
	void setOptionLayout();
	void setBottomLabel();
	void showPage(bool bShow);
	void InitDlg();
	void ShowTopAndBottom(bool bShow);
	void UpdateLayout();

	void UpdateShowVideos();
	void ShowWidgets();
	void ResetWidgets();
	void RequestUserName(unsigned int uid);
	
	VideoWidget* videoWidget[VIDEO_COUNT];
	QVector<WidgetInfo> widgetInfos_;
	int curPage = 0;
	const int widgetsCount = 4;

	QString cmdRequestUserName = "requestUserName";

public :
	void CloseDlg();
private slots:
	void on_settingsButton_clicked();
	void on_btnPrePage_clicked();
	void on_btnNextPage_clicked();
	void on_minButton_clicked();
	void on_closeButton_clicked();
	void on_openPlayerComplete();
	void on_playerError(int ec);
	void on_fullScreen(unsigned int uid, bool bFull);
	void on_muteVideo(unsigned int uid, bool mute);
	void on_muteAudio(unsigned int uid, bool mute);
	void onUserJoined(unsigned int uid, int elapsed);
	void onUserOffline(unsigned int uid, int elapsed);
	void onStreamMessage(unsigned uid, QString message);
	void on_settingDlg_close();
	void on_parentMax_slot(bool childMax);
public slots:
	void on_maxButton_clicked();
public:
	void SetControlMax(bool max);
	void maxButtonChange(bool max);
	void on_fullMaxButton_clicked(bool bMax);
signals:
	void parentMaxSignal(bool childMax);
	void closeRoom(bool bExtend);
	void showRemoteDlg();
};

#endif // DlgVideoRoom_H
