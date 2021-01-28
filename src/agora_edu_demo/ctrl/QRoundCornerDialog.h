#pragma once
#include <QtWidgets/QDialog>
#include <QtGui/QPainter>
#include <QtGui/QBitmap>
#include <QtGui/qevent.h>

class QRoundCornerDialog : public QDialog
{
public:
	QRoundCornerDialog(QWidget*);
	~QRoundCornerDialog();
	void paintEvent(QPaintEvent *e);
private:
	/*void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	int m_nMouseClick_X_Coordinate;
	int m_nMouseClick_Y_Coordinate;*/
};

