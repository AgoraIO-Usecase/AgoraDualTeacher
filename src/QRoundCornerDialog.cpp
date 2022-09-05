#include "QRoundCornerDialog.h"
#include <QGuiApplication>
#include "SettingsData.h"
#include "QDebug"
QRoundCornerDialog::QRoundCornerDialog(QWidget* dialog)
	:QDialog(dialog)
{
	//| Qt::WindowSystemMenuHint
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::FramelessWindowHint| Qt::Window | Qt::WindowMinMaxButtonsHint);

	QRect rc = { 0,0,0,0 };
	auto screens = QGuiApplication::screens();
	for (int i = 0; i < screens.size(); ++i) {
		rc = screens[i]->geometry();
	}

	rcMainScreen = screens[0]->geometry();
	setting.bExtend = screens.size() > 1;
	//setting.bExtend = false;
	rate = screens[0]->devicePixelRatio();
	if (screens.size() > 1) {
		rate2 = screens[1]->devicePixelRatio();
		rcSecondScreen = screens[1]->geometry();
	}
	else
 		rcSecondScreen = { 0 , 0, 0, 0};
	int w = rcMainScreen.width() * rate;

	initRate = (float)w / 1920.0f;
	//top button
	topButtonSpace = 25 * initRate;
	topButtonWidth = 50 * initRate;
	topButtonTopMargin = 10 * initRate;
	topButtonRightMargin = 20 * initRate;
	bottomFontSize = 24 * initRate;
	bottomLineHeight = 29 * initRate;
	//dpiType = DPI_1080;
	bottomLabelW = 200 * initRate;
	bottomLabelBottomMargin = 35 * initRate;
}


QRoundCornerDialog::~QRoundCornerDialog()
{
}


//void QRoundCornerDialog::paintEvent(QPaintEvent *e)
//{
//	Q_UNUSED(e);

	/*round corner*/
	/*QBitmap bmp(this->size());
	bmp.fill();
	QPainter painter(&bmp);
	painter.setPen(Qt::NoPen);

	painter.setBrush(Qt::black);
	//painter.setRenderHint(QPainter::HighQualityAntialiasing);
	painter.drawRoundedRect(bmp.rect(), 0, 0);
	setMask(bmp);*/
//}

void QRoundCornerDialog::mousePressEvent(QMouseEvent *event) {
	m_nMouseClick_X_Coordinate = event->x();
	m_nMouseClick_Y_Coordinate = event->y();
}

void QRoundCornerDialog::mouseMoveEvent(QMouseEvent *event) {
	//move(event->globalX() - m_nMouseClick_X_Coordinate, event->globalY() - m_nMouseClick_Y_Coordinate);
}

void QRoundCornerDialog::changeEvent(QEvent* event)
{
	QEvent::Type t = event->type();
	qDebug() << "changeEvent:" << t << "\n";
	
	if (event->type() == QEvent::WindowStateChange)
	{
		Qt::WindowStates state = windowState();
		if(state != Qt::WindowMinimized && state != Qt::WindowNoState)
		{
			setWindowState(Qt::WindowNoState);
		}
		else
			setWindowState(state);
	}
	QDialog::changeEvent(event);
}
