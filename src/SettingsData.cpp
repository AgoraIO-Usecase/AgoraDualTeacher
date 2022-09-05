#include "SettingsData.h"

CSettingsData* CSettingsData::GetSettingsData()
{
	static CSettingsData settingsData;
	return &settingsData;
}
CSettingsData setting;

CSettingsData::CSettingsData()
	:  encoder_type(ENCODER_SOFTWARE)
	, resolution_type(VIDEO_RESOLUTION_480P)
	, microphoneId(QString(""))
	, speakerId(QString(""))
	, videoSource1Id(QString(""))
	, microphoneVolume(0)
	, speakerVolume(0)
	, className("")
{
	userInfo.uid = 0;
	userInfo2.uid = 0;
	Resolution  res = { 1280, 720 };
	resolution.push_back(res);
	/*resolution[0].width = 1280;
	resolution[0].height = 720;
	resolution[1].width = 1920;
	resolution[1].height = 1080;
	resolution[2].width = 3840;
	resolution[2].height = 2160;*/
}

CSettingsData::~CSettingsData()
{

}

void CSettingsData::SetupVideoResolution(int w, int h, int fps, int bit)
{
	//resolution.width = w;
	//resolution.height = h;
	frameRate = fps;
}
