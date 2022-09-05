#ifndef DlgSettingAudio_H
#define DlgSettingAudio_H

#include<QDialog>
#include "ui_DlgSettingAudio.h"
#include "QRoundCornerDialog.h"
#include <unordered_map>
#include "SettingsData.h"
class DlgSettingAudio : public QRoundCornerDialog
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	DlgSettingAudio(float initRate, float rate, float rate2, QWidget* course, QWidget *parent = 0, bool bSecond = false);
	~DlgSettingAudio();
	bool IsMax() { return bMax; }
private:
	Ui::DlgSettingAudio ui;
	QWidget* agoraCourse = nullptr;
	bool bSecond = false;
	bool bMax = true;
	//UI
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
	//audio
	int audioLeftMargin = 562;
	int audioRightMargin = 460;
	int audioLayout = 75;
	int audioSpace = 75;
	int audio3ALeftMargin = 540;
	int versionSpacing = 30;
	int verticalLayout_MicInfoBottomMargin = 110;
	int labelSpeakerInfoH = 53;
	int labelSpeakerInfoW = 190;
	int label3AW = 230;

	//right mic
	int verticalLayout_MicSettingSpacing = 55;
	int verticalLayout_MicSpace = 13;
	int verticalLayout_SpeakerSettingSpacing = 48;
	int horizontalLayout_MicDeviceSpace = 34;
	int horizontalLayout_MicDeviceLeftMargin = 20;
	int deviceCtrlW = 480;
	int micCtrlH = 37;
	int volumeCtrl = 500;
	int labW = 78;
	int labH = 37;
	int testBtnW = 96;
	int testBtnH = 65;
	int lineW = 630;
	int horizontalLayout_AGCSpace = 80;
	int horizontalLayout_AGCTopMargin = 20;
	int horizontalLayout_AGCRightMargin = 50;
	int backForwardW = 52;
	int combox3AW = 140;
	int fontMidSize = 38;
	int lineMidHeight = 53;
	int fontLittleSize = 26;
	int lineLittleHeight = 37;
	int micSelectTextPadding = 50;

	int bottomTopMargin = 170;
	void InitDlg();
	void setTopButtonLayout();
	void setTitleLayout();
	void setAudioLayout();
	void setBottomLabel();

	//
	int fps[4];
	std::map<int, std::pair<int, int> > m_mapResolution;
	
	bool testMic_ = false;
	bool testSpeaker_ = false;
	QHash<QString, QString> micInfos_;
	QHash<QString, QString> speakerInfos_;

	void UpdateCtrls(); 
	void onExit(); 
private slots:
	void on_minButton_clicked();
	void on_closeButton_clicked();
	void on_btnBack_clicked();
	void on_btnTestMic_clicked();
	void on_btnTestSpeaker_clicked();
	void onVolumeIndication_slot(unsigned int volume, unsigned int speakerNumber, int totalVolume);
	void on_btnMicSelect_clicked();
	void on_btnSpeakerSelect_clicked();
	void on_btnAGCStatsPre_clicked();
	void on_btnAGCStatsNext_clicked();
	void on_btnAECStatsPre_clicked();
	void on_btnAECStatsNext_clicked();
	void on_btnANSStatsPre_clicked();
	void on_btnANSStatsNext_clicked();
	void on_parentMax_slot(bool childMax);
public slots:
	void on_maxButton_clicked();
public:
	void maxButtonChange(bool max);
signals:
	void parentMaxSignal(bool childMax);
};

#endif // DlgSettingAudio_H
