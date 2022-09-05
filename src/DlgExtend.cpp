#include "DlgExtend.h"
#include<qstyleditemdelegate.h>
#include <QMap>
#include "DlgInfo.h"
#include "VideoWidget.h"
#include "DlgSettings.h"
#include "SettingsData.h"
#include "AgoraRtcEngine.h"
#include <QDebug>
#include "DlgVideoRoom.h"
DlgExtend::DlgExtend(float rate, float rate2, DlgVideoRoom* dlg, QWidget *parent)
	: QRoundCornerDialog(parent)
{
	ui.setupUi(this);
	this->rate2 = rate2;
	this->rate = rate;
	InitDlg();
	agoraCourse = parent;
	dlgRoom = dlg;
}


DlgExtend::~DlgExtend()
{

}
void DlgExtend::SetRate(float rate, float rate2)
{
	this->rate2 = rate2;
	this->rate = rate;
}
void DlgExtend::UpdateShowVideos()
{
	QMap<unsigned int, VideoWidget*> map;
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		videoWidget[i]->SetWidgetInfo(widgetInfos_[i]);
		map.insert(widgetInfos_[i].userInfo.uid, videoWidget[i]);
	}
	rtcEngine->SetVideoWidgetEx(map);
}

void DlgExtend::ResetWidgets()
{ 
	for (int i = 0; i < widgetsCount; ++i) {
		videoWidget[i]->Reset();
	}
}

void DlgExtend::on_closeButton_clicked()
{
	QString strInfo = QString::fromStdWString(L"确认退出房间么");
	DlgInfo dlg(strInfo, rate, rate2, initRate, true, this);
	connect(&dlg, &DlgInfo::parentMaxSignal,
		this, &DlgExtend::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	if (dlg.exec() == QDialog::Accepted) {
		rtcEngine->ResetVideoWidgetsEx();
		if (setting.enabledVideoSource2) {
			rtcEngine->StopVideoSource2();
			rtcEngine->VideoSourceLeave(setting.className.toUtf8(), setting.userInfo2.uid);
			rtcEngine->SetJoined2(false);
		}
		rtcEngine->VideoSourceLeave(setting.className.toUtf8(), setting.userInfo.uid);
		rtcEngine->SetJoined(false);
	
		CloseDlg();
		emit closeRoom(true);
		dlg.close();
	}
}


void DlgExtend::CloseDlg()
{
	ResetWidgets();
	widgetInfos_.clear();
	videoWidget[2]->hide();
	videoWidget[3]->hide();
	hide();
	rtcEngine->ResetVideoWidgets();
}

void DlgExtend::on_settingsButton_clicked()
{
	if (!dlgSettings) {
		dlgSettings = new DlgSettings(initRate, setting.bExtend, rate, rate2, agoraCourse, this);
		
		if (!bMax && dlgSettings->IsMax())
			dlgSettings->maxButtonChange(true);
	}
	dlgSettings->show();
	if (setting.bExtend)
		connect(dlgSettings, &DlgSettings::closeSettingDlg,
			this, &DlgExtend::on_settingDlg_close);
}


void DlgExtend::on_settingDlg_close()
{
	if (IsUpdateVideoSource()) {
		rtcEngine->ResetVideoWidgetsEx();
		ResetWidgets();
		widgetInfos_.clear();
		SetVideoSource();
	}

	if (setting.enabledVideoSource1
		&& !rtcEngine->IsJoined()) {
		rtcEngine->SetJoined(true);
		agora::rtc::VideoEncoderConfiguration config;
		config.dimensions.width = setting.resolution[setting.resIndex].width;
		config.dimensions.height = setting.resolution[setting.resIndex].height;
		config.frameRate = setting.frameRate;
		rtcEngine->VideoSource1JoinChannel(setting.enabledVideoSource1, "", setting.className.toUtf8(), setting.userInfo.uid, config);
	}
	else if (!setting.enabledVideoSource1
		&& rtcEngine->IsJoined()) {
		rtcEngine->SetJoined(false);
		rtcEngine->VideoSourceLeave(setting.className.toUtf8(), setting.userInfo.uid);
	}


	if (setting.enabledVideoSource2) {
		if (!rtcEngine->IsJoined2()) {
			rtcEngine->SetJoined2(true);
			bool subscribe = !rtcEngine->IsvideoSource1Subscribe();
			rtcEngine->VideoSource2JoinChannel(setting.enabledVideoSource2, "", setting.className.toUtf8(), setting.userInfo2.uid, subscribe, subscribe);
		}
		else if (!rtcEngine->IsJoined() && rtcEngine->IsvideoSource1Subscribe()) {//
			rtcEngine->UpdateVideoSource2Subscribe(true);
		}
	}
	else if (!setting.enabledVideoSource2
		&& rtcEngine->IsJoined2()) {
		rtcEngine->SetJoined2(false);
		rtcEngine->VideoSourceLeave(setting.className.toUtf8(), setting.userInfo2.uid);
	}
}

bool DlgExtend::IsUpdateVideoSource()
{
	if (setting.enabledVideoSource1 &&
		setting.enabledVideoSource2 &&
		setting.userInfo.uid == videoWidget[0]->GetUID() &&
		setting.userInfo2.uid == videoWidget[1]->GetUID()) {
		return false;
	}
	else if (setting.enabledVideoSource1 &&
		!setting.enabledVideoSource2 &&
		setting.userInfo.uid == videoWidget[0]->GetUID() &&
		0 == videoWidget[1]->GetUID()) {
		return false;
	}
	else if (!setting.enabledVideoSource1 &&
		setting.enabledVideoSource2 &&
		0 == videoWidget[0]->GetUID() &&
		setting.userInfo2.uid == videoWidget[1]->GetUID()) {
		return false;
	}
	else if (!setting.enabledVideoSource1 &&
		!setting.enabledVideoSource2 &&
		0 == videoWidget[0]->GetUID() &&
		0 == videoWidget[1]->GetUID()) {
		return false;
	}
	else
		return true;
}

void DlgExtend::SetVideoSource(bool disableVideo)
{
	if (setting.enabledVideoSource1) {
		WidgetInfo info = { setting.userInfo , false, false };
		bool bFind = false;
		for (int i = 0; i < widgetInfos_.size(); ++i) {
			if (widgetInfos_[i].userInfo.uid == setting.userInfo.uid)
				bFind = true;
		}

		if (!bFind ) {
			if (disableVideo)
				rtcEngine->EnableLocalVideo(false);
			widgetInfos_.push_back(info);
		}
	}

	if (setting.enabledVideoSource2) {
		WidgetInfo info = { setting.userInfo2 , false, false };
		bool bFind = false;
		for (int i = 0; i < widgetInfos_.size(); ++i) {
			if (widgetInfos_[i].userInfo.uid == setting.userInfo2.uid) {
				bFind = true;
			}
		}
		if (!bFind)
		  widgetInfos_.push_back(info);
	}
	UpdateShowVideos();
}

void DlgExtend::showEvent(QShowEvent *event)
{
 	SetVideoSource(false);
	if (setting.enabledVideoSource1) {
		rtcEngine->EnableLocalVideo(true);
	}
	if (setting.enabledVideoSource2) {
		connect(AgoraRtcEngine::GetAgoraRtcEngine(), &AgoraRtcEngine::openPlayerComplete,
			this, &DlgExtend::on_openPlayerComplete);
		connect(AgoraRtcEngine::GetAgoraRtcEngine(), &AgoraRtcEngine::playerError,
			this, &DlgExtend::on_playerError);
		rtcEngine->OpenVideoSource2(setting.videoSource2Url);
	}
	QRoundCornerDialog::showEvent(event);
}

void DlgExtend::hideEvent(QHideEvent *event)
{
	QRoundCornerDialog::hideEvent(event);
}

void DlgExtend::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == VK_RETURN) {
		return;
	}

	QDialog::keyPressEvent(event);
}

void DlgExtend::on_openPlayerComplete()
{
	UpdateShowVideos();
	rtcEngine->PlayVideoSource2();
}

void DlgExtend::on_playerError(int ec)
{
	rtcEngine->StopVideoSource2();
	UpdateShowVideos();
	QString strInfo = QString::fromStdWString(L"视频源2开启失败:%1").arg(ec);
	DlgInfo dlg(strInfo, rate, rate2);
	connect(&dlg, &DlgInfo::parentMaxSignal,
		this, &DlgExtend::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	dlg.exec();
	return;
}


void DlgExtend::on_fullScreen(unsigned int uid, bool bFull)
{
	if (!bMax) {
		on_fullMaxButton_clicked(!bFull);
	}

	for (int i = 0; i < widgetsCount; ++i) {
		if (videoWidget[i]->GetUID() == uid) {
			if (bFull) 
				videoWidget[i]->MaximizeWidget(this->width() , this->height());
			else 
				videoWidget[i]->RestoreWidget();
			videoWidget[i]->UpdateButtonPos();
		}
		else {
			if (bFull)
				videoWidget[i]->hide();
			else
				videoWidget[i]->show();
		}
	}
	ShowTopAndBottom(!bFull);
}

void DlgExtend::on_muteVideo(unsigned int uid, bool mute)
{
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		if (uid == widgetInfos_[i].userInfo.uid) {
			widgetInfos_[i].muteVideo = mute;
		}
	}
}

void DlgExtend::on_muteAudio(unsigned int uid, bool mute)
{
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		if (uid == widgetInfos_[i].userInfo.uid) {
			widgetInfos_[i].muteAudio = mute;
		}
	}
}
