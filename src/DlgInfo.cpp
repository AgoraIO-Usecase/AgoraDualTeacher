#include "DlgInfo.h"
#include<qstyleditemdelegate.h>
#include "SettingsData.h"


DlgInfo::DlgInfo(QString info, float rate, float rate2, float initRate, bool bSecond, QWidget *parent)
	: QRoundCornerDialog(parent)
	, information(info)
{
	this->bSecond = bSecond;
	this->rate2 = rate2;
	this->rate = rate;
	this->initRate = initRate;
	ui.setupUi(this);
	InitDlg();
}


DlgInfo::~DlgInfo()
{

}

void DlgInfo::on_btnOK_clicked()
{
	accept();
}

void DlgInfo::on_btnCancel_clicked()
{
	reject();
}

void DlgInfo::on_minButton_clicked()
{
	setWindowState(Qt::WindowMinimized);
}

void DlgInfo::on_closeButton_clicked()
{
	close();
}

void DlgInfo::on_btnBack_clicked()
{
	close();
}

void DlgInfo::UpdateCtrls()
{
	// Audio
	
	//Video
		//Layout
	}

void DlgInfo::showEvent(QShowEvent *event)
{
	QRoundCornerDialog::showEvent(event);
}

void DlgInfo::hideEvent(QHideEvent *event)
{
	QRoundCornerDialog::hideEvent(event);
}