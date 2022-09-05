#include "AgoraRtcEngine.h"

#include <QDebug>
#include<QMessageBox>
#include "DlgSettings.h"  
#include <QCoreApplication>
#include <VideoWidget.h>
#include <QMutexLocker>
//#include "agora_log.h"
//#include <mutex>
//#include <thread>

class AgoraRtcEngineEvent : public agora::rtc::IRtcEngineEventHandler
{
	AgoraRtcEngine* m_engine;
public:
	bool bEx = false;
	AgoraRtcEngineEvent(AgoraRtcEngine* engine)
		:m_engine(engine)
	{}
	virtual void onVideoStopped() override
	{
		//emit m_engine.videoStopped();
	}
	virtual void onJoinChannelSuccess(const char* channel, agora::rtc::uid_t uid, int elapsed) override
	{
		emit m_engine->joinedChannelSuccess(channel, uid, elapsed);
	}
	
	virtual void onUserJoined(agora::rtc::uid_t uid, int elapsed) override
	{
		if (uid == setting.userInfo.uid || uid == setting.userInfo2.uid)
			return;
		unsigned int userId = (unsigned int)uid;
		//if (!bEx)
			emit m_engine->userJoined(userId, elapsed);
	}
	virtual void onUserOffline(agora::rtc::uid_t uid, agora::rtc::USER_OFFLINE_REASON_TYPE reason) override
	{
		if (!bEx)
			emit m_engine->userOffline(uid, reason);
	}
	virtual void onFirstLocalVideoFrame(int width, int height, int elapsed) override
	{
		//emit m_engine.firstLocalVideoFrame(width, height, elapsed);
	}
	virtual void onFirstRemoteVideoDecoded(agora::rtc::uid_t uid, int width, int height, int elapsed) override
	{
		//emit m_engine.firstRemoteVideoDecoded(uid, width, height, elapsed);
	}
	virtual void onFirstRemoteVideoFrame(agora::rtc::uid_t uid, int width, int height, int elapsed) override
	{
		//emit m_engine.firstRemoteVideoFrameDrawn(uid, width, height, elapsed);
	}

	virtual void onAudioVolumeIndication(const agora::rtc::AudioVolumeInfo* speakers, unsigned int speakerNumber, int totalVolume)
	{
		if (m_engine)
			emit m_engine->volumeIndication(totalVolume, speakerNumber, totalVolume);
	}

	virtual void onStreamMessage(agora::rtc::uid_t userId, int streamId, const char* data, size_t length, uint64_t sentTs) override {
		if (m_engine) {
			emit m_engine->streamMessage(userId, QString::fromStdString(data));
		}
	}

	virtual void onLeaveChannel(const agora::rtc::RtcStats& stats) {
		if (!bEx)
			emit m_engine->leaveChannelSignal();
	}
};

class AgoraRtcEngineEventEx : public agora::rtc::IRtcEngineEventHandlerEx
{
	AgoraRtcEngine* m_engine;
public:
	bool bEx = false;
	virtual void onJoinChannelSuccess(const agora::rtc::RtcConnection& connection, int elapsed) override
	{
		emit m_engine->joinedChannelSuccess(connection.channelId, connection.localUid, elapsed);
	}
};

AgoraRtcEngine AgoraRtcEngine::agoraRtcEngine = nullptr;
agora::rtc::IRtcEngine* AgoraRtcEngine::m_rtcEngine = nullptr;
agora::rtc::IRtcEngineEx* AgoraRtcEngine::m_rtcEngineEx = nullptr;
AgoraRtcEngineEvent AgoraRtcEngine::m_eventHandler = nullptr;

AgoraRtcEngineEvent m_eventHandler2 = nullptr;

AgoraRtcEngine::AgoraRtcEngine(QObject *parent)
	: QObject(parent)
{
	InitVideoFrame();
}

AgoraRtcEngine::~AgoraRtcEngine()
{
	RegisterVideoFrameObserver(false);
	if (media_player_) {
		media_player_->registerPlayerSourceObserver(nullptr);
		m_rtcEngine->destroyMediaPlayer(media_player_);
		media_player_ = nullptr;
	}
	m_rtcEngine->release();
}

void AgoraRtcEngine::InitVideoFrame()
{
	/*for (int i = 0; i < VIDEO_COUNT; ++i) {
		agora::media::base::VideoFrame* videoFrame = new agora::media::base::VideoFrame();
		videoFrame->yStride = 3840;
		videoFrame->uStride = 1920;
		videoFrame->vStride = 1920;
		videoFrame->width = 3840;
		videoFrame->height = 2160;
		videoFrame->yBuffer = new uint8_t[videoFrame->yStride * videoFrame->height];
		videoFrame->uBuffer = new uint8_t[videoFrame->uStride * videoFrame->height / 2];
		videoFrame->vBuffer = new uint8_t[videoFrame->vStride * videoFrame->height / 2];
		videoFrame->type = agora::media::base::VIDEO_PIXEL_I420;
		videoFrames_.push_back(videoFrame);
	}*/
}

AgoraRtcEngine* AgoraRtcEngine::GetAgoraRtcEngine()
{
	if (m_rtcEngine == NULL) {
		m_rtcEngine = createAgoraRtcEngine();
	}
	else
		return &agoraRtcEngine;
	return &agoraRtcEngine;
}

int AgoraRtcEngine::Init()
{
	m_eventHandler = AgoraRtcEngineEvent(&agoraRtcEngine);
	m_eventHandler2 = AgoraRtcEngineEvent(&agoraRtcEngine);
	agora::rtc::RtcEngineContext context;
	context.appId = APP_ID;
	context.eventHandler = &m_eventHandler;
	int ret = m_rtcEngine->initialize(context);
	if (ret != 0) {
		m_rtcEngine->release(true);
		return -1;
	}
	
	m_rtcEngineEx = (agora::rtc::IRtcEngineEx*)m_rtcEngine;

	media_player_ = m_rtcEngine->createMediaPlayer();
	media_player_->registerPlayerSourceObserver(this);
	RegisterVideoFrameObserver(true);
	return ret;
}

agora::rtc::IRtcEngineEx* AgoraRtcEngine::GetEngine()
{
	if (m_rtcEngine == NULL) {
		m_rtcEngine = createAgoraRtcEngine();
		m_rtcEngineEx = (agora::rtc::IRtcEngineEx*)m_rtcEngine;
	}
	return m_rtcEngineEx;
}

void AgoraRtcEngine::VideoSource1SendStreamMessage(QString message)
{
	int streadmId = -1;
	agora::rtc::DataStreamConfig config;
	int ret = m_rtcEngineEx->createDataStreamEx(&streadmId, config, connection_);

	std::string str = message.toStdString().data();

	ret = m_rtcEngineEx->sendStreamMessageEx(streadmId, str.data(), str.length(), connection_);
}

void AgoraRtcEngine::VideoSource2SendStreamMessage(QString message)
{
	int streadmId = -1;
	agora::rtc::DataStreamConfig config;
	int ret = m_rtcEngineEx->createDataStreamEx(&streadmId, config, connection2_);

	std::string str = message.toStdString().data();

	ret = m_rtcEngineEx->sendStreamMessageEx(streadmId, str.data(), str.length(), connection2_);
}

bool AgoraRtcEngine::LocalVideoPreview(HWND hVideoWnd, bool bPreviewOn, agora::media::base::RENDER_MODE_TYPE mode)
{
	int nRet = 0;
	agora::rtc::VideoCanvas vc;
	vc.uid = 0;
	vc.renderMode = mode;
	if (bPreviewOn) {
		vc.view = hVideoWnd;
		m_rtcEngine->setupLocalVideo(vc);
		nRet = m_rtcEngine->startPreview();
	}
	else{
		if (!joined)
			nRet = m_rtcEngine->stopPreview();
		vc.view = NULL;
		m_rtcEngine->setupLocalVideo(vc);
	}

	return nRet == 0 ? true : false;
}


bool AgoraRtcEngine::SetEncoderType(int type)
{
//	agora::rtc::AParameter apm(*m_rtcEngine);
	int ret = 0;//apm->setInt("che.hardware_encoding", type);

	return ret == 0 ? TRUE : FALSE;
}

void AgoraRtcEngine::UpdateVideoSource2Subscribe(bool subscribe)
{
	agora::rtc::ChannelMediaOptions option;
	option.autoSubscribeAudio = subscribe;
	option.autoSubscribeVideo = subscribe;
	option.publishAudioTrack = false;
	option.publishCameraTrack = false;
	option.publishMediaPlayerVideoTrack = true;
	option.publishMediaPlayerAudioTrack = true;
	option.publishMediaPlayerId = media_player_->getMediaPlayerId();
	option.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
	option.channelProfile = agora::CHANNEL_PROFILE_LIVE_BROADCASTING;
	m_rtcEngineEx->updateChannelMediaOptionsEx(option, connection2_);
}

bool AgoraRtcEngine::VideoSource1JoinChannel(bool enableVideo, const char* token, const char* channel, agora::rtc::uid_t uid, const const agora::rtc::VideoEncoderConfiguration& config)
{
	//视频源2在频道内需要取消视频源2的订阅，由视频源1订阅
	if (joined2) {
		UpdateVideoSource2Subscribe(false);
	}
	agora::rtc::ChannelMediaOptions option;
	option.autoSubscribeAudio = true;
	option.autoSubscribeVideo = true;
	option.publishAudioTrack = true;
	option.publishCameraTrack = enableVideo;
	option.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
	option.channelProfile = agora::CHANNEL_PROFILE_LIVE_BROADCASTING;
	szChannelId_ = channel;
	localUid_ = uid;
	
	connection_.channelId = szChannelId_.c_str();
	connection_.localUid = uid;
	//agora::rtc::IRtcEngineEx* engine = (agora::rtc::IRtcEngineEx*)m_rtcEngine;
	agora::base::AParameter apm(*m_rtcEngine);
	apm->setParameters("{\"che.video.quick_adapt_network\" : false}");
	int ret = m_rtcEngineEx->joinChannelEx(token, connection_, option, &m_eventHandler);//joinChannel(token, channel, uid, option);
	if (ret == 0) {
		m_rtcEngineEx->muteRemoteAudioStreamEx(setting.userInfo.uid, true, connection2_);
		m_rtcEngineEx->muteRemoteVideoStreamEx(setting.userInfo.uid, true, connection2_);
		if (joined2) {
			m_rtcEngineEx->muteRemoteAudioStreamEx(setting.userInfo.uid, true, connection2_);
			m_rtcEngineEx->muteRemoteVideoStreamEx(setting.userInfo.uid, true, connection2_);
		}
	}
	m_rtcEngineEx->setVideoEncoderConfigurationEx(config, connection_);
	
	videoSource1Subscribe = true;
	
	return ret == 0 ? TRUE : FALSE;
}

void AgoraRtcEngine::SetVideoEncoderConfigurationEx(agora::rtc::VideoEncoderConfiguration config)
{
	m_rtcEngineEx->setVideoEncoderConfigurationEx(config, connection_);
}

int AgoraRtcEngine::VideoSource2JoinChannel(bool enableVideo, const char* token, const char* channel, agora::rtc::uid_t uid, bool subscribeAudio, bool subscribeVideo )
{
	agora::rtc::ChannelMediaOptions option;
	option.autoSubscribeAudio = subscribeAudio;
	option.autoSubscribeVideo = subscribeVideo;
	option.publishAudioTrack = false;
	option.publishCameraTrack = false;
	option.publishMediaPlayerVideoTrack = true;
	option.publishMediaPlayerAudioTrack = true;
	option.publishMediaPlayerId = media_player_->getMediaPlayerId();
	option.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
	option.channelProfile = agora::CHANNEL_PROFILE_LIVE_BROADCASTING;
	
	szChannelId_ = channel;
	connection2_.channelId = szChannelId_.c_str();
	connection2_.localUid = uid;
	m_eventHandler2.bEx = true;
	agora::base::AParameter apm(*m_rtcEngine);
	apm->setParameters("{\"che.video.quick_adapt_network\" : false}");
	int ret = m_rtcEngineEx->joinChannelEx(token, connection2_, option, &m_eventHandler2);
	if (ret == 0) {
		if (joined) {
			m_rtcEngineEx->muteRemoteAudioStreamEx(setting.userInfo2.uid, true, connection_);
			m_rtcEngineEx->muteRemoteVideoStreamEx(setting.userInfo2.uid, true, connection_);
		}
		m_rtcEngineEx->muteRemoteAudioStreamEx(setting.userInfo.uid, true, connection2_);
		m_rtcEngineEx->muteRemoteVideoStreamEx(setting.userInfo.uid, true, connection2_);
	}
	if (!joined) videoSource1Subscribe = false;
	return ret;
}

bool AgoraRtcEngine::EnableVolumeIndication(int interval, int smooth)
{
	
	int ret = m_rtcEngine->enableAudioVolumeIndication(interval, smooth, false);
	return ret == 0 ? true : false;
}

void AgoraRtcEngine::MuteAllRemoteVideo(bool bMute)
{
	m_rtcEngine->muteAllRemoteVideoStreams(bMute);
}
void AgoraRtcEngine::MuteAllRemoteAudio(bool bMute)
{
	
	m_rtcEngine->muteAllRemoteAudioStreams(bMute);
}

void AgoraRtcEngine::MuteRemoteVideo(unsigned int uid, bool bMute)
{
	
	m_rtcEngineEx->muteRemoteVideoStreamEx(uid, bMute, connection_);
}
void AgoraRtcEngine::MuteRemoteAudio(unsigned int uid, bool bMute)
{
	m_rtcEngineEx->muteRemoteAudioStreamEx(uid, bMute, connection_);
}

void AgoraRtcEngine::MuteLocalVideo(bool bMute)
{
	agora::rtc::ChannelMediaOptions option;
	option.autoSubscribeAudio = true;
	option.autoSubscribeVideo = true;
	option.publishAudioTrack = !muteLocalAudio_;
	option.publishCameraTrack = !bMute;
	muteLocalVideo_ = bMute;
	option.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
	option.channelProfile = agora::CHANNEL_PROFILE_LIVE_BROADCASTING;
	
	m_rtcEngineEx->updateChannelMediaOptionsEx(option, connection_);
}
void AgoraRtcEngine::MuteLocalAudio(bool bMute)
{
	agora::rtc::ChannelMediaOptions option;
	option.autoSubscribeAudio = true;
	option.autoSubscribeVideo = true;
	option.publishAudioTrack = !bMute;
	option.publishCameraTrack = muteLocalVideo_;
	muteLocalAudio_ = bMute;
	option.clientRoleType = agora::rtc::CLIENT_ROLE_BROADCASTER;
	option.channelProfile = agora::CHANNEL_PROFILE_LIVE_BROADCASTING;

	m_rtcEngineEx->updateChannelMediaOptionsEx(option, connection_);
}

void AgoraRtcEngine::StartPrimary()
{
	agora::rtc::CameraCapturerConfiguration config;
	//config.cameraDirection = agora::rtc::CAMERA_FRONT;
	//strcpy(config.deviceId,  setting.videoSource1Id.toStdString().c_str());
	config.format.width = setting.resolution[setting.resIndex].width;
	config.format.height = setting.resolution[setting.resIndex].height;
	config.format.fps = setting.frameRate;
	int ret = m_rtcEngine->startPrimaryCameraCapture(config);
}

void AgoraRtcEngine::EnableLocalVideo(bool enable)
{
	int ret = 0;
	ret = m_rtcEngine->enableLocalVideo(enable);
}

void AgoraRtcEngine::EnableAGC(bool bEnable)
{
	agora::base::AParameter apm(*m_rtcEngine);
	if (bEnable)
		apm->setParameters("{\"che.audio.enable.agc\" : true}");
	else
		apm->setParameters("{\"che.audio.enable.agc\" : false}");
}

void AgoraRtcEngine::EnableAEC(bool bEnable)
{
	agora::base::AParameter apm(*m_rtcEngine);
	if (bEnable)
		apm->setParameters("{\"che.audio.enable.aec\" : true}");
	else
		apm->setParameters("{\"che.audio.enable.aec\" : false}");
}

void AgoraRtcEngine::EnableANS(bool bEnable)
{
	agora::base::AParameter apm(*m_rtcEngine);
	if (bEnable)
		apm->setParameters("{\"che.audio.enable.ns\" : true}");
	else
		apm->setParameters("{\"che.audio.enable.ns\" : false}");
}

void AgoraRtcEngine::SetFecParameter(int fec)
{
	char szParam[30] = { 0 };
	sprintf(szParam, "{\"rtc.fec_method\": %d}", fec);
	agora::base::AParameter apm(*m_rtcEngine);
	apm->setParameters(szParam);
}

int AgoraRtcEngine::OpenVideoSource2(QString url)
{
	if (!media_player_)
		return -1;
	return media_player_->open(url.toStdString().c_str(), 0);
}

int AgoraRtcEngine::PlayVideoSource2()
{
	if (!media_player_)
		return -1;
	return media_player_->play();
}

int AgoraRtcEngine::ShowVideoSource2(agora::media::base::view_t view)
{
	if (!media_player_)
		return -1;
	return media_player_->setView(view);
}

int AgoraRtcEngine::StopVideoSource2()
{
	return media_player_->stop();
}

int AgoraRtcEngine::PauseVideoSource2(bool bPause)
{
	if (bPause)
		return media_player_->pause();
	else
		return media_player_->resume();
}

int AgoraRtcEngine::VideoSourceLeave(const char* channel, unsigned int  uid)
{
	agora::rtc::IRtcEngineEx* engine = (agora::rtc::IRtcEngineEx*)m_rtcEngine;
	agora::rtc::RtcConnection connection;
	connection.channelId = channel;
	connection.localUid = uid;
	int ret = engine->leaveChannelEx(connection);

	if (setting.userInfo.uid == uid)
		videoSource1Subscribe = false;
	else if(!joined)
		videoSource1Subscribe = false;
	return ret;
}

void AgoraRtcEngine::SetVideoWidget(QMap<unsigned int, VideoWidget*> widgets)
{
	QMutexLocker lockMap(&mtxVideos_);
	videoWidgets_ = widgets;
}

void AgoraRtcEngine::ResetVideoWidgets()
{
	QMutexLocker lockMap(&mtxVideos_);
	videoWidgets_.clear();
}

void AgoraRtcEngine::SetVideoWidgetEx(QMap<unsigned int, VideoWidget*> widgets)
{
	QMutexLocker lockMap(&mtxVideosEx_);
	videoWidgetsEx_ = widgets;
}

void AgoraRtcEngine::ResetVideoWidgetsEx()
{
	QMutexLocker lockMap(&mtxVideosEx_);
	videoWidgetsEx_.clear();
}


void AgoraRtcEngine::onPlayerSourceStateChanged(agora::media::base::MEDIA_PLAYER_STATE state,
	agora::media::base::MEDIA_PLAYER_ERROR ec)
{
	if (agora::media::base::PLAYER_STATE_OPEN_COMPLETED == state) {
		emit openPlayerComplete();
	}
	else if (ec != agora::media::base::PLAYER_ERROR_NONE) {
		emit playerError(ec);
	}
}

bool AgoraRtcEngine::RegisterVideoFrameObserver(bool bEnable)
{
	agora::util::AutoPtr<agora::media::IMediaEngine> mediaEngine;
	//query interface agora::AGORA_IID_MEDIA_ENGINE in the engine.
	mediaEngine.queryInterface(m_rtcEngine, agora::rtc::AGORA_IID_MEDIA_ENGINE);
	int nRet = 0;

	if (mediaEngine.get() == NULL)
		return FALSE;
	if (bEnable) {
		//register agora video frame observer.
		nRet = mediaEngine->registerVideoFrameObserver(this);
	}
	else {
		//unregister agora video frame observer.
		nRet = mediaEngine->registerVideoFrameObserver(nullptr);
	}
	return nRet == 0 ? TRUE : FALSE;
}

bool AgoraRtcEngine::onCaptureVideoFrame(VideoFrame& videoFrame)
{
	if (setting.bExtend) {
		QMutexLocker lockMap(&mtxVideosEx_);
		if (videoWidgetsEx_.find(setting.userInfo.uid)
			== videoWidgetsEx_.end())
			return true;

		videoWidgetsEx_[setting.userInfo.uid]->CopyVideoFrame(videoFrame);
	}
	else {
		QMutexLocker lockMap(&mtxVideos_);
		if (videoWidgets_.find(setting.userInfo.uid)
			== videoWidgets_.end())
			return true;

		videoWidgets_[setting.userInfo.uid]->CopyVideoFrame(videoFrame);
	}
	emit renderSignal();
	return true;
}

bool AgoraRtcEngine::onMediaPlayerVideoFrame(VideoFrame& videoFrame, int mediaPlayerId)
{
	if (setting.bExtend) {
		QMutexLocker lockMap(&mtxVideosEx_);
		if (videoWidgetsEx_.find(setting.userInfo2.uid)
			== videoWidgetsEx_.end())
			return true;

		videoWidgetsEx_[setting.userInfo2.uid]->CopyVideoFrame(videoFrame);
	}
	else {
		QMutexLocker lockMap(&mtxVideos_);
		if (videoWidgets_.find(setting.userInfo2.uid)
			== videoWidgets_.end())
			return true;

		videoWidgets_[setting.userInfo2.uid]->CopyVideoFrame(videoFrame);
	}
	emit renderSignal();
	return true;
}

bool AgoraRtcEngine::onRenderVideoFrame(const char* channelId, agora::rtc::uid_t remoteUid, VideoFrame& videoFrame)
{
	QMutexLocker lockMap(&mtxVideos_);
	if (videoWidgets_.find(remoteUid)
		== videoWidgets_.end())
		return true;
	if (remoteUid != setting.userInfo2.uid)
		videoWidgets_[remoteUid]->CopyVideoFrame(videoFrame);
	emit renderSignal();
	return true;
}

/// <summary>
/// device
/// </summary>
void AgoraRtcEngine::InitAudioDevice()
{
	audioManager_ = new agora::rtc::AAudioDeviceManager(AgoraRtcEngine::GetEngine());
	if (audioManager_->get() == NULL) {
		delete audioManager_;
		audioManager_ = NULL;
		return;
	}
	mics_ = (*audioManager_)->enumerateRecordingDevices();
	speakers_ = (*audioManager_)->enumeratePlaybackDevices();
	
}

void AgoraRtcEngine::DestroyAudioDevice()
{  
	if (mics_) {
		mics_->release();
		mics_ = NULL;
	}
	if (speakers_) {
		speakers_->release();
		speakers_ = NULL;
	}
	if (audioManager_ != NULL) {
		delete audioManager_;
		audioManager_ = NULL;
	}
}

void AgoraRtcEngine::InitVideoDevice()
{
	videoManager_ = new agora::rtc::AVideoDeviceManager(AgoraRtcEngine::GetEngine());
	if (videoManager_->get() == NULL) {
		delete videoManager_;
		videoManager_ = NULL;
		return;
	}
	videos_ = (*videoManager_)->enumerateVideoDevices();
}
void AgoraRtcEngine::DestroyVideoDevice()
{
	if (videos_) {
		videos_->release();
		videos_ = NULL;
	}
	if (videoManager_ != NULL) {
		delete videoManager_;
		videoManager_ = NULL;
	}
}

int  AgoraRtcEngine::GetMicVolume()
{
	int nVol = 0;
	if (audioManager_ && *audioManager_ != NULL)
		(*audioManager_)->getRecordingDeviceVolume(&nVol);
	return (int)nVol;
}

bool AgoraRtcEngine::SetMicVolume(int nVol)
{
	int nRet = -1;
	if (audioManager_ && *audioManager_ != NULL)
		nRet = (*audioManager_)->setRecordingDeviceVolume((int)nVol);
	return nRet == 0 ? true : false;
}

int  AgoraRtcEngine::GetMicCount()
{
	if (mics_ != NULL)
		return (int)mics_->getCount();

	return 0;
}

QString AgoraRtcEngine::GetCurMicID()
{
	char szDeviceID[agora::rtc::MAX_DEVICE_ID_LENGTH] = { 0 };

	if (audioManager_ != NULL && *audioManager_ != NULL)
		(*audioManager_)->getRecordingDevice(szDeviceID);
	return QString::fromUtf8(szDeviceID);
}
bool AgoraRtcEngine::SetCurMic(const QString deviceID)
{
	if (audioManager_ == NULL || *audioManager_ == NULL)
		return FALSE;

	int nRet = (*audioManager_)->setRecordingDevice(deviceID.toUtf8());
	return nRet == 0 ? TRUE : FALSE;
}

bool AgoraRtcEngine::GetMic(int nIndex, QString& rDeviceName, QString& rDeviceID)
{
	rDeviceName.clear();
	rDeviceID.clear();
	char szDeviceName[agora::rtc::MAX_DEVICE_ID_LENGTH];
	char szDeviceID[agora::rtc::MAX_DEVICE_ID_LENGTH];

	if (nIndex >= GetMicCount())
		return FALSE;

	int nRet = mics_->getDevice(nIndex, szDeviceName, szDeviceID);
	if (nRet != 0)
		return FALSE;

	rDeviceName = QString::fromUtf8(szDeviceName);
	rDeviceID = QString::fromUtf8(szDeviceID);
	return TRUE;
}

void AgoraRtcEngine::TestMicDevice(bool bTestOn)
{
	if (audioManager_ == NULL || *audioManager_ == NULL)
		return;

	if (bTestOn) {
		(*audioManager_)->startRecordingDeviceTest(20);
	}
	else if (!bTestOn ) {
		(*audioManager_)->stopRecordingDeviceTest();
	}
}


int  AgoraRtcEngine::GetSpeakerVolume()
{
	int nVol = 0;
	if (audioManager_ && *audioManager_ != NULL)
		(*audioManager_)->getRecordingDeviceVolume(&nVol);
	return (int)nVol;
}

bool AgoraRtcEngine::SetSpeakerVolume(int nVol)
{
	int nRet = -1;
	if (audioManager_ && *audioManager_ != NULL)
		nRet = (*audioManager_)->setRecordingDeviceVolume((int)nVol);
	return nRet == 0 ? true : false;
}

int  AgoraRtcEngine::GetSpeakerCount()
{
	if (speakers_ != NULL)
		return (int)speakers_->getCount();

	return 0;
}

QString AgoraRtcEngine::GetCurSpeakerID()
{
	char szDeviceID[agora::rtc::MAX_DEVICE_ID_LENGTH] = { 0 };

	if (audioManager_ != NULL && *audioManager_ != NULL)
		(*audioManager_)->getPlaybackDevice(szDeviceID);
	return QString::fromUtf8(szDeviceID);
}
bool AgoraRtcEngine::SetCurSpeaker(const QString deviceID)
{
	if (audioManager_ == NULL || *audioManager_ == NULL)
		return FALSE;

	int nRet = (*audioManager_)->setPlaybackDevice(deviceID.toUtf8());
	return nRet == 0 ? TRUE : FALSE;
}

bool AgoraRtcEngine::GetSpeaker(int nIndex, QString& rDeviceName, QString& rDeviceID)
{
	rDeviceName.clear();
	rDeviceID.clear();
	char szDeviceName[agora::rtc::MAX_DEVICE_ID_LENGTH];
	char szDeviceID[agora::rtc::MAX_DEVICE_ID_LENGTH];

	if (nIndex >= GetSpeakerCount())
		return FALSE;

	int nRet = speakers_->getDevice(nIndex, szDeviceName, szDeviceID);
	if (nRet != 0)
		return FALSE;

	rDeviceName = QString::fromUtf8(szDeviceName);
	rDeviceID = QString::fromUtf8(szDeviceID);
	return TRUE;
}

void AgoraRtcEngine::TestSpeakerDevice(bool bTestOn)
{
	if (audioManager_ == NULL || *audioManager_ == NULL)
		return;

	if (bTestOn) {
		QString testPath = QCoreApplication::applicationDirPath() + QString("/ID_TEST_AUDIO.wav");
		(*audioManager_)->startPlaybackDeviceTest(testPath.toUtf8().data());
	}
	else {
		(*audioManager_)->stopPlaybackDeviceTest();
	}
	bTestOn = !bTestOn;
}

int AgoraRtcEngine::GetVideoDeviceCount()
{
	if (videos_ == NULL)
		return 0;

	return (int)videos_->getCount();
}

QString AgoraRtcEngine::GetCurVideoDeviceID()
{
	char szDeviceID[agora::rtc::MAX_DEVICE_ID_LENGTH] = { 0 };

	if (videoManager_ != NULL && *videoManager_ != NULL)
		(*videoManager_)->getDevice(szDeviceID);
	if (curVideoDevice.isEmpty()) {
		(*videoManager_)->getDevice(szDeviceID);
		curVideoDevice = QString::fromUtf8(szDeviceID);
	}
	return curVideoDevice;
}

bool AgoraRtcEngine::SetCurVideoDevice(const QString deviceID)
{
	if (videoManager_ == NULL || *videoManager_ == NULL)
		return FALSE;

	int nRet = (*videoManager_)->setDevice(deviceID.toStdString().c_str());
	curVideoDevice = deviceID;
	return nRet == 0 ? TRUE : FALSE;

}

bool AgoraRtcEngine::GetVideoDevice(int nIndex, QString& rDeviceName, QString& rDeviceID)
{
	rDeviceName.clear();
	rDeviceID.clear();
	char szDeviceName[agora::rtc::MAX_DEVICE_ID_LENGTH];
	char szDeviceID[agora::rtc::MAX_DEVICE_ID_LENGTH];

	if (nIndex >= GetVideoDeviceCount())
		return FALSE;

	if (videos_ == NULL)
		return FALSE;

	int nRet = videos_->getDevice(nIndex, szDeviceName, szDeviceID);
	if (nRet != 0)
		return FALSE;

	rDeviceName = QString::fromUtf8(szDeviceName);
	rDeviceID = QString::fromUtf8(szDeviceID);
	return TRUE;
}
void AgoraRtcEngine::GetVideoDeviceInfo(QVector<agora::rtc::VideoFormat>& infos)
{
	char szDeviceID[agora::rtc::MAX_DEVICE_ID_LENGTH] = { 0 };

	if (videoManager_ != NULL && *videoManager_ != NULL) {
		if (curVideoDevice.isEmpty()) {
			(*videoManager_)->getDevice(szDeviceID);
			curVideoDevice = QString::fromUtf8(szDeviceID);
		}
		int count = (*videoManager_)->numberOfCapabilities(curVideoDevice.toStdString().c_str());//(szDeviceID);
		for (int i = 0; i < count; ++i) {
			agora::rtc::VideoFormat format;
			(*videoManager_)->getCapability(curVideoDevice.toStdString().c_str(), i, format);
			if (format.width == 1280 && format.height == 720 && format.fps == 30) {
				infos.push_back(format);
			}
			else if (format.width == 1920 && format.height == 1080 && format.fps == 30) {
				infos.push_back(format);
			}
			else if (format.width == 1920 && format.height == 1080 && format.fps == 60) {
				infos.push_back(format);
			}
			else if (format.width == 3840 && format.height == 2160 && format.fps == 60) {
				infos.push_back(format);
			}
		}
	}
}
