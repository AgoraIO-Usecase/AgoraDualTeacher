#pragma once
#include <QObject>
#include <QSettings>

#include <unordered_map>
class AgoraQtLog : public QObject
{
	Q_OBJECT

public:
	AgoraQtLog(QObject *parent = nullptr);
	~AgoraQtLog();
	static AgoraQtLog* GetAgoraQtLog();
	void agora_output_log(QString logFormat); 
	//void agora_output_userInfo(UserInfo& info, QString funcName); 
	void agora_output_userInfo(std::unordered_map<std::string, std::string> mapAttributes, QString funcName);
private:
	static AgoraQtLog agoraQtLog;
};

class AgoraIniFile : public QObject
{
	Q_OBJECT
public:
	AgoraIniFile(QObject *parent = nullptr);
	~AgoraIniFile();

	static AgoraIniFile* GetAgoraIniFile();

	void readIni(QString key, unsigned int uid);
	void writeIni(QString key, unsigned int uid);
	QSettings *configIniFile;
private:
	static AgoraIniFile agoraQtIniFile;
};
//