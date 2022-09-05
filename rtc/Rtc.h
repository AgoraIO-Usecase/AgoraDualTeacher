#pragma once

#define APPID "aab8b8f5a8cd4469a63042fcfafe7063"
#include <string>
#include "IAgoraRtcEngine.h"
#include "IAgoraMediaEngine.h"
#include "./EventHandler.h"
#include <QObject>
#include "SDL.h"
#if  defined(_WIN32)
#include <Windows.h>
#define CanvasType HWND
#elif  defined(__APPLE__)
#define CanvasType unsigned int
#endif
struct EncoderParam {
	EncoderParam()
		: fps(15)
		, bitrate(500)
		, width(640)
		, height(360)
	{}
	int fps;
	int bitrate;
	int width;
	int height;
};

class Rtc : public QObject, public agora::media::IVideoFrameObserver {
	Q_OBJECT
public:
	Rtc();
	bool init(const char* appid, CanvasType windowid = 0);
	bool enter(const char* roomid, unsigned int userid, bool subScribeVideo, bool p2p) ;

	void leave() ;
	void uninit() ;
	
	void setVideoEncoderConfiguration(EncoderParam param);
	void updateScreenCaptureParam(EncoderParam param);
	void setClientRole(bool bHost);
	bool startScreenCapture(EncoderParam param, agora::rtc::Rectangle screenRect, bool cursor);
	int send(unsigned char* buff, int len);
	static Rtc& GetRtcEngine();
	IRtcEngine* GetEngine() { return m_lpAgoraEngine; }
	void SetVOS(QString serverIP ) { ip = serverIP; }


	virtual bool onCaptureVideoFrame(VideoFrame& videoFrame) override { return true; }

	virtual bool onSecondaryCameraCaptureVideoFrame(VideoFrame& videoFrame) override { return true; }

	virtual bool onScreenCaptureVideoFrame(VideoFrame& videoFrame) { return true; }
	
	virtual bool onMediaPlayerVideoFrame(VideoFrame& videoFrame, int mediaPlayerId) override { return true; }

	virtual bool onSecondaryScreenCaptureVideoFrame(VideoFrame& videoFrame) override { return true; }

	virtual bool onRenderVideoFrame(const char* channelId, agora::rtc::uid_t remoteUid, VideoFrame& videoFrame) override;

	virtual bool onTranscodedVideoFrame(VideoFrame& videoFrame) override { return true; }
	void SetRenderer(SDL_Renderer* render);
	void SetRemoteVideoInfo(RemoteVideoStats videoStats) {
		remoteFPS = videoStats.decoderOutputFrameRate;
		remoteVideoBitrate = videoStats.receivedBitrate;
		remoteLostFrame = videoStats.frameLossRate;
	}
	void SetRtcStats(RtcStats stats) {
		//remoteVideoBitrate = stats.rxVideoBytes;
		rxAudioKBitRate    = stats.rxAudioKBitRate;
		lastmileDely       = stats.lastmileDelay;        
	}
	void setColor(SDL_Color clr) { color = clr; }
public:
	void showFrame(CanvasType hwnd,agora::rtc::uid_t userId, int elapsed);
signals:
	void onFirstRemoteVideoDecoded(unsigned int userId, int width, int height, int elapsed);
	void onUserJoined(unsigned int uid, int elapsed);
	void onUserOffline(unsigned int uid);
	void onStreamMessage(uid_t userId, int streamId, QString data, size_t length, uint64_t sentTs);
	void onJoinChannelSuccess();
	void onLeaveChannel();
private:
	void setParameters();
	agora::rtc::IRtcEngine* m_lpAgoraEngine;
	EventHandler m_eventHandler;
	std::string roomId;
//	conn_id_t connectionId;
	int connectionId;
	int streamId_ = 0;
	bool host_ = false;
	QString ip = "";

	SDL_Renderer* renderer = nullptr;
	SDL_Texture* texture = nullptr;
	int width = 0;
	int height = 0;

	
	SDL_Surface* surface = nullptr;
	SDL_Texture* textureInfo = nullptr;
	int remoteFPS = 0;
	int remoteVideoBitrate = 0;
	int remoteLostFrame = 0;
	int lastmileDely = 0;
	int rxAudioKBitRate = 0;
	int currentRemoteFPS = 0;
	int currentRemoteVideoBitrate = 0;
	int currentRemoteLostFrame = 0;
	int currentLastmileDely = 0;
	int currentRxAudioKBitRate = 0;


	SDL_Rect dstRect = { 50, 50, 1200, 50 };
	SDL_Color color = { 255, 0, 0 };

	SDL_mutex* renderMutex = nullptr;
};
