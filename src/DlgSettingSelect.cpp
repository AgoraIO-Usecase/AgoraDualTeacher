#include "DlgSettingSelect.h"
#include<qstyleditemdelegate.h>
#include "SettingsData.h"
#include "AgoraRtcEngine.h"
#include <QFont>
#include <QFontMetrics>
#include <QDebug>
DlgSettingSelect::DlgSettingSelect(float initRate, float rate, float rate2, bool bSecond, QHash<QString, QString> items, QString selectedId, SELECT_TYPE type, QString title, QWidget *parent)
	: QRoundCornerDialog(parent)
	, selectedId_(selectedId)
	, select_type_(type)
	, title_(title)
{
	items_ = items;
	ui.setupUi(this);
	this->rate2 = rate2;
	this->rate = rate;
	this->bSecond = bSecond;
	this->initRate = initRate;
	InitDlg();
}

DlgSettingSelect::~DlgSettingSelect()
{

}


void DlgSettingSelect::radioItemClicked()
{
	QRadioButton* btn = (QRadioButton*)sender();
	selectedId_ = itemButtons_[btn];

	switch (select_type_) {
	case SELECT_TYPE_VIDEO1: {
		qDebug() << "id:" << selectedId_;
		qDebug() << "\n";
		rtcEngine->SetCurVideoDevice(selectedId_);
		
		setting.videoSource1Id = selectedId_;

	}
						   break;
	case SELECT_TYPE_MIC: {
		rtcEngine->SetCurMic(selectedId_);
		setting.microphoneId = selectedId_;
	}
						break;
	case SELECT_TYPE_SPEAKER: {
		rtcEngine->SetCurSpeaker(selectedId_);
		setting.speakerId = selectedId_;
	}
						break;
	default:
		break;
	}
}