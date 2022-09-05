#ifndef DlgInfo_H
#define DlgInfo_H

#include<QDialog>
#include "ui_DlgInfo.h"
#include "QRoundCornerDialog.h"


class DlgInfo : public QRoundCornerDialog
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	DlgInfo(QString info, float rate, float rate2, float initRate = 1.0f, bool bSecond = false, QWidget *parent = 0);
	~DlgInfo();
	bool IsMax() { return bMax; }
protected:
	virtual void showEvent(QShowEvent *event) override;
	virtual void hideEvent(QHideEvent *event) override;
	
private:
	Ui::DlgInfo ui;
	bool bMax = true;
	int fps[4];
	std::map<int, std::pair<int, int> > m_mapResolution;
	QString information = "";
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
	bool bSecond = false;
	//option
	int buttonW = 430;
	int buttonH = 66;

	int labFontSize = 26;
	int labLineHeight = 37;
	int labW = 260;
	int labH = 50;
	int btnFontSize = 30;
	int btnLineHeight = 42;
	
	int versionSpacing = 30;
	void InitDlg();
	void setTopButtonLayout();
	void setTitleLayout();
	void setOptionLayout();
	void setBottomLabel();

	void InitData();
	void UpdateCtrls(); 
	
private slots:
	void on_btnBack_clicked();
	void on_btnOK_clicked();
	void on_btnCancel_clicked();
	void on_minButton_clicked();
	void on_closeButton_clicked();
	void on_parentMax_slot(bool childMax);
public slots:
	void on_maxButton_clicked();
public:
	void maxButtonChange(bool max, bool bMain = false);
signals:
	void parentMaxSignal(bool childMax);
	void changeMaxShowVideos();
};

#endif // DlgInfo_H
