#include "DeviceManager.h"


AgoraDeviceManager* AgoraDeviceManager::GetAgoraDeviceManager()
{
	return &agoraDeviceManager;
}

AgoraDeviceManager AgoraDeviceManager::agoraDeviceManager;

AgoraDeviceManager::AgoraDeviceManager()
{
}


AgoraDeviceManager::~AgoraDeviceManager()
{
}
