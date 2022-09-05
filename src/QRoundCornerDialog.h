#pragma once
#include<QDialog>
#include <QPainter>
#include <QBitmap>
#include <qevent.h>
#include "SettingsData.h"
class QRoundCornerDialog : public QDialog
{
public:
	QRoundCornerDialog(QWidget*);
	~QRoundCornerDialog();
	virtual void changeEvent(QEvent* event) override;
protected:

	float initRate = 1.0f;
	float rate = 1.0f;
	float rate2 = 1.0f;
	int topButtonWidth = 50;
	int topButtonSpace = 26;
	int topButtonTopMargin = 10;
	int topButtonRightMargin = 20;
	int bottomFontSize = 24;
	int bottomLineHeight = 29;

	int bottomLabelW = 200;
	int bottomLabelBottomMargin = 35;
private:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	int m_nMouseClick_X_Coordinate;
	int m_nMouseClick_Y_Coordinate;
protected:
	QRect rcMainScreen;
	QRect rcSecondScreen;
};

