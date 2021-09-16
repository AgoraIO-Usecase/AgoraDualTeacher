#include "AgoraSlider.h"
#include <qpainter.h>
#include <QMouseEvent>
AgoraSlider::AgoraSlider(QWidget *parent) : QWidget(parent) {
  show();
  m_curPoint = QPoint(11, 0);
  m_circleBorderSize = 2;
  //m_circleRadious = (this->height() - 2 * m_circleBorderSize) / 5;
  m_circleRadious = 3;
  m_circleBorderColor = QColor(0x00, 0x73, 0xFF);
  m_maxValue = 100;
  m_minValue = 0;
}

AgoraSlider::~AgoraSlider() {}

void AgoraSlider::setMaxValue(qreal value) { m_maxValue = value; }

void AgoraSlider::setMinValue(qreal value) { m_minValue = value; }

void AgoraSlider::setValue(qreal value) {
  m_curValue = value;
  int posX = (this->width() - (m_circleBorderSize+m_circleRadious) * 2) * (m_curValue - m_minValue) /
                 (m_maxValue - m_minValue) +
             m_circleRadious+m_circleBorderSize;
  m_curPoint = QPoint(posX, 0);
  update();
}

void AgoraSlider::setBkColor(QColor color) { m_bkColor = color; }

void AgoraSlider::setFrontColor(QColor color) { m_frontColor = color; }

void AgoraSlider::setCircleColor(QColor color) { m_circleColor = color; }

void AgoraSlider::setCircleBorder(int size, QColor color) {
  m_circleBorderSize = size;
  m_circleBorderColor = color;
}

qreal AgoraSlider::getCurValue() const { return m_curValue; }

qreal AgoraSlider::getMinValue() const { return m_minValue; }

qreal AgoraSlider::getMaxValue() const { return m_maxValue; }

void AgoraSlider::paintEvent(QPaintEvent *e) {
  QPainter painter(this);
  painter.setBrush(m_bkColor);
  painter.setPen(Qt::NoPen);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);
  painter.drawRoundedRect(
      QRect(m_circleRadious + m_circleBorderSize, this->height() / 5 * 2,
            m_curPoint.x() - m_circleRadious + m_circleBorderSize,
            this->height() / 5 /2),
      this->height() / 5/2, this->height() / 5/2);
  painter.setBrush(m_frontColor);
  painter.drawRoundedRect(
      QRect(m_curPoint.x(), this->height() / 5 * 2,
            this->width() - m_curPoint.x()- m_circleRadious - m_circleBorderSize, this->height() / 5/2),
                          this->height() / 5 / 2, this->height() / 5 / 2);
  painter.setBrush(m_circleBorderColor);
  painter.drawEllipse(
      QPointF(m_curPoint.x(),
              ((this->height() - m_circleRadious + m_circleBorderSize-1) /  2)),
                      m_circleRadious + m_circleBorderSize,
                      m_circleRadious + m_circleBorderSize);
  painter.setBrush(m_circleColor);
  painter.drawEllipse(QPointF(m_curPoint.x(), 
	  ((this->height() - m_circleRadious + m_circleBorderSize-1) / 2)),
                      m_circleRadious, m_circleRadious);
}

void AgoraSlider::mousePressEvent(QMouseEvent *event) { getCurPoint(event); }

void AgoraSlider::mouseMoveEvent(QMouseEvent *event) { getCurPoint(event); }

void AgoraSlider::getCurPoint(QMouseEvent *event) {
	if (event->pos().x() < m_circleRadious + m_circleBorderSize) {
    m_curPoint = QPoint(m_circleRadious + m_circleBorderSize, 0);
  } else if (event->pos().x() > this->width() - m_circleRadious- m_circleBorderSize) {
    m_curPoint = QPoint(this->width() - m_circleRadious - m_circleBorderSize, 0);
  } else {
    m_curPoint = event->pos();
  }
  update();
  m_curValue = (qreal)(m_curPoint.x() - m_circleRadious- m_circleBorderSize) /
                   (this->width() - 2 * (m_circleRadious+ m_circleBorderSize)) *
                   (m_maxValue - m_minValue) +
               m_minValue;
  emit PositonChangeSignal(m_curValue);
}
