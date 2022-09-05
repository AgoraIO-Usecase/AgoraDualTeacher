#include "DlgSettingVideo.h"
#include<qstyleditemdelegate.h>
#include "SettingsData.h"
#include "AgoraRtcEngine.h"
#include "DlgSettingSelect.h"
#include "DlgInfo.h"
#include "DlgSettings.h"
#include "DlgSettings.h"
DlgSettingVideo::DlgSettingVideo(float initRate, float rate, float rate2, QWidget* course, QWidget *parent, bool bSecond)
	: QRoundCornerDialog(parent)
{
	ui.setupUi(this);
	this->bSecond = bSecond;
	this->initRate = initRate;
	this->rate2 = rate2;
	this->rate = rate;
	
	UpdateVideoDeviceInfo();

	InitDlg();
	
	SetVideoEncoder();
	UpdateCtrls();
	//rtcEngine->GetEngine()
	//int getCapability(const char* deviceIdUTF8, const uint32_t deviceCapabilityNumber, VideoFormat & capability)
	if (setting.enabledVideoSource2 && rtcEngine->IsJoined2() ) {
		rtcEngine->ShowVideoSource2((HWND)ui.widgetVideo2->winId());
	}
	else if (setting.enabledVideoSource2 ) {
		rtcEngine->OpenVideoSource2(ui.lineEditVideoSource2->text());
	}
	else {

	}
	agoraCourse = course;
}

DlgSettingVideo::~DlgSettingVideo()
{
}

void DlgSettingVideo::onCancel()
{
	if (setting.enabledVideoSource1) {
		SetVideoEncoder();
		rtcEngine->LocalVideoPreview((HWND)ui.widgetVideo1->winId(), false);
	}
	if (setting.enabledVideoSource2) {
		if (!rtcEngine->IsJoined() && !rtcEngine->IsJoined2())
			rtcEngine->StopVideoSource2();
	}
	setting.videoSource2Url = ui.lineEditVideoSource2->text();
	close();
}

void DlgSettingVideo::UpdateCtrls()
{
	// video
	videoInfos_.clear();
	int count = rtcEngine->GetVideoDeviceCount();
	if (count > 0) {
		QString curId = rtcEngine->GetCurVideoDeviceID();
		for (int i = 0; i < count; ++i) {
			DeviceInfo info;
			rtcEngine->GetVideoDevice(i, info.name, info.id);
			videoInfos_.insert(info.id, info.name);
		}

		if (setting.videoSource1Id.isEmpty()) {
			setting.videoSource1Id = curId;
		}
		QString name = videoInfos_[setting.videoSource1Id];
		ui.btnVideoSource1->setText(videoInfos_[setting.videoSource1Id]);

		if (setting.enabledVideoSource1)
			rtcEngine->LocalVideoPreview((HWND)ui.widgetVideo1->winId(), true);
	}
}

void DlgSettingVideo::UpdateVideoDeviceInfo()
{
	fpsInfos_.clear();
	setting.resolution.clear();
	videoDeviceInfos_.clear();
	int64_t start_time = GetTickCount64();
	rtcEngine->GetVideoDeviceInfo(videoDeviceInfos_);
	int64_t end_time = GetTickCount64();
	QString str = QString("intervals:%1").arg(end_time - start_time);
	qDebug() << str << "\n";

	int curIndex = -1;
	bool bFindFrameRate = false;
	for (int i = 0; i < videoDeviceInfos_.size(); ++i) {
		QString strFPS = QString("%1").arg(videoDeviceInfos_[i].fps);
		fpsInfos_.insert(strFPS, strFPS + "fps");
		Resolution res = { videoDeviceInfos_[i].width, videoDeviceInfos_[i].height };
		setting.resolution.push_back(res);
		if (videoDeviceInfos_[i].fps == setting.frameRate)
			bFindFrameRate = true;
	}

	if (videoDeviceInfos_.size() > 0) {
		if (!bFindFrameRate)
			setting.frameRate = videoDeviceInfos_[0].fps;
		if (setting.resIndex >= videoDeviceInfos_.size())
			setting.resIndex = 0;
	}
	SetResolution();
	QString strFPS = QString("%1").arg(videoDeviceInfos_[0].fps);
	ui.labVideo1FPSInfo->setText(fpsInfos_[QString("%1").arg(setting.frameRate)]);
}

void DlgSettingVideo::on_btnVideoSource1_clicked()
{
	DlgSettingSelect dlg(initRate, rate, rate2, bSecond, videoInfos_, setting.videoSource1Id, SELECT_TYPE_VIDEO1, QString::fromStdWString(L"视频源"));
	connect(&dlg, &DlgSettingSelect::parentMaxSignal,
		this, &DlgSettingVideo::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	dlg.exec();
	UpdateVideoDeviceInfo();
	ui.btnVideoSource1->setText(videoInfos_[setting.videoSource1Id]);
}

void DlgSettingVideo::SetResolution()
{
	ui.labVideo1ResInfo->setText(QString("%1*%2")
		.arg(setting.resolution[setting.resIndex].width)
		.arg(setting.resolution[setting.resIndex].height));

}

void DlgSettingVideo::on_btnVideo1Res_clicked()
{
	
}
void DlgSettingVideo::on_btnVideo1StatsPre_clicked() {
	if (!setting.enabledVideoSource1)
		return;
	ui.labVideo1Stat->setText(QString::fromStdWString(L"关闭"));
	ui.labVideo1Stats->setText(QString::fromStdWString(L"关闭"));
	setting.enabledVideoSource1 = false;
	rtcEngine->LocalVideoPreview(NULL, false);
	rtcEngine->EnableLocalVideo(false);
	ui.widgetVideo1->update();
	
}
void DlgSettingVideo::on_btnVideo1StatsNext_clicked()
{
	if (setting.enabledVideoSource1)
		return;
	ui.labVideo1Stat->setText(QString::fromStdWString(L"开启"));
	ui.labVideo1Stats->setText(QString::fromStdWString(L"开启"));
	setting.enabledVideoSource1 = true;
	rtcEngine->EnableLocalVideo(true);
	rtcEngine->LocalVideoPreview((HWND)ui.widgetVideo1->winId(), true);
}

void DlgSettingVideo::on_btnvideo2StatsPre_clicked() 
{
	if (!setting.enabledVideoSource2)
		return;
	ui.labvideo2Stat->setText(QString::fromStdWString(L"关闭"));
	ui.labvideo2Stats->setText(QString::fromStdWString(L"关闭"));
	rtcEngine->StopVideoSource2();
	setting.enabledVideoSource2 = false;
	ui.widgetVideo2->update();
	
}

void DlgSettingVideo::on_btnvideo2StatsNext_clicked()
{
	if (setting.enabledVideoSource2)
		return;
	if (ui.lineEditVideoSource2->text().isEmpty()) {
		QString strInfo = QString::fromStdWString(L"请输入视频源2的地址");
		DlgInfo dlg(strInfo, rate, rate2);
		connect(&dlg, &DlgInfo::parentMaxSignal,
			this, &DlgSettingVideo::on_parentMax_slot);
		if (!bMax && dlg.IsMax())
			dlg.maxButtonChange(true);
		dlg.exec();
		return;
	}
	int ret = rtcEngine->OpenVideoSource2(ui.lineEditVideoSource2->text());
	//if (!rtcEngine->IsJoined() && !rtcEngine->IsJoined()) {
		connect(AgoraRtcEngine::GetAgoraRtcEngine(), &AgoraRtcEngine::playerError,
			this, &DlgSettingVideo::on_playerError);
//	}
	
	//rtcEngine->LocalVideoPreview((HWND)ui.widgetVideo2->winId(), true);
}

void DlgSettingVideo::on_playerError(int ec)
{
	rtcEngine->StopVideoSource2();
	QString strInfo = QString::fromStdWString(L"视频源2开启失败:%1").arg(ec);
	DlgInfo dlg(strInfo, rate, rate2, initRate, bSecond);
	connect(&dlg, &DlgInfo::parentMaxSignal,
		this, &DlgSettingVideo::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	dlg.exec();
}

void DlgSettingVideo::SetVideoEncoder()
{
	agora::rtc::VideoEncoderConfiguration config;
	config.dimensions.width = setting.resolution[setting.resIndex].width;
	config.dimensions.height = setting.resolution[setting.resIndex].height;
	config.frameRate = setting.frameRate;
	if (rtcEngine->IsJoined() && setting.enabledVideoSource1)
		rtcEngine->SetVideoEncoderConfigurationEx(config);
}

void DlgSettingVideo::on_btnVideo1ResPre_clicked()
{
	if (setting.resIndex == 0 )
		return;
	--setting.resIndex;
	if (setting.resIndex == 0) {
		setting.frameRate = 30;
		ui.labVideo1FPSInfo->setText(fpsInfos_[QString("%1").arg(setting.frameRate)]);
	}
		 
	SetResolution();
	SetVideoEncoder();
}

void DlgSettingVideo::on_btnVideo1ResNext_clicked()
{
	if (setting.resIndex == setting.resolution.size() - 1)
		return;
	++setting.resIndex;
	SetResolution();
	SetVideoEncoder();
}


void DlgSettingVideo::on_btnVideo1FPSPre_clicked()
{
	if (setting.frameRate == 30 ||
		(setting.resolution[setting.resIndex].width == 3840 &&
			setting.resolution[setting.resIndex].height == 2160 &&
			setting.frameRate == 60))
		return;
	setting.frameRate = 30;
	ui.labVideo1FPSInfo->setText(fpsInfos_[QString("%1").arg(setting.frameRate)]);
	SetVideoEncoder();
}
void DlgSettingVideo::on_btnVideo1FPSNext_clicked() 
{
	if (setting.frameRate == 60 ||
		(setting.resolution[setting.resIndex].width == 1280 &&
			setting.resolution[setting.resIndex].height == 720 &&
			setting.frameRate == 30)) {
		return;
	}
	
	if (fpsInfos_.find("60fps") == fpsInfos_.end())
		return;

	setting.frameRate = 60;
	
	ui.labVideo1FPSInfo->setText(fpsInfos_[QString("%1").arg(setting.frameRate)]);
	SetVideoEncoder();
}

void DlgSettingVideo::on_openPlayerComplete()
{
	ui.labvideo2Stats->setText(QString::fromStdWString(L"开启"));
	ui.labvideo2Stat->setText(QString::fromStdWString(L"开启"));
	setting.enabledVideoSource2 = true;

	rtcEngine->ShowVideoSource2((HWND)ui.widgetVideo2->winId());
	rtcEngine->PlayVideoSource2();
}
void DlgSettingVideo::on_parentMax_slot(bool childMax)
{
	if (bMax != childMax) {
		on_maxButton_clicked();
	}
}