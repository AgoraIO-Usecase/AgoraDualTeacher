#include "DlgSettingAudio.h"
#include "DlgSettingSelect.h"
#include<qstyleditemdelegate.h>

#include "AgoraRtcEngine.h"
DlgSettingAudio::DlgSettingAudio(float initRate, float rate, float rate2, QWidget* course, QWidget* parent, bool bSecond)
	: QRoundCornerDialog(parent)
{
	ui.setupUi(this);
	this->bSecond = bSecond;
	this->initRate = initRate;
	this->rate2 = rate2;
	this->rate = rate;
	InitDlg();
	UpdateCtrls();
	agoraCourse = course;
}

DlgSettingAudio::~DlgSettingAudio()
{

}


void DlgSettingAudio::UpdateCtrls()
{
	// Audio
	micInfos_.clear();
	speakerInfos_.clear();
	int count = rtcEngine->GetMicCount();
	if (count > 0) {
		QString currentMicId = rtcEngine->GetCurMicID();
		for (int i = 0; i < count; ++i) {
			DeviceInfo info;
			rtcEngine->GetMic(i, info.name, info.id);
			micInfos_.insert(info.id, info.name);
		}

		if (setting.microphoneId.isEmpty()) {
			setting.microphoneId = currentMicId;
		}
		QString name = micInfos_[setting.microphoneId];
		ui.btnMicSelect->setText(micInfos_[setting.microphoneId]);
	}

	count = rtcEngine->GetSpeakerCount();
	if (count > 0) {
		QString currentSpeakerId = rtcEngine->GetCurSpeakerID();
		for (int i = 0; i < count; ++i) {
			DeviceInfo info;
			rtcEngine->GetSpeaker(i, info.name, info.id);
			speakerInfos_.insert(info.id, info.name);
		}
		if (setting.speakerId.isEmpty()) {
			setting.speakerId = currentSpeakerId;
		}
		QString name = speakerInfos_[setting.speakerId];
		ui.btnSpeakerSelect->setText(speakerInfos_[setting.speakerId]);
	}
}

void DlgSettingAudio::on_closeButton_clicked()
{
	onExit();
}

void DlgSettingAudio::onExit()
{
	if (testMic_)
		rtcEngine->TestMicDevice(false);
	close();
}

void DlgSettingAudio::on_btnBack_clicked()
{
	close();
}
	
void DlgSettingAudio::on_btnTestMic_clicked()
{
	testMic_ = !testMic_;
	if (testMic_) {
		connect(rtcEngine,
			&AgoraRtcEngine::volumeIndication,
			this, &DlgSettingAudio::onVolumeIndication_slot);
	}
	else {
		disconnect(rtcEngine,
			&AgoraRtcEngine::volumeIndication,
			this, &DlgSettingAudio::onVolumeIndication_slot);
		ui.progressBarMicVolume->setValue(0);
	}
	rtcEngine->TestMicDevice(testMic_);
	testMic_ ? ui.btnTestMic->setText(QString::fromStdWString(L"停止"))
		: ui.btnTestMic->setText(QString::fromStdWString(L"测试"));
}

void DlgSettingAudio::on_btnTestSpeaker_clicked()
{
	testSpeaker_ = !testSpeaker_;
	rtcEngine->TestSpeakerDevice(testSpeaker_);

	testSpeaker_ ? ui.btnTestSpeaker->setText(QString::fromStdWString(L"停止"))
		: ui.btnTestSpeaker->setText(QString::fromStdWString(L"测试"));
}

void DlgSettingAudio::onVolumeIndication_slot(unsigned int volume, unsigned int speakerNumber, int totalVolume)
{
	ui.progressBarMicVolume->setValue(volume);
}

void DlgSettingAudio::on_btnMicSelect_clicked()
{
	DlgSettingSelect dlg(initRate, rate, rate2, bSecond, micInfos_, setting.microphoneId, SELECT_TYPE_MIC, QString::fromStdWString(L"麦克风"));
	connect(&dlg, &DlgSettingSelect::parentMaxSignal,
		this, &DlgSettingAudio::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	dlg.exec();
	ui.btnMicSelect->setText(micInfos_[setting.microphoneId]);
}

void DlgSettingAudio::on_btnSpeakerSelect_clicked()
{
	DlgSettingSelect dlg(initRate, rate, rate2, bSecond, speakerInfos_, setting.speakerId, SELECT_TYPE_SPEAKER, QString::fromStdWString(L"扬声器"));
	connect(&dlg, &DlgSettingSelect::parentMaxSignal,
		this, &DlgSettingAudio::on_parentMax_slot);
	if (!bMax && dlg.IsMax())
		dlg.maxButtonChange(true);
	dlg.exec();
	ui.btnSpeakerSelect->setText(speakerInfos_[setting.speakerId]);
}

void DlgSettingAudio::on_btnAGCStatsPre_clicked()
{
	if (!setting.agcOn)
		return;
	ui.labelAGCStats->setText(QString::fromStdWString(L"关闭"));
	setting.agcOn = false;
	rtcEngine->EnableAGC(setting.agcOn);

}
void DlgSettingAudio::on_btnAGCStatsNext_clicked()
{
	if (setting.agcOn)
		return;
	ui.labelAGCStats->setText(QString::fromStdWString(L"开启"));
	setting.agcOn = true;
	rtcEngine->EnableAGC(setting.agcOn);
}

void DlgSettingAudio::on_btnAECStatsPre_clicked()
{
	if (!setting.aecOn)
		return;
	ui.labelAECStats->setText(QString::fromStdWString(L"关闭"));
	setting.aecOn = false;
	rtcEngine->EnableAGC(setting.aecOn);
}
void DlgSettingAudio::on_btnAECStatsNext_clicked()
{
	if (setting.aecOn)
		return;
	ui.labelAECStats->setText(QString::fromStdWString(L"开启"));
	setting.aecOn = true;
	rtcEngine->EnableAGC(setting.aecOn);
}

void DlgSettingAudio::on_btnANSStatsPre_clicked()
{
	if (!setting.ansOn) 
		return;
	ui.labelANSStats->setText(QString::fromStdWString(L"关闭"));
	setting.ansOn = false;
	rtcEngine->EnableAGC(setting.ansOn);
}

void DlgSettingAudio::on_btnANSStatsNext_clicked()
{
	if (setting.ansOn)
		return;
	ui.labelANSStats->setText(QString::fromStdWString(L"开启"));
	setting.ansOn = true;
	rtcEngine->EnableAGC(setting.ansOn);
}