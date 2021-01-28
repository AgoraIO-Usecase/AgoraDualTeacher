#include "AgoraVideoWidget.h"
#include <QtCore/QMetaType>
#include <QtWidgets/QAction>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMenu>
#include <sstream>
#include "video_render_opengl.h"

#define BOTTOM_SPACE 10
#define LEFT_SPACE 10
#define RIGHT_SPACE 10
#define SCREEN_AND_AV_SPACE 10  //ср╡Ю
#define AV_WIDGET_WIDTH 74
#define SCREEN_BTN_RIGHT_SAPCE \
  (AV_WIDGET_WIDTH + SCREEN_AND_AV_SPACE + RIGHT_SPACE)
#define MAIN_AV_WIDGET_WIDTH \
  (SCREEN_BTN_RIGHT_SAPCE + SCREEN_AND_AV_SPACE + RIGHT_SPACE + 240 + 15 + 20)
#define LIST_WIDTH 275

int screen_width = 32;

AgoraVideoWidget::AgoraVideoWidget(QWidget* parent)
    : QOpenGLWidget(parent),
      m_render(new VideoRendererOpenGL(width(), height())),
      m_frame(nullptr),
      m_rotation(180),
      m_mirrored(false),
      bShowHndsup(false) {
  qRegisterMetaType<WidgetInfo>("WidgitInfo");
  InitChildWidget();
  setMouseTracking(true);
  setObjectName(QStringLiteral("agoraWidgetRight"));

  connect(this, SIGNAL(frameDelivered()), this, SLOT(renderFrame()));
  m_receiver = [this](agora::edu::IEduVideoFrame* frame) {
    this->deliverFrame(frame, frame->getRotation(), frame->getMirror());
  };
}

AgoraVideoWidget::~AgoraVideoWidget() { cleanup(); }

int AgoraVideoWidget::setViewProperties(int zOrder, float left, float top,
                                        float right, float bottom) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_render)
    return m_render->setStreamProperties(zOrder, left, top, right, bottom);
  return -1;
}

void AgoraVideoWidget::InitChildWidget() {
  QHBoxLayout* layout = new QHBoxLayout(this);
  layout = new QHBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);

  widgetFrame = new QFrame(this);
  widgetFrame->setStyleSheet(
      QLatin1String("background-color:#FFD8D8D8; background-image: "
                    "url(\":/image/resource/icon-camera-off.png\"); "
                    "background-repeat: none; background-position: "
                    "center;"));
  layout->addWidget(widgetFrame);
  this->setLayout(layout);

  /* widgetContainer = new QWidget(this);
   widgetContainer->setObjectName(QStringLiteral("widgetContainer"));
   widgetContainer->setMinimumSize(QSize(74, 32));
   widgetContainer->setMaximumSize(QSize(74, 32));
   widgetContainer->setStyleSheet(
       QLatin1String("QWidget#widgetContainer{\n"
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
   line = new QFrame(widgetContainer);
   line->setObjectName(QStringLiteral("line"));
   line->setGeometry(QRect(37, 10, 2, 12));
   line->setStyleSheet(
       QStringLiteral("background-color: rgba(255, 255, 255,77);"));
   line->setFrameShape(QFrame::VLine);
   line->setFrameShadow(QFrame::Sunken);*/
}

void AgoraVideoWidget::initializeGL() {}

void AgoraVideoWidget::resizeGL(int w, int h) {
  m_render->setSize(w, h);
  emit viewSizeChanged(w, h);
  widgetFrame->setGeometry(0, 0, w, h);
}

void AgoraVideoWidget::paintGL() {
  if (!m_render) return;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_render->isInitialized()) {
      m_render->initialize(width(), height());
    }
  }
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_render->setFrameInfo(m_rotation, m_mirrored);
    if (m_frame && m_stream_info) {
      m_render->renderFrame(m_frame);
    } else {
      widgetFrame->show();
    }
  }
}

void AgoraVideoWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  emit touchedSig();
}

void AgoraVideoWidget::SetRenderMode(int mode) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_render) {
    m_render->setRenderMode(mode);
  }
}

int AgoraVideoWidget::deliverFrame(const agora::edu::IEduVideoFrame* videoFrame,
                                   int rotation, bool mirrored) {
  if (videoFrame->IsZeroSize()) return -1;
  int r;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_rotation = rotation;
    m_mirrored = mirrored;

    if (m_frame) {
      m_frame->release();
      m_frame = nullptr;
    }
    r = videoFrame->copyFrame(&m_frame);
  }
  // notify the render thread to redraw the incoming frame
  emit frameDelivered();
  return r;
}

void AgoraVideoWidget::cleanup() {
  if (m_stream_info) {
    delete m_stream_info;
    m_stream_info = nullptr;
  }

  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_frame) {
      m_frame->release();
      m_frame = nullptr;
    }
  }

  m_render.reset();
  emit widgetInvalidated();
}

void AgoraVideoWidget::renderFrame() {
  if (widgetFrame->isVisible()) widgetFrame->hide();
  update();
}

void AgoraVideoWidget::UpdateWidget() { SetBackground(); }

void AgoraVideoWidget::UpdateWidgetInfo(const WidgetInfo& info) {
  widgetInfo = info;
  UpdateWidget();
}
void AgoraVideoWidget::ResetVideoFrame() {
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_frame) {
      m_frame->release();
      m_frame = nullptr;
    }
  }
  update();
}

void AgoraVideoWidget::ResetVideoWidget() {}

void AgoraVideoWidget::GetWidgetInfo(WidgetInfo& info) {}

unsigned int AgoraVideoWidget::GetUid() {
  return atoll(widgetInfo.stream_info.user_info.user_uuid);
}

agora::edu::delegate_render AgoraVideoWidget::GetReceiver() {
  return m_receiver;
}

void AgoraVideoWidget::SetBackground() {
  widgetFrame->setStyleSheet(
      QLatin1String("background-color:#FFD8D8D8; background-image: "
                    "url(:/image/resource/icon-camera-off@2x.png); "
                    "background-repeat: none; background-position: "
                    "center;"));
}
