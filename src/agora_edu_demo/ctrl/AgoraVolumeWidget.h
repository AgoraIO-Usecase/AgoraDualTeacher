#pragma once
#include <QColor>
#include <QPixmap>
#include <QSize>
#include <QWidget>
class AgoraVolumeWidget : public QWidget {
 public:
  Q_OBJECT

 public:
  AgoraVolumeWidget(QWidget *parent);
  virtual ~AgoraVolumeWidget();

  void setImage(const QPixmap &pixMap);
  void setBkColor(const QColor &color);
  void setFrontColor(const QColor &color);

  void setValue(int value);
  void setSpacer(int value);
  void setRange(int value);
  void setRectSize(const QSize &size);
  int getValue();

 protected:
  void paintEvent(QPaintEvent *e);

 signals:
  void PositonChangeSignal(qreal curValue);

 private:
  QColor m_bkColor;
  QColor m_frontColor;

  QPixmap m_pixmap;
  QSize m_rcSize;
  int m_spacer;
  int m_range;
  int m_value;
};
