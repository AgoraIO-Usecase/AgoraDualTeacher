#include "AgoraVolumeWidget.h"
#include <QPainter>
#include <QPen>
AgoraVolumeWidget::AgoraVolumeWidget(QWidget* parent) : QWidget(parent) {
  show();
  m_pixmap = QPixmap(size().height(), size().height());
  m_pixmap.fill(QColor(0, 0, 0));
  m_range = 100;
  m_value = 0;
  m_spacer = 4;
  m_rcSize = QSize(4, height());
  m_bkColor = QColor(0xE2, 0xE2, 0xF0);
  m_frontColor = QColor(0x00, 0x73, 0xff);
}

AgoraVolumeWidget::~AgoraVolumeWidget() {}

void AgoraVolumeWidget::setImage(const QPixmap& pixMap) { m_pixmap = pixMap; }

void AgoraVolumeWidget::setBkColor(const QColor& color) { m_bkColor = color; }

void AgoraVolumeWidget::setFrontColor(const QColor& color) {
  m_frontColor = color;
}

void AgoraVolumeWidget::setValue(int value) {
  m_value = value;
  update();
  emit PositonChangeSignal(m_value);
}


void AgoraVolumeWidget::setSpacer(int value) { m_spacer = value; }

void AgoraVolumeWidget::setRange(int value) { m_range = value; }

void AgoraVolumeWidget::setRectSize(const QSize& size) { m_rcSize = size; }

int AgoraVolumeWidget::getValue() { return m_value; }

void AgoraVolumeWidget::paintEvent(QPaintEvent* e) {
  QPainter painter(this);
  painter.drawPixmap(0, (height()- 25)/2, m_pixmap);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  QPen penFront;
  QPen penBk;
  penFront.setWidth(m_rcSize.width());
  penFront.setColor(m_frontColor);

  penBk.setWidth(m_rcSize.width());
  penBk.setColor(m_bkColor);

  int startDrawLinePosX = m_pixmap.width() + 10;
  int endDrawLinePosX = size().width();
  int drawRcLength = endDrawLinePosX - startDrawLinePosX < 0
                         ? 0
                         : endDrawLinePosX - startDrawLinePosX;
  if (m_rcSize.width() == 0 || m_range == 0) return;
  int countRect =
      (drawRcLength - m_rcSize.width()) / (m_rcSize.width() + m_spacer);
  int curMaxRect = countRect * (m_value / (double)m_range);
  for (int i = 0; i < countRect; i++) {
    if (i < curMaxRect) {
      painter.setPen(penFront);
    } else {
      painter.setPen(penBk);
    }
    painter.drawLine(QPoint(m_rcSize.width() / 2 + startDrawLinePosX +
                                (m_rcSize.width() + m_spacer) * i,
                            size().height()),
                     QPoint(m_rcSize.width() / 2 + startDrawLinePosX +
                                (m_rcSize.width() + m_spacer) * i,
                            0));
  }
}
