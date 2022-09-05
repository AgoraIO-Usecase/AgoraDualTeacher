#ifndef DlgSettingVideo_H
#define DlgSettingVideo_H

#include<QDialog>
#include "ui_DlgSettingVideo.h"
#include "QRoundCornerDialog.h"
#include <QMap>
#include <QHash>
#include <QSet>
class DlgSettingVideo : public QRoundCornerDialog
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	DlgSettingVideo(float initRate, float rate, float rate2, QWidget* course, QWidget* parent = 0, bool bSecond = false);
	~DlgSettingVideo();
	bool IsMax() { return bMax; }
private:
	Ui::DlgSettingVideo ui;
	QWidget* agoraCourse = nullptr;
	bool bSecond = false;
	bool bMax = true;
	//title
	int titleHeight = 67;
	int titleFontSize = 48;
	int titleTopMargin = 48;
	int titleBottomMargin = 60;
	int titleLeftMargin = 135;
	int titleRightMargin = 886;
	int titleSpacing = 676;
	int backbtnWidth = 78;
	int labTitleW = 192;
	int labTitleH = 67;

	//videsource
	int sourceLeftMargin = 390;
	int sourceRightMargin = 296;
	int sourceLayout = 75;
	int sourceSpace = 75;
	int videoInfoSpace = 12;
	int videoInfoW = 89;
	int videoInfoH = 37;
	int videoWidgetW = 540;
	int videoWidgetH = 302;
	int horizonalVideoSource1BottomMargin = 40;

	//vdeisource1
	int videoSource1Space = 30;
	int verticalLayout_Video1Space = 13;
	int horizontalLayout_video1_1Space = 34;
	int horizontalLayout_video1_1LeftMargin = 20;
	int labelInfoW = 104;
	int labelInfoH = 37;
	int videoDeviceW = 420;

	int horizontalLayout_ResVideo1Space = 184;
	int backForwardW = 52;
	int resBtnW = 153;
	int lineSpace = 13;
	//
	int fontSize = 26;
	int lineHeight = 37;
	int buttonW = 430;
	int buttonH = 66;
	int micSelectTextPadding = 50;
	int versionSpacing = 30;
	int lineW = 630;

	int edtFontSize = 18;
	int edtPadding = 20;
	int edtLineHeight = 50;
	int horizontalLayout_video2_1Spacing = 47;
	int edtWidth = 415;
	int edtHeight = 50;
	void InitDlg();
	void setTopButtonLayout();
	void setTitleLayout();
	void setVideoSourceLayout();
	void setBottomLabel();
	void setEditStyle(bool bEnable);
	void SetResolution();
	void SetEnabledVideoSource1();
	void SetEnabledVideoSource2();
	void UpdateVideoDeviceInfo();
	int fps[4];
	std::map<int, std::pair<int, int> > m_mapResolution;
	QHash<QString, QString> videoInfos_;
	QMap<QString, QString> fpsInfos_;
	QMap<QString, QString> resolutionInfos_;
	QVector<agora::rtc::VideoFormat> videoDeviceInfos_;
	void InitData();
	void UpdateCtrls();
private:
	void onCancel();
	void SetVideoEncoder();
private slots:
	void on_minButton_clicked();
	void on_closeButton_clicked();
	void on_btnBack_clicked();
	void on_btnVideoSource1_clicked();
	void on_btnVideo1Res_clicked();
	void on_btnVideo1ResPre_clicked();
	void on_btnVideo1FPSPre_clicked();
	void on_btnVideo1StatsPre_clicked();
	void on_btnvideo2StatsPre_clicked();
	void on_btnVideo1ResNext_clicked();
	void on_btnVideo1FPSNext_clicked();
	void on_btnVideo1StatsNext_clicked();
	void on_btnvideo2StatsNext_clicked();
	void on_openPlayerComplete();
	void on_parentMax_slot(bool childMax);
	void on_playerError(int ec);
public slots:
	void on_maxButton_clicked();
public:
	void maxButtonChange(bool max);
signals:
	void parentMaxSignal(bool childMax);
};

#endif // DlgSettingVideo_H
