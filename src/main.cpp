#include "agoracourse.h"
#include <QApplication>
#include <QGuiApplication>
#include <QDir>
//#include <BugTrap.h>

#include "DlgSettingVideo.h"
#include "DlgSettingSelect.h"
#include "DlgVersion.h"
#include "DlgSettings.h"
#include "DlgSettingAudio.h"
#include "DlgVideoRoom.h"
int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	AgoraCourse w;
	w.show();

	bool b = a.exec();
	
	return b;
}
