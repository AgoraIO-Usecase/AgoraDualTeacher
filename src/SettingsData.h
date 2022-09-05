#pragma once
#include <unordered_map>
#include <IAgoraRtcEngine.h>
#include <QString>
#include <QRect>
#include <QVector>
typedef struct tagDeviceInfo
{
	QString id = "";
	QString name = "";
}DeviceInfo;

typedef struct tagUserInfo
{
	unsigned int uid = 0;
	QString name = "";
}UserInfo;

typedef struct tagWidgetInfo
{
	UserInfo userInfo;
	bool muteAudio = false;
	bool muteVideo = false;
}WidgetInfo;

enum VIDEO_RESOLUTION_TYPE
{
	VIDEO_RESOLUTION_240P = 0,
	VIDEO_RESOLUTION_360P,
	VIDEO_RESOLUTION_480P,
	VIDEO_RESOLUTION_720P,
	VIDEO_RESOLUTION_1080P
};

enum ENCODER_TYPE{
	ENCODER_SOFTWARE = 0,
	ENCODER_NVIDIA,
	ENCODER_INTEL
};
typedef struct tagResolution
{
	int width = 1920;
	int height = 1080;
}Resolution;
enum STUDENT_1VN_TYPE
{
	STUDENT_1V1 = 0,
	STUDENT_1V4 ,
	STUDENT_1V9 ,
	STUDENT_1V12,
	STUDENT_1V16,
};

enum DPI_TYPE {
	DPI_1080 = 0,
	DPI_2K,
	DPI_4K,
};
class CSettingsData
{
public:
	CSettingsData();
	~CSettingsData();

	static CSettingsData* GetSettingsData();

	void SetupVideoResolution(int w, int h, int fps, int bit);
	
public:
	int frameRate = 30;
	
	QVector<Resolution> resolution;
	int resIndex = 0;
	VIDEO_RESOLUTION_TYPE resolution_type;
	int encoder_type;
	
	QString microphoneId;
	QString speakerId;
	QString videoSource1Id;
	int microphoneVolume;
	int speakerVolume;
	bool enabledVideoSource1 = true;
	bool enabledVideoSource2 = false;
	bool agcOn = true;
	bool aecOn = true;
	bool ansOn = true;
	QRect rcMainScreen;
	float rate = 1.0f;
	UserInfo userInfo;
	UserInfo userInfo2;
	QString className;
	QString videoSource2Url = "";// "rtmp://ongoing.pull-rtmp.bsc.agoramde.agoraio.cn/live/agora123";//"rtmp://ongoing.pull-rtmp.bsc.agoramde.agoraio.cn/live/test1234";
	bool bExtend = false;
	DPI_TYPE dpiType = DPI_1080;
private:
	
};
extern CSettingsData setting;
//#define setting  CSettingsData::GetSettingsData()