#include "QRoundCornerDialog.h"


QRoundCornerDialog::QRoundCornerDialog(QWidget* dialog)
	:QDialog(dialog)
{
}


QRoundCornerDialog::~QRoundCornerDialog()
{
}


void QRoundCornerDialog::paintEvent(QPaintEvent *e)
{
	Q_UNUSED(e);

	QBitmap bmp(this->size());
	bmp.fill();
	QPainter painter(&bmp);
	painter.setPen(Qt::NoPen);

	painter.setBrush(Qt::black);
	painter.setRenderHint(QPainter::HighQualityAntialiasing);

	painter.drawRoundedRect(bmp.rect(), 10, 10);
	setMask(bmp);
}
/*
void QRoundCornerDialog::mousePressEvent(QMouseEvent *event) {
	m_nMouseClick_X_Coordinate = event->x();
	m_nMouseClick_Y_Coordinate = event->y();
}

void QRoundCornerDialog::mouseMoveEvent(QMouseEvent *event) {
	move(event->globalX() - m_nMouseClick_X_Coordinate, event->globalY() - m_nMouseClick_Y_Coordinate);
}*/
