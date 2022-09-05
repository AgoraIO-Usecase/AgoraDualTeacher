#pragma once

#include "AgoraPlayoutManager.h"
#include "AgoraAudInputManager.h"
#include "AgoraCameraManager.h"

class AgoraDeviceManager
{
public:
	AgoraDeviceManager();
	~AgoraDeviceManager();
	CAgoraAudInputManager inputManager;
	CAgoraPlayoutManager playoutManager;
	CAgoraCameraManager  cameraManager;

	static AgoraDeviceManager* GetAgoraDeviceManager();
private:
	static AgoraDeviceManager agoraDeviceManager;

	

};

