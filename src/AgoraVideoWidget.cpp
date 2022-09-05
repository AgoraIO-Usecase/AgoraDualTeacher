#include "AgoraVideoWidget.h"
#include <QLayout>
#include <QAction>
#include <QMenu>
#include <QMetaType>

#include <IAgoraMediaEngine.h>
//#include "video_render_opengl.h"


//#include "AgoraQtScreen.h"

//#include "agora_log.h"

#define BOTTOM_SPACE 10
#define LEFT_SPACE   10
#define RIGHT_SPACE  10
#define SCREEN_AND_AV_SPACE 10//右侧
#define AV_WIDGET_WIDTH  74
#define SCREEN_BTN_RIGHT_SAPCE (AV_WIDGET_WIDTH + SCREEN_AND_AV_SPACE + RIGHT_SPACE)
#define MAIN_AV_WIDGET_WIDTH (SCREEN_BTN_RIGHT_SAPCE+ SCREEN_AND_AV_SPACE + RIGHT_SPACE +240+15+20)// 240右侧列表宽度，15 右侧列表左边的间隙，20右侧列表右边的间隙
#define LIST_WIDTH 275

int screen_width = 32;

AgoraVideoWidget::AgoraVideoWidget(QWidget *parent)
	: QOpenGLWidget(parent)
	//, m_render(new VideoRendererOpenGL(width(), height()))
	, m_frame(nullptr)
	, m_rotation(180)
	, m_mirrored(false)
	, bSkip(false)
	, bShowHndsup(false)
	, showList(false)
	, scrollArea(nullptr)
	, bCallingOnline(false)
{
	qRegisterMetaType<WidgetInfo>("WidgitInfo");
	InitChildWidget();
	setMouseTracking(true);
/*	skipToExtendScreen = new QAction(this);
	skipToExtendScreen->setObjectName(QStringLiteral("SkipToExtendScreen"));
	skipToExtendScreen->setCheckable(true);
	backToMainScreen = new QAction(this);
	backToMainScreen->setObjectName(QStringLiteral("BackToMainScreen"));
	backToMainScreen->setCheckable(true);*/

	setObjectName(QStringLiteral("agoraWidgetRight"));
	//widgetInfo.widgetUserInfo = UserInfo();

	setContextMenuPolicy(Qt::CustomContextMenu);

	connect(this, SIGNAL(frameDelivered()), this, SLOT(renderFrame()));
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
		this, SLOT(on_RightClick_customContextMenuRequested(const QPoint &)));
	connect(screenButton, &QPushButton::clicked, this, &AgoraVideoWidget::on_screenButton_clicked);
	connect(userButton, &QPushButton::clicked, this, &AgoraVideoWidget::on_userButton_clicked);
	connect(audioButton, &QPushButton::clicked, this, &AgoraVideoWidget::on_audioButton_clicked);
	connect(videoButton, &QPushButton::clicked, this, &AgoraVideoWidget::on_videoButton_clicked);
	connect(speakerButton, &QPushButton::clicked, this, &AgoraVideoWidget::on_speakerButton_clicked);
}

AgoraVideoWidget::~AgoraVideoWidget()
{
	cleanup();
}

void AgoraVideoWidget::on_screenButton_clicked()
{
	if (!widgetInfo.bShareScreen){
		
		
		
	}
	else{
		widgetInfo.bShareScreen = false;
		
		screen_width = 32;
		
	}

	if (scrollArea){
		screenButton->setGeometry(width() - SCREEN_BTN_RIGHT_SAPCE - LIST_WIDTH - screen_width, height() - BOTTOM_SPACE - 32, screen_width, 32);
	}
	else{

		screenButton->setGeometry(width() - SCREEN_BTN_RIGHT_SAPCE - screen_width, height() - BOTTOM_SPACE - 32, screen_width, 32);
	}
	//SetScreenButton(AgoraQtJson::GetAgoraQtJson()->localuserInfo);
}

void AgoraVideoWidget::on_userButton_clicked()
{

		UpdateWidget();

}
void AgoraVideoWidget::on_audioButton_clicked()
{

	
}

void AgoraVideoWidget::on_videoButton_clicked()
{
	
}

void AgoraVideoWidget::on_speakerButton_clicked()
{
	widgetInfo.bMuteSpeaker = !widgetInfo.bMuteSpeaker;
	//SetSpeakerButton(AgoraQtJson::GetAgoraQtJson()->localuserInfo);
	
}
void AgoraVideoWidget::InitChildWidget()
{
	
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	
	widgetFrame = new QFrame(this);
	widgetFrame->setStyleSheet(QLatin1String("background-color:#FFD8D8D8; background-image: url(:/AgoraCourse/Resources/pic-student-small.png); background-repeat: none; background-position: center;"));//(QString("background-color:#FFD8D8D8; border-image: url(%1)").arg(":/Resource/Resources/pic-student.png"));//
	layout->addWidget(widgetFrame);
	this->setLayout(layout);

	userButton = new QPushButton(this);
	userButton->setMinimumSize(QSize(98, 32));
	userButton->setAutoDefault(false);
	userButton->setFlat(true);
	userButton->setObjectName(QStringLiteral("userButton"));

	widgetContainer = new QWidget(this);
	widgetContainer->setObjectName(QStringLiteral("widgetContainer"));
	widgetContainer->setMinimumSize(QSize(74, 32));
	widgetContainer->setMaximumSize(QSize(74, 32));
	widgetContainer->setStyleSheet(QLatin1String("QWidget#widgetContainer{\n"
		"    background-color:rgba(0, 0, 0, 128);\n"
		"    color:rgba(255,255,255,1);\n"
		"    border-style: outset;\n"
		"    border-width: 1px;\n"
		"    border-radius: 14px;\n"
		"    border-color: rgba(0, 0, 0, 128);\n"
		"    font: bold 16px;\n"
		"    height: 34px;\n"
		"}\n"
		"QWidget#widgetContainer:hover{\n"
		"    background-color: rgba(0, 0, 0, 128);\n"
		"    color:rgba(255,255,255,1);\n"
		"    border-style: inset;\n"
		"    border-width: 1px;\n"
		"    border-radius: 14px;\n"
		"    border-color: rgba(0, 0, 0, 128);\n"
		"    font: bold 16px;\n"
		"    height: 34px;\n"
		"}\n"
		"QWidget#widgetContainer:pressed{\n"
		"    background-color:rgba(0, 0, 0, 128);\n"
		"    color:rgba(255,255,255,1);\n"
		"    border-style: outset;\n"
		"    border-width: 1px;\n"
		"    border-radius: 14px;\n"
		"    border-color: rgba(0, 0, 0, 128);\n"
		"    font: bold 16px;\n"
		"    height: 34px;\n"
		"};"));
	videoButton = new QPushButton(widgetContainer);
	videoButton->setObjectName(QStringLiteral("videoButton"));
	videoButton->setGeometry(QRect(2, 0, 32, 32));
	videoButton->setMaximumSize(QSize(32, 32));

	videoButton->setAutoDefault(false);
	videoButton->setFlat(true);
	line = new QFrame(widgetContainer);
	line->setObjectName(QStringLiteral("line"));
	line->setGeometry(QRect(37, 10, 2, 12));
	line->setStyleSheet(QStringLiteral("background-color: rgba(255, 255, 255,77);"));
	line->setFrameShape(QFrame::VLine);
	line->setFrameShadow(QFrame::Sunken);
	audioButton = new QPushButton(widgetContainer);
	audioButton->setObjectName(QStringLiteral("audioButton"));
	audioButton->setGeometry(QRect(40, 0, 32, 32));
	audioButton->setMaximumSize(QSize(32, 32));
	
	audioButton->setAutoDefault(false);
	audioButton->setFlat(true);

	screenButton = new QPushButton(this);
	screenButton->setObjectName(QStringLiteral("screenButton"));
	screenButton->setMaximumSize(QSize(110, 32));
	screenButton->setMinimumSize(QSize(32, 32));
	screenButton->setAutoDefault(false);
	screenButton->setFlat(true);

	speakerButton = new QPushButton(this);
	speakerButton->setObjectName(QStringLiteral("speakerButton"));
	speakerButton->setMaximumSize(QSize(32, 32));
	speakerButton->setAutoDefault(false);
	speakerButton->setFlat(true);
}


void AgoraVideoWidget::initializeGL()
{
}

void AgoraVideoWidget::resizeGL(int w, int h)
{
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::resizeGL enter"));

	//m_render->setSize(w, h);
	emit viewSizeChanged(w, h);

	widgetFrame->setGeometry(0, 0, w, h);
	userButton->setGeometry(LEFT_SPACE, h - BOTTOM_SPACE - 32, 98, 32);
	if (widgetInfo.bShareScreen)
		screen_width = 110;
	else
		screen_width = 32;
	if (scrollArea ){
		scrollArea->setGeometry(w - 275, 0, w, h);
		speakerButton->setGeometry(w - RIGHT_SPACE - LIST_WIDTH - 32, h - BOTTOM_SPACE - 32, 32, 32);
		widgetContainer->setGeometry(w - RIGHT_SPACE - LIST_WIDTH - 74, h - BOTTOM_SPACE - 32, 74, 32);
		screenButton->setGeometry(w - SCREEN_BTN_RIGHT_SAPCE - LIST_WIDTH - screen_width, h - BOTTOM_SPACE - 32, screen_width, 32);
	}
	else{
		speakerButton->setGeometry(w - RIGHT_SPACE - 32, h - BOTTOM_SPACE - 32, 32, 32);
		widgetContainer->setGeometry(w - RIGHT_SPACE - 74, h - BOTTOM_SPACE - 32, 74, 32);
		screenButton->setGeometry(w - SCREEN_BTN_RIGHT_SAPCE - screen_width, h - BOTTOM_SPACE - 32, screen_width, 32);
	}	
}

void AgoraVideoWidget::paintGL()
{
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::paintGL enter"));

	/*if (!m_render)
		return;

	if (!m_render->isInitialized()){
		m_render->initialize(width(), height());
		//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::paintGL initialize"));
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_render->setFrameInfo(m_rotation, m_mirrored);
		if (m_frame){
			m_render->renderFrame(*m_frame);
			//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::paintGL renderFrame"));
		}
		else{
			widgetFrame->show();
			//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::paintGL show background"));

		}
	}*/
}

void AgoraVideoWidget::SetRenderMode(int mode)
{
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::SetRenderMode enter"));
	std::lock_guard<std::mutex> lock(m_mutex);
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::SetRenderMode after lock_guard"));

	/*if (m_render){
		m_render->setRenderMode(mode);
	}*/
}

int AgoraVideoWidget::deliverFrame(const agora::media::IVideoFrame& videoFrame, int rotation, bool mirrored)
{
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::deliverFrame enter"));

	/*if (videoFrame.IsZeroSize())
		return -1;
	int r;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_rotation = rotation;
		m_mirrored = mirrored;
		r = videoFrame.copyFrame(&m_frame);

		AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::deliverFrame fater lock_guard"));

	}*/

	//notify the render thread to redraw the incoming frame
	emit frameDelivered();
	
	return 0;
}

int AgoraVideoWidget::setViewProperties(int zOrder, float left, float top, float right, float bottom)
{
	

	return -1;
}

void AgoraVideoWidget::cleanup()
{
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::cleanup"));
	/*{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (m_frame)
		{
			m_frame->release();
			m_frame = nullptr;
		}
		AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::cleanup after lock_guard"));
	}
	m_render.reset();*/
	emit widgetInvalidated();
}

void AgoraVideoWidget::renderFrame()
{
	if (widgetFrame->isVisible())
		widgetFrame->hide();
	
}

void AgoraVideoWidget::UpdateScrollArea(QScrollArea* scroll)
{
	if (scroll){
		scrollArea = scroll;
		QRect rc = geometry();
		scrollArea->setGeometry(rc.width() - 275, 0, rc.width(), rc.height());
		int w = rc.width();
		int h = rc.height();
		speakerButton->setGeometry(w - RIGHT_SPACE - LIST_WIDTH - 32, h - BOTTOM_SPACE - 32, 32, 32);
		widgetContainer->setGeometry(w - RIGHT_SPACE - LIST_WIDTH - 74, h - BOTTOM_SPACE - 32, 74, 32);
		screenButton->setGeometry(w - SCREEN_BTN_RIGHT_SAPCE - LIST_WIDTH - screen_width, h - BOTTOM_SPACE - 32, 32, 32);
		scrollArea->show();
	}
	else{
		int w = width();
		int h = height();
		speakerButton->setGeometry(w - RIGHT_SPACE - 32, h - BOTTOM_SPACE - 32, 32, 32);
		widgetContainer->setGeometry(w - RIGHT_SPACE - 74, h - BOTTOM_SPACE - 32, 74, 32);
		screenButton->setGeometry(w - SCREEN_BTN_RIGHT_SAPCE - screen_width, h - BOTTOM_SPACE - 32, 32, 32);
		scrollArea = scroll;
	}
}

void AgoraVideoWidget::UpdateWidget()
{
	//UserInfo localInfo = AgoraQtJson::GetAgoraQtJson()->localuserInfo;
	/*SetUserButton(localInfo);
	SetScreenButton(localInfo);
	SetAVWidget(localInfo);
	SetSpeakerButton(localInfo);*/

	SetBackground();
}

void AgoraVideoWidget::UpdateWidgetInfo(const WidgetInfo& info)
{
	widgetInfo = info;
	UpdateWidget();
}
void AgoraVideoWidget::ResetVideoFrame()
{
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("AgoraVideoWidget::ResetVideoFrame"));

	update();
}


void AgoraVideoWidget::ResetVideoWidget()
{
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("VideoRenderImpl::ResetVideoWidget"));

	WidgetInfo info;
	info.bShareScreen = false;
	info.bOccupied    = false;
	//info.host_role    = AgoraQtJson::GetAgoraQtJson()->localuserInfo.role;
	//info.host_uid     = AgoraQtJson::GetAgoraQtJson()->localuserInfo.uid;
	info.skip_type    = CAN_NOT_SKIP;
	//info.widgetUserInfo = UserInfo();
	UpdateWidgetInfo(info);
	//ResetVideoFrame();
}

void AgoraVideoWidget::GetWidgetInfo(WidgetInfo& info)
{
	//info.widgetUserInfo = widgetInfo.widgetUserInfo;
	info.bOccupied      = widgetInfo.bOccupied;
	//info.host_role      = widgetInfo.host_role;
	info.host_uid       = widgetInfo.host_uid;
	info.bShareScreen   = widgetInfo.bShareScreen;
}

unsigned int AgoraVideoWidget::GetUid()
{
	return 0;
	//return widgetInfo.widgetUserInfo.uid;
}

void AgoraVideoWidget::SetSkipFlag(SKIP_SCREEN_TYPE type)
{
	widgetInfo.skip_type = type;
}

SKIP_SCREEN_TYPE AgoraVideoWidget::GetSkipFlag()
{
	return widgetInfo.skip_type;
}


void AgoraVideoWidget::SetBackground()
{
	/*if (widgetInfo.widgetUserInfo.role == CLASS_ROLE_TEACHER){
		widgetFrame->setStyleSheet(QLatin1String("background-color:#FFD8D8D8; background-image: url(:/AgoraCourse/Resources/pic-teacher-small.png); background-repeat: none; background-position: center;"));//(QString("background-color:#FFD8D8D8; border-image: url(%1)").arg(":/Resource/Resources/pic-student.png"));//

	}
	else{
		widgetFrame->setStyleSheet(QLatin1String("background-color:#FFD8D8D8; background-image: url(:/AgoraCourse/Resources/pic-student-small.png); background-repeat: none; background-position: center;"));
	}*/
}

/*void AgoraVideoWidget::SetUserButton(UserInfo& localUserInfo)
{
	userButton->setText(widgetInfo.widgetUserInfo.userName);


	userButton->setStyleSheet("QPushButton{\n"
		"    background-color: rgba(0, 0, 0, 128);\n"
		"    color:rgba(255,255,255,1);\n"
		"    border-style: outset;\n"
		"    border-width: 1px;\n"
		"    border-radius: 16px;\n"
		"    border-color: rgba(0, 0, 0, 128);\n"
		"    font: 12px;\n"
		"    height: 32px;\n"
		"}\n"
		"QPushButton:hover{\n"
		"    background-color: rgba(0, 0, 0, 128);\n"
		"    color:rgba(255,255,255,1);\n"
		"    border-style: inset;\n"
		"    border-width: 1px;\n"
		"    border-radius: 16px;\n"
		"    border-color: rgba(0, 0, 0, 128);\n"
		"    font: 12px;\n"
		"    height: 32px;\n"
		"}\n"
		"QPushButton:pressed{\n"
		"    background-color: rgba(0, 0, 0, 128);\n"
		"    color:rgba(255,255,255,1);\n"
		"    border-style: outset;\n"
		"    border-width: 1px;\n"
		"    border-radius: 16px;\n"
		"    border-color: rgba(0, 0, 0, 128);\n"
		"    font: 12px;\n"
		"    height: 32px;\n"
		"};");

	if (localUserInfo.role == CLASS_ROLE_TEACHER
		&& widgetInfo.widgetUserInfo.role == CLASS_ROLE_STUDENT){//本地是老师 widget是学生，显示点名
		if (widgetInfo.widgetUserInfo.online){//学生连麦
			userButton->setStyleSheet("QPushButton{\n"
				"    background-image: url(:/AgoraCourse/Resources/icon-choosen white.png);"
				"    background-repeat: none; background-position: right;"
				"    background-color: rgb(68, 162, 252);"
				"    padding-right: 24;"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 16px;\n"
				"    border-color: rgb(68, 162, 252);"
				"    font: 12px;\n"
				"    height: 32px;\n"
				"}\n"
				"QPushButton:hover{\n"
				"    background-image: url(:/AgoraCourse/Resources/icon-choosen white.png);"
				"    background-repeat: none; background-position: right;"
				"    background-color: rgb(68, 162, 252);"
				"    padding-right: 24;"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: inset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 16px;\n"
				"    border-color: rgb(68, 162, 252);"
				"    font: 12px;\n"
				"    height: 32px;\n"
				"QPushButton:pressed{\n"
				"    background-image: url(:/AgoraCourse/Resources/icon-choosen white.png);"
				"    background-repeat: none; background-position: right;"
				"    background-color: rgb(68, 162, 252);"
				"    padding-right: 24;"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 16px;\n"
				"    border-color: rgb(68, 162, 252);"
				"    font: 12px;\n"
				"    height: 32px;\n"
				"};");

		}
		else{//学生未连麦
			userButton->setStyleSheet("QPushButton{\n"
				"    background-image: url(:/AgoraCourse/Resources/icon-choosen white.png);"
				"    background-repeat: none; background-position: right;"
				"    padding-right: 24;"
				"    background-color: rgba(0, 0, 0, 128);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 16px;\n"
				"    border-color: rgba(0, 0, 0, 128);\n"
				"    font: 12px;\n"
				"    height: 32px;\n"
				"}\n"
				"QPushButton:hover{\n"
				"    background-image: url(:/AgoraCourse/Resources/icon-choosen blue.png);"
				"    background-repeat: none; background-position: right;"
				"    padding-right: 24;"
				"    background-color: rgba(0, 0, 0, 128);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: inset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 16px;\n"
				"    border-color: rgba(0, 0, 0, 128);\n"
				"    font: 12px;\n"
				"    height: 32px;\n"
				"}\n"
				"QPushButton:pressed{\n"
				"    background-image: url(:/AgoraCourse/Resources/icon-choosen blue.png);"
				"    background-repeat: none; background-position: right;"
				"    padding-right: 24;"
				"    background-color: rgba(0, 0, 0, 128);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 16px;\n"
				"    border-color: rgba(0, 0, 0, 128);\n"
				"    font: 12px;\n"
				"    height: 32px;\n"
				"};");

		}
	
	}
	else if (localUserInfo.role == CLASS_ROLE_STUDENT
		&& widgetInfo.widgetUserInfo.role == CLASS_ROLE_STUDENT
		//&& widgetInfo.widgetUserInfo.uid == localUserInfo.uid//
		&& widgetInfo.widgetUserInfo.online){//蓝色高亮但是没有手
		userButton->setStyleSheet("QPushButton{\n"
			"    background-color: rgb(68, 162, 252);"
			"    color:rgba(255,255,255,1);\n"
			"    border-style: outset;\n"
			"    border-width: 1px;\n"
			"    border-radius: 16px;\n"
			"    border-color: rgb(68, 162, 252);"
			"    font: 12px;\n"
			"    height: 32px;\n"
			"}"

			"QPushButton:hover{\n"
			"    background-color: rgb(68, 162, 252);"
			"    color:rgba(255,255,255,1);\n"
			"    border-style: inset;\n"
			"    border-width: 1px;\n"
			"    border-radius: 16px;\n"
			"    border-color: rgb(68, 162, 252);"
			"    font: 12px;\n"
			"    height: 32px;\n"
			"}"
			"QPushButton:pressed{\n"
			"    background-color: rgb(68, 162, 252);"
			"    color:rgba(255,255,255,1);\n"
			"    border-style: outset;\n"
			"    border-width: 1px;\n"
			"    border-radius: 16px;\n"
			"    border-color:rgb(68, 162, 252);"
			"    font: 12px;\n"
			"    height: 32px;\n"
			"};");
	}

}

void AgoraVideoWidget::SetScreenButton(UserInfo& localUserInfo)
{
	if (localUserInfo.role == CLASS_ROLE_TEACHER
		&& widgetInfo.widgetUserInfo.uid == localUserInfo.uid){
		
		if (widgetInfo.bShareScreen){//共享
			screenButton->setStyleSheet(QLatin1String("QPushButton#screenButton{"
				"   background-image: url(:/AgoraCourse/Resources/icon-share white.png);\n"
				"    background-repeat: none; \n"
				"    background-color: rgb(68, 162, 252);\n"
				"    background-color: rgb(68, 162, 252);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-color:rgba(68, 162, 252, 255);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 16px;\n"
				"    border-color: beige;\n"
				"    font: 14px ;\n"
				"    height: 32px;\n"
				"    padding-left:25px;\n"
				"};"));
			screenButton->setText(QString::fromLocal8Bit("退出共享"));
		}
		else{//未共享
			screenButton->setStyleSheet(QLatin1String("QPushButton#screenButton{\n"
				"    background-color:rgba(68, 162, 252, 255);\n"
				"    border-style: inset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color:rgba(68, 162, 252, 255);\n"
				"    font: bold 16px;\n"
				"    height: 32px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-share white.png);\n"
				"    background-repeat: none; \n"
				"};"));
			screenButton->setText(QString(""));
		}
	
		screenButton->show();
		SetAVWidget(AgoraQtJson::GetAgoraQtJson()->localuserInfo);
	}
	else
		screenButton->hide();

	
	
}*/

/*void AgoraVideoWidget::SetAVWidget(UserInfo& localUserInfo)
{
	bool showSpeaker = false;
	if (localUserInfo.role == CLASS_ROLE_STUDENT
		&& localUserInfo.uid != widgetInfo.widgetUserInfo.uid){
		widgetContainer->hide();
		showSpeaker = true;
	}
	else{
		widgetContainer->show();
		showSpeaker = false;
	}

	if (!showSpeaker){
		if (widgetInfo.widgetUserInfo.isMuteAudio){//&&widgetInfo.widgetUserInfo.online
			audioButton->setStyleSheet(QLatin1String("QWidget#audioButton{\n"
				"    background-color:rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border:none;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-microphone off-gray.png);\n"
				"}\n"
				"QWidget#audioButton:hover{\n"
				"    background-color: rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: inset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"    \n"
				"	background-image: url(:/AgoraCourse/Resources/icon-microphone off-blue.png);\n"
				"}\n"
				"QWidget#audioButton:pressed{\n"
				"    background-color:rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-microphone off-blue.png"
				"on-blue.png);\n"
				"};"));
		}
		else{
			audioButton->setStyleSheet(QLatin1String("QWidget#audioButton{\n"
				"    background-color:rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border:none;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    font: bold 16px;\n"
				"    height: 32px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-microphone on-white.png);\n"
				"}\n"
				"QWidget#audioButton:hover{\n"
				"    background-color: rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: inset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"    \n"
				"	background-image: url(:/AgoraCourse/Resources/icon-microphone on-blue.png);\n"
				"}\n"
				"QWidget#audioButton:pressed{\n"
				"    background-color:rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-microphone on-blue.png);"
				"};"));
		}

		if (widgetInfo.widgetUserInfo.isMuteVideo){//&& widgetInfo.widgetUserInfo.online

			videoButton->setStyleSheet(QLatin1String("QWidget#videoButton{\n"
				"    background-color:rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border:none;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 34px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-camera off-gray.png);\n"
				"}\n"
				"QWidget#videoButton:hover{\n"
				"    background-color: rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: inset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-camera off-blue.png);\n"
				"}\n"
				"QWidget#videoButton:pressed{\n"
				"    background-color:rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-camera off-blue.png);\n"
				"};"));
		}
		else{
			videoButton->setStyleSheet(QLatin1String("QWidget#videoButton{\n"
				"    background-color:rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border:none;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    font: bold 16px;\n"
				"    height: 34px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-camera on-white.png);\n"
				"}\n"
				"QWidget#videoButton:hover{\n"
				"    background-color: rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: inset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"    \n"
				"	background-image: url(:/AgoraCourse/Resources/icon-camera on-blue.png);\n"
				"}\n"
				"QWidget#videoButton:pressed{\n"
				"    background-color:rgba(0, 0, 0, 0);\n"
				"    color:rgba(255,255,255,1);\n"
				"    border-style: outset;\n"
				"    border-width: 1px;\n"
				"    border-radius: 14px;\n"
				"    border-color: rgba(0, 0, 0, 0);\n"
				"    height: 32px;\n"
				"	background-image: url(:/AgoraCourse/Resources/icon-camera on-blue.png);\n"
				"};"));
		}
	}


}

void AgoraVideoWidget::SetSpeakerButton(UserInfo& localUserInfo)
{
	bool showSpeaker = false;
	if (localUserInfo.role == CLASS_ROLE_STUDENT
		&& localUserInfo.uid != widgetInfo.widgetUserInfo.uid){
		speakerButton->show();
		showSpeaker = true;
	}
	else{
		speakerButton->hide();
		showSpeaker = false;
	}
	
	if (showSpeaker){
		//if (localUserInfo.online)
		if (widgetInfo.bMuteSpeaker)
		    speakerButton->setStyleSheet(QLatin1String("QPushButton#speakerButton{\n"
			"    background-color:rgba(216, 216, 216, 0);\n"
			"    border:none;\n"
			"    border-color: rgba(0, 0, 0, 1);\n"
			"    height: 32px;\n"
			"	background-image: url(:/AgoraCourse/Resources/icon-speaker off-gray.png);\n"
			"    back-repeat:none;\n"
			"}"
			"QPushButton#speakerButton:hover{\n"
			"    background-color:rgba(216, 216, 216, 0);\n"
			"    border:none;\n"
			"    border-color: rgba(0, 0, 0, 1);\n"
			"    height: 32px;\n"
			"	background-image: url(:/AgoraCourse/Resources/icon-speaker off-blue.png);\n"
			"    back-repeat:none;;\n"
			"}\n"
			"QPushButton#speakerButton:pressed{\n"
			"   background-color:rgba(216, 216, 216, 0);\n"
			"    border:none;\n"
			"    border-color: rgba(0, 0, 0, 1);\n"
			"    height: 32px;\n"
			"    \n"
			"	background-image: url(:/AgoraCourse/Resources/icon-speaker off-blue.png);\n"
			"    back-repeat:none;\n"
			"};\n"
			""));
		else
			speakerButton->setStyleSheet(QLatin1String("QPushButton#speakerButton{\n"
			"    background-color:rgba(216, 216, 216, 0);\n"
			"    border:none;\n"
			"    border-color: rgba(0, 0, 0, 0);\n"
			"    height: 32px;\n"
			"	background-image: url(:/AgoraCourse/Resources/icon-speaker on-white.png);\n"
			"    back-repeat:none;\n"
			"}\n"
			"QPushButton#speakerButton:hover{\n"
			"    background-color:rgba(216, 216, 216, 0);\n"
			"    border:none;\n"
			"    border-color: rgba(0, 0, 0, 0);\n"
			"    height: 32px;\n"
			"	background-image: url(:/AgoraCourse/Resources/icon-speaker on-blue.png);\n"
			"    back-repeat:none;;\n"
			"}\n"
			"QPushButton#speakerButton:pressed{\n"
			"   background-color:rgba(216, 216, 216, 0);\n"
			"    border:none;\n"
			"    border-color: rgba(0, 0, 0, 0);\n"
			"    height: 32px;\n"
			"	background-image: url(:/AgoraCourse/Resources/icon-speaker on-blue.png);\n"
			"    back-repeat:none;\n"
			"};"
			""));
	}
}*/


/*void AgoraVideoWidget::UpdateLocalAndChangeUserInfoMuteState()
{
	UserInfo changeUserInfo = widgetInfo.widgetUserInfo;
	UserInfo localUserInfo = AgoraQtJson::GetAgoraQtJson()->localuserInfo;
	if (localUserInfo.uid != changeUserInfo.uid){//mute 远端
		if (localUserInfo.role == CLASS_ROLE_STUDENT
			&& changeUserInfo.role == CLASS_ROLE_STUDENT){//本地是学生,变化的是学生
			if (changeUserInfo.online&& !changeUserInfo.isMuteAudio){
				//AgoraRtcEngine::GetAgoraRtcEngine()->MuteRemoteAudio(changeUserInfo.uid, false);
			}
			else{
				//AgoraRtcEngine::GetAgoraRtcEngine()->MuteRemoteAudio(changeUserInfo.uid, true);
			}

			
		}
	}
	else{
	
	}
}

void AgoraVideoWidget::on_RightClick_customContextMenuRequested(const QPoint &pos)
{ 
	if (widgetInfo.skip_type == CAN_NOT_SKIP
		|| widgetInfo.skip_type == BACK_TO_MAIN)
		return;

	if (AgoraQtScreen::GetAgoraQtScreen()->GetScreenCount() == 1)
		return;

	QMenu popup(this);
	QString menuStyle = QLatin1String(
		"QMenu::item{background-color: rgba(217, 217, 217, 255);color: #FF333333;},"
		"QMenu::item:selected{background-color:  rgba(217, 217, 217, 255);color: #FF333333;};");
	this->setStyleSheet(menuStyle);
	QAction* action = NULL;
	//setStyleSheet(QStringLiteral("QMenu{background-color: rgb(0, 255, 255); width:130; height:30;}, QMenu::item{background-color: rgba(217, 217, 217, 255);}; color: #FF333333;"));
	if (widgetInfo.skip_type == SKIP_TO_SCREEN)
		action  = popup.addAction(QString::fromLocal8Bit("投到到扩展屏"), this, SLOT(on_actionSkipToExtendScreen_triggered()));
	else
		action  = popup.addAction(QString::fromLocal8Bit("回到主屏"), this, SLOT(on_actionSkipToExtendScreen_triggered()));
	popup.exec(QCursor::pos());

	popup.removeAction(action);
}*/

void AgoraVideoWidget::on_actionSkipToExtendScreen_triggered()
{
	if (widgetInfo.skip_type == SKIP_TO_SCREEN)
		emit skipToScreen(widgetInfo);
	else{
		emit skipToScreen(widgetInfo);
	}
}

void AgoraVideoWidget::on_actionBackToMainScreen_triggered()
{

}
