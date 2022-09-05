#ifndef DlgSettingSelect_H
#define DlgSettingSelect_H

#include<QDialog>
#include <QRadioButton>
#include "ui_DlgSettingSelect.h"
#include "QRoundCornerDialog.h"
#include <QHash>
#include <QMap>

enum SELECT_TYPE {
	SELECT_TYPE_VIDEO1 = 0,
	SELECT_TYPE_VIDEO2,
	SELECT_TYPE_MIC,
	SELECT_TYPE_SPEAKER
};

class DlgSettingSelect : public QRoundCornerDialog
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	DlgSettingSelect(float initRate, float rate, float rate2, bool bSecond, QHash<QString, QString> items, QString selectedId, SELECT_TYPE type, QString title, QWidget* parent = 0);
	~DlgSettingSelect();
	bool IsMax() { return bMax; }
private:
	Ui::DlgSettingSelect ui;
	bool bSecond = false;
	bool bMax = true;
	float initRate = 1.0f;
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

	int itemW = 620;
	int itemH = 66;
	//option
	int gridMargin = 650;
	int gridSpace = 20;
	
	int versionSpacing = 30;

	int labFontSize = 26;
	int labLineHeight = 37;
	int btnFontize = 30;
	int btnLineHeight = 42;
	int bottomLabelTopMargin = 170;
	int itemCheckWidth = 36;
	int itemPadding = 20;

	void InitDlg();
	void setTopButtonLayout();
	void setTitleLayout();
	void setOptionLayout();
	void setBottomLabel();
	void InitItems();

	void UpdateCtrls(); 
	QHash<QString, QString> items_; //id name
	QMap<QRadioButton*, QString> itemButtons_; // radioButon id
	QString selectedId_;
	SELECT_TYPE select_type_ = SELECT_TYPE_VIDEO1;
	QString title_;
private slots:
	void on_btnBack_clicked();
	void on_btnOK_clicked();
	void on_btnCancel_clicked();
	void on_minButton_clicked();
	void on_closeButton_clicked();
	void radioItemClicked();
public slots:
	void on_maxButton_clicked();
public:
	void maxButtonChange(bool max);
signals:
	void parentMaxSignal(bool childMax);
	
};

#endif // DlgSettingSelect_H
