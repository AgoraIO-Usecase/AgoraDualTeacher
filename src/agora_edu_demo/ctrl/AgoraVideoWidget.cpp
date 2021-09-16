#include "AgoraVideoWidget.h"
#include <QMouseEvent>
#include <QtCore/QMetaType>
#include <QtWidgets/QAction>
#include <QtWidgets/QLayout>
#include <QtWidgets/QMenu>
#include <sstream>
#include <QPainter>
#include <QBitmap>
#include "video_render_opengl.h"

#ifdef Q_OS_WIN
#include <Windowsx.h>
#include <qt_windows.h>
#endif

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
  setWindowFlags(Qt::FramelessWindowHint);
  m_infoTestEdit = new QTextEdit(this);
  m_infoTestEdit->setStyleSheet(
	STR(
    font-family : "Microsoft YaHei";
    background-color: rgba(0, 0, 0, 128);
	color : white;
    border: hide;
    border-radius : 8px;
    font-size : 15px; 
 ));
  m_infoTestEdit->setVerticalScrollBarPolicy(
      Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
  m_infoTestEdit->setReadOnly(true);
  m_infoTestEdit->resize(100, 45);
  m_infoTestEdit->hide();
  m_infoTestEdit->move(10, 10);

  connect(this, SIGNAL(frameDelivered()), this, SLOT(renderFrame()));
  m_receiver = [this](agora::edu::IEduVideoFrame* frame) {
    this->deliverFrame(frame, frame->getRotation(), frame->getMirror());
  };
}

AgoraVideoWidget::~AgoraVideoWidget() { cleanup(); }

void AgoraVideoWidget::SetEduUserInfo(const agora::edu::EduUser& user_info) {
  m_user_info = user_info;
  if (func_flags_ & FunctionalFlag::SHOW_INFO) {
    m_infoTestEdit->clear();
    m_infoTestEdit->append(
        QString::fromLocal8Bit(edu_role_type_to_str(user_info.role)));
    m_infoTestEdit->append(QString::fromLocal8Bit(user_info.user_name));
    m_infoTestEdit->show();
  }
}

void AgoraVideoWidget::SetEduStream(const agora::edu::EduStream* stream) {
  std::lock_guard<std::mutex> _(m_mutex);
  m_infoTestEdit->clear();
  if (stream == nullptr) {
    m_infoTestEdit->hide();
    if (m_stream_info) {
      delete m_stream_info;
      m_stream_info = nullptr;
    }
    if (m_frame) m_frame->release();
    m_frame = nullptr;
    update();
    return;
  }

  if (m_stream_info == nullptr) m_stream_info = new agora::edu::EduStream;
  *m_stream_info = *stream;

  if (func_flags_ & FunctionalFlag::SHOW_INFO) {
    m_infoTestEdit->append(
        QString::fromLocal8Bit(edu_role_type_to_str(stream->user_info.role)));
    m_infoTestEdit->append(QString::fromLocal8Bit(stream->user_info.user_name));
    m_infoTestEdit->show();
  }

  update();
}

void AgoraVideoWidget::SetFuntionalFlags(FunctionalFlags flags) {
  func_flags_ = flags;
  if (func_flags_ & FunctionalFlag::SHOW_INFO) {
    m_infoTestEdit->show();
  } else {
    m_infoTestEdit->hide();
  }
}

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
  
}

void AgoraVideoWidget::initializeGL() {}

void AgoraVideoWidget::resizeGL(int w, int h) {
  m_render->setSize(w, h);
  emit viewSizeChanged(w, h);
  //QBitmap bmp(this->size());
  //bmp.fill();
  //QPainter painter(&bmp);
  //painter.setPen(Qt::black);
  //painter.setBrush(Qt::black);
  //painter.setRenderHints(QPainter::HighQualityAntialiasing |
  //                       QPainter::SmoothPixmapTransform);
  //painter.drawRoundedRect(bmp.rect(), 15, 15);
  //setMask(bmp);
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

void AgoraVideoWidget::mousePressEvent(QMouseEvent* event) {
  if (func_flags_ & SUPPORT_DRAG && event->button() == Qt::LeftButton) {
    window_top_left_point_ = frameGeometry().topLeft();
    is_drag_ = true;
    mouse_start_point_ = event->globalPos();
    setCursor(Qt::OpenHandCursor);
  }

  QWidget::mousePressEvent(event);
}

void AgoraVideoWidget ::mouseMoveEvent(QMouseEvent* event) {
  if (is_drag_) {
    QPoint distance = event->globalPos() - mouse_start_point_;
    if (QWidget* widget = (QWidget*)this->parent()) {
      auto size = widget->frameGeometry().size() - this->frameGeometry().size();

      QPoint pos;
      if (distance.x() < 0) {
        pos.setX(std::max(0, (window_top_left_point_ + distance).x()));
      } else {
        pos.setX(
            std::min(size.width(), (window_top_left_point_ + distance).x()));
      }

      if (distance.y() < 0) {
        pos.setY(std::max(46, (window_top_left_point_ + distance).y()));
      } else {
        pos.setY(
            std::min(size.height(), (window_top_left_point_ + distance).y()));
      }
      this->move(pos);
    }
  }

  QWidget::mouseMoveEvent(event);
}

void AgoraVideoWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    is_drag_ = false;
    setCursor(Qt::ArrowCursor);
  }

  QWidget::mouseReleaseEvent(event);
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
