#ifndef WidgetSettings_H
#define WidgetSettings_H

#include<QWidget>
#include<QFrame>
#include <QPushButton>
#include <QOpenGLWidget>
#include "ui_VideoWidget.h"
#include "SettingsData.h"
#include "video_render_opengl.h"
#include <memory>
#include <mutex>
class VideoWidget: public QOpenGLWidget
{
	friend class AgoraRtcEngine;
	Q_OBJECT

public:
	VideoWidget(float initRate, float rate, QWidget *parent = 0);
	~VideoWidget();
	void SetRate(float rate) { rate_ = rate; }
	virtual void initializeGL() override;
	virtual void resizeGL(int w, int h) override;
	virtual void paintGL() override;
	void SetUserInfo(UserInfo info); 
	void SetWidgetInfo(WidgetInfo info);
	void CopyVideoFrame(agora::media::base::VideoFrame& videoFrame);
	unsigned int GetUID() { return userInfo.uid; }
	void UpdateButtonPos();
	void RestoreWidget();
	void MaximizeWidget(int w, int h);
	void Reset();
	bool IsMax() { return bMax; }
private:
	Ui::VideoWidget ui;
	QPushButton* btnUser;
	QPushButton* btnCamera;
	QPushButton* btnMic;
	QPushButton* btnFullScreen;
	bool bMax = true;
	int widgetW = 710;
	int widgetH = 400;
	int btnUserW = 116;
	int btnUserH = 45;
	int userX = 20;
	int btnW = 48;
	int btnSpacing = 16;
	int btnPadding = 20;
	float rate_ = 1.0f;

	std::unique_ptr<VideoRendererOpenGL> m_render;
	std::mutex m_mutex;
	//usage of m_frame should be guarded by m_mutex
	int m_rotation;

	UserInfo userInfo;
	bool muteAudio = false;
	bool muteVideo = false;
	bool fullScreen = false;
	bool render = false;
	agora::media::base::VideoFrame m_frame;

	void InitButton();
	
	void SetMicButtonStats(bool mute);
	void SetCameraButtonStats(bool mute);
	void SetFullScreenButtonStats(bool full);
	void InitWidget(); 
	void InitVideoFrame();

	float initRate_ = 1.0f;
	//DPI_TYPE dpiType_ = DPI_1080;
private slots:
	void on_btnCamera_clicked();
	void on_btnMic_clicked();
	void on_btnFullScreen_clicked();
	void renderFrame();
signals:
	void fullScreenSignal(unsigned int uid, bool bFull);
	void muteVideoSignal(unsigned int uid, bool bMute);
	void muteAudioSignal(unsigned int uid, bool bMute);
public:
	void maxButtonChange(bool max, int w, int h, float rate);
};

#endif // WidgetSettings_H
