#pragma once

#include <QWidget>

#include "ui_AgoraVideoListWidget.h"

#include "util.h"

#include "AgoraVideoWidget.h"

using namespace agora::edu;

struct VideoWidgetInfo {
  View* view = nullptr;
  delegate_render delegate_func;
};

class AgoraVideoWidget;
class AgoraVideoListWidget : public QWidget {
  Q_OBJECT

 public:
  AgoraVideoListWidget(QWidget* parent = Q_NULLPTR);
  ~AgoraVideoListWidget();

  void AddVideoWidgetDirectly(AgoraVideoWidget* video_widget);
  void RemoveVideoWidgetDirectly(AgoraVideoWidget* video_widget);
  VideoWidgetInfo AddVideoWidget(const EduStream& stream);
  AgoraVideoWidget* RemoveVideoWidget(const std::string& stream_uuid);
  size_t GetVideoListWidgetsCount() { return video_widgets_.size(); }
  AgoraVideoWidget* GetVideoListWidget(const size_t& index) {
    return video_widgets_[index];
  }

  size_t GetUuidVideoListWidgetsCount() { return uuid_video_widgets_.size(); }
  AgoraVideoWidget* GetUuidVideoListWidget(const size_t& index) {
    return uuid_video_widgets_[index].second;
  }

 protected:
  void resizeEvent(QResizeEvent* event);

 private:
  Ui::AgoraVideoListWidget ui;
  int agora_video_widget_width_ = 0;
  int agora_video_widget_height_ = 0;

  std::vector<AgoraVideoWidget*> video_widgets_;
  std::vector<UuidWidgetPair> uuid_video_widgets_;
};
