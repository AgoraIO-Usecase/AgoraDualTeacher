#ifndef DlgSettings_H
#define DlgSettings_H

#include<QDialog>
#include "ui_DlgSettings.h"
#include "QRoundCornerDialog.h"

#include "DlgSettingAudio.h"
#include "DlgSettingVideo.h"
#include "DlgVersion.h"

class DlgSettings : public QRoundCornerDialog
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	DlgSettings(float initRate, bool bSecond , float rate, float rate2, QWidget* course, QWidget *parent = 0);
	~DlgSettings();
	bool IsMax() { return bMax; }
private:
	Ui::DlgSettings ui;
	DlgVersion* dlgVersion = nullptr;
	DlgSettingVideo* dlgSettingVideo = nullptr;
	QWidget* agoraCourse = nullptr;
	std::map<int, std::pair<int, int> > m_mapResolution;
	bool bSecond = false;
	bool bMax = true;
	//title
	int titleHeight = 67;
	int titleWidth = 96;
	int titleFontSize = 48;
	int titleTopMargin = 48;
	int titleBottomMargin = 172;
	int titleLeftMargin = 135;
	int titleRightMargin = 910;
	int titleSpacing = 700;
	int backbtnWidth = 78;
	int labSettingW = 96;
	int labSettingH = 67;

	//option
	int optionButtonR = 320;
	int optionLabelW = 130;
	int optionLabelH = 45;
	int optionLeftMargin = 374;
	int optionLayoutSpacing = 106;
	int optionSpacing = 20;
	int optionLabFontSize = 32;
	int optionLabLineHeight = 45;

	int bottomLabelTopMargin = 268;
	QRect rcPosition = { 0,0,0,0 };

	void setTopButtonLayout();
	void setTitleLayout();
	void setOptionLayout();
	void setBottomLabel();
	void InitDlg();
public:
	void maxButtonChange(bool max);
signals:
	void parentMaxSignal(bool childMax);
	void closeSettingDlg();
private slots:
	void on_btnBack_clicked();
	void on_btnAudio_clicked();
	void on_btnVideo_clicked();
	void on_minButton_clicked();
	void on_closeButton_clicked();
	void on_btnVersion_clicked();
	void on_parentMax_slot(bool childMax);
public slots:
	void on_maxButton_clicked();
};

#endif // DlgSettings_H
