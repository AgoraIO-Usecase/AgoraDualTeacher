#ifndef DlgExtend_H
#define DlgExtend_H

#include<QDialog>
#include "ui_DlgExtend.h"
#include "QRoundCornerDialog.h"
#include <QVector>
#include "SettingsData.h"
#include <unordered_set>
#include "DlgSettings.h"
#define VIDEO_COUNT 4
class VideoWidget;
class DlgVideoRoom;
class DlgExtend : public QRoundCornerDialog
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	DlgExtend(float rate, float rate2, DlgVideoRoom* dlg, QWidget *parent = 0);
	~DlgExtend();
	bool IsMax() { return bMax; }
	void SetRate(float rate, float rate2);
protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	
private:
	Ui::DlgExtend ui;
	QWidget* agoraCourse = nullptr;
	DlgSettings* dlgSettings = nullptr;
	DlgVideoRoom* dlgRoom = nullptr;
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
	void InitDlg();
	void ShowTopAndBottom(bool bShow);
	void UpdateLayout();

	void UpdateShowVideos();
	void ResetWidgets();
	bool IsUpdateVideoSource();
	void SetVideoSource(bool disableVideo = true);
	VideoWidget* videoWidget[2];
	QVector<WidgetInfo> widgetInfos_;
	
	const int widgetsCount = 2;
public:
	void CloseDlg();
public slots:
	void on_minButton_clicked();
	void on_closeButton_clicked();
	void on_settingsButton_clicked();
	void on_openPlayerComplete();
	void on_playerError(int ec);
	void on_fullScreen(unsigned int uid, bool bFull);
	void on_muteVideo(unsigned int uid, bool mute);
	void on_muteAudio(unsigned int uid, bool mute);
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
};

#endif // DlgExtend_H
