
#include "agora_log.h"
#include <stdio.h>
#include <string>
#include <stdarg.h>
#include <time.h>
#include <QDir>
#include <QDateTime>

FILE* agoralog_file = NULL;
bool startLogService(const char* path)
{
	if (agoralog_file)
		return false;
	if (path) {
		agoralog_file = fopen(path, "w");
		return true;
	}
	return false;
}

void stopLogService()
{
	if (agoralog_file) {
		fflush(agoralog_file);
		fclose(agoralog_file);
		agoralog_file = nullptr;
	}
}


void agora_log(const char *format, ...)
{
	//
	va_list la;
	va_start(la, format);
	if (!agoralog_file)
		return;
	vfprintf(agoralog_file, format, la);
	va_end(la);
	fprintf(agoralog_file, "\n");
	fflush(agoralog_file);
}

std::string getLogTime()
{
	time_t rawtime;
	struct tm timeinfo;

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	char szTime[100] = { 0 };

	/*sprintf_s(szTime, "%04d-%02d-%02d %02d:%02d:%02d",
					timeinfo.tm_year + 1900, timeinfo.tm_mon, timeinfo.tm_mday
					, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);*/
	sprintf_s(szTime, "%02d:%02d:%02d",
		timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

	return szTime;
}

std::string getLogFileName()
{
	time_t rawtime;
	struct tm timeinfo;

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	char szTime[100] = { 0 };

	sprintf_s(szTime, "agora_%04d-%02d-%02d_%02d-%02d-%02d.log",
		timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday
		, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

	return szTime;
}


AgoraQtLog AgoraQtLog::agoraQtLog;
AgoraQtLog::AgoraQtLog(QObject *parent)
	: QObject(parent)
{
	QString currentPath = QDir::currentPath();
	currentPath += QString("/log/");
	QDir pathDir(currentPath);
	if (!pathDir.exists()){
		pathDir.mkdir(currentPath);
	}
	
	QDateTime current_time = QDateTime::currentDateTime();
	QString logFileTime = current_time.toString("yyyy-MM-dd_hh-mm-ss");
	
	QString fullPath = QString("%1.log").arg(logFileTime);
	//fullPath = currentPath + fullPath;
	startLogService((currentPath+fullPath).toUtf8().data());
}

AgoraQtLog::~AgoraQtLog()
{
	stopLogService();
}

AgoraQtLog* AgoraQtLog::GetAgoraQtLog()
{
	return &agoraQtLog;
}

void AgoraQtLog::agora_output_log(QString logFormat)
{
	QDateTime current_time = QDateTime::currentDateTime();
	QString logFileTime = current_time.toString("hh:mm:ss");
	
	QString logInfo = QString("%1    %2").arg(logFileTime).arg(logFormat);
	agora_log(logInfo.toLocal8Bit().data());
}

void AgoraQtLog::agora_output_userInfo(UserInfo& info, QString funcName)
{
	QString role_type = info.role ? QString("teacher") : QString("student");
	//AgoraQtLog::GetAgoraQtLog()->agora_output_log(QString("class:%1, uid:%2, userName:%3, role:%4").arg(info.class_name).arg(info.uid).arg(info.user_name).arg(role_type));
}

void AgoraQtLog::agora_output_userInfo(std::unordered_map<std::string, std::string> mapAttributes, QString funcName)
{

}

AgoraIniFile AgoraIniFile::agoraQtIniFile;

AgoraIniFile* AgoraIniFile::GetAgoraIniFile()
{
	return &agoraQtIniFile;
}

AgoraIniFile::AgoraIniFile(QObject *parent)
{
	QString currentPath = QDir::currentPath();
	QString fullpath = currentPath + QString("config.ini");
	configIniFile = new QSettings(currentPath + QString("/config.ini"), QSettings::IniFormat);
}

AgoraIniFile::~AgoraIniFile()
{
	delete configIniFile;
	configIniFile = NULL;
}

void AgoraIniFile::readIni(QString key, unsigned int uid)
{
	uid = configIniFile->value(key).toUInt();
}

void AgoraIniFile::writeIni(QString key, unsigned int uid)
{
	configIniFile->setValue(key, uid);
}