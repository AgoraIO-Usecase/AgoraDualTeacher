#include "DlgSettings.h"
#include<qstyleditemdelegate.h>
#include "SettingsData.h"
#include "AgoraRtcEngine.h"
#include "DlgInfo.h"
#include "agoracourse.h"
DlgSettings::DlgSettings(float initRate, bool bSecond, float rate, float rate2, QWidget* course, QWidget *parent)
	: QRoundCornerDialog(parent)
{
    rcPosition = rcMainScreen;
	this->rate2 = rate2;
	this->rate = rate;
	this->bSecond = bSecond;
	this->initRate = initRate;
	this->bMax = bMax;
	agoraCourse = course;
	InitDlg();
}

DlgSettings::~DlgSettings()
{

}

void DlgSettings::on_btnAudio_clicked()
{
	rtcEngine->InitAudioDevice();
	DlgSettingAudio dlg(initRate, rate, rate2, agoraCourse, this, bSecond);
	connect(&dlg, &DlgSettingAudio::parentMaxSignal,
		this, &DlgSettings::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	dlg.exec();
	rtcEngine->DestroyAudioDevice();
}
void DlgSettings::on_btnVideo_clicked()
{
	rtcEngine->InitVideoDevice();
	DlgSettingVideo dlg(initRate, rate, rate2, agoraCourse, this, bSecond);
	connect(&dlg, &DlgSettingVideo::parentMaxSignal,
		this, &DlgSettings::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	dlg.exec();
	rtcEngine->DestroyVideoDevice();
}
