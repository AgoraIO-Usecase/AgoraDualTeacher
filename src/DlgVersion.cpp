#include "DlgVersion.h"
#include<qstyleditemdelegate.h>
#include "SettingsData.h"
#include "DlgSettings.h"
DlgVersion::DlgVersion(float initRate, float rate, float rate2, QWidget* course, QWidget *parent, bool bSecond)
	: QRoundCornerDialog(parent)
{
	ui.setupUi(this);
	this->bSecond = bSecond;
	this->initRate = initRate;
	this->rate2 = rate2;
	this->rate = rate;
	agoraCourse = course;
	InitDlg();
}

DlgVersion::~DlgVersion()
{

}