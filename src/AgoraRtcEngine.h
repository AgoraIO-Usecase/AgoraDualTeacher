#ifndef AGORARTCENGINE_H
#define AGORARTCENGINE_H

#include <AgoraBase.h>
#include <IAgoraMediaEngine.h>
#include <IAgoraMediaPlayer.h>
#include <IAgoraMediaPlayerSource.h>
#include <IAgoraRtcEngine.h>

#include <QMap>
#include <QMutex>
#include <QObject>
#include <memory>

#include "AgoraEnv.h"


#define _400_PREVIEW_5 0
class AgoraRtcEngineEvent;
class AgoraRtcEngineEventEx;
class VideoWidget;
class AgoraRtcEngine : public QObject, public agora::media::IVideoFrameObserver
	, public agora::rtc::IMediaPlayerSourceObserver
{
	Q_OBJECT

public:
	AgoraRtcEngine(QObject *parent = NULL);
	~AgoraRtcEngine();
	static AgoraRtcEngine* GetAgoraRtcEngine();
	static agora::rtc::IRtcEngineEx* GetEngine();
	int Init();
	bool IsJoined() { return joined; }
	void SetJoined(bool b) { joined = b; }
	bool IsJoined2() { 
		return joined2; 
	}
	void SetJoined2(bool b) { joined2 = b; }
	bool LocalVideoPreview(HWND hVideoWnd, bool bPreviewOn = TRUE, agora::media::base::RENDER_MODE_TYPE mode = agora::media::base::RENDER_MODE_TYPE::RENDER_MODE_HIDDEN);
	bool SetEncoderType(int type);
	void VideoSource1SendStreamMessage(QString message);
	void VideoSource2SendStreamMessage(QString message);
	bool VideoSource1JoinChannel(bool enableVideo, const char* token, const char* channel,  agora::rtc::uid_t uid, const agora::rtc::VideoEncoderConfiguration & config);
	int VideoSource2JoinChannel(bool enableVideo, const char* token, const char* channel, agora::rtc::uid_t uid, bool subscribeAudio, bool subscribeVideo);

	void SetVideoEncoderConfigurationEx(agora::rtc::VideoEncoderConfiguration config);
	bool EnableVolumeIndication(int interval , int smooth);
	
	void MuteAllRemoteVideo(bool bMute);
	void MuteAllRemoteAudio(bool bMute);
	void MuteRemoteVideo(unsigned int uid, bool bMute);
	void MuteRemoteAudio(unsigned int uid, bool bMute);
	void MuteLocalAudio(bool bMute);
	void MuteLocalVideo( bool bMute);
	void EnableLocalVideo(bool enable);
	void StartPrimary();
	void EnableAGC(bool bEnable);
	void EnableAEC(bool bEnable);
	void EnableANS(bool bEnable);
	void SetFecParameter(int fec);
	//IVideoFrameObserver
	bool RegisterVideoFrameObserver(bool bEnable);
	virtual bool onCaptureVideoFrame(VideoFrame& videoFrame) override;
	virtual bool onSecondaryCameraCaptureVideoFrame(VideoFrame& videoFrame) override { return true; }
	virtual bool onScreenCaptureVideoFrame(VideoFrame& videoFrame) override { return true; }
	virtual bool onMediaPlayerVideoFrame(VideoFrame& videoFrame, int mediaPlayerId)override;
	virtual bool onSecondaryScreenCaptureVideoFrame(VideoFrame& videoFrame) override { return true; }
	virtual bool onRenderVideoFrame(const char* channelId, agora::rtc::uid_t remoteUid, VideoFrame& videoFrame)override;
	virtual bool onTranscodedVideoFrame(VideoFrame& videoFrame) override { return true; }
	virtual bool getMirrorApplied() override  { return true; }
	//device
	void InitAudioDevice();
	void DestroyAudioDevice();
	void InitVideoDevice();
	void DestroyVideoDevice();
	//mic
	int  GetMicVolume();
	bool SetMicVolume(int nVol);
	int  GetMicCount();
	QString GetCurMicID();
	bool SetCurMic(const QString deviceID);
	bool GetMic(int nIndex, QString& rDeviceName, QString& rDeviceID);
	void TestMicDevice(bool bTestOn);

	int  GetSpeakerVolume();
	bool SetSpeakerVolume(int nVol);
	int  GetSpeakerCount();
	QString GetCurSpeakerID();
	bool SetCurSpeaker(const QString deviceID);
	bool GetSpeaker(int nIndex, QString& rDeviceName, QString& rDeviceID);
	void TestSpeakerDevice(bool bTestOn);

	//video
	int GetVideoDeviceCount();
	QString GetCurVideoDeviceID();
	bool SetCurVideoDevice(const QString deviceID);
	bool GetVideoDevice(int nIndex, QString& rDeviceName, QString& rDeviceID);
	void GetVideoDeviceInfo(QVector<agora::rtc::VideoFormat>& infos);
	//media player
	int OpenVideoSource2(QString url);
	int PlayVideoSource2();
	int StopVideoSource2();
	int PauseVideoSource2(bool bPause);
	int VideoSourceLeave(const char* channel, unsigned int  uid);
	int ShowVideoSource2(agora::media::base::view_t view);
	void SetVideoWidget(QMap<unsigned int, VideoWidget*> widgets);
	void ResetVideoWidgets();
	void SetVideoWidgetEx(QMap<unsigned int, VideoWidget*> widgets);
	void ResetVideoWidgetsEx();
	void onPlayerSourceStateChanged(agora::media::base::MEDIA_PLAYER_STATE state,
		agora::media::base::MEDIA_PLAYER_ERROR ec) override;

	virtual void onPositionChanged(int64_t position)  override {}

	virtual void onPlayerEvent(agora::media::base::MEDIA_PLAYER_EVENT eventCode, int64_t elapsedTime, const char* message)  override {}

	virtual void onMetaData(const void* data, int length) override {}
	virtual void onPlayBufferUpdated(int64_t playCachedBuffer) override {}

	virtual void onPreloadEvent(const char* src, agora::media::base::PLAYER_PRELOAD_EVENT event)  override {}
	virtual void onCompleted()  override {}
	virtual void onAgoraCDNTokenWillExpire()  override {}
	virtual void onPlayerSrcInfoChanged(const agora::media::base::SrcInfo& from, const agora::media::base::SrcInfo& to)  override {}
#if _400_PREVIEW_5
	virtual void onPlayerInfoUpdated(const agora::media::base::PlayerUpdatedInfo& info) override {}
	virtual void onAudioVolumeIndication(int volume)  override {}
#else
	virtual void onPlayerIdsRenew(const char* jsonIds) override{}
#endif
	bool IsvideoSource1Subscribe(){return videoSource1Subscribe;}
	void UpdateVideoSource2Subscribe(bool subscribe);
private:
	
	void InitVideoFrame();
	static AgoraRtcEngine agoraRtcEngine;
	static agora::rtc::IRtcEngine* m_rtcEngine;
	static agora::rtc::IRtcEngineEx* m_rtcEngineEx;
	
	static AgoraRtcEngineEvent m_eventHandler;
	agora::agora_refptr<agora::rtc::IMediaPlayer> media_player_ = nullptr;
	//audio device
	agora::rtc::AAudioDeviceManager* audioManager_ = nullptr;
	agora::rtc::IAudioDeviceCollection *mics_ = nullptr;
	agora::rtc::IAudioDeviceCollection *speakers_ = nullptr;
	//video device
	agora::rtc::AVideoDeviceManager* videoManager_ = nullptr;
	agora::rtc::IVideoDeviceCollection* videos_  = nullptr;
	std::map<std::string, std::string> mapVideos_;
	QMap<unsigned int, VideoWidget*> videoWidgets_;
	QMap<unsigned int, VideoWidget*> videoWidgetsEx_;
	QMutex mtxVideos_;
	QMutex mtxVideosEx_;
	std::string szChannelId_;
	agora::rtc::uid_t localUid_;
	agora::rtc::RtcConnection connection2_;
	agora::rtc::RtcConnection connection_;
	bool joined = false;
	bool joined2 = false;
	
	bool muteLocalVideo_ = false;
	bool muteLocalAudio_ = false;
	bool videoSource1Subscribe = true;
	QString curVideoDevice = "";
signals:
	void userOffline(unsigned int uid, int elapsed);
	void userJoined(unsigned int uid, int elapsed);
	void volumeIndication(unsigned int volume, unsigned int speakerNumber, int totalVolume);
	void joinedChannelSuccess(const char* channel, agora::rtc::uid_t uid, int elapsed);
	void joinedChannelSuccessEx(const char* channel, agora::rtc::uid_t uid, int elapsed);
	void renderSignal();
	void openPlayerComplete();
	void playerError(int ec);
	void streamMessage(unsigned int uid, QString name);
	void leaveChannelSignal();
};

#define rtcEngine AgoraRtcEngine::GetAgoraRtcEngine()

#endif // AGORARTCENGINE_H
