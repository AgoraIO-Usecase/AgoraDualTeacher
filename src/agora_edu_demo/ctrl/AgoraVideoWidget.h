#ifndef AGORAVIDEOWIDGET_H
#define AGORAVIDEOWIDGET_H

#define STR(str) #str

#include <QFlags>
#include <QTextEdit>
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

struct WidgetInfo {
  agora::edu::EduStream stream_info;
};

static char* edu_role_type_to_str(const agora::edu::EduRoleType& type) {
  switch (type) {
    case agora::edu::EDU_ROLE_TYPE_ASSISTANT:
      return "Assistant";
    case agora::edu::EDU_ROLE_TYPE_TEACHER:
      return "Teacher";
    case agora::edu::EDU_ROLE_TYPE_STUDENT:
      return "Student";
    case agora::edu::EDU_ROLE_TYPE_INVALID:
    default:
      return "Invalid Role Type";
  }
}

class VideoRendererOpenGL;
class AgoraVideoWidget : public QOpenGLWidget {
  // friend class AgoraVideoLayoutManager;
  Q_OBJECT
 public:
  enum FunctionalFlag {
    NONE = 0,
    SHOW_INFO = 1,
    SUPPORT_DRAG = 2,
  };

  Q_DECLARE_FLAGS(FunctionalFlags, FunctionalFlag)
 public:
  explicit AgoraVideoWidget(QWidget* parent = 0);
  ~AgoraVideoWidget();

  void SetEduUserInfo(const agora::edu::EduUser& user_info);

  void SetEduStream(const agora::edu::EduStream* stream);

  agora::edu::EduStream* GetEduStream() { return m_stream_info; };

  void SetFuntionalFlags(FunctionalFlags flags);
  void AppendFuntionalFlags(FunctionalFlags flags) {
    SetFuntionalFlags(func_flags_ | flags);
  }

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
  virtual void mousePressEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  virtual void mouseMoveEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  virtual void mouseReleaseEvent(QMouseEvent* event) Q_DECL_OVERRIDE;
  virtual void mouseDoubleClickEvent(QMouseEvent* event) Q_DECL_OVERRIDE;

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

  FunctionalFlags func_flags_ = SHOW_INFO;
  QTextEdit* m_infoTestEdit = nullptr;

  bool is_drag_ = false;
  QPoint mouse_start_point_;
  QPoint window_top_left_point_;

  agora::edu::EduUser m_user_info;
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

Q_DECLARE_OPERATORS_FOR_FLAGS(AgoraVideoWidget::FunctionalFlags)

#endif  // AGORAVIDEOWIDGET_H
