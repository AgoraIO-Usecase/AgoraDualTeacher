#ifndef AGORAVIDEOWIDGET_H
#define AGORAVIDEOWIDGET_H

#include <QtWidgets/QLabel>
#include <QtWidgets/QOpenglWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QWidget>
#include <memory>
#include <mutex>
#include "EduStream.h"
#include "IEduUserService.h"
#include "IEduVideoFrame.h"

namespace agora {}  // namespace agora

struct WidgetInfo {
  agora::edu::EduStream stream_info;
};

class VideoRendererOpenGL;
class AgoraVideoWidget : public QOpenGLWidget {
  // friend class AgoraVideoLayoutManager;
  Q_OBJECT

 public:
  explicit AgoraVideoWidget(QWidget* parent = 0);
  ~AgoraVideoWidget();

  void SetEduStream(const agora::edu::EduStream* stream) {
    std::lock_guard<std::mutex> _(m_mutex);
    if (stream == nullptr) {
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
    update();
  }

  agora::edu::EduStream* GetEduStream() { return m_stream_info; };

  int setViewProperties(int zOrder, float left, float top, float right,
                        float bottom);
  int deliverFrame(const agora::edu::IEduVideoFrame* videoFrame, int rotation,
                   bool mirrored);
  void UpdateWidget();
  void UpdateWidgetInfo(const WidgetInfo& info);
  void GetWidgetInfo(WidgetInfo& info);
  unsigned int GetUid();
  agora::edu::delegate_render GetReceiver();

  void ResetVideoWidget();
  void ResetVideoFrame();
  void SetRenderMode(int mode);

 protected:
  virtual void initializeGL() Q_DECL_OVERRIDE;
  virtual void resizeGL(int w, int h) Q_DECL_OVERRIDE;
  virtual void paintGL() Q_DECL_OVERRIDE;
  virtual void mouseDoubleClickEvent(QMouseEvent* event);

 private:
  void InitChildWidget();

  void SetBackground();

 private slots:
  void renderFrame();
  void cleanup();

 signals:
  void touchedSig();
  void frameDelivered();
  void widgetInvalidated();
  void viewSizeChanged(int width, int height);

 private:
  QWidget* widgetContainer;
  QFrame* line;
  QFrame* widgetFrame;

  agora::edu::EduStream* m_stream_info = nullptr;
  // agora video frame
  std::unique_ptr<VideoRendererOpenGL> m_render;
  std::mutex m_mutex;
  // usage of m_frame should be guarded by m_mutex
  agora::edu::IEduVideoFrame* m_frame;
  agora::edu::delegate_render m_receiver;
  int m_rotation;
  bool m_mirrored;

  // widget info
  WidgetInfo widgetInfo;
  bool bShowHndsup;

  bool start_switch = true;
};

#endif  // AGORAVIDEOWIDGET_H
