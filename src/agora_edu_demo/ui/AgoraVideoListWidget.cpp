#include <QResizeEvent>

#include "AgoraVideoListWidget.h"

AgoraVideoListWidget::AgoraVideoListWidget(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  ui.VideoListWidgets->layout()->setAlignment(Qt::AlignHCenter);
}

AgoraVideoListWidget::~AgoraVideoListWidget() {}

void AgoraVideoListWidget::resizeEvent(QResizeEvent* event) {
  agora_video_widget_width_ = (event->size().width() - 200) / 4;
  agora_video_widget_height_ = event->size().height();
  for (auto& widget : uuid_video_widgets_) {
    widget.second->setMaximumWidth(agora_video_widget_width_);
    widget.second->setMaximumHeight(agora_video_widget_height_);
    widget.second->setMinimumWidth(agora_video_widget_width_);
    widget.second->setMinimumHeight(agora_video_widget_height_);
  }

  for (auto& widget : video_widgets_) {
    widget->setMaximumWidth(agora_video_widget_width_);
    widget->setMaximumHeight(agora_video_widget_height_);
    widget->setMinimumWidth(agora_video_widget_width_);
    widget->setMinimumHeight(agora_video_widget_height_);
  }
}

void AgoraVideoListWidget::AddVideoWidgetDirectly(
    AgoraVideoWidget* video_widget) {
  video_widget->setMaximumWidth(agora_video_widget_width_);
  video_widget->setMaximumHeight(agora_video_widget_height_);
  video_widget->setMinimumWidth(agora_video_widget_width_);
  video_widget->setMinimumHeight(agora_video_widget_height_);
  ui.VideoListWidgets->layout()->addWidget(video_widget);
  video_widgets_.push_back(video_widget);
  video_widget->show();
}

void AgoraVideoListWidget::RemoveVideoWidgetDirectly(
    AgoraVideoWidget* video_widget) {
  ui.VideoListWidgets->layout()->removeWidget(video_widget);
  auto find_it = std::find_if(
      video_widgets_.begin(), video_widgets_.end(),
      [&](const AgoraVideoWidget* widget) { return video_widget == widget; });

  if (find_it == video_widgets_.end()) {
    return;
  }

  ui.VideoListWidgets->layout()->removeWidget(*find_it);
  video_widgets_.erase(find_it);
}

VideoWidgetInfo AgoraVideoListWidget::AddVideoWidget(const EduStream& stream) {
  AgoraVideoWidget* video_widget = new AgoraVideoWidget(this);
  video_widget->SetEduStream(&stream);
  video_widget->setMaximumWidth(agora_video_widget_width_);
  video_widget->setMaximumHeight(agora_video_widget_height_);
  video_widget->setMinimumWidth(agora_video_widget_width_);
  video_widget->setMinimumHeight(agora_video_widget_height_);

  VideoWidgetInfo info;
  info.delegate_func = video_widget->GetReceiver();
  info.view = (View*)video_widget->winId();

  uuid_video_widgets_.push_back({stream.stream_uuid, video_widget});
  ui.VideoListWidgets->layout()->addWidget(video_widget);
  video_widget->show();

  return info;
}

AgoraVideoWidget* AgoraVideoListWidget::RemoveVideoWidget(
    const std::string& uuid) {
  auto find_it =
      std::find_if(uuid_video_widgets_.begin(), uuid_video_widgets_.end(),
                   [&](const UuidWidgetPair& widget_pair) {
                     return widget_pair.first.compare(uuid) == 0;
                   });

  if (find_it == uuid_video_widgets_.end()) {
    return nullptr;
  }

  auto widget = find_it->second;
  ui.VideoListWidgets->layout()->removeWidget(find_it->second);
  uuid_video_widgets_.erase(find_it);

  return widget;
}