#ifndef AGORAVIDEOWIDGET_H
#define AGORAVIDEOWIDGET_H


#include <QOpenGLWidget>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <mutex>
#include <memory>

namespace agora {
	namespace media {
		class IVideoFrame;
	}
}

enum SKIP_SCREEN_TYPE{
	CAN_NOT_SKIP = 0,//已经有一个在扩展屏，需要先回去
	SKIP_TO_SCREEN,
	BACK_TO_MAIN
};

struct WidgetInfo{
	unsigned int  host_uid;
	//CLASS_ROLE    host_role;
	//UserInfo      widgetUserInfo;
	bool bShareScreen;
	bool bOccupied;
	bool bMuteSpeaker;
	//bool bSkip;
	//bool bExtend;
	SKIP_SCREEN_TYPE skip_type;
	WidgetInfo()
		: host_uid(0)
		, bShareScreen(false)
		, bOccupied(false)
		//, host_role(CLASS_ROLE_STUDENT)
		, bMuteSpeaker(false)
		, skip_type(SKIP_TO_SCREEN)
		//, bSkip(true)
		//, bExtend(false)
	{}
	WidgetInfo(const WidgetInfo& info)
		: host_uid(info.host_uid)
		, bShareScreen(info.bShareScreen)
		, bOccupied(info.bOccupied)
		//, host_role(info.host_role)
		//, widgetUserInfo(info.widgetUserInfo)
		, bMuteSpeaker(info.bMuteSpeaker)
		, skip_type(info.skip_type)
		//, bSkip(info.bSkip)
		//, bExtend(info.bExtend)
	{

	}

};

class AgoraVideoLayoutManager;
class VideoRendererOpenGL;
class AgoraVideoWidget : public QOpenGLWidget
{
	//friend class AgoraVideoLayoutManager;
	Q_OBJECT

public:
	explicit AgoraVideoWidget(QWidget *parent = 0);
	~AgoraVideoWidget();

	int setViewProperties(int zOrder, float left, float top, float right, float bottom);
	int deliverFrame(const agora::media::IVideoFrame& videoFrame, int rotation, bool mirrored);

	void UpdateScrollArea(QScrollArea* scroll);
	void UpdateWidget();
	void UpdateWidgetInfo(const WidgetInfo& info);
	void GetWidgetInfo(WidgetInfo& info);
	unsigned int GetUid();
	
	void ResetVideoWidget();
	void ResetVideoFrame();
//	void UpdateLocalAndChangeUserInfoMuteState();
	void SetSkipFlag(SKIP_SCREEN_TYPE type);
	void SetRenderMode(int mode);
	SKIP_SCREEN_TYPE GetSkipFlag();

	void SetCallingOnlineFlag(bool b){ bCallingOnline = b; }
	//void ShowBackground();
protected:
	virtual void initializeGL() Q_DECL_OVERRIDE;
	virtual void resizeGL(int w, int h) Q_DECL_OVERRIDE;
	virtual void paintGL() Q_DECL_OVERRIDE;
private:
	void InitChildWidget();

	/*void SetUserButton(UserInfo& info);
	void SetScreenButton(UserInfo& info);
	void SetAVWidget(UserInfo& info);
	void SetSpeakerButton(UserInfo& info);*/
	void SetBackground();

private slots:
    void on_screenButton_clicked();
	void on_userButton_clicked();
	void on_audioButton_clicked();
	void on_videoButton_clicked();
	void on_speakerButton_clicked();
	void renderFrame();
	void cleanup();
	//void on_RightClick_customContextMenuRequested(const QPoint &pos);
	void on_actionSkipToExtendScreen_triggered();
	void on_actionBackToMainScreen_triggered();
signals:
	void frameDelivered();
	void widgetInvalidated();
	void viewSizeChanged(int width, int height);

	void skipToScreen(WidgetInfo info);
private:
	QPushButton* userButton;
	QPushButton *screenButton;
	QWidget *widgetContainer;
	QPushButton *videoButton;
	QFrame *line;
	QPushButton *audioButton;
	QFrame* widgetFrame;
	QPushButton *speakerButton;
	QAction *skipToExtendScreen;
	QAction *backToMainScreen;
	//scroll list
	QScrollArea *scrollArea;
	bool showList;
	//agora video frame
	//std::unique_ptr<VideoRendererOpenGL> m_render;	
	std::mutex m_mutex;
	//usage of m_frame should be guarded by m_mutex
	agora::media::IVideoFrame* m_frame;

	int m_rotation;
	bool m_mirrored;

	// widget info
	WidgetInfo widgetInfo;
	bool bSkip;
	
	bool bShowHndsup;

	bool bCallingOnline;//点名开关，true 正在点名中，false可以点名
};

#endif // AGORAVIDEOWIDGET_H
