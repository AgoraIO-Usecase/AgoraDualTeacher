#include "VideoWidget.h"
#include<qstyleditemdelegate.h>
#include "AgoraRtcEngine.h"
VideoWidget::VideoWidget(float initRate, float rate, QWidget *parent)
	:QOpenGLWidget(parent)
	, m_rotation(0)
	, initRate_(initRate)
	, rate_(rate)
{
	userInfo.uid = 0;
	userInfo.name = "";
	ui.setupUi(this);
	InitWidget();

	m_render = std::make_unique<VideoRendererOpenGL>(  widgetW, widgetH);

	
	InitVideoFrame();
}

VideoWidget::~VideoWidget()
{
	delete[] m_frame.yBuffer;
	delete[] m_frame.uBuffer;
	delete[] m_frame.vBuffer;
	m_frame.yBuffer = nullptr;
	m_frame.uBuffer = nullptr;
	m_frame.vBuffer = nullptr;
}

void VideoWidget::InitVideoFrame()
{
	m_frame.yStride = 3840;
	m_frame.uStride = 1920;
	m_frame.vStride = 1920;
	m_frame.width = 3840;
	m_frame.height = 2160;
	m_frame.yBuffer = new uint8_t[m_frame.yStride * m_frame.height];
	m_frame.uBuffer = new uint8_t[m_frame.uStride * m_frame.height / 2];
	m_frame.vBuffer = new uint8_t[m_frame.vStride * m_frame.height / 2];
	memset(m_frame.yBuffer, 0, m_frame.yStride * m_frame.height);
	memset(m_frame.uBuffer, 128, m_frame.uStride * m_frame.height / 2);
	memset(m_frame.vBuffer, 128, m_frame.vStride * m_frame.height / 2);
	m_frame.type = agora::media::base::VIDEO_PIXEL_I420;
}

void VideoWidget::initializeGL()
{
}

void VideoWidget::resizeGL(int w, int h)
{
	m_render->setSize(w * rate_ , h * rate_ + 1);
	ui.widgetFrame->setGeometry(0, 0, w , h);
	ui.verticalLayoutWidget->setGeometry(0, 0, w, h);
}

void VideoWidget::paintGL()
{
	if (!m_render)
		return;

	if (!m_render->isInitialized()) {
		m_render->initialize(widgetW, widgetH);
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_render->setFrameInfo(m_rotation);
		if (userInfo.uid != 0  && !muteVideo && render) {
			m_render->renderFrame(m_frame);
		}
		else if(!ui.widgetFrame->isVisible()){
			ui.widgetFrame->show();
		}
	}
}

void VideoWidget::SetUserInfo(UserInfo info)
{
	userInfo.name = info.name;
	userInfo.uid = info.uid;
	btnUser->setText(userInfo.name);
}

void VideoWidget::SetWidgetInfo(WidgetInfo info)
{
	SetUserInfo(info.userInfo);
	muteAudio = info.muteAudio;
	muteVideo = info.muteVideo;
	SetCameraButtonStats(muteVideo);
	SetMicButtonStats(muteAudio);
}

void VideoWidget::Reset()
{
	SetUserInfo(UserInfo());

	muteAudio = false;
	muteVideo = false;
	fullScreen = false;
	render = false;
	SetCameraButtonStats(muteVideo);
	SetMicButtonStats(muteAudio);
}

void VideoWidget::CopyVideoFrame(agora::media::base::VideoFrame& videoFrame)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	m_render->setFrameInfo(m_rotation);
	m_frame.yStride = videoFrame.yStride;
	m_frame.uStride = videoFrame.uStride;
	m_frame.vStride = videoFrame.vStride;
	m_frame.width = videoFrame.width;
	m_frame.height = videoFrame.height;
	m_frame.rotation = videoFrame.rotation;
	m_frame.avsync_type = videoFrame.avsync_type;
	m_frame.renderTimeMs = videoFrame.renderTimeMs;
	m_frame.type = videoFrame.type;
	if(m_frame.yBuffer)
	memcpy(m_frame.yBuffer, videoFrame.yBuffer, videoFrame.yStride * videoFrame.height);
	if(m_frame.uBuffer)
	memcpy(m_frame.uBuffer, videoFrame.uBuffer, videoFrame.uStride * videoFrame.height / 2);
	if(m_frame.vBuffer)
	memcpy(m_frame.vBuffer, videoFrame.vBuffer, videoFrame.vStride * videoFrame.height / 2);
}

void VideoWidget::renderFrame()
{
	if (ui.widgetFrame->isVisible())
		ui.widgetFrame->hide();
	render = true;
	update();
}

void VideoWidget::on_btnCamera_clicked()
{
	if (userInfo.uid == 0)
		return;

	muteVideo = !muteVideo;
	if (userInfo.uid == setting.userInfo.uid) {
		rtcEngine->MuteLocalVideo(muteVideo);
	}
	else if (userInfo.uid == setting.userInfo2.uid) {
		rtcEngine->PauseVideoSource2(muteVideo);
	}
	else {
		rtcEngine->MuteRemoteVideo(userInfo.uid, muteVideo);
	}

	SetCameraButtonStats(muteVideo);
	muteVideo ? ui.widgetFrame->show() : ui.widgetFrame->hide();
	emit muteVideoSignal(userInfo.uid, muteVideo);
}
void VideoWidget::on_btnMic_clicked()
{
	if (userInfo.uid == 0)
		return;
	muteAudio = !muteAudio;
	if (userInfo.uid == setting.userInfo.uid) {
		rtcEngine->MuteLocalAudio(muteAudio);
	}
	else {
		rtcEngine->MuteRemoteAudio(userInfo.uid, muteAudio);
	}
	SetMicButtonStats(muteAudio);
	emit muteAudioSignal(userInfo.uid, muteAudio);
}
void VideoWidget::on_btnFullScreen_clicked()
{
	if (userInfo.uid == 0)
		return;
	fullScreen = !fullScreen;
	SetFullScreenButtonStats(fullScreen);
	emit fullScreenSignal(userInfo.uid, fullScreen);
}