#include <QRegularExpressionValidator>
#include <QRegularExpression >
#include "DlgSettings.h"
#include "DlgVersion.h"
#include "DlgInfo.h"
#include "DlgVideoRoom.h"
#include "DlgSettingAudio.h"
#include "DlgSettingVideo.h"
#include "DlgSettingSelect.h"
#include "DlgExtend.h"
#include "VideoWidget.h"
#include "AgoraCourse.h"
#include "AgoraRtcEngine.h"
///////////////////////////////////////////////////////////////
//////////AgoraCourse
///////////////////////////////////////////////////////////////
void AgoraCourse::InitDlg()
{
	setting.rcMainScreen = rcMainScreen;
	setting.rate = rate;
	
	if (setting.bExtend) {
		rate = rate2;
		rcMainScreen = rcSecondScreen;
		int w = rcSecondScreen.width() * rate;
		initRate = w / 1920.0f;
	}
	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	this->setMaximumSize(QSize(rcMainScreen.width(), rcMainScreen.height()));
	this->setMinimumSize(QSize(rcMainScreen.width() * 3 / 4, rcMainScreen.height() * 3 / 4));
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	ui.roomNameEdit->setMaxLength(64);
	topButtonRightMargin = 20 * initRate;
	topButtonTopMargin = 10 * initRate;
	//title
	titleHeight = 80 * initRate;
	titleFontSize = 58 * initRate;
	titleTopMargin = 40 * initRate;
	titleBottomMargin = 115 * initRate;
	titleWidth = 500 * initRate;

	edtFonSize = 18 * initRate;
	edtPaddingLeft = 26 * initRate;
	edtLayoutSpacing = 60 * initRate;
	edtHeight = 90 * initRate;
	edtWidth = 610 * initRate;
	lineHeight = 50 * initRate;
	//login button
	loginButtonTopMargin = 200 * initRate;
	loginButtonHeight = 90 * initRate;
	loginButtonWidth = 240 * initRate;
	loginBtnFontSize = 38 * initRate;
	loginBtnLineHeight = 53 * initRate;
	//initRate Change 
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
    QString style = QString::fromUtf8("border-image: url(:/AgoraCourse/Resources/dualTeacher/white-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);");
	ui.horizontalLayoutWidget->setStyleSheet(style);

	QRegularExpression  regx("[a-zA-Z0-9]+$");
	QValidator* validator = new QRegularExpressionValidator(regx, ui.roomNameEdit);
	ui.roomNameEdit->setValidator(validator);
	//ui.radioButton_Teacher->setChecked(true);
	setWindowIcon(QIcon(":/AgoraCourse/Resources/AgoraCourse.png"));
}

QRect ResetMaxRect(QRect rcMainScreen, bool maximize)
{
	int width = rcMainScreen.width();
	int height = rcMainScreen.height();
	if (!maximize) width = width * 3 / 4;
	else width = width * 4 / 3;
	if (!maximize) height = height * 3 / 4;
	else  height = height * 4 / 3;
	int x = rcMainScreen.x() + (rcMainScreen.width() - width) / 2;
	int y = rcMainScreen.y() + (rcMainScreen.height() - height) / 2;
	rcMainScreen.setX(x);
	rcMainScreen.setY(y);
	rcMainScreen.setWidth(width);
	rcMainScreen.setHeight(height);
	return rcMainScreen;
}
void AgoraCourse::maxButtonChange(bool max)
{
	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	if (setting.bExtend) {
		rate = rate2;
		rcSecondScreen = ResetMaxRect(rcSecondScreen, !max);
		rcMainScreen = rcSecondScreen;
	}else
		rcMainScreen = ResetMaxRect(rcMainScreen, !max);
	
	rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	setEditLayout();
	setLoginButton();
	setBottomLabel();
	ui.horizontalLayoutWidget->hide();
	ui.horizontalLayoutWidget->show();
}

void AgoraCourse::setTopButtonLayout()
{
	float w = topButtonWidth / rate;
	float buttonLeftMargin = rcMainScreen.width() - (w * 4 + 3 * topButtonSpace / rate);
	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	ui.minButton->setMaximumSize(w, w);
	ui.minButton->setMinimumSize(w, w);

	ui.maxButton->setMaximumSize(w, w);
	ui.maxButton->setMinimumSize(w, w);
	ui.settingsButton->setMaximumSize(w, w);
	ui.settingsButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins(buttonLeftMargin, topButtonTopMargin / rate, topButtonRightMargin / rate, 0);
}

void AgoraCourse::setTitleLayout()
{
	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: %2;\n"
		"color: #000000;\n"
		"line-height: %3px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(int(500 / rate) ).arg(int(titleHeight / rate));
	ui.labelTitle->setStyleSheet(style);
	ui.labelTitle->setMaximumSize(QSize(titleWidth / rate, titleHeight / rate));
	ui.labelTitle->setMinimumSize(QSize(titleWidth / rate, titleHeight / rate));
	float titleLeftMargin = (float)(rcMainScreen.width() - titleWidth / rate) / 2;
	ui.horizontalLayout_Title->setContentsMargins((int)titleLeftMargin, titleTopMargin / rate, (int)titleLeftMargin, titleBottomMargin / rate);
}

void AgoraCourse::setEditLayout()
{
	//edit
	QString style = QString::fromUtf8("QLineEdit#roomNameEdit{\n"
		"	border-image: url(:/AgoraCourse/Resources/dualTeacher/);\n"
		"	background-color: rgb(255, 255, 255);\n"
		"	font: %1px;\n"
		"    border: 1px solid;\n"
		"    padding-left: %2px;\n"
		"    border-color: rgb(255, 255, 255);\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color: beige;\n"
		"    font-weight: 500;\n"
		"    color: #B8B6B5;\n"
		"    line-height: %3px;\n"
		"};").arg((int)(edtFonSize / rate)).arg((int)(edtPaddingLeft / rate)).arg((int)(lineHeight / rate));
	ui.roomNameEdit->setStyleSheet(style);
	style = QString::fromUtf8("QLineEdit#userNameEdit{\n"
		"	border-image: url(:/AgoraCourse/Resources/dualTeacher/);\n"
		"	background-color: rgb(255, 255, 255);\n"
		"	font: %1px;\n"
		"    border: 1px solid;\n"
		"    padding-left: %2px;\n"
		"    border-color: rgb(255, 255, 255);\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color: beige;\n"
		"    font-weight: 500;\n"
		"    color: #B8B6B5;\n"
		"    line-height: %3px;\n"
		"};").arg((int)(edtFonSize / rate)).arg((int)(edtPaddingLeft / rate)).arg((int)(lineHeight / rate));
	ui.userNameEdit->setStyleSheet(style);

	QRect rc = ui.roomNameEdit->geometry();
	ui.roomNameEdit->setMaximumSize(QSize(edtWidth / rate, edtHeight / rate));
	ui.roomNameEdit->setMinimumSize(QSize(edtWidth / rate, edtHeight / rate));
	ui.userNameEdit->setMaximumSize(QSize(edtWidth / rate, edtHeight / rate));
	ui.userNameEdit->setMinimumSize(QSize(edtWidth / rate, edtHeight / rate));
	float edtleftMargin = (float)(rcMainScreen.width() - edtWidth) / 2;
	ui.verticalLayout_edit->setSpacing(edtLayoutSpacing / rate);
	ui.verticalLayout_edit->setContentsMargins(QMargins(edtleftMargin / rate, 0, edtleftMargin / rate, 0));
}

void AgoraCourse::setLoginButton()
{
	//login
	int line_height = loginBtnLineHeight / rate;
	int fontSize = loginBtnFontSize / rate;
	QString style = QString::fromUtf8("QPushButton#loginButton{\n"
		"	border-image: url(:/AgoraCourse/Resources/dualTeacher/);\n"
		"    font-size: %1px;\n"
		"    background-color: #484848;\n"
		"    border-style: outset;\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:  #484848;\n"
		"   font-weight: 500;\n"
		"   color: #FFFFFF;\n"
		"   line-height: %2px;\n"
		"}\n"
		"\n"
		"QPushButton#loginButton:hover{\n"
		"  border-image: url(:/AgoraCourse/Resources/dualTeacher/);\n"
		"    font-size: %3px;\n"
		"      background-color: #484848;\n"
		"    border-style: outset;\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:  #484848;\n"
		"   font-weight: 500;\n"
		"   color: #FFFFFF;\n"
		"   line-height: %4px;\n"
		"}\n"
		"QPushButton#loginButton:pressed{\n"
		"	border-image: url(:/AgoraCourse/Resources/dualTeacher/);\n"
		"    font-size: %5px;\n"
		"    background-color: #484848;\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:  #484848;\n"
		"   font-weight: 500;\n"
		"   color: #FFFFFF;\n"
		"   line-height: %6px;\n"
		"};").arg(fontSize).arg(line_height).arg(fontSize).arg(line_height).arg(fontSize).arg(line_height);
	ui.loginButton->setStyleSheet(style);

	float btnleftMargin = (rcMainScreen.width() - (float)loginButtonWidth / rate) / 2;
	ui.loginButton->setMaximumSize(loginButtonWidth / rate, loginButtonHeight / rate);
	ui.loginButton->setMinimumSize(loginButtonWidth / rate, loginButtonHeight / rate);
	ui.horizontalLayout_login->setContentsMargins(QMargins(btnleftMargin, loginButtonTopMargin / rate, btnleftMargin, 0));
}

void AgoraCourse::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(bottomLabelW / rate);
	float leftMargin = (rcMainScreen.width() - bottomLabelW / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin, (float)loginButtonTopMargin / rate, leftMargin, bottomLabelBottomMargin / rate));
}

void AgoraCourse::showEvent(QShowEvent* event)
{
	QDialog::showEvent(event);

	setTopButtonLayout();
	setTitleLayout();
	setEditLayout();
	setLoginButton();
	setBottomLabel();
}

void AgoraCourse::on_settingsButton_clicked()
{
	//设置对话框
	QRect rc = this->screen()->availableGeometry();
	if (!dlgSettings) {
		float r = bMax ? rate * 3 / 4 : rate;
		float r2 = bMax ? rate2 * 3 / 4 : rate2;
		
		dlgSettings = new DlgSettings(initRate, setting.bExtend, rate, rate2, this, this);
		connect(dlgSettings, &DlgSettings::parentMaxSignal,
			this, &AgoraCourse::on_parentMax_slot);
		if (!bMax && dlgSettings->IsMax())
			dlgSettings->maxButtonChange(true);
	}
	else if ((!bMax && dlgSettings->IsMax()) || (bMax && !dlgSettings->IsMax()))
			dlgSettings->on_maxButton_clicked();
	
	dlgSettings->show();
}

void AgoraCourse::on_minButton_clicked()
{
	setWindowState(Qt::WindowMinimized);
}

void AgoraCourse::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	bMax = !bMax;
}

void AgoraCourse::on_closeButton_clicked()
{
	close();
}
void AgoraCourse::on_parentMax_slot(bool childMax)
{
	if (bMax != childMax) {
		on_maxButton_clicked();
	}
}
///////////////////////////////////////////////////////////////
//////////DlgSettings
///////////////////////////////////////////////////////////////
void DlgSettings::InitDlg()
{
	ui.setupUi(this);
	
	if (bSecond) {
		rate = rate2;
		rcMainScreen = rcSecondScreen;
	}
	
	QRect rc = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rc);
	//title
	titleHeight = 67* initRate;
	titleWidth = 96* initRate;
	titleFontSize = 48* initRate;
	titleTopMargin = 48* initRate;
	titleBottomMargin = 172* initRate;
	titleLeftMargin = 135* initRate;
	titleRightMargin = 910* initRate;
	titleSpacing = 700* initRate;
	backbtnWidth = 78* initRate;
	labSettingW = 96* initRate;
	labSettingH = 67* initRate;

	//option
	optionButtonR = 320* initRate;
	optionLabelW = 130* initRate;
	optionLabelH = 45* initRate;
	optionLeftMargin = 374* initRate;
	optionLayoutSpacing = 106* initRate;
	optionSpacing = 20* initRate;

	optionLabFontSize = 32* initRate;
	optionLabLineHeight = 45* initRate;
	bottomLabelTopMargin = 268* initRate;
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

	QString style = QString::fromUtf8("QWidget#horizontalLayoutWidget{"
		"background-image: url(:/AgoraCourse/Resources/dualTeacher/);"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/white-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);}");
	
	ui.horizontalLayoutWidget->setStyleSheet(style);
	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	setBottomLabel();
}

void DlgSettings::setTopButtonLayout()
{
	float w = topButtonWidth / rate;
	float buttonLeftMargin = rcMainScreen.width() - (w * 3 + 2 * topButtonSpace / rate);
	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	ui.minButton->setMaximumSize(w, w);
	ui.minButton->setMinimumSize(w, w);

	ui.maxButton->setMaximumSize(w, w);
	ui.maxButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins(buttonLeftMargin, topButtonTopMargin / rate, topButtonRightMargin / rate, 0);
}

void DlgSettings::setTitleLayout()
{
	ui.btnBack->setMaximumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));
	ui.btnBack->setMinimumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));

	ui.labSetting->setMaximumSize(QSize(labSettingW / rate, labSettingH / rate));
	ui.labSetting->setMinimumSize(QSize(labSettingW / rate, labSettingH / rate));

	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(titleHeight / rate);
	ui.labSetting->setStyleSheet(style);
	ui.labSetting->setMaximumSize(QSize(titleWidth  / rate, titleHeight / rate));
	ui.labSetting->setMinimumSize(QSize(titleWidth / rate, titleHeight / rate));

	int spacing = titleSpacing / rate;
	ui.horizontalLayout_Setting->setSpacing(spacing);

	int l = (float)titleLeftMargin / rate;
	int t = (float)titleTopMargin / rate;
	int r = (float)titleRightMargin / rate;
	int b = (float)titleBottomMargin / rate;
	ui.horizontalLayout_Setting->setContentsMargins(l, t, r, b);
}

void DlgSettings::setOptionLayout()
{
	ui.btnVideo->setMaximumSize(QSize(optionButtonR / rate, optionButtonR / rate));
	ui.btnVideo->setMinimumSize(QSize(optionButtonR / rate, optionButtonR / rate));
	ui.btnAudio->setMaximumSize(QSize(optionButtonR / rate, optionButtonR / rate));
	ui.btnAudio->setMinimumSize(QSize(optionButtonR / rate, optionButtonR / rate));

	ui.btnVersion->setMaximumSize(QSize(optionButtonR / rate, optionButtonR / rate));
	ui.btnVersion->setMinimumSize(QSize(optionButtonR / rate, optionButtonR / rate));

	ui.labelVideo->setMaximumSize(QSize(optionButtonR / rate, optionLabelH / rate));
	ui.labelVideo->setMinimumSize(QSize(optionButtonR / rate, optionLabelH / rate));

	ui.labelAudio->setMaximumSize(QSize(optionButtonR / rate, optionLabelH / rate));
	ui.labelAudio->setMinimumSize(QSize(optionButtonR / rate, optionLabelH / rate));

	ui.labelVersion->setMaximumSize(QSize(optionButtonR / rate, optionLabelH / rate));
	ui.labelVersion->setMinimumSize(QSize(optionButtonR / rate, optionLabelH / rate));

	QVector<QString> vecName;
	QVector<QLabel*> labs;
	vecName.push_back("labelVideo");
	vecName.push_back("labelAudio");
	vecName.push_back("labelVersion");
	labs.push_back(ui.labelVideo);
	labs.push_back(ui.labelAudio);
	labs.push_back(ui.labelVersion);

	for (int i = 0; i < labs.size(); ++i) {
		QString style = QString::fromUtf8("QLabel#%1{\n"
			"font-size: %2px;\n"
			"font-family: PingFangSC-Medium, PingFang SC;\n"
			"font-weight: 500;\n"
			"color: rgba(0, 0, 0, 0.5);\n"
			"line-height: %3px;\n"
			"}").arg(vecName[i]).arg((int)(optionLabFontSize / rate)).arg((int)(optionLabLineHeight / rate));
		labs[i]->setStyleSheet(style);
	}

	int spacing = (float)optionSpacing / rate;
	ui.verticalLayout_Video->setSpacing(spacing);
	ui.verticalLayout_Version->setSpacing(spacing);
	ui.verticalLayout_Audio->setSpacing(spacing);
	spacing = (float)optionLayoutSpacing / rate;
	ui.horizontalLayout_OptionButton->setSpacing(spacing);

	int l = (float)optionLeftMargin / rate;
	int t = 0;
	int r = (float)optionLeftMargin / rate;
	int b = 0;
	ui.horizontalLayout_OptionButton->setContentsMargins(l, t, r, b);

}

void DlgSettings::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(200.0f / rate);
	float leftMargin = (rcMainScreen.width() - bottomLabelW / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin, bottomLabelTopMargin / rate, leftMargin, bottomLabelBottomMargin / rate));
}


void DlgSettings::on_minButton_clicked()
{
	agoraCourse->setWindowState(Qt::WindowMinimized);
}

void DlgSettings::on_closeButton_clicked()
{
	emit closeSettingDlg();
	close();
}

void DlgSettings::on_btnBack_clicked()
{
	emit closeSettingDlg();
	close();
}

void DlgSettings::on_btnVersion_clicked()
{
	if (!dlgVersion) {
		dlgVersion = new DlgVersion(initRate, rate, rate2, agoraCourse, this, bSecond);
		connect(dlgVersion, &DlgVersion::parentMaxSignal,
			this, &DlgSettings::on_parentMax_slot);
		if (!bMax && dlgVersion->IsMax())
			dlgVersion->maxButtonChange(true);
	}
	else if ((!bMax && dlgVersion->IsMax()) || (bMax && !dlgVersion->IsMax()))
		dlgVersion->on_maxButton_clicked();
	dlgVersion->show();
}
void DlgSettings::maxButtonChange(bool max)
{
	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	if (setting.bExtend) {
		rate = rate2;
		rcSecondScreen = ResetMaxRect(rcSecondScreen, !max);
		rcMainScreen = rcSecondScreen;
	}
	else
		rcMainScreen = ResetMaxRect(rcMainScreen, !max);

	rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	setBottomLabel();
	bMax = !max;
	ui.horizontalLayoutWidget->hide();
	ui.horizontalLayoutWidget->show();
}

void DlgSettings::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	emit parentMaxSignal(bMax);
	//hide();
	//show();
}
void DlgSettings::on_parentMax_slot(bool childMax)
{
	if (bMax != childMax) {
		on_maxButton_clicked();
	}
}
///////////////////////////////////////////////////////////////
//////////DlgSettingAudio
///////////////////////////////////////////////////////////////
void DlgSettingAudio::InitDlg()
{
	if (bSecond) {
		rate = rate2;
		rcMainScreen = rcSecondScreen;
	}
	//title
	titleHeight = 67 * initRate;
	titleFontSize = 48 * initRate;
	titleTopMargin = 48 * initRate;
	titleBottomMargin = 60 * initRate;
	titleLeftMargin = 135 * initRate;
	titleRightMargin = 886 * initRate;
	titleSpacing = 676 * initRate;
	backbtnWidth = 78 * initRate;
	labTitleW = 192 * initRate;
	labTitleH = 67 * initRate;

	//audio
	audioLeftMargin = 562 * initRate;
	audioRightMargin = 460 * initRate;
	audioLayout = 75 * initRate;
	audioSpace = 75 * initRate;
	audio3ALeftMargin = 540 * initRate;
	versionSpacing = 30 * initRate;
	verticalLayout_MicInfoBottomMargin = 110 * initRate;
	labelSpeakerInfoH = 53 * initRate;
	labelSpeakerInfoW = 190 * initRate;
	label3AW = 230 * initRate;

	//right mic
	verticalLayout_MicSettingSpacing = 55 * initRate;
	verticalLayout_MicSpace = 13* initRate;
	verticalLayout_SpeakerSettingSpacing = 48* initRate;
	horizontalLayout_MicDeviceSpace = 34* initRate;
	horizontalLayout_MicDeviceLeftMargin = 20* initRate;
	deviceCtrlW = 480* initRate;
	micCtrlH = 37* initRate;
	volumeCtrl = 500* initRate;
	labW = 78* initRate;
	labH = 37* initRate;
	testBtnW = 96* initRate;
	testBtnH = 65* initRate;
	lineW = 630* initRate;
	horizontalLayout_AGCSpace = 80* initRate;
	horizontalLayout_AGCTopMargin = 20* initRate;
	horizontalLayout_AGCRightMargin = 50* initRate;
	backForwardW = 52* initRate;
	combox3AW = 140* initRate;
	fontMidSize = 38* initRate;
	lineMidHeight = 53* initRate;
	fontLittleSize = 26* initRate;
	lineLittleHeight = 37* initRate;
	micSelectTextPadding = 50* initRate;
	bottomTopMargin = 170* initRate;

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
	setMinimumSize(QSize(rcMainScreen.width() * 3 / 4, rcMainScreen.height() * 3 / 4));

	this->setGeometry(rcMainScreen);
	QString style = QString::fromUtf8("QWidget#horizontalLayoutWidget{"
		"background-image: url(:/AgoraCourse/Resources/dualTeacher/);"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/white-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);}");
	if (bSecond) {
		QRect rc = { 0, 0, rcSecondScreen.width(), rcSecondScreen.height() };
		ui.horizontalLayoutWidget->setGeometry(rc);
	}
	else {
		ui.horizontalLayoutWidget->setGeometry(rcMainScreen);
	}
	
	ui.horizontalLayoutWidget->setStyleSheet(style);
	setTopButtonLayout();
	setTitleLayout();
	setAudioLayout();
	setBottomLabel();

	ui.btnAGCStatsPre->setEnabled(setting.agcOn);
	ui.btnAGCStatsNext->setEnabled(setting.agcOn);
	ui.labelAGCStats->setText(setting.agcOn ?
		QString::fromStdWString(L"开启") : 
		QString::fromStdWString(L"关闭"));
	ui.btnAECStatsPre->setEnabled(setting.aecOn);
	ui.btnAECStatsNext->setEnabled(setting.aecOn);
	ui.labelAECStats->setText(setting.agcOn ?
		QString::fromStdWString(L"开启") :
		QString::fromStdWString(L"关闭"));
	ui.btnANSStatsPre->setEnabled(setting.ansOn);
	ui.btnANSStatsPre->setEnabled(setting.ansOn);
	ui.labelANSStats->setText(setting.ansOn ?
		QString::fromStdWString(L"开启") :
		QString::fromStdWString(L"关闭"));
}

void DlgSettingAudio::setTopButtonLayout()
{
	float w = topButtonWidth / rate;
	float buttonLeftMargin = rcMainScreen.width() - (w * 3 + 2 * topButtonSpace / rate);
	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	ui.minButton->setMaximumSize(w, w);
	ui.minButton->setMinimumSize(w, w);

	ui.maxButton->setMaximumSize(w, w);
	ui.maxButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins((int)buttonLeftMargin, topButtonTopMargin, topButtonRightMargin / rate, 0);

}
void DlgSettingAudio::setTitleLayout()
{
	ui.btnBack->setMaximumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));
	ui.btnBack->setMinimumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));

	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));

	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(titleHeight / rate);
	ui.labTitle->setStyleSheet(style);
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, titleHeight / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, titleHeight / rate));

	int spacing = (float)titleSpacing / rate;
	ui.horizontalLayout_Setting->setSpacing(spacing);

	int l = (float)titleLeftMargin / rate;
	int t = (float)titleTopMargin / rate;
	int r = (float)titleRightMargin / rate;
	int b = (float)titleBottomMargin / rate;
	ui.horizontalLayout_Setting->setContentsMargins(l, t, r, b);
}

void DlgSettingAudio::setAudioLayout()
{
	ui.horizontalLayout_Mic->setSpacing(audioSpace);
	ui.horizontalLayout_Mic->setContentsMargins(audioLeftMargin / rate, 0, audioRightMargin / rate, audioLayout / rate);

	ui.horizontalLayout_Speaker->setSpacing(audioSpace);
	ui.horizontalLayout_Speaker->setContentsMargins(audioLeftMargin / rate, 0, audioRightMargin / rate, audioLayout / rate);

	ui.horizontalLayout_3ASetting->setSpacing(audioSpace);
	ui.horizontalLayout_3ASetting->setContentsMargins(audio3ALeftMargin / rate, 0, audioRightMargin / rate, 0);
	ui.verticalLayout_MicInfo->setContentsMargins(0, 0, 0, verticalLayout_MicInfoBottomMargin / rate);
	ui.verticalLayout_MicInfo->setContentsMargins(0, 0, 0, verticalLayout_MicInfoBottomMargin / rate);
	ui.verticalLayout_SpeakerInfo->setContentsMargins(0, 0, 0, verticalLayout_MicInfoBottomMargin / rate);
	ui.verticalLayout_SpeakerInfo->setContentsMargins(0, 0, 0, verticalLayout_MicInfoBottomMargin / rate);

	ui.labelMicInfo->setMaximumSize(QSize(labelSpeakerInfoW / rate, labelSpeakerInfoH / rate));
	ui.labelMicInfo->setMinimumSize(QSize(labelSpeakerInfoW / rate, labelSpeakerInfoH / rate));
	ui.labelSpeakerInfo->setMaximumSize(QSize(labelSpeakerInfoW / rate, labelSpeakerInfoH / rate));
	ui.labelSpeakerInfo->setMinimumSize(QSize(labelSpeakerInfoW / rate, labelSpeakerInfoH / rate));
	ui.label3A->setMaximumSize(QSize(label3AW / rate, labelSpeakerInfoH / rate));
	ui.label3A->setMinimumSize(QSize(label3AW / rate, labelSpeakerInfoH / rate));
	//右侧麦克风
	ui.verticalLayout_MicSetting->setSpacing(verticalLayout_MicSettingSpacing / rate);
	ui.verticalLayout_Mic->setSpacing(verticalLayout_MicSpace / rate);
	ui.horizontalLayout_MicDevice->setSpacing(horizontalLayout_MicDeviceSpace / rate);
	ui.horizontalLayout_MicDevice->setContentsMargins(horizontalLayout_MicDeviceLeftMargin / rate, 0, 0, 0);

	ui.labelMic->setMaximumSize(QSize(labW / rate, labH / rate));
	ui.labelMic->setMinimumSize(QSize(labW / rate, labH / rate));
	ui.btnMicSelect->setMaximumSize(QSize(deviceCtrlW / rate, micCtrlH / rate));
	ui.btnMicSelect->setMinimumSize(QSize(deviceCtrlW / rate, micCtrlH / rate));

	ui.horizontalLayout_MicTest->setContentsMargins(0, (labH - testBtnW) / rate, 0, (labH - testBtnW) / rate);
	ui.horizontalLayout_MicTest->setSpacing(horizontalLayout_MicDeviceSpace / rate);
	ui.btnTestMic->setMaximumSize(QSize(testBtnW / rate, testBtnH / rate));
	ui.btnTestMic->setMinimumSize(QSize(testBtnW / rate, testBtnH / rate));
	ui.progressBarMicVolume->setMaximumSize(QSize(volumeCtrl / rate, micCtrlH / rate));
	ui.progressBarMicVolume->setMinimumSize(QSize(volumeCtrl / rate, micCtrlH / rate));
	
	ui.lineMic->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineMic->setMinimumSize(QSize(lineW / rate, 1));
	ui.lineSpeaker->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineSpeaker->setMinimumSize(QSize(lineW / rate, 1));
	ui.lineAGC->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineAGC->setMinimumSize(QSize(lineW / rate, 1));
	ui.lineAEC->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineAEC->setMinimumSize(QSize(lineW / rate, 1));
	ui.lineANS->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineANS->setMinimumSize(QSize(lineW / rate, 1));
	//扬声器
	ui.verticalLayout_SpeakerSetting->setSpacing(verticalLayout_SpeakerSettingSpacing / rate);
	ui.verticalLayout_Speaker->setSpacing(verticalLayout_MicSpace / rate);
	ui.horizontalLayout_SpeakerDevice->setSpacing(horizontalLayout_MicDeviceSpace / rate);
	ui.horizontalLayout_SpeakerDevice->setContentsMargins(horizontalLayout_MicDeviceLeftMargin / rate, 0, 0, 0);

	ui.labelSpeaker->setMaximumSize(QSize(labW / rate, labH / rate));
	ui.labelSpeaker->setMinimumSize(QSize(labW / rate, labH / rate));
	ui.btnSpeakerSelect->setMaximumSize(QSize(deviceCtrlW / rate, micCtrlH / rate));
	ui.btnSpeakerSelect->setMinimumSize(QSize(deviceCtrlW / rate, micCtrlH / rate));

	ui.horizontalLayout_SpeakerTest->setSpacing(horizontalLayout_MicDeviceSpace / rate);
	ui.btnTestSpeaker->setMaximumSize(QSize(testBtnW / rate, testBtnH / rate));
	ui.btnTestSpeaker->setMinimumSize(QSize(testBtnW / rate, testBtnH / rate));
	ui.progressBarSpeakerVolume->setMaximumSize(QSize(volumeCtrl / rate, micCtrlH / rate));
	ui.progressBarSpeakerVolume->setMinimumSize(QSize(volumeCtrl / rate, micCtrlH / rate));
	//AGC
	ui.horizontalLayout_AGC->setSpacing(horizontalLayout_AGCSpace / rate);
	ui.horizontalLayout_AGC->setContentsMargins(horizontalLayout_AGCTopMargin / rate, 0, horizontalLayout_AGCRightMargin / rate, 0);

	ui.btnAGCStatsPre->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnAGCStatsPre->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnAGCStatsNext->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnAGCStatsNext->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.labelAGCStats->setMaximumSize(QSize(labW / rate, labH / rate));
	ui.labelAGCStats->setMinimumSize(QSize(labW / rate, labH / rate));

	ui.labAGCSelect->setMaximumSize(QSize(combox3AW / rate, labH / rate));
	ui.labAGCSelect->setMinimumSize(QSize(combox3AW / rate, labH / rate));

	//AEC
	ui.horizontalLayout_AECDevice->setSpacing(horizontalLayout_AGCSpace / rate);
	ui.horizontalLayout_AECDevice->setContentsMargins(horizontalLayout_AGCTopMargin / rate, 0, horizontalLayout_AGCRightMargin / rate, 0);

	ui.btnAECStatsPre->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnAECStatsPre->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnAECStatsNext->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnAECStatsNext->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.labelAECStats->setMaximumSize(QSize(labW / rate, labH / rate));
	ui.labelAECStats->setMinimumSize(QSize(labW / rate, labH / rate));

	ui.labAECSelect->setMaximumSize(QSize(combox3AW / rate, labH / rate));
	ui.labAECSelect->setMinimumSize(QSize(combox3AW / rate, labH / rate));

	//ANS
	ui.horizontalLayout_ANSDevice->setSpacing(horizontalLayout_AGCSpace / rate);
	ui.horizontalLayout_ANSDevice->setContentsMargins(horizontalLayout_AGCTopMargin / rate, 0, horizontalLayout_AGCRightMargin / rate, 0);

	ui.btnANSStatsPre->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnANSStatsPre->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnANSStatsNext->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnANSStatsNext->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.labelANSStats->setMaximumSize(QSize(labW / rate, labH / rate));
	ui.labelANSStats->setMinimumSize(QSize(labW / rate, labH / rate));

	ui.labANSSelect->setMaximumSize(QSize(combox3AW / rate, labH / rate));
	ui.labANSSelect->setMinimumSize(QSize(combox3AW / rate, labH / rate));

	int labelFontSize = fontMidSize / rate;
	int line_height = lineMidHeight / rate;
	QString style = QString::fromUtf8("QLabel#labelMicInfo{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelMicInfo->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labelSpeakerInfo{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelSpeakerInfo->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#label3A{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.label3A->setStyleSheet(style);

	labelFontSize = fontLittleSize / rate;
	line_height = lineLittleHeight / rate;
	style = QString::fromUtf8("QLabel#labelMic{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelMic->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labelSpeaker{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelSpeaker->setStyleSheet(style);

	style = QString::fromUtf8("QPushButton#btnTestMic{\n"
		"    background-color: rgba(72, 72, 72, 1);\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:   #FFFFFFFF;\n"
		"   font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #FFFFFF;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);

	ui.btnTestMic->setStyleSheet(style);

	style = QString::fromUtf8("QPushButton#btnTestSpeaker{\n"
		"    background-color: rgba(72, 72, 72, 1);\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:   #FFFFFFFF;\n"
		"   font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #FFFFFF;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.btnTestSpeaker->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labelAGCStats{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelAGCStats->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labelAECStats{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelAECStats->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labelANSStats{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelANSStats->setStyleSheet(style);



	style = QString::fromUtf8("QPushButton#btnMicSelect{\n"
		"	background-image: url(:/AgoraCourse/Resources/dualTeacher/traingle.png);\n"
		"   background-position:right;\n"
		"   background-repeat:none;\n"
		"   border:none;\n"
		"   background-color: rgba(255, 255, 255,0);\n"
		"   font-size: %1px;\n"
		"   font-family: PingFangSC-Medium, PingFang SC;\n"
		"   font-weight: 500;\n"
		"   color: #333333;\n"
		"   line-height: %2px;\n"
		"   text-align:left;\n"
		"   padding-right: %3px;\n"
		"}").arg(labelFontSize).arg(line_height).arg((int)(micSelectTextPadding / rate));
	ui.btnMicSelect->setStyleSheet(style);
	style = QString::fromUtf8("\n"
		"                    QPushButton#btnSpeakerSelect{\n"
		"                    background-image: url(:/AgoraCourse/Resources/dualTeacher/traingle.png);\n"
		"                    background-position:right;\n"
		"                    background-repeat:none;\n"
		"                    border:none;\n"
		"                    background-color: rgba(255, 255, 255,0);\n"
		"                    font-size: %1px;\n"
		"                    font-family: PingFangSC-Medium, PingFang SC;\n"
		"                    font-weight: 500;\n"
		"                    color: #333333;\n"
		"                    line-height: %2px;\n"
		"                    text-align:left;\n"
		"                    padding-right: %3px;\n"
		"                    }\n"
		"                  ").arg(labelFontSize).arg(line_height).arg((int)(micSelectTextPadding / rate));
	ui.btnSpeakerSelect->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labAGCSelect{\n"
		"     border:none;\n"
		"	background-color: rgba(255, 255, 255,0);\n"
		"    font-size: %1px;\n"
		"   font-family: PingFangSC-Semibold, PingFang SC;\n"
		"   font-weight: 600;\n"
		"   color:rgba(51, 51, 51, 1);\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labAGCSelect->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labAECSelect{\n"
		"     border:none;\n"
		"	background-color: rgba(255, 255, 255,0);\n"
		"    font-size: %1px;\n"
		"   font-family: PingFangSC-Semibold, PingFang SC;\n"
		"   font-weight: 600;\n"
		"   color:rgba(51, 51, 51, 1);\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labAECSelect->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labANSSelect{\n"
		"     border:none;\n"
		"	background-color: rgba(255, 255, 255,0);\n"
		"    font-size: %1px;\n"
		"   font-family: PingFangSC-Semibold, PingFang SC;\n"
		"   font-weight: 600;\n"
		"   color:rgba(51, 51, 51, 1);\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labANSSelect->setStyleSheet(style);
}
void DlgSettingAudio::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(bottomLabelW / rate);
	float leftMargin = (rcMainScreen.width() - bottomLabelW / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin,bottomTopMargin / rate, leftMargin, bottomLabelBottomMargin / rate));
}
void DlgSettingAudio::on_minButton_clicked()
{
	agoraCourse->setWindowState(Qt::WindowMinimized);
}

void DlgSettingAudio::on_parentMax_slot(bool childMax)
{
	if (bMax != childMax) {
		on_maxButton_clicked();
	}
}
void DlgSettingAudio::maxButtonChange(bool max)
{
	if (setting.bExtend) {
		rate = rate2;
		rcSecondScreen = ResetMaxRect(rcSecondScreen, !max);
		rcMainScreen = rcSecondScreen;
	}
	else
		rcMainScreen = ResetMaxRect(rcMainScreen, !max);

	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	setAudioLayout();
	setBottomLabel();
	bMax = !max;
	ui.horizontalLayoutWidget->hide();
	ui.horizontalLayoutWidget->show();
}

void DlgSettingAudio::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	emit parentMaxSignal(bMax);
}
///////////////////////////////////////////////////////////////
///////DlgSettingVideo
///////////////////////////////////////////////////////////////
void DlgSettingVideo::InitDlg()
{
	if (bSecond) {
		rate = rate2;
		rcMainScreen = rcSecondScreen;
	}

	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	setMinimumSize(QSize(rcMainScreen.width() * 3 / 4, rcMainScreen.height() * 3 / 4));
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	//title
	titleHeight = 67* initRate;
	titleFontSize = 48* initRate;
	titleTopMargin = 48* initRate;
	titleBottomMargin = 10* initRate;
	titleLeftMargin = 135* initRate;
	titleRightMargin = 886* initRate;
	titleSpacing = 676* initRate;
	backbtnWidth = 78* initRate;
	labTitleW = 192* initRate;
	labTitleH = 67* initRate;

	//videsource
	sourceLeftMargin = 390* initRate;
	sourceRightMargin = 296* initRate;
	sourceLayout = 75* initRate;
	sourceSpace = 75* initRate;
	videoInfoSpace = 12* initRate;
	videoInfoW = 89* initRate;
	videoInfoH = 37* initRate;
	videoWidgetW = 540* initRate;
	videoWidgetH = 302* initRate;

	//vdeisource1
	videoSource1Space = 30* initRate;
	verticalLayout_Video1Space = 13* initRate;
	horizontalLayout_video1_1Space = 34* initRate;
	horizontalLayout_video1_1LeftMargin = 0;// 20 * initRate;
	labelInfoW = 104* initRate;
	labelInfoH = 37* initRate;
	videoDeviceW = 420* initRate;

	horizontalLayout_ResVideo1Space = 184* initRate;
	backForwardW = 52* initRate;
	resBtnW = 153* initRate;
	lineSpace = 13* initRate;
	//
	fontSize = 26* initRate;
	lineHeight = 37* initRate;
	buttonW = 430* initRate;
	buttonH = 66* initRate;
	micSelectTextPadding = 60* initRate;
	versionSpacing = 30* initRate;
	lineW = 630* initRate;

	edtFontSize = 18* initRate;
	edtPadding = 20* initRate;
	edtLineHeight = 50* initRate;

	horizontalLayout_video2_1Spacing = 47* initRate;
	edtWidth = 415* initRate;
	edtHeight = 50* initRate;
	horizonalVideoSource1BottomMargin = 40 * initRate;

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
	
	QString style = QString::fromUtf8("QWidget#horizontalLayoutWidget{"
		"background-image: url(:/AgoraCourse/Resources/dualTeacher/);"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/white-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);}");
	
	ui.horizontalLayoutWidget->setStyleSheet(style);
	setEditStyle(true);
	setTopButtonLayout();
	setTitleLayout();
	setVideoSourceLayout();
	setBottomLabel();
	setEditStyle(setting.enabledVideoSource2);

	SetEnabledVideoSource1();
	SetEnabledVideoSource2();

	ui.lineEditVideoSource2->setText(setting.videoSource2Url);
	connect(AgoraRtcEngine::GetAgoraRtcEngine(), &AgoraRtcEngine::openPlayerComplete,
		this, &DlgSettingVideo::on_openPlayerComplete);
}

void DlgSettingVideo::SetEnabledVideoSource2()
{
	ui.labvideo2Stat->setText(setting.enabledVideoSource2 ?
		QString::fromStdWString(L"开启") :
		QString::fromStdWString(L"关闭"));
	ui.labvideo2Stats->setText(setting.enabledVideoSource2 ?
		QString::fromStdWString(L"开启") :
		QString::fromStdWString(L"关闭"));

	ui.lineEditVideoSource2->setEnabled(true);
}


void DlgSettingVideo::SetEnabledVideoSource1()
{
	ui.labVideo1Stat->setText(setting.enabledVideoSource1 ?
		QString::fromStdWString(L"开启") :
		QString::fromStdWString(L"关闭"));
	ui.labVideo1Stats->setText(setting.enabledVideoSource1 ?
		QString::fromStdWString(L"开启") :
		QString::fromStdWString(L"关闭"));
	ui.btnVideo1FPSPre->setEnabled(setting.enabledVideoSource1);
	ui.btnVideo1FPSNext->setEnabled(setting.enabledVideoSource1);

	ui.btnVideo1ResPre->setEnabled(setting.enabledVideoSource1);
	ui.btnVideo1ResNext->setEnabled(setting.enabledVideoSource1);
}


void DlgSettingVideo::setTopButtonLayout()
{
	float w = topButtonWidth / rate;
	float buttonLeftMargin = rcMainScreen.width() - (w * 3 + 2 * topButtonSpace / rate);
	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	ui.minButton->setMaximumSize(w, w);
	ui.minButton->setMinimumSize(w, w);

	ui.maxButton->setMaximumSize(w, w);
	ui.maxButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins(buttonLeftMargin, topButtonTopMargin / rate, topButtonRightMargin / rate, 0);
}

void DlgSettingVideo::setTitleLayout()
{
	ui.btnBack->setMaximumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));
	ui.btnBack->setMinimumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));

	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));

	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(titleHeight / rate);
	ui.labTitle->setStyleSheet(style);
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, titleHeight / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, titleHeight / rate));

	int spacing = (float)titleSpacing / rate;
	ui.horizontalLayout_Setting->setSpacing(spacing);

	int l = (float)titleLeftMargin / rate;
	int t = (float)titleTopMargin / rate;
	int r = (float)titleRightMargin / rate;
	int b = (float)titleBottomMargin / rate;
	ui.horizontalLayout_Setting->setContentsMargins(l, t, r, b);
}

void DlgSettingVideo::setVideoSourceLayout()
{
	int labelFontSize = fontSize / rate;
	int line_height = lineHeight / rate;
	ui.horizontalLayout_video1->setContentsMargins(sourceLeftMargin / rate, 0, sourceRightMargin / rate, horizonalVideoSource1BottomMargin / rate);
	ui.horizontalLayout_video2->setContentsMargins(sourceLeftMargin / rate, 0, sourceRightMargin / rate, 0);
	ui.horizontalLayout_video1->setSpacing(sourceSpace / rate);
	ui.horizontalLayout_video2->setSpacing(sourceSpace / rate);

	//左侧视频部分
	ui.verticalLayout_Video1Show->setSpacing(videoInfoSpace / rate);
	ui.verticalLayout_Video2Show->setSpacing(videoInfoSpace / rate);
	ui.labelVideo1Info->setMaximumSize(QSize(videoInfoW / rate, videoInfoH / rate));
	ui.labelVideo1Info->setMinimumSize(QSize(videoInfoW / rate, videoInfoH / rate));
	ui.labelvideo2Info->setMaximumSize(QSize(videoInfoW / rate, videoInfoH / rate));
	ui.labelvideo2Info->setMinimumSize(QSize(videoInfoW / rate, videoInfoH / rate));
	ui.widgetVideo1->setMaximumSize(QSize(videoWidgetW / rate, videoWidgetH / rate));
	ui.widgetVideo1->setMinimumSize(QSize(videoWidgetW / rate, videoWidgetH / rate));
	ui.widgetVideo2->setMaximumSize(QSize(videoWidgetW / rate, videoWidgetH / rate));
	ui.widgetVideo2->setMinimumSize(QSize(videoWidgetW / rate, videoWidgetH / rate));
	//右侧设置部分
	ui.verticalLayout_Video1Setting->setSpacing(videoSource1Space / rate);

	//视频源
	ui.verticalLayout_Video1->setSpacing(verticalLayout_Video1Space / rate);
	ui.horizontalLayout_video1_1->setSpacing(horizontalLayout_video1_1Space / rate);
	ui.horizontalLayout_video1_1->setContentsMargins(horizontalLayout_video1_1LeftMargin / rate, 0, 0, 0);
	ui.labVideo1->setMaximumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.labVideo1->setMinimumSize(QSize(labelInfoW / rate, labelInfoH / rate));

	ui.btnVideoSource1->setMinimumSize(QSize(videoDeviceW / rate, labelInfoH / rate));
	ui.btnVideoSource1->setMaximumSize(QSize(videoDeviceW / rate, labelInfoH / rate));
	ui.btnVideoSource2->setMinimumSize(QSize(videoDeviceW / rate, labelInfoH / rate));
	ui.btnVideoSource2->setMaximumSize(QSize(videoDeviceW / rate, labelInfoH / rate));
	// 分辨率 
	ui.verticalLayout_video1_2->setSpacing(verticalLayout_Video1Space / rate);
	ui.horizontalLayout_ResVideo1->setSpacing(horizontalLayout_ResVideo1Space / rate);
	ui.horizontalLayout_ResVideo1->setContentsMargins(horizontalLayout_video1_1LeftMargin / rate, 0, 0, 0);
	ui.labVideo1Res->setMaximumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.labVideo1Res->setMinimumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.btnVideo1ResPre->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1ResPre->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1ResNext->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1ResNext->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui. labVideo1ResInfo->setMaximumSize(QSize(resBtnW / rate, labelInfoH / rate));
	ui.labVideo1ResInfo->setMinimumSize(QSize(resBtnW / rate, labelInfoH / rate));
	//帧率
	ui.verticalLayout_video1_3->setSpacing(verticalLayout_Video1Space / rate);
	ui.horizontalLayout_Vide1FPS->setSpacing(horizontalLayout_ResVideo1Space / rate);
	ui.horizontalLayout_Vide1FPS->setContentsMargins(horizontalLayout_video1_1LeftMargin / rate, 0, 0, 0);
	ui.labVideo1FPS->setMaximumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.labVideo1FPS->setMinimumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.btnVideo1FPSPre->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1FPSPre->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1FPSNext->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1FPSNext->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.labVideo1FPSInfo->setMaximumSize(QSize(resBtnW / rate, labelInfoH / rate));
	ui.labVideo1FPSInfo->setMinimumSize(QSize(resBtnW / rate, labelInfoH / rate));
	//启用
	ui.verticalLayout_video1_4->setSpacing(verticalLayout_Video1Space / rate);
	ui.horizontalLayout_Video1Stats->setSpacing(horizontalLayout_ResVideo1Space / rate);
	ui.horizontalLayout_Video1Stats->setContentsMargins(horizontalLayout_video1_1LeftMargin / rate, 0, 0, 0);
	ui.labVideo1Stats->setMaximumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.labVideo1Stats->setMinimumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.btnVideo1StatsPre->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1StatsPre->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1StatsNext->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnVideo1StatsNext->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.labVideo1Stat->setMaximumSize(QSize(resBtnW / rate, labelInfoH / rate));
	ui.labVideo1Stat->setMinimumSize(QSize(resBtnW / rate, labelInfoH / rate));
	//视频源
	ui.verticalLayout_video2->setSpacing(verticalLayout_Video1Space / rate);
	ui.horizontalLayout_video2_1->setSpacing(horizontalLayout_video1_1Space / rate);
	ui.horizontalLayout_video2_1->setContentsMargins(horizontalLayout_video1_1LeftMargin / rate, 0, 0, 0);
	ui.labvideo2->setMaximumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.labvideo2->setMinimumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.btnVideoSource2->setMaximumHeight(labelInfoH / rate);
	ui.btnVideoSource2->setMinimumHeight(labelInfoH / rate);
	//输入设置
	ui.horizontalLayout_ResVideo2->setSpacing(verticalLayout_Video1Space / rate);
	ui.horizontalLayout_video2_1->setSpacing(horizontalLayout_video2_1Spacing / rate);
	ui.horizontalLayout_ResVideo2->setContentsMargins(horizontalLayout_video1_1LeftMargin / rate, 0, 0, 0);
	ui.labvideo2Res->setMaximumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.labvideo2Res->setMinimumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.lineEditVideoSource2->setMaximumSize(QSize(edtWidth / rate, edtHeight / rate));
	ui.lineEditVideoSource2->setMinimumSize(QSize(edtWidth / rate, edtHeight / rate));
	//启用
	ui.verticalLayout_video2_4->setSpacing(verticalLayout_Video1Space / rate);
	ui.horizontalLayout_Vide1Stats->setSpacing(horizontalLayout_ResVideo1Space / rate);
	ui.horizontalLayout_Vide1Stats->setContentsMargins(horizontalLayout_video1_1LeftMargin / rate, 0, 0, 0);
	ui.labvideo2Stats->setMaximumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.labvideo2Stats->setMinimumSize(QSize(labelInfoW / rate, labelInfoH / rate));
	ui.btnvideo2StatsPre->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnvideo2StatsPre->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnvideo2StatsNext->setMaximumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.btnvideo2StatsNext->setMinimumSize(QSize(backForwardW / rate, backForwardW / rate));
	ui.labvideo2Stat->setMaximumSize(QSize(resBtnW / rate, labelInfoH / rate));
	ui.labvideo2Stat->setMinimumSize(QSize(resBtnW / rate, labelInfoH / rate));

	ui.lineVideo1Source->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineVideo1Source->setMinimumSize(QSize(lineW / rate, 1));
	ui.lineVideo1_1->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineVideo1_1->setMinimumSize(QSize(lineW / rate, 1));
	

	ui.lineVideo1_2->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineVideo1_2->setMinimumSize(QSize(lineW / rate, 1));

	ui.lineVideo1_3->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineVideo1_3->setMinimumSize(QSize(lineW / rate, 1));
	ui.linevideo2Source->setMinimumSize(QSize(lineW / rate, 1));
	ui.linevideo2Source->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineVideo2_2->setMaximumSize(QSize(lineW / rate, 1));
	ui.lineVideo2_2->setMinimumSize(QSize(lineW / rate, 1));
	ui.lineVide2_3->setMinimumSize(QSize(lineW / rate, 1));
	ui.lineVide2_3->setMaximumSize(QSize(lineW / rate, 1));
	QString style = QString::fromUtf8("QWidget#lineVide2_3{\n"
		"border: none;\n"
		"background-color:  rgba(122, 120, 120, 1);\n"
		"}");
	ui.lineVide2_3->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labelVideo1Info{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelVideo1Info->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labelvideo2Info{\n"
		"font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labelvideo2Info->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labVideo1{\n"
		"  font-size: %1px;\n"
		"  font-family: PingFangSC-Medium, PingFang SC;\n"
		"  font-weight: 500;\n"
		"   color: #333333;\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labVideo1->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labVideo1Res{\n"
		"  font-size: %1px;\n"
		"  font-family: PingFangSC-Medium, PingFang SC;\n"
		"  font-weight: 500;\n"
		"   color: #333333;\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labVideo1Res->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labVideo1ResInfo{\n"
		" font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px;\n"
		"	background-color: rgba(255, 255, 255,0);\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labVideo1ResInfo->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labVideo1FPS{\n"
		"  font-size: %1px;\n"
		"  font-family: PingFangSC-Medium, PingFang SC;\n"
		"  font-weight: 500;\n"
		"   color: #333333;\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labVideo1FPS->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labVideo1FPSInfo{\n"
		" font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px;\n"
		"	background-color: rgba(255, 255, 255,0);\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labVideo1FPSInfo->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labVideo1Stats{\n"
		"  font-size: %1px;\n"
		"  font-family: PingFangSC-Medium, PingFang SC;\n"
		"  font-weight: 500;\n"
		"   color: #333333;\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labVideo1Stats->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labVideo1Stat{\n"
		" font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px;\n"
		"	background-color: rgba(255, 255, 255,0);\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labVideo1Stat->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labvideo2{\n"
		"  font-size: %1px;\n"
		"  font-family: PingFangSC-Medium, PinggiFang SC;\n"
		"  font-weight: 500;\n"
		"   color: #333333;\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labvideo2->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labvideo2Res{\n"
		"  font-size: %1px;\n"
		"  font-family: PingFangSC-Medium, PingFang SC;\n"
		"  font-weight: 500;\n"
		"   color: #333333;\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labvideo2Res->setStyleSheet(style);
	style = QString::fromUtf8("QLabel#labvideo2Stats{\n"
		"  font-size: %1px;\n"
		"  font-family: PingFangSC-Medium, PingFang SC;\n"
		"  font-weight: 500;\n"
		"   color: #333333;\n"
		"   line-height: %2px;\n"
		"}").arg(labelFontSize).arg(line_height);
	ui.labvideo2Stats->setStyleSheet(style);

	style = QString::fromUtf8("QLabel#labvideo2Stat{\n"
		" font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #333333;\n"
		"line-height: %2px;\n"
		"	background-color: rgba(255, 255, 255,0);\n"
		"}").arg(labelFontSize).arg(line_height);;
	ui.labvideo2Stat->setStyleSheet(style);
	style = QString::fromUtf8("\n"
		"                    QPushButton#btnVideoSource1{\n"
		"                    background-image: url(:/AgoraCourse/Resources/dualTeacher/traingle.png);\n"
		"                    background-position:right;\n"
		"                    background-repeat:none;\n"
		"                    border:none;\n"
		"                    background-color: rgba(255, 255, 255,0);\n"
		"                    font-size: %1px;\n"
		"                    font-family: PingFangSC-Medium, PingFang SC;\n"
		"                    font-weight: 500;\n"
		"                    color: #333333;\n"
		"                    line-height: %2px;\n"
		"                    text-align:right;\n"
		"                    padding-right: %3px;\n"
		"                    }\n"
		"                  ").arg(labelFontSize).arg(line_height).arg((int)(micSelectTextPadding / rate));
	ui.btnVideoSource1->setStyleSheet(style);

	style = QString::fromUtf8("\n"
		"                    QPushButton#btnVideoSource2{\n"
		"                    background-image: url(:/AgoraCourse/Resources/dualTeacher/traingle.png);\n"
		"                    background-position:right;\n"
		"                    background-repeat:none;\n"
		"                    border:none;\n"
		"                    background-color: rgba(255, 255, 255,0);\n"
		"                    font-size: %1px;\n"
		"                    font-family: PingFangSC-Medium, PingFang SC;\n"
		"                    font-weight: 500;\n"
		"                    color: #333333;\n"
		"                    line-height: %2px;\n"
		"                    text-align:right;\n"
		"                    padding-right: %3px;\n"
		"                    }\n"
		"                  ").arg(labelFontSize).arg(line_height).arg((int)(micSelectTextPadding / rate));
	ui.btnVideoSource2->setStyleSheet(style);
}

void DlgSettingVideo::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(200.0f / rate);
	float leftMargin = (rcMainScreen.width() - 200.0f / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin, (float)30.0f / rate, leftMargin, 40.0f / rate));
}

void DlgSettingVideo::setEditStyle(bool bEnable)
{
	bEnable = true;
	QString style;
	int font = edtFontSize / rate;
	int padding = edtPadding / rate;
	int lineHeight = edtLineHeight / rate;
	if (bEnable)
		style =
		QString::fromUtf8("QLineEdit#lineEditVideoSource2{\n"
			"	border-image: url(:/AgoraCourse/Resources/dualTeacher/);\n"
			"	background-color: rgb(255, 255, 255);\n"
			"	font: %1px;\n"
			"    border: 1px solid;\n"
			"    padding-left: %2px;\n"
			"    border-color: rgb(255, 255, 255);\n"
			"    border-width: 1px;\n"
			"    border-radius: 5px;\n"
			"    border-color: beige;\n"
			"    font-weight: 500;\n"
			"    color: #B8B6B5;\n"
			"    line-height: %3px;\n"
			"};").arg(font).arg(padding).arg(lineHeight);
	else
		style =
		QString::fromUtf8("QLineEdit#lineEditVideoSource2{\n"
			"	border-image: url(:/AgoraCourse/Resources/dualTeacher/);\n"
			"	background-color: rgba(255, 255, 255, 0.4);\n"
			"	font: %1px;\n"
			"    border: 1px solid;\n"
			"    padding-left: %2px;\n"
			"    border-color: rgba(255, 255, 255, 0.4);\n"
			"    border-width: 1px;\n"
			"    border-radius: 5px;\n"
			"    border-color: beige;\n"
			"    font-weight: 500;\n"
			"    color: #B8B6B5;\n"
			"    line-height: %3px;\n"
			"};").arg(font).arg(padding).arg(lineHeight);
	ui.lineEditVideoSource2->setStyleSheet(style);
	ui.lineEditVideoSource2->setEnabled(bEnable);
}
void DlgSettingVideo::on_minButton_clicked()
{
	agoraCourse->setWindowState(Qt::WindowMinimized);
}

void DlgSettingVideo::on_closeButton_clicked()
{
	onCancel();
}

void DlgSettingVideo::on_btnBack_clicked()
{
	onCancel();
}
void DlgSettingVideo::maxButtonChange(bool max)
{
	if (setting.bExtend) {
		rate = rate2;
		rcSecondScreen = ResetMaxRect(rcSecondScreen, !max);
		rcMainScreen = rcSecondScreen;
	}
	else
		rcMainScreen = ResetMaxRect(rcMainScreen, !max);

	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	ui.horizontalLayoutWidget->hide();
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	setVideoSourceLayout();
	setBottomLabel();
	bMax = !max;
	
	ui.horizontalLayoutWidget->show();
}

void DlgSettingVideo::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	emit parentMaxSignal(bMax);
}
//////////////////////////////////////////////////////////////
////DlgSettingSelect
/////////////////////////////////////////////////////////////
void DlgSettingSelect::InitDlg()
{
	if (bSecond) {
		rate = rate2;
		rcMainScreen = rcSecondScreen;
	}

	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	setMaximumSize(QSize(rcMainScreen.width() * 4, rcMainScreen.height() * 4 ));
	setMinimumSize(QSize(rcMainScreen.width() * 3 / 4, rcMainScreen.height() * 3 / 4));
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	//title
	titleHeight = 67* initRate;
	titleFontSize = 48* initRate;
	titleTopMargin = 48* initRate;
	titleBottomMargin = 60* initRate;
	titleLeftMargin = 135* initRate;
	titleRightMargin = 886* initRate;
	titleSpacing = 676* initRate;
	backbtnWidth = 78* initRate;
	labTitleW = 192* initRate;
	labTitleH = 67* initRate;

	itemW = 620* initRate;
	itemH = 66* initRate;
	//option
	gridMargin = 650* initRate;
	gridSpace = 20* initRate;

	versionSpacing = 30* initRate;

	labFontSize = 26* initRate;
	labLineHeight = 37* initRate;
	btnFontize = 30* initRate;
	btnLineHeight = 42* initRate;
	bottomLabelTopMargin = 170* initRate;

	itemCheckWidth = 36* initRate;
	itemPadding = 20* initRate;

	ui.labTitle->setText(title_);
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
	
	QString style = QString::fromUtf8("QWidget#horizontalLayoutWidget{"
		"background-image: url(:/AgoraCourse/Resources/dualTeacher/);"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/black-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);}");

	ui.horizontalLayoutWidget->setStyleSheet(style);
	
	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	setBottomLabel();
	InitItems();
	
}

void DlgSettingSelect::setTopButtonLayout()
{
	float buttonLeftMargin = rcMainScreen.width() - ui.closeButton->geometry().width() * 4 - topButtonSpace * 4;

	float w = topButtonWidth / rate;

	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	ui.minButton->setMaximumSize(w, w);
	ui.minButton->setMinimumSize(w, w);

	ui.maxButton->setMaximumSize(w, w);
	ui.maxButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins((int)buttonLeftMargin, topButtonTopMargin, topButtonRightMargin / rate, 0);
}

void DlgSettingSelect::setTitleLayout()
{
	ui.btnBack->setMaximumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));
	ui.btnBack->setMinimumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));

	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));

	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: 500;\n"
		"color: #FFFFFF;\n"
		"line-height: %2px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(titleHeight / rate);
	ui.labTitle->setStyleSheet(style);
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, titleHeight / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, titleHeight / rate));

	int spacing = (float)titleSpacing / rate;
	ui.horizontalLayout_Info->setSpacing(spacing);

	int l = (float)titleLeftMargin / rate;
	int t = (float)titleTopMargin / rate;
	int r = (float)titleRightMargin / rate;
	int b = (float)titleBottomMargin / rate;
	ui.horizontalLayout_Info->setContentsMargins(l, t, r, b);
}

void DlgSettingSelect::setOptionLayout()
{
	ui.gridLayout->setContentsMargins(gridMargin / rate, 0, gridMargin / rate, 0);
	ui.gridLayout->setSpacing(gridSpace / rate);
	QString style = QString::fromUtf8("QLabel#labName{\n"
		"font-size: %2px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #FFFFFF;\n"
		"line-height: %3px;\n"
		"}").arg((int)(labFontSize / rate)).arg((int)(labLineHeight / rate));

	style = QString::fromUtf8("QPushButton#btnOK{\n"
		"    background-color: #FFFFFFFF;\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:   #FFFFFFFF;\n"
		"    font-size: %1px;\n"
		"    font-family: PingFangSC-Medium, PingFang SC;\n"
		"    font-weight: 500;\n"
		"    color: #000000;\n"
		"    line-height: %2px;\n"
		"}").arg((int)(btnFontize / rate)).arg((int)(btnLineHeight / rate));
	//ui.btnOK->setStyleSheet(style);

	style = QString::fromUtf8("QPushButton#btnCancel{\n"
		"    background-color: rgba(0,0,0,0.2);\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:  rgba(0,0,0,0.2);\n"
		"   font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #FFFFFF;\n"
		"line-height: %2px;\n"
		"};").arg((int)(btnFontize / rate)).arg((int)(btnLineHeight / rate));

	int spacing = (float)versionSpacing / rate;
}

void DlgSettingSelect::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(bottomLabelW / rate);
	float leftMargin = (rcMainScreen.width() - bottomLabelW / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin, (float)bottomLabelTopMargin / rate, leftMargin, bottomLabelBottomMargin / rate));
}

void DlgSettingSelect::InitItems()
{
	int index = 0;
	int fontSize = labFontSize / rate;
	int lineHeight = labLineHeight / rate;
	int padding = itemPadding / rate;
	int checkWidth = itemCheckWidth / rate;
	for (auto item = items_.begin();
		item != items_.end(); ++item) {
		QRadioButton* radioItem = new QRadioButton(this);
		QSizePolicy sizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
		radioItem->setObjectName(QString::fromUtf8("radioItem"));
		sizePolicy.setHeightForWidth(radioItem->sizePolicy().hasHeightForWidth());
		radioItem->setSizePolicy(sizePolicy);
		radioItem->setMinimumSize(QSize(itemW / rate * 3 / 4, itemH / rate * 3 / 4));
		radioItem->setMaximumSize(QSize(itemW / rate, itemH / rate));
		radioItem->setLayoutDirection(Qt::RightToLeft);
		radioItem->setAutoFillBackground(false);
		QFont font("Microsoft YaHei UI", fontSize);
		font.setFamily("Microsoft YaHei UI");
		QFontMetrics fm(font);

		int spacing = 50.0f / rate;
		radioItem->setStyleSheet(QString::fromUtf8("\n"
			"                 QRadioButton{\n"
			"                 text-align: right;\n"
			"                 font-size: %1px;\n"
			"                 font-family: PingFangSC-Medium, PingFang SC;\n"
			"                 font-weight: 500;\n"
			"                 line-height: %2px;\n"
			"                 padding-left:%3px;\n"
			"                 spacing:%4px;\n"
			"                 padding-right:%5px;\n"
			"                 }\n"
			"                 QRadioButton::checked{\n"
			"                 background-color: rgba(255, 255, 255, 1);\n"
			"                 color:rgba(51,51,51,1);\n"
			"                 text-align: left;\n"
			"                 }\n"
			"                 QRadioButton::unchecked{\n"
			"                 background-color: rgba(255, 255, 255, 0.4);\n"
			"                 color:rgba(51,51,51,1);\n"
			"                 text-align: left;\n"
			"                 }\n"
			"                 QRadioButton::indicator:unchecked{\n"
			"                 border-image: url(:/AgoraCourse/Resources/);\n"
			"                 background-repeat: none;\n"
			"                 backg"
			"round-position: center;\n"
			"                 width:%6px;\n"
			"                 height:%7px;\n"
			"\n"
			"                 }\n"
			"                 QRadioButton::indicator:checked{\n"
			"                 border-image: url(:/AgoraCourse/Resources/dualTeacher/selected.png);\n"
			"                 background-repeat: none;\n"
			"                 background-position: center;\n"
			"                 width:%8;\n"
			"                 height:%9px;\n"
			"                 text-align: left;\n"
			"                 };\n"
			"               ").arg(fontSize).arg(lineHeight).arg(padding).arg(spacing).arg(padding)
			.arg(checkWidth).arg(checkWidth).arg(checkWidth).arg(checkWidth));



		radioItem->setIconSize(QSize(checkWidth, checkWidth));
		if (item.key() == selectedId_)
			radioItem->setChecked(true);

		itemButtons_.insert(radioItem, item.key());
		radioItem->setText(item.value());
		ui.gridLayout->addWidget(radioItem, index++, 0, 1, 1);

		connect(radioItem, SIGNAL(clicked()),
			this, SLOT(radioItemClicked()));
	
	}
}
void DlgSettingSelect::UpdateCtrls()
{
	int fontSize = labFontSize / rate;
	int lineHeight = labLineHeight / rate;
	int padding = itemPadding / rate;
	int checkWidth = itemCheckWidth / rate;
	for (auto iter = itemButtons_.constBegin();
		iter != itemButtons_.constEnd(); ++iter) {
		QRadioButton* radioItem = iter.key();//(QRadioButton*)ui.gridLayout->itemAt(i);
		radioItem->setMinimumSize(QSize(itemW / rate * 3 / 4, itemH / rate * 3 / 4));
		radioItem->setMaximumSize(QSize(itemW / rate, itemH / rate));
		int spacing = 50.0f / rate;
		radioItem->setStyleSheet(QString::fromUtf8("\n"
			"                 QRadioButton{\n"
			"                 text-align: right;\n"
			"                 font-size: %1px;\n"
			"                 font-family: PingFangSC-Medium, PingFang SC;\n"
			"                 font-weight: 500;\n"
			"                 line-height: %2px;\n"
			"                 padding-left:%3px;\n"
			"                 spacing:%4px;\n"
			"                 padding-right:%5px;\n"
			"                 }\n"
			"                 QRadioButton::checked{\n"
			"                 background-color: rgba(255, 255, 255, 1);\n"
			"                 color:rgba(51,51,51,1);\n"
			"                 text-align: left;\n"
			"                 }\n"
			"                 QRadioButton::unchecked{\n"
			"                 background-color: rgba(255, 255, 255, 0.4);\n"
			"                 color:rgba(51,51,51,1);\n"
			"                 text-align: left;\n"
			"                 }\n"
			"                 QRadioButton::indicator:unchecked{\n"
			"                 border-image: url(:/AgoraCourse/Resources/);\n"
			"                 background-repeat: none;\n"
			"                 backg"
			"round-position: center;\n"
			"                 width:%6px;\n"
			"                 height:%7px;\n"
			"\n"
			"                 }\n"
			"                 QRadioButton::indicator:checked{\n"
			"                 border-image: url(:/AgoraCourse/Resources/dualTeacher/selected.png);\n"
			"                 background-repeat: none;\n"
			"                 background-position: center;\n"
			"                 width:%8;\n"
			"                 height:%9px;\n"
			"                 text-align: left;\n"
			"                 };\n"
			"               ").arg(fontSize).arg(lineHeight).arg(padding).arg(spacing).arg(padding)
			.arg(checkWidth).arg(checkWidth).arg(checkWidth).arg(checkWidth));



		  radioItem->setIconSize(QSize(checkWidth, checkWidth));
	}
}
void DlgSettingSelect::on_btnOK_clicked()
{
	accept();
}

void DlgSettingSelect::on_btnCancel_clicked()
{
	reject();
}

void DlgSettingSelect::on_minButton_clicked()
{
	setWindowState(Qt::WindowMinimized);
}

void DlgSettingSelect::on_closeButton_clicked()
{
	close();
}

void DlgSettingSelect::on_btnBack_clicked()
{
	close();
}
void DlgSettingSelect::maxButtonChange(bool max)
{
	if (setting.bExtend) {
		rate = rate2;
		rcSecondScreen = ResetMaxRect(rcSecondScreen, !max);
		rcMainScreen = rcSecondScreen;
	}
	else
		rcMainScreen = ResetMaxRect(rcMainScreen, !max);

	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	setBottomLabel();
	UpdateCtrls();
	bMax = !max;
	ui.horizontalLayoutWidget->hide();
	ui.horizontalLayoutWidget->show();
}

void DlgSettingSelect::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	emit parentMaxSignal(bMax);
}
//////////////////////////////////////////////////////////////
////DlgVersion
/////////////////////////////////////////////////////////////
void DlgVersion::InitDlg()
{
	if (bSecond) {
		rate = rate2;
		rcMainScreen = rcSecondScreen;
	}
	setMinimumSize(QSize(rcMainScreen.width() * 3 / 4, rcMainScreen.height() * 3 / 4));
	//title
	titleHeight = 67* initRate;
	titleFontSize = 48* initRate;
	titleTopMargin = 48* initRate;
	titleBottomMargin = 135* initRate;
	titleLeftMargin = 135* initRate;
	titleRightMargin = 886* initRate;
	titleSpacing = 676* initRate;
	backbtnWidth = 78* initRate;
	labTitleW = 144* initRate;
	labTitleH = 78* initRate;

	//option
	optionButtonR = 306* initRate;
	optionLabelW = 216* initRate;
	optionLabelH = 45* initRate;
	versionSpacing = 30* initRate;

	fontSize = 32* initRate;
	lineHeight = 45* initRate;
	bottomTopMargin = 170* initRate;

	largeFontSize = 36* initRate;
	largeLineHeight = 50* initRate;
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
	this->setGeometry(rcMainScreen);
	QString style = QString::fromUtf8("QWidget#horizontalLayoutWidget{"
		"background-image: url(:/AgoraCourse/Resources/dualTeacher/);"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/white-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);}");
	if (bSecond) {
		QRect rc = { 0, 0, rcSecondScreen.width(), rcSecondScreen.height() };
		ui.horizontalLayoutWidget->setGeometry(rc);
	}
	else {
		ui.horizontalLayoutWidget->setGeometry(rcMainScreen);
	}
	ui.horizontalLayoutWidget->setStyleSheet(style);
	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	setBottomLabel();
}
void DlgVersion::setTopButtonLayout()
{
	float w = topButtonWidth / rate;
	float buttonLeftMargin = rcMainScreen.width() - (w * 3 + 2 * topButtonSpace / rate);
	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	ui.minButton->setMaximumSize(w, w);
	ui.minButton->setMinimumSize(w, w);

	ui.maxButton->setMaximumSize(w, w);
	ui.maxButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins(buttonLeftMargin, topButtonTopMargin / rate, topButtonRightMargin / rate, 0);
}

void DlgVersion::setTitleLayout()
{
	ui.btnBack->setMaximumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));
	ui.btnBack->setMinimumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));

	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));

	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(titleHeight / rate);
	ui.labTitle->setStyleSheet(style);
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, titleHeight / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, titleHeight / rate));

	int spacing = (float)titleSpacing / rate;
	ui.horizontalLayout_Setting->setSpacing(spacing);

	int l = (float)titleLeftMargin / rate;
	int t = (float)titleTopMargin / rate;
	int r = (float)titleRightMargin / rate;
	int b = (float)titleBottomMargin / rate;
	ui.horizontalLayout_Setting->setContentsMargins(l, t, r, b);
}

void DlgVersion::setOptionLayout()
{
	ui.btnVersion->setMaximumSize(QSize(optionButtonR / rate, optionButtonR / rate));
	ui.btnVersion->setMinimumSize(QSize(optionButtonR / rate, optionButtonR / rate));

	QString style = QString::fromUtf8("QLabel#labName{\n"
		"font-size: %2px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: rgba(0, 0, 0, 1);\n"
		"line-height: %3px;\n"
		"}").arg((int)(largeFontSize / rate)).arg((int)(largeLineHeight / rate));
	ui.labName->setStyleSheet(style);
	ui.labName->setMaximumSize(QSize(optionButtonR / rate, largeLineHeight / rate));
	ui.labName->setMinimumSize(QSize(optionButtonR / rate, largeLineHeight / rate));

	QVector<QString> vecName;
	QVector<QLabel*> labs;
	vecName.push_back("labVersion");
	vecName.push_back("labTime");
	labs.push_back(ui.labVersion);
	labs.push_back(ui.labTime);
	for (int i = 0; i < labs.size(); ++i) {
		QString style = QString::fromUtf8("QLabel#%1{\n"
			"font-size: %2px;\n"
			"font-family: PingFangSC-Medium, PingFang SC;\n"
			"font-weight: 500;\n"
			"color: rgba(0, 0, 0, 1);\n"
			"line-height: %3px;\n"
			"}").arg(vecName[i]).arg((int)(fontSize / rate)).arg((int)(lineHeight / rate));
		labs[i]->setStyleSheet(style);
		labs[i]->setMaximumSize(QSize(optionButtonR / rate, lineHeight / rate));
		labs[i]->setMinimumSize(QSize(optionButtonR / rate, lineHeight / rate));
	}
	int spacing = (float)versionSpacing / rate;
	ui.verticalLayout_Version->setSpacing(spacing);
}

void DlgVersion::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(bottomLabelW / rate);
	float leftMargin = (rcMainScreen.width() - bottomLabelW / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin, (float)bottomTopMargin / rate, leftMargin, bottomLabelBottomMargin / rate));
}

void DlgVersion::on_minButton_clicked()
{
	agoraCourse->setWindowState(Qt::WindowMinimized);
}

void DlgVersion::on_closeButton_clicked()
{
	close();
}

void DlgVersion::on_btnBack_clicked()
{
	close();
}

void DlgVersion::maxButtonChange(bool max)
{
	if (setting.bExtend) {
		rate = rate2;
		rcSecondScreen = ResetMaxRect(rcSecondScreen, !max);
		rcMainScreen = rcSecondScreen;
	}
	else
		rcMainScreen = ResetMaxRect(rcMainScreen, !max);

	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	setBottomLabel();
	bMax = !max;
	ui.horizontalLayoutWidget->hide();
	ui.horizontalLayoutWidget->show();
}

void DlgVersion::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	emit parentMaxSignal(bMax);
}

//////////////////////////////////////////////////////////////
////DlgInfo
/////////////////////////////////////////////////////////////
void DlgInfo::InitDlg()
{
	if (bSecond) {
		rate = rate2;
		rcMainScreen = rcSecondScreen;
	}
	QRect rc = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	ui.horizontalLayoutWidget->setGeometry(rc);
	setMinimumSize(QSize(rcMainScreen.width() * 3 / 4, rcMainScreen.height() * 3 / 4));
	setMaximumSize(QSize(rcMainScreen.width(), rcMainScreen.height()));
	this->setGeometry(rcMainScreen);
	//title
	titleHeight = 67* initRate;
	titleFontSize = 48* initRate;
	titleTopMargin = 48* initRate;
	titleBottomMargin = 135* initRate;
	titleLeftMargin = 135* initRate;
	titleRightMargin = 886* initRate;
	titleSpacing = 676* initRate;
	backbtnWidth = 78* initRate;
	labTitleW = 144* initRate;
	labTitleH = 78* initRate;

	//option
	buttonW = 430* initRate;
	buttonH = 66* initRate;

	versionSpacing = 30* initRate;


	labFontSize = 26* initRate;
	labLineHeight = 37* initRate;
	labW = 260* initRate;
	labH = 50* initRate;
	btnFontSize = 30* initRate;
	btnLineHeight = 42* initRate;
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
	QString style = QString::fromUtf8("QWidget#horizontalLayoutWidget{"
		"background-image: url(:/AgoraCourse/Resources/dualTeacher/);"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/black-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);}");	
 	ui.horizontalLayoutWidget->setStyleSheet(style);
	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	setBottomLabel();

	ui.labName->setText(information);
	ui.minButton->hide();
	ui.maxButton->hide();
}

void DlgInfo::setTopButtonLayout()
{
	float w = topButtonWidth / rate;
	float buttonLeftMargin = rcMainScreen.width() - (w * 2 + 1 * topButtonSpace / rate);
	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	//ui.minButton->setMaximumSize(w, w);
	//ui.minButton->setMinimumSize(w, w);

	//ui.maxButton->setMaximumSize(w, w);
	//ui.maxButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins(buttonLeftMargin, topButtonTopMargin / rate, topButtonRightMargin / rate, 0);
}

void DlgInfo::setTitleLayout()
{
	ui.btnBack->setMaximumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));
	ui.btnBack->setMinimumSize(QSize(backbtnWidth / rate, backbtnWidth / rate));

	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));

	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: 500;\n"
		"color: #FFFFFF;\n"
		"line-height: %2px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(titleHeight / rate);
	ui.labTitle->setStyleSheet(style);
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, titleHeight / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, titleHeight / rate));

	int spacing = (float)titleSpacing / rate;
	ui.horizontalLayout_Info->setSpacing(spacing);

	int l = (float)titleLeftMargin / rate;
	int t = (float)titleTopMargin / rate;
	int r = (float)titleRightMargin / rate;
	int b = (float)titleBottomMargin / rate;
	ui.horizontalLayout_Info->setContentsMargins(l, t, r, b);
}

void DlgInfo::setOptionLayout()
{
	ui.btnOK->setMaximumSize(QSize(buttonW / rate, buttonH / rate));
	ui.btnOK->setMinimumSize(QSize(buttonW / rate, buttonH / rate));

	ui.btnCancel->setMaximumSize(QSize(buttonW / rate, buttonH / rate));
	ui.btnCancel->setMinimumSize(QSize(buttonW / rate, buttonH / rate));

	QString style = QString::fromUtf8("QLabel#labName{\n"
		"font-size: %2px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #FFFFFF;\n"
		"line-height: %3px;\n"
		"}").arg((int)(labFontSize / rate)).arg((int)(labLineHeight / rate));
	ui.labName->setStyleSheet(style);
	ui.labName->setMaximumSize(QSize(rcMainScreen.width() / rate, labH / rate));
	ui.labName->setMinimumSize(QSize(labW / rate, labH / rate));

	style = QString::fromUtf8("QPushButton#btnOK{\n"
		"    background-color: #FFFFFFFF;\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:   #FFFFFFFF;\n"
		"    font-size: %1px;\n"
		"    font-family: PingFangSC-Medium, PingFang SC;\n"
		"    font-weight: 500;\n"
		"    color: #000000;\n"
		"    line-height: %2px;\n"
		"}").arg((int)(btnFontSize / rate)).arg((int)(btnLineHeight / rate));
	ui.btnOK->setStyleSheet(style);

	style = QString::fromUtf8("QPushButton#btnCancel{\n"
		"    background-color: rgba(0,0,0,0.2);\n"
		"    border-width: 1px;\n"
		"    border-radius: 5px;\n"
		"    border-color:  rgba(0,0,0,0.2);\n"
		"   font-size: %1px;\n"
		"font-family: PingFangSC-Medium, PingFang SC;\n"
		"font-weight: 500;\n"
		"color: #FFFFFF;\n"
		"line-height: %2px;\n"
		"};").arg((int)(btnFontSize / rate)).arg((int)(btnLineHeight / rate));
	ui.btnCancel->setStyleSheet(style);

	int spacing = (float)versionSpacing / rate;
	ui.verticalLayout_Version->setSpacing(spacing);
}

void DlgInfo::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(200.0f / rate);
	float leftMargin = (rcMainScreen.width() - 200.0f / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin, (float)170.0f / rate, leftMargin, 40.0f / rate));
}

void DlgInfo::maxButtonChange(bool max, bool bMain)
{
	if (setting.bExtend && !bMain) {
		rate = rate2;
		rcSecondScreen = ResetMaxRect(rcSecondScreen, !max);
		rcMainScreen = rcSecondScreen;
	}
	else
		rcMainScreen = ResetMaxRect(rcMainScreen, !max);

	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	setBottomLabel();
	bMax = !max;
	ui.horizontalLayoutWidget->hide();
	ui.horizontalLayoutWidget->show();
}

void DlgInfo::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	emit parentMaxSignal(bMax);
}

void DlgInfo::on_parentMax_slot(bool childMax)
{
	if (bMax != childMax) {
		on_maxButton_clicked();
	}
}
//////////////////////////////////////////////////////////////
/////DlgVideoRoom
/////////////////////////////////////////////////////////////
void DlgVideoRoom::InitDlg()
{
	//title
	titleHeight = 67* initRate;
	titleFontSize = 48* initRate;
	titleTopMargin = 48* initRate;
	titleBottomMargin = 20* initRate;
	labTitleW = 192* initRate;
	labTitleH = 67* initRate;

	//video widget
	videoW = 710* initRate;
	videoH = 400* initRate;
	videoSpace = 36* initRate;
	videoMargin = 73* initRate;
	pageBtnW = 52* initRate;
	pageBtnH = 52* initRate;
	midMargin = 98* initRate;
	bottomTopMargin = 18* initRate;
	if (setting.bExtend)
		ui.settingsButton->hide();

	setMinimumSize(QSize(rcMainScreen.width() * 3 / 4, rcMainScreen.height() * 3 / 4));
	setMaximumSize(QSize(rcMainScreen.width(), rcMainScreen.height()));


	this->setGeometry(rcMainScreen);
	QString style = QString::fromUtf8("QWidget#horizontalLayoutWidget{"
		"background-image: url(:/AgoraCourse/Resources/dualTeacher/);"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/white-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);}");
	ui.horizontalLayoutWidget->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setStyleSheet(style);
	setTopButtonLayout();
	setTitleLayout();
	for (int i = 0; i < 4; ++i) {
		videoWidget[i] = new VideoWidget(initRate, rate, ui.horizontalLayoutWidget);
		videoWidget[i]->setObjectName(QString::fromUtf8("widget_%1").arg(i));
		videoWidget[i]->setMinimumSize(QSize(int(videoW / rate), int(videoH / rate)));
		videoWidget[i]->setMaximumSize(QSize(int(videoW / rate), int(videoH / rate)));
		if (i < 2)
			ui.horizontalLayout_Row1->addWidget(videoWidget[i]);
		else
			ui.horizontalLayoutRow2->addWidget(videoWidget[i]);
	}
	videoWidget[2]->hide();
	videoWidget[3]->hide();

	setOptionLayout();
	setBottomLabel();
	showPage(false);
	for (int i = 0; i < 4; ++i) {
		connect(videoWidget[i], &VideoWidget::fullScreenSignal,
			this, &DlgVideoRoom::on_fullScreen);
		connect(videoWidget[i], &VideoWidget::muteVideoSignal,
			this, &DlgVideoRoom::on_muteVideo);
		connect(videoWidget[i], &VideoWidget::muteAudioSignal,
			this, &DlgVideoRoom::on_muteAudio);
	}

	connect(rtcEngine, &AgoraRtcEngine::userJoined,
		this, &DlgVideoRoom::onUserJoined);
	connect(rtcEngine, &AgoraRtcEngine::userOffline,
		this, &DlgVideoRoom::onUserOffline);

	connect(rtcEngine, &AgoraRtcEngine::streamMessage,
		this, &DlgVideoRoom::onStreamMessage);
}


void DlgVideoRoom::setTopButtonLayout()
{
	float w = topButtonWidth / rate;
	float buttonLeftMargin = rcMainScreen.width() - (w * 4 + 3 * topButtonSpace / rate);
	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	ui.minButton->setMaximumSize(w, w);
	ui.minButton->setMinimumSize(w, w);

	ui.maxButton->setMaximumSize(w, w);
	ui.maxButton->setMinimumSize(w, w);

	ui.settingsButton->setMaximumSize(w, w);
	ui.settingsButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins(buttonLeftMargin, topButtonTopMargin / rate, topButtonRightMargin / rate, 0);
}

void DlgVideoRoom::setTitleLayout()
{
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));

	ui.btnPrePage->setMaximumSize(QSize(pageBtnW / rate, pageBtnH / rate));
	ui.btnPrePage->setMinimumSize(QSize(pageBtnW / rate, pageBtnH / rate));
	ui.btnNextPage->setMaximumSize(QSize(pageBtnW / rate, pageBtnH / rate));
	ui.btnNextPage->setMinimumSize(QSize(pageBtnW / rate, pageBtnH / rate));

	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(titleHeight / rate);
	ui.labTitle->setStyleSheet(style);
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.horizontalLayout_Title->setContentsMargins(0, 0, titleBottomMargin / rate, 0);
	ui.horizontalLayout_Mid->setContentsMargins(midMargin / rate, 0, midMargin / rate, 0);
}

void DlgVideoRoom::showPage(bool bShow)
{
	if (bShow) {
		ui.btnPrePage->setStyleSheet(QString::fromUtf8("border-image: url(:/AgoraCourse/Resources/dualTeacher/icon-backward.png);"));
		ui.btnNextPage->setStyleSheet(QString::fromUtf8("border-image: url(:/AgoraCourse/Resources/dualTeacher/icon-forward.png);"));
	}
	else {
		ui.btnPrePage->setStyleSheet(QString::fromUtf8("background-color:#00000000; border-image: url(:/AgoraCourse/Resources/dualTeacher/);"));
		ui.btnNextPage->setStyleSheet(QString::fromUtf8("background-color:#00000000;border-image: url(:/AgoraCourse/Resources/dualTeacher/);"));
	}
}

void DlgVideoRoom::setOptionLayout()
{
	//ui.verticalLayout_Widget->setMinimumSize(QSize((2 * videoH + videoSpace) / rate, (2 * videoW + videoSpace) / rate));
	ui.verticalLayout_Widget->setSpacing(videoSpace / rate);
	int verticalMargin = 10;
	if (!videoWidget[3]->isVisible())
		verticalMargin = (videoH + videoSpace) / (rate * 2.0);
	ui.verticalLayout_Widget->setContentsMargins(videoMargin / rate, verticalMargin, videoMargin / rate, verticalMargin);
	ui.horizontalLayout_Row1->setSpacing(videoSpace / rate);
	ui.horizontalLayoutRow2->setSpacing(videoSpace / rate);
}

void DlgVideoRoom::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(bottomLabelW / rate);
	float leftMargin = (rcMainScreen.width() - bottomLabelW / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin, bottomTopMargin / rate, leftMargin, bottomLabelBottomMargin / rate));
}

void DlgVideoRoom::UpdateLayout()
{
	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	ui.btnPrePage->show();
	ui.btnNextPage->show();
}

void DlgVideoRoom::ShowTopAndBottom(bool bShow)
{
	if (bShow) {
		ui.labTitle->show();
		ui.minButton->show();
		ui.maxButton->show();
		ui.closeButton->show();
		ui.settingsButton->show();
		ui.label->show();
		UpdateLayout();
	}
	else {
		ui.horizontalLayout_Title->setContentsMargins(0, 0, 0, 0);
		ui.horizontalLayout_titleButton->setContentsMargins(0, 0, 0, 0);
		ui.horizontalLayout_Mid->setContentsMargins(0, 0, 0, 0);
		ui.verticalLayout_Widget->setSpacing(0);
		int verticalMargin = (videoH + videoSpace) / 2.0f;
		ui.verticalLayout_Widget->setContentsMargins(0, 0, 0, 0);
		ui.horizontalLayout_Row1->setSpacing(0);
		ui.horizontalLayoutRow2->setSpacing(0);
		ui.btnPrePage->hide();
		ui.btnNextPage->hide();

		ui.labTitle->hide();
		ui.minButton->hide();
		ui.maxButton->hide();
		ui.settingsButton->hide();
		ui.label->hide();
		ui.closeButton->hide();
	}
}


void DlgVideoRoom::SetControlMax(bool max)
{
	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	rcMainScreen = ResetMaxRect(rcMainScreen, !max);

	rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	for (int i = 0; i < 4; ++i) {
		videoWidget[i]->maxButtonChange(max, videoW, videoH, rate);
	}
	setOptionLayout();
	setBottomLabel();
	ui.horizontalLayoutWidget->hide();
	ui.horizontalLayoutWidget->show();
}

void DlgVideoRoom::maxButtonChange(bool max)
{
	SetControlMax(max);
	bMax = !max;
}

void DlgVideoRoom::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	if (!setting.bExtend)
		emit parentMaxSignal(bMax);
}

void DlgVideoRoom::on_fullMaxButton_clicked(bool bMax)
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	SetControlMax(bMax);
}

void DlgVideoRoom::on_parentMax_slot(bool childMax)
{
	if (bMax != childMax) {
		on_maxButton_clicked();
	}
}

//////////////////////////////////////////////////////////////
/////VideoWidget
/////////////////////////////////////////////////////////////
void VideoWidget::InitWidget()
{
	widgetW = 710* initRate_;
	widgetH = 400* initRate_;
	btnUserW = 116* initRate_;
	btnUserH = 45* initRate_;
	userX = 20* initRate_;
	btnW = 48* initRate_;
	btnSpacing = 16* initRate_;
	btnPadding = 20* initRate_;

	
	RestoreWidget();
	InitButton();

	ui.widgetFrame->setGeometry(0, 0, widgetW, widgetH);
	ui.verticalLayoutWidget->setGeometry(0, 0, widgetW, widgetH);

	connect(AgoraRtcEngine::GetAgoraRtcEngine()
		, &AgoraRtcEngine::renderSignal
		, this, &VideoWidget::renderFrame);
}

void VideoWidget::InitButton()
{
	btnUser = new QPushButton(ui.verticalLayoutWidget);
	btnUser->setObjectName(QString::fromUtf8("btnUser"));
	QSizePolicy sizePolicy;
	sizePolicy.setHeightForWidth(btnUser->sizePolicy().hasHeightForWidth());
	btnUser->setSizePolicy(sizePolicy);
	btnUser->setMinimumSize(QSize(btnUserW / rate_ * 3 / 4, btnUserH / rate_ * 3 / 4));
	btnUser->setMaximumSize(QSize(btnUserW / rate_, btnUserH / rate_));
	int radius = btnUserH / rate_ /2;
	btnUser->setGeometry(userX / rate_, userX / rate_, btnUserW / rate_, btnUserH / rate_);
	btnUser->setStyleSheet(QString::fromUtf8("QPushButton#btnUser{\n"
		"border-width: 1px;\n"
		"background-color: rgba(255,255,255,0.6);\n"
		"border-color:rgba(255,255,255,0.6);\n"
		"border-radius: %1px;\n"
		"}").arg(radius));
	
	// full screen
	btnFullScreen = new QPushButton(ui.verticalLayoutWidget);
	btnFullScreen->setObjectName(QString::fromUtf8("btnFullScreen"));
	btnFullScreen->setMinimumSize(QSize(btnW / rate_ * 3 / 4, btnW / rate_ * 3 / 4));
	btnFullScreen->setMaximumSize(QSize(btnW / rate_, btnW / rate_));

	btnFullScreen->setFlat(true);

	int y = (widgetH - btnPadding - btnW) / rate_;
	int x = (widgetW - btnPadding - btnW) / rate_;
	btnFullScreen->setGeometry(x, y, btnW / rate_, btnW / rate_);
	connect(btnFullScreen, &QPushButton::clicked, this, &VideoWidget::on_btnFullScreen_clicked);
	//mic
	btnMic = new QPushButton(ui.verticalLayoutWidget);
	btnMic->setObjectName(QString::fromUtf8("btnMic"));
	btnMic->setMinimumSize(QSize(btnW / rate_ * 3 / 4, btnW / rate_ * 3 / 4));
	btnMic->setMaximumSize(QSize(btnW / rate_, btnW / rate_));


	
	btnMic->setFlat(true);

	x = x - (btnW + btnSpacing) / rate_;
	btnMic->setGeometry(x, y, btnW / rate_, btnW / rate_);
	connect(btnMic, &QPushButton::clicked, this, &VideoWidget::on_btnMic_clicked);
	//camera
	btnCamera = new QPushButton(ui.verticalLayoutWidget);
	btnCamera->setObjectName(QString::fromUtf8("btnCamera"));
	sizePolicy.setHeightForWidth(btnCamera->sizePolicy().hasHeightForWidth());
	btnCamera->setSizePolicy(sizePolicy);
	btnCamera->setMinimumSize(QSize(btnW / rate_ * 3 / 4, btnW / rate_ * 3 / 4));
	btnCamera->setMaximumSize(QSize(btnW / rate_, btnW / rate_));
	x = x - (btnW + btnSpacing) / rate_;
	btnCamera->setGeometry(x, y, btnW / rate_, btnW / rate_);


	btnFullScreen->setStyleSheet(QString::fromUtf8("QPushButton#btnFullScreen{\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/fullscreen-func.png);\n"
		"background-position:center;\n"
		"background-repeat: none;\n"
		"background-color: rgba(0, 0, 0, 0.55);\n"
		"border-color: #565656;\n"
		"border-width: 1px;\n"
		"border-radius: 6px;\n"
		"};"));
	QString img = QString("microphone-func.png");//microphone-func.png

	QString style = QString::fromUtf8("QPushButton#btnMic{\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/%1);\n"
		"background-position:center;\n"
		"background-repeat: none;\n"
		"background-color: rgba(0, 0, 0, 0.55);\n"
		"border-color: #565656;\n"
		"border-width: 1px;\n"
		"border-radius: 6px;\n"
		"};").arg(img);
	btnMic->setStyleSheet(style);
	btnCamera->setStyleSheet(QString::fromUtf8("QPushButton#btnCamera{\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/%1);\n"
		"background-position:center;\n"
		"background-repeat: none;\n"
		"background-color: rgba(0, 0, 0, 0.55);\n"
		"border-color: #565656;\n"
		"border-width: 1px;\n"
		"border-radius: 6px;\n"
		"};").arg("camera-func.png"));
	btnCamera->setFlat(true);

	connect(btnCamera, &QPushButton::clicked, this, &VideoWidget::on_btnCamera_clicked);
}

void VideoWidget::RestoreWidget()
{
	int w = widgetW / rate_;
	int h = widgetH / rate_;
	setMaximumSize(QSize(w, h));
	setMinimumSize(QSize(w , h));
	setGeometry(QRect(0, 0, w, h));
	ui.widgetFrame->setMaximumSize(QSize(w, h));
	ui.widgetFrame->setMinimumSize(QSize(w , h));
}

void VideoWidget::MaximizeWidget(int width, int height)
{
	int w = width;// / rate_;
	int h = height;// / rate_;
	setMaximumSize(QSize(w, h));
	setMinimumSize(QSize(w, h));
	setGeometry(QRect(0, 0, w, h));
	ui.widgetFrame->setMaximumSize(QSize(w, h));
	ui.widgetFrame->setMinimumSize(QSize(w, h));
}

void VideoWidget::UpdateButtonPos()
{
	btnUser->setMinimumSize(QSize(btnUserW / rate_ * 3 / 4, btnUserH / rate_ * 3 / 4));
	btnUser->setMaximumSize(QSize(btnUserW / rate_, btnUserH / rate_));
	int radius = btnUserH / rate_ / 2;
	btnUser->setGeometry(userX / rate_, userX / rate_, btnUserW / rate_, btnUserH / rate_);
	btnUser->setStyleSheet(QString::fromUtf8("QPushButton#btnUser{\n"
		"border-width: 1px;\n"
		"background-color: rgba(255,255,255,0.6);\n"
		"border-color:rgba(255,255,255,0.6);\n"
		"border-radius: %1px;\n"
		"}").arg(radius));

	int y = this->height() - (btnPadding + btnW) / rate_;
	int x = this->width() - (btnPadding + btnW) / rate_;
	btnFullScreen->setGeometry(x, y, btnW / rate_, btnW / rate_);

	x = x - (btnW + btnSpacing) / rate_;
	btnMic->setGeometry(x, y, btnW / rate_, btnW / rate_);
	x = x - (btnW + btnSpacing) / rate_;
	btnCamera->setGeometry(x, y, btnW / rate_, btnW / rate_);
}

void VideoWidget::SetMicButtonStats(bool mute)
{
	//mic
	QString style = QString::fromUtf8("QPushButton#btnMic{\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/%1);\n"
		"background-position:center;\n"
		"background-repeat: none;\n"
		"background-color: rgba(0, 0, 0, 0.55);\n"
		"border-color: #565656;\n"
		"border-width:  1px;\n"
		"border-radius: 6px;\n"
		"};").arg(mute ? "microphone-off-func.png" : "microphone-func.png");
	
	btnMic->setStyleSheet(style);
}


void VideoWidget::SetCameraButtonStats(bool mute)
{
	//mic
	QString style = QString::fromUtf8("QPushButton#btnCamera{\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/%1);\n"
		"background-position:center;\n"
		"background-repeat: none;\n"
		"background-color: rgba(0, 0, 0, 0.55);\n"
		"border-color: #565656;\n"
		"border-width: 1px;\n"
		"border-radius: 6px;\n"
		"};").arg(mute ? "camera-off-func.png" : "camera-func.png");

	btnCamera->setStyleSheet(style);
	btnCamera->setFlat(true);
}

void VideoWidget::SetFullScreenButtonStats(bool full)
{
	//mic
	QString style = QString::fromUtf8("QPushButton#btnFullScreen{\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/%1);\n"
		"background-position:center;\n"
		"background-repeat: none;\n"
		"background-color: rgba(0, 0, 0, 0.55);\n"
		"border-color: #565656;\n"
		"border-width: 1px;\n"
		"border-radius: 6px;\n"
		"};").arg(full ? "fullmin-func.png" : "fullscreen-func.png");

	btnFullScreen->setStyleSheet(style);
}

void VideoWidget::maxButtonChange(bool max, int w, int h, float rate)
{
	rate_ = rate;
	QRect rc = { 0, 0, (int)((float)w / rate_), (int)((float)h / rate_) };
	setMaximumSize(QSize(rc.width(), rc.height()));
	setMinimumSize(QSize(rc.width(), rc.height() ));
	setGeometry(rc);
	ui.widgetFrame->setMaximumSize(QSize(rc.width(), rc.height()));
	ui.widgetFrame->setMinimumSize(QSize(rc.width() , rc.height() ));
	ui.widgetFrame->setGeometry(rc);
	ui.verticalLayoutWidget->setGeometry(rc);
	UpdateButtonPos();
	
	bMax = !max;
}

//////////////////////////////////////////////////////////////
/////DlgExtend
/////////////////////////////////////////////////////////////
void DlgExtend::InitDlg()
{
	//ui.minButton->hide();
	//ui.maxButton->hide();
	//ui.closeButton->hide();
	rcMainScreen = rcSecondScreen;
	setMinimumSize(QSize(rcMainScreen.width() * 3 / 4, rcMainScreen.height() * 3 / 4));

	rate = rate2;
	int w = rcSecondScreen.width() * rate;
	initRate = w / 1920.0f;
	//top button
	topButtonSpace = 25* initRate;
	topButtonWidth = 50* initRate;
	topButtonTopMargin = 10* initRate;
	topButtonRightMargin = 20* initRate;
	bottomFontSize = 24* initRate;
	bottomLineHeight = 29* initRate;

	bottomLabelW = 200* initRate;
	bottomLabelBottomMargin = 35* initRate;
	//title
	titleHeight = 67* initRate;
	titleFontSize = 48* initRate;
	titleTopMargin = 48* initRate;
	titleBottomMargin = 20* initRate;
	labTitleW = 192* initRate;
	labTitleH = 67* initRate;
	//video widget
	videoW = 710* initRate;
	videoH = 400* initRate;
	videoSpace = 36* initRate;
	videoMargin = 73* initRate;
	pageBtnW = 52* initRate;
	pageBtnH = 52* initRate;
	midMargin = 98* initRate;
	bottomTopMargin = 18* initRate;

	this->setGeometry(rcSecondScreen);
	QString style = QString::fromUtf8("QWidget#horizontalLayoutWidget{"
		"background-image: url(:/AgoraCourse/Resources/dualTeacher/);"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher/white-blur-bk.jpg);\n"
		"background-color: rgba(255, 255, 255, 0.7);}");
	QRect rc = { 0, 0, rcSecondScreen.width(), rcSecondScreen.height() };
	ui.horizontalLayoutWidget->setGeometry(rc);
	ui.horizontalLayoutWidget->setStyleSheet(style);
	setTopButtonLayout();
	setTitleLayout();
	for (int i = 0; i < 2; ++i) {
		videoWidget[i] = new VideoWidget(initRate, rate, ui.horizontalLayoutWidget);
		videoWidget[i]->setObjectName(QString::fromUtf8("widget_%1").arg(i));
		videoWidget[i]->setMinimumSize(QSize((videoW / rate) , (videoH / rate) ));
		videoWidget[i]->setMaximumSize(QSize(int(videoW / rate), int(videoH / rate)));
		videoWidget[i]->RestoreWidget();
		if (i < 2)
			ui.horizontalLayout_Row1->addWidget(videoWidget[i]);
		else
			ui.horizontalLayoutRow2->addWidget(videoWidget[i]);
	}
	
	setOptionLayout();
	setBottomLabel();
	
	for (int i = 0; i < 2; ++i) {
		connect(videoWidget[i], &VideoWidget::fullScreenSignal,
			this, &DlgExtend::on_fullScreen);
		connect(videoWidget[i], &VideoWidget::muteVideoSignal,
			this, &DlgExtend::on_muteVideo);
		connect(videoWidget[i], &VideoWidget::muteAudioSignal,
			this, &DlgExtend::on_muteAudio);
	}
}


void DlgExtend::setTopButtonLayout()
{
	float w = topButtonWidth / rate;
	float buttonLeftMargin = rcMainScreen.width() - (w * 4 + 3 * topButtonSpace / rate);
	ui.closeButton->setMaximumSize(w, w);
	ui.closeButton->setMinimumSize(w, w);

	ui.minButton->setMaximumSize(w, w);
	ui.minButton->setMinimumSize(w, w);

	ui.maxButton->setMaximumSize(w, w);
	ui.maxButton->setMinimumSize(w, w);

	ui.settingsButton->setMaximumSize(w, w);
	ui.settingsButton->setMinimumSize(w, w);
	//layout
	ui.horizontalLayout_titleButton->setSpacing(topButtonSpace / rate);
	ui.horizontalLayout_titleButton->setContentsMargins(buttonLeftMargin, topButtonTopMargin / rate, topButtonRightMargin / rate, 0);
}


void DlgExtend::setTitleLayout()
{
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));

	ui.btnPrePage->setMaximumSize(QSize(pageBtnW / rate, pageBtnH / rate));
	ui.btnPrePage->setMinimumSize(QSize(pageBtnW / rate, pageBtnH / rate));
	ui.btnNextPage->setMaximumSize(QSize(pageBtnW / rate, pageBtnH / rate));
	ui.btnNextPage->setMinimumSize(QSize(pageBtnW / rate, pageBtnH / rate));

	//title
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);\n"
		"font-weight: 500;\n"
		"color: #000000;\n"
		"line-height: %2px;\n"
		"background-color:#00000000").arg(int(titleFontSize / rate)).arg(titleHeight / rate);
	ui.labTitle->setStyleSheet(style);
	ui.labTitle->setMaximumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.labTitle->setMinimumSize(QSize(labTitleW / rate, labTitleH / rate));
	ui.horizontalLayout_Title->setContentsMargins(0, 0, titleBottomMargin / rate, 0);
	ui.horizontalLayout_Mid->setContentsMargins(midMargin / rate, 0, midMargin / rate, 0);
}
void DlgExtend::setOptionLayout()
{
	//ui.verticalLayout_Widget->setMinimumSize(QSize((2 * videoH + videoSpace) / rate, (2 * videoW + videoSpace) / rate));
	ui.verticalLayout_Widget->setSpacing(videoSpace / rate);
	int verticalMargin = videoH / (rate * 2.0);
	ui.verticalLayout_Widget->setContentsMargins(videoMargin / rate, verticalMargin, videoMargin / rate, verticalMargin);
	ui.horizontalLayout_Row1->setSpacing(videoSpace / rate);
	ui.horizontalLayoutRow2->setSpacing(0);
}

void DlgExtend::setBottomLabel()
{
	//corpration label
	QString style = QString::fromUtf8("font-size: %1px;\n"
		"font-family: Helvetica;\n"
		"color: #575757;\n"
		"line-height: %2px;\n"
		"background-color: rgba(0, 0, 0, 0);\n"
		"border-image: url(:/AgoraCourse/Resources/dualTeacher);").arg((int)(bottomFontSize / rate)).arg((int)(bottomLineHeight / rate));
	ui.label->setStyleSheet(style);
	ui.label->setMaximumWidth(bottomLabelW / rate);
	float leftMargin = (rcSecondScreen.width() - bottomLabelW / rate) / 2;
	ui.horizontalLayout_2->setContentsMargins(QMargins(leftMargin, (float)bottomTopMargin / rate, leftMargin, bottomLabelBottomMargin / rate));
}

void DlgExtend::UpdateLayout()
{
	setTopButtonLayout();
	setTitleLayout();
	setOptionLayout();
	ui.btnPrePage->show();
	ui.btnNextPage->show();
}


void DlgExtend::ShowTopAndBottom(bool bShow)
{
	if (bShow) {
		ui.labTitle->show();
		ui.minButton->show();
		ui.maxButton->show();
		ui.closeButton->show();
		ui.settingsButton->show();
		ui.label->show();
		UpdateLayout();
	}
	else {
		ui.horizontalLayout_Title->setContentsMargins(0, 0, 0, 0);
		ui.horizontalLayout_titleButton->setContentsMargins(0, 0, 0, 0);
		ui.horizontalLayout_Mid->setContentsMargins(0, 0, 0, 0);
		ui.verticalLayout_Widget->setSpacing(0);
		int verticalMargin = (videoH + videoSpace) / 2.0f;
		ui.verticalLayout_Widget->setContentsMargins(0, 0, 0, 0);
		ui.horizontalLayout_Row1->setSpacing(0);
		ui.horizontalLayoutRow2->setSpacing(0);
		ui.btnPrePage->hide();
		ui.btnNextPage->hide();

		ui.labTitle->hide();
		ui.minButton->hide();
		ui.maxButton->hide();
		ui.settingsButton->hide();
		ui.label->hide();
		ui.closeButton->hide();
	}
}

void DlgExtend::on_minButton_clicked()
{
	agoraCourse->setWindowState(Qt::WindowMinimized);
}


void DlgExtend::maxButtonChange(bool max)
{
	SetControlMax(max);
	bMax = !max;
}

void DlgExtend::on_maxButton_clicked()
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	maxButtonChange(bMax);
	emit parentMaxSignal(bMax);
}

void DlgExtend::SetControlMax(bool max)
{
	QRect rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	if (setting.bExtend) {
		rate = rate2;
		rcSecondScreen = ResetMaxRect(rcSecondScreen, !max);
		rcMainScreen = rcSecondScreen;
	}
	else
		rcMainScreen = ResetMaxRect(rcMainScreen, !max);

	rcWidget = { 0, 0, rcMainScreen.width(), rcMainScreen.height() };
	this->setGeometry(rcMainScreen);
	ui.horizontalLayoutWidget->setGeometry(rcWidget);

	setTopButtonLayout();
	setTitleLayout();
	for (int i = 0; i < 2; ++i) {
		videoWidget[i]->maxButtonChange(max, videoW, videoH, rate);
	}
	setOptionLayout();
	setBottomLabel();
	ui.horizontalLayoutWidget->hide();
	ui.horizontalLayoutWidget->show();
}
void DlgExtend::on_fullMaxButton_clicked(bool bMax)
{
	if (!bMax) {
		rate = rate * 3 / 4;
		rate2 = rate2 * 3 / 4;
	}
	else {
		rate = rate * 4 / 3;
		rate2 = rate2 * 4 / 3;
	}
	SetControlMax(bMax);
}

void DlgExtend::on_parentMax_slot(bool childMax)
{
	if (bMax != childMax) {
		on_maxButton_clicked();
	}
}