#include "agoracourse.h"
#include <QTime>

#include <time.h>
#include "DlgSettings.h"
#include "DlgVideoRoom.h"
#include "DlgExtend.h"
#include "DlgInfo.h"
#include <QScreen>
#include "AgoraRtcEngine.h"

AgoraCourse::AgoraCourse(QWidget* parent)
	: QRoundCornerDialog(parent)
{
	ui.setupUi(this);
	InitDlg();
	
	QString appid = APP_ID;
	if (appid.isEmpty())
	{
		QString strInfo = QString::fromStdWString(L"请输入APPID");
		DlgInfo dlg(strInfo, rate, rate2);
		connect(&dlg, &DlgInfo::parentMaxSignal,
			this, &AgoraCourse::on_parentMax_slot);
		if (!bMax && dlg.IsMax())
			dlg.maxButtonChange(true);
		dlg.exec();
		QApplication::quit();
		return;
	}
	else {
		rtcEngine->Init();
	}
	connect(rtcEngine, &AgoraRtcEngine::joinedChannelSuccess,
		this, &AgoraCourse::onJoinChannelSuccess);

	connect(rtcEngine, &AgoraRtcEngine::leaveChannelSignal,
		this, &AgoraCourse::onLeaveChannel);

	ui.roomNameEdit->setText("dualteacher");
	ui.userNameEdit->setText(QString::fromStdWString(L"老师123"));

	dlgRoom = new DlgVideoRoom(this);
	dlgExtend = new DlgExtend(rate, rate2, dlgRoom,this);
	connect(dlgRoom, &DlgVideoRoom::closeRoom,
		this, &AgoraCourse::onCloseRoom);

	connect(dlgExtend, &DlgExtend::closeRoom,
		this, &AgoraCourse::onCloseRoom);
}

AgoraCourse::~AgoraCourse()
{
}

void AgoraCourse::resizeEvent(QResizeEvent* event)
{
	QDialog::resizeEvent(event);
}

void AgoraCourse::onCloseRoom(bool bExtend)
{
	if (bExtend) {
		dlgRoom->CloseDlg();
	}
	else {
		if (!dlgRoom->isVisible()
			&& dlgExtend->isVisible()) {
			setWindowState(Qt::WindowMaximized);
			dlgExtend->CloseDlg();
		}
	}
	//class room exit
	connect(rtcEngine, &AgoraRtcEngine::joinedChannelSuccess,
		this, &AgoraCourse::onJoinChannelSuccess);
}

void AgoraCourse::on_loginButton_clicked()
{
	QString class_name = ui.roomNameEdit->text();
	QString strInfo = QString::fromStdWString(L"请输入房间名");
	if (class_name.isEmpty()){
		DlgInfo dlg(strInfo, rate, rate2);
		connect(&dlg, &DlgInfo::parentMaxSignal,
			this, &AgoraCourse::on_parentMax_slot);
		if (!bMax && dlg.IsMax())
			dlg.maxButtonChange(true);
		dlg.exec();
		return;
	}

	QByteArray byteString = class_name.toUtf8();
	
	QString user_name = ui.userNameEdit->text();

	strInfo = QString::fromStdWString(L"请输入用户名");
	if (user_name.isEmpty()){
		DlgInfo dlg(strInfo, rate, rate2);
		connect(&dlg, &DlgInfo::parentMaxSignal,
			this, &AgoraCourse::on_parentMax_slot);
		if (!bMax && dlg.IsMax())
			dlg.maxButtonChange(true);
		dlg.exec();
		return;
	}
	byteString = user_name.toUtf8();
	if (setting.userInfo.uid == 0) {
		setting.userInfo.uid = randomUid();
		setting.userInfo2.uid = setting.userInfo.uid + 1;
	}
	setting.userInfo.name = user_name;
	setting.className = class_name;
	setting.userInfo2.name = user_name;
	setting.className = class_name;
	agora::rtc::VideoEncoderConfiguration config;
	config.dimensions.width = setting.resolution[setting.resIndex].width;
	config.dimensions.height = setting.resolution[setting.resIndex].height;
	config.frameRate = setting.frameRate;
	rtcEngine->SetFecParameter(2);
	bool subscribe = !setting.enabledVideoSource1;
	if (setting.enabledVideoSource1)
		rtcEngine->VideoSource1JoinChannel(setting.enabledVideoSource1, "", setting.className.toUtf8(), setting.userInfo.uid, config);
	if (setting.enabledVideoSource2) {
		int ret = rtcEngine->VideoSource2JoinChannel(true, "", setting.className.toUtf8(), setting.userInfo2.uid, subscribe, subscribe);
		if (ret != 0) {
			QString strInfo = QString::fromStdWString(L"视频源2加入房间失败:%1").arg(ret);
			DlgInfo dlg(strInfo, rate, rate2);
			connect(&dlg, &DlgInfo::parentMaxSignal,
				this, &AgoraCourse::on_parentMax_slot);
			if (!bMax && dlg.IsMax())
				dlg.maxButtonChange(true);
			dlg.exec();
			return;
		}
		rtcEngine->SetJoined2(true);
	}
}

// 1 teacher camera 2 teacher desktop 3 student
unsigned int AgoraCourse::randomUid( )
{
	uint r = time(NULL);
	unsigned int uid = r % 100000000;
	return uid;
}

void AgoraCourse::onJoinChannelSuccess(const char* channel, agora::rtc::uid_t uid, int elapsed)
{
	rtcEngine->SetJoined(true);
	showClassRoomDlg();
	disconnect(rtcEngine, &AgoraRtcEngine::joinedChannelSuccess,
		this, &AgoraCourse::onJoinChannelSuccess);
}

void AgoraCourse::onLeaveChannel()
{
	setWindowState(Qt::WindowMaximized);
}

void AgoraCourse::showClassRoomDlg() 
{
	if (setting.bExtend) {
		connect(dlgExtend, &DlgExtend::parentMaxSignal,
			this, &AgoraCourse::on_parentMax_slot);
		dlgExtend->SetRate(rate, rate2);
		if (!bMax && dlgExtend->IsMax())
			dlgExtend->maxButtonChange(true);
		dlgExtend->show();
	}
	else {
		connect(dlgRoom, &DlgVideoRoom::parentMaxSignal,
	     this, &AgoraCourse::on_parentMax_slot);
		dlgRoom->SetRate(rate, rate2);
		if (!bMax && dlgRoom->IsMax())
			dlgRoom->maxButtonChange(true);
		dlgRoom->show();
	}
}