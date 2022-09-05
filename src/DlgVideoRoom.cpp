#include "DlgVideoRoom.h"
#include<qstyleditemdelegate.h>
#include <QMap>
#include "DlgInfo.h"
#include "VideoWidget.h"
#include "DlgSettings.h"
#include "SettingsData.h"
#include "AgoraRtcEngine.h"
#include <QDebug>
#include "agoracourse.h"
#include "DlgSettings.h"
#include "DlgExtend.h"
DlgVideoRoom::DlgVideoRoom(QWidget* parent)
	: QRoundCornerDialog(parent)
{
	ui.setupUi(this);
	InitDlg();
	agoraCourse = parent;
}
void DlgVideoRoom::SetRate(float rate, float rate2)
{
	this->rate2 = rate2;
	this->rate = rate;
}
DlgVideoRoom::~DlgVideoRoom()
{
	disconnect(AgoraRtcEngine::GetAgoraRtcEngine(), &AgoraRtcEngine::openPlayerComplete,
		this, &DlgVideoRoom::on_openPlayerComplete);
	disconnect(AgoraRtcEngine::GetAgoraRtcEngine(), &AgoraRtcEngine::playerError,
		this, &DlgVideoRoom::on_playerError);
}

void DlgVideoRoom::UpdateShowVideos()
{
	if (widgetInfos_.size() > 4)
		showPage(true);
	else
		showPage(false);
	QMap<unsigned int, VideoWidget*> map;
	int firstIndex = curPage * widgetsCount;
	
	for (int i = firstIndex; i < widgetInfos_.size() && i < (firstIndex + widgetsCount); ++i) {
		int index = i % widgetsCount;
		if (widgetInfos_[i].userInfo.uid == setting.userInfo2.uid)
			rtcEngine->PauseVideoSource2(false);
		videoWidget[index]->SetWidgetInfo(widgetInfos_[i]);
		map.insert(widgetInfos_[i].userInfo.uid, videoWidget[index]);
	}
	rtcEngine->SetVideoWidget(map);
}

void DlgVideoRoom::ShowWidgets()
{
	if (widgetInfos_.size() < 3) {
		videoWidget[2]->hide();
		videoWidget[3]->hide();
		update();
	}
	else if (widgetInfos_.size() < 5) {
		videoWidget[2]->show();
		videoWidget[3]->show();
	}
}

void DlgVideoRoom::ResetWidgets()
{
	for (int i = 0; i < widgetsCount; ++i) {
		if (videoWidget[i]->GetUID() == setting.userInfo2.uid)
			rtcEngine->PauseVideoSource2(true);
		videoWidget[i]->Reset();
	}
}

void DlgVideoRoom::RequestUserName(unsigned int uid)
{
	if (rtcEngine->IsJoined()) {
		QString message = QString("%1:%2").arg(cmdRequestUserName).arg(setting.userInfo.name);
		rtcEngine->VideoSource1SendStreamMessage(message);
	}

	if (rtcEngine->IsJoined2()) {
		QString message = QString("%1:%2").arg(cmdRequestUserName).arg(setting.userInfo2.name);
		rtcEngine->VideoSource2SendStreamMessage(message);
	}
}

void DlgVideoRoom::on_minButton_clicked()
{
	agoraCourse->setWindowState(Qt::WindowMinimized);
}

void DlgVideoRoom::on_closeButton_clicked()
{
	QString strInfo = QString::fromStdWString(L"确认退出房间么");
	DlgInfo dlg(strInfo, rate, rate);
	connect(&dlg, &DlgInfo::parentMaxSignal,
		this, &DlgVideoRoom::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true, true);
	
	if (dlg.exec() == QDialog::Accepted) {
		rtcEngine->ResetVideoWidgets();
		if (setting.enabledVideoSource2) {
			rtcEngine->StopVideoSource2();
			rtcEngine->SetJoined2(false);
			rtcEngine->VideoSourceLeave(setting.className.toUtf8(), setting.userInfo2.uid);
		}
		rtcEngine->VideoSourceLeave(setting.className.toUtf8(), setting.userInfo.uid);
		rtcEngine->SetJoined(false);
		CloseDlg();
		emit closeRoom(false);
		dlg.close();
	}
}

void DlgVideoRoom::CloseDlg()
{
	curPage = 0;
	ResetWidgets();
	widgetInfos_.clear();
	videoWidget[2]->hide();
	videoWidget[3]->hide();
	hide();
	rtcEngine->ResetVideoWidgets();
}

void DlgVideoRoom::on_settingDlg_close()
{
	if (setting.enabledVideoSource1) {
		if (!rtcEngine->IsJoined()) {
			rtcEngine->SetJoined(true);
			agora::rtc::VideoEncoderConfiguration config;
			config.dimensions.width = setting.resolution[setting.resIndex].width;
			config.dimensions.height = setting.resolution[setting.resIndex].height;
			config.frameRate = setting.frameRate;
			rtcEngine->VideoSource1JoinChannel(setting.enabledVideoSource1, "", setting.className.toUtf8(), setting.userInfo.uid, config);
			onUserJoined(setting.userInfo.uid, 0);
		}
	}
	else {
		if (rtcEngine->IsJoined()) {
			rtcEngine->SetJoined(false);
			rtcEngine->VideoSourceLeave(setting.className.toUtf8(), setting.userInfo.uid);
			onUserOffline(setting.userInfo.uid, 0);
		}
	}

	if (setting.enabledVideoSource2) {
		if (!rtcEngine->IsJoined2()) {
			rtcEngine->SetJoined2(true);
			bool subscribe = !rtcEngine->IsvideoSource1Subscribe();
			rtcEngine->VideoSource2JoinChannel(setting.enabledVideoSource2, "", setting.className.toUtf8(), setting.userInfo2.uid, subscribe, subscribe);
			onUserJoined(setting.userInfo2.uid, 0);
		}
		else if (!rtcEngine->IsJoined()) {//
			rtcEngine->UpdateVideoSource2Subscribe(true);
		}
	}
	else {
		if (rtcEngine->IsJoined2()) {
			rtcEngine->SetJoined2(false);
			rtcEngine->VideoSourceLeave(setting.className.toUtf8(), setting.userInfo2.uid);
			onUserOffline(setting.userInfo2.uid, 0);
		}
	}
}

void DlgVideoRoom::on_settingsButton_clicked()
{
	if (!dlgSettings) {
		dlgSettings = new DlgSettings(initRate, setting.bExtend, rate, rate2, agoraCourse, this);
		if (!setting.bExtend)
			connect(dlgSettings, &DlgSettings::closeSettingDlg,
				this, &DlgVideoRoom::on_settingDlg_close);
	}
	dlgSettings->show();
}

void DlgVideoRoom::on_btnPrePage_clicked()
{
	if (curPage == 0)
		return;
	curPage--;
	rtcEngine->ResetVideoWidgets();
	ResetWidgets();
	UpdateShowVideos();
}

void DlgVideoRoom::on_btnNextPage_clicked()
{
	int totalPage = widgetInfos_.size() / widgetsCount + (widgetInfos_.size() % widgetsCount != 0);
	if (curPage == totalPage  - 1)
		return;
	curPage++;
	rtcEngine->ResetVideoWidgets();
	ResetWidgets();
	ShowWidgets();
	UpdateShowVideos();
}

void DlgVideoRoom::showEvent(QShowEvent *event)
{
	QRoundCornerDialog::showEvent(event);
	if (setting.bExtend) {
		return;
	}
	
	if (setting.enabledVideoSource1) {
		bool bFind = false;
		for (int i = 0; i < widgetInfos_.size(); ++i) {
			if (widgetInfos_[i].userInfo.uid == setting.userInfo.uid) {
				bFind = true;
				return;
			}
		}
		if (!bFind) {
			WidgetInfo info = { setting.userInfo , false, false };
			widgetInfos_.push_back(info);
		}
	}

	if (setting.enabledVideoSource2) {
		bool bFind = false;
		for (int i = 0; i < widgetInfos_.size(); ++i) {
			if (widgetInfos_[i].userInfo.uid == setting.userInfo2.uid) {
				bFind = true;
				return;
			}
		}
		if (!bFind) {
			WidgetInfo info = { setting.userInfo2 , false, false };
			widgetInfos_.push_back(info);
		}
	}

	if (setting.enabledVideoSource2) {
		rtcEngine->OpenVideoSource2(setting.videoSource2Url);
		connect(AgoraRtcEngine::GetAgoraRtcEngine(), &AgoraRtcEngine::openPlayerComplete,
			this, &DlgVideoRoom::on_openPlayerComplete);
		connect(AgoraRtcEngine::GetAgoraRtcEngine(), &AgoraRtcEngine::playerError,
			this, &DlgVideoRoom::on_playerError);
	}
	else
		UpdateShowVideos();
}

void DlgVideoRoom::hideEvent(QHideEvent *event)
{
	//QRoundCornerDialog::hideEvent(event);
}
void DlgVideoRoom::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == VK_RETURN) {
		return;
	}

	QDialog::keyPressEvent(event);
}
void DlgVideoRoom::on_openPlayerComplete()
{
	UpdateShowVideos();
	rtcEngine->PlayVideoSource2();
}

void DlgVideoRoom::on_playerError(int ec)
{
	rtcEngine->StopVideoSource2();
	UpdateShowVideos();
	QString strInfo = QString::fromStdWString(L"视频源2开启失败:%1").arg(ec);
	DlgInfo dlg(strInfo, rate, rate2);
	connect(&dlg, &DlgInfo::parentMaxSignal,
		this, &DlgVideoRoom::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	dlg.exec();
	return;
}

void DlgVideoRoom::on_fullScreen(unsigned int uid, bool bFull)
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
			else if (i < 2)
				videoWidget[i]->show();
			else if(i > 1)
				widgetInfos_.size() > 2 ? videoWidget[i]->show() : videoWidget[i]->hide();
				
		}
	}
	ShowTopAndBottom(!bFull);
}

void DlgVideoRoom::on_muteVideo(unsigned int uid, bool mute)
{
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		if (uid == widgetInfos_[i].userInfo.uid) {
			widgetInfos_[i].muteVideo = mute;
		}
	}
}

void DlgVideoRoom::on_muteAudio(unsigned int uid, bool mute)
{
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		if (uid == widgetInfos_[i].userInfo.uid) {
			widgetInfos_[i].muteAudio = mute;
		}
	}
}

void DlgVideoRoom::onUserJoined(unsigned int uid, int elapsed)
{
	if (!isVisible())
		show();
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		qDebug() << "onUserJoined: uid " << widgetInfos_[i].userInfo.uid << ", name " << widgetInfos_[i].userInfo.name;
	}
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		if (widgetInfos_[i].userInfo.uid == uid) {
			qDebug() << "onUserJoined already exist:" << uid << "\n";
			return;
		}
	}

	UserInfo userInfo = { uid, "" };
	if (uid == setting.userInfo.uid)
		userInfo = setting.userInfo;
	else if (uid == setting.userInfo2.uid)
		userInfo = setting.userInfo2;

	WidgetInfo info = { userInfo , false, false };
	widgetInfos_.push_back(info);
	if (uid == setting.userInfo2.uid && widgetInfos_.size() > 2) {
		for (int i = widgetInfos_.size() - 1; i > 1; --i) {
			widgetInfos_[i] = widgetInfos_[i - 1];
		}
		WidgetInfo info = { setting.userInfo2, false, false };
		widgetInfos_[1] = info;
	}else if (uid == setting.userInfo.uid && widgetInfos_.size() > 1) {
		for (int i = widgetInfos_.size() - 1; i > 0; --i) {
			widgetInfos_[i] = widgetInfos_[i - 1];
		}
		WidgetInfo info = { setting.userInfo, false, false };
		widgetInfos_[0] = info;
	}
	
	if (uid != setting.userInfo.uid && uid != setting.userInfo2.uid)
		RequestUserName(uid);
	rtcEngine->ResetVideoWidgets();
	ShowWidgets();
	UpdateLayout();
	UpdateShowVideos();
}

void DlgVideoRoom::onUserOffline(unsigned int uid, int elapsed)
{ 
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		qDebug() << "onUserOffline: uid " << widgetInfos_[i].userInfo.uid << ", name " << widgetInfos_[i].userInfo.name;
	}
	int leaveIndex = -1;
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		if (widgetInfos_[i].userInfo.uid == uid) {
			widgetInfos_.removeAt(i);
			leaveIndex = i;
			break;
		}
	}

	int leavePage = leaveIndex / widgetsCount;
	if (leavePage <= curPage) {
		if (widgetInfos_.size() > 0 && widgetInfos_.size() % widgetsCount == 0) {
			on_btnPrePage_clicked();
		}
		else {
			ResetWidgets();
			
			UpdateShowVideos();
			ShowWidgets();
			UpdateLayout();
		}
	}

	for (int i = 0; i < widgetInfos_.size(); ++i) {
		qDebug() << "onUserOffline: uid " << widgetInfos_[i].userInfo.uid << ", name " << widgetInfos_[i].userInfo.name;
	}
	bool bShow = false;
	for (int i = 0; i < widgetInfos_.size(); ++i) {
		if (widgetInfos_[i].userInfo.uid != 0) {
			bShow = true;
			break;
		}
	}

	if (!bShow) hide();
}

void DlgVideoRoom::onStreamMessage(unsigned uid, QString message)
{
	if (message.indexOf(cmdRequestUserName) == 0) {
		qsizetype pos = message.indexOf(":");
		QString strName = message.mid(pos + 1);
		std::wstring str = strName.toStdWString();
		WidgetInfo info;
		for (int i = 0; i < widgetInfos_.size(); ++i) {
			if (widgetInfos_[i].userInfo.uid == uid) {
				widgetInfos_[i].userInfo.name = QString::fromStdWString(str);
				info = widgetInfos_[i];
			}
		}

		for (int i = 0; i < widgetsCount; ++i) {
			if (videoWidget[i]->GetUID() == uid) {
				videoWidget[i]->SetWidgetInfo(info);
			}
		}
	}
}