#ifndef DlgVersion_H
#define DlgVersion_H

#include<QDialog>
#include "ui_DlgVersion.h"
#include "QRoundCornerDialog.h"

class DlgVersion : public QRoundCornerDialog
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	DlgVersion(float initRate, float rate, float rate2, QWidget* course, QWidget* parent = 0, bool bSecond = false);
	~DlgVersion();
	bool IsMax() { return bMax; }
private:
	Ui::DlgVersion ui;
	QWidget* agoraCourse = nullptr;
	bool bSecond = false;
	bool bMax = true;
	//title
	int titleHeight = 67;
	int titleFontSize = 48;
	int titleTopMargin = 48;
	int titleBottomMargin = 135;
	int titleLeftMargin = 135;
	int titleRightMargin = 886;
	int titleSpacing = 676;
	int backbtnWidth = 78;
	int labTitleW = 144;
	int labTitleH = 78;

	//option
	int optionButtonR = 306;
	int optionLabelW = 216;
	int optionLabelH = 45;
	int versionSpacing = 30;
	int largeFontSize = 36;
	int largeLineHeight = 50;
	int fontSize = 32;
	int lineHeight = 45;
	int bottomTopMargin = 170;

	void setTopButtonLayout();
	void setTitleLayout();
	void setOptionLayout();
	void setBottomLabel();
	void InitDlg();

private slots:
	void on_btnBack_clicked();
	void on_minButton_clicked();
	void on_closeButton_clicked();
	//void on_parentMax_slot(bool childMax);
public slots:
	void on_maxButton_clicked();
public:
	void maxButtonChange(bool max);
signals:
	void parentMaxSignal(bool childMax);
};

#endif // DlgVersion_H
