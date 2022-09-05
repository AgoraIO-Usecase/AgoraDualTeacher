#include "./Rtc.h"
#include "IAgoraRtcEngine.h"
#include "SDL_ttf.h"
#include "processenv.h"
#include <QUuid>
#include <QDateTime>
#include <QSysInfo>
#include <QCryptographicHash>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <thread>
#include "Config.h"
#include <mutex>
extern struct EnjoyUserInfo userinfo; 

Rtc& Rtc::GetRtcEngine()
{
    static Rtc rtc;
    return rtc;
}

static TTF_Font* font = nullptr;

Rtc::Rtc():m_lpAgoraEngine(nullptr){}
bool Rtc::init(const char* appid, CanvasType hwnd)
{
    m_lpAgoraEngine = (agora::rtc::IRtcEngine*)createAgoraRtcEngine();
    m_eventHandler.init();
    agora::rtc::RtcEngineContext ctx_ex;
    ctx_ex.appId = appid;
    ctx_ex.enableAudioDevice = true;
    ctx_ex.eventHandler = &m_eventHandler;
    ctx_ex.context = hwnd;
    ctx_ex.channelProfile = agora::CHANNEL_PROFILE_CLOUD_GAMING;
  
    m_lpAgoraEngine->initialize(ctx_ex);
    
    //m_lpAgoraEngine->setLogLevel(agora::commons::LOG_LEVEL::LOG_LEVEL_INFO);
	m_lpAgoraEngine->enableVideo();
   
    DataStreamConfig config;
    config.syncWithAudio = false;
    config.ordered = false;
    int ret = m_lpAgoraEngine->createDataStream(&streamId_, config);
    TTF_Init();
    std::string fongPath = "";
#if defined(_WIN32)
    fongPath = "C:\\Windows\\Fonts\\simhei.TTF";
#endif
    font = TTF_OpenFont(fongPath.data(), 30);

    renderMutex = SDL_CreateMutex();
    return true;
}

void Rtc::setVideoEncoderConfiguration(EncoderParam param)
{
    VideoEncoderConfiguration config;
    config.frameRate         = param.fps;
    config.bitrate           = param.bitrate;
    config.dimensions.width  = param.width;
    config.dimensions.height = param.height;

    m_lpAgoraEngine->setVideoEncoderConfiguration(config);
}

void Rtc::updateScreenCaptureParam(EncoderParam param)
{

}

void Rtc::setClientRole(bool bHost)
{
    m_lpAgoraEngine->setClientRole( CLIENT_ROLE_BROADCASTER);
    m_lpAgoraEngine->muteLocalVideoStream(!bHost);
    host_ = bHost;
}

bool Rtc::startScreenCapture(EncoderParam param, agora::rtc::Rectangle screenRect, bool cursor)
{
    agora::rtc::ScreenCaptureParameters scp;
    scp.captureMouseCursor = true;
    scp.bitrate = param.bitrate;
    scp.dimensions.width = param.width;
    scp.dimensions.height = param.height;
    scp.frameRate = param.fps;
    scp.captureMouseCursor = cursor;
    return 0 == m_lpAgoraEngine->startScreenCaptureByScreenRect(screenRect, screenRect, scp);
}

void Rtc::setParameters()
{
    agora::base::AParameter apm(m_lpAgoraEngine);
    apm->setInt("rtc.video.playout_delay_max", 0);
    apm->setInt("rtc.video.playout_delay_min", 0);
    //关闭pacer
    apm->setBool("rtc.enable_webrtc_pace", false);
    //关闭vpr
    apm->setBool("che.video.vpr.enable", false);
    // vos
    //58.211.82.171:4005 udp
    //61.164.250.30:4055 aut
    QString param = QString("{\"rtc.vos_list\":[\"%1\"").arg(ip);
    if (!param.isEmpty())
        apm->setParameters(param.toStdString().c_str());
}

bool Rtc::enter(const char* roomid, unsigned int userid, bool subScribeVideo, bool p2p)
{
    const char* token = nullptr;
    ChannelMediaOptions op;
    op.publishCameraTrack = false;
    op.publishScreenTrack = true;
    op.publishCustomVideoTrack = false;
    op.publishAudioTrack  = true;
    op.autoSubscribeVideo = subScribeVideo;
    op.autoSubscribeAudio = true;
    op.enableAudioRecordingOrPlayout = true;
    op.clientRoleType = CLIENT_ROLE_TYPE::CLIENT_ROLE_BROADCASTER;
    op.channelProfile = agora::CHANNEL_PROFILE_CLOUD_GAMING;
    roomId = roomid;
    agora::base::AParameter apm(m_lpAgoraEngine);
    if(p2p)
        apm->setParameters("{\"rtc.default_p2\":true}");
    else
        apm->setParameters("{\"rtc.default_p2\":false}");
    setParameters();
    if (!userinfo.host) {
        agora::media::IMediaEngine* pMediaEngine = nullptr;
        m_lpAgoraEngine->queryInterface(AGORA_IID_MEDIA_ENGINE, (void**)&pMediaEngine);
        pMediaEngine->registerVideoFrameObserver(this);
    }
 
    int ret = m_lpAgoraEngine->joinChannel(token,roomid, userid, op);
    if (ret != 0) {
         return false;
    }
    return true;
}

void Rtc::leave()
{
    if (m_lpAgoraEngine) {
		int ret = m_lpAgoraEngine->leaveChannel();//leaveChannelEx(roomId.c_str(), connectionId);
        if (ret != 0) {
        }
    }
    return;
}

void Rtc::uninit()
{   
    if(m_lpAgoraEngine){
        m_lpAgoraEngine->release();
        m_lpAgoraEngine = nullptr;
    }
  //  app->onRtcUninit();
}

void Rtc::showFrame(CanvasType hwnd, agora::rtc::uid_t userId, int elapsed)
{
    VideoCanvas canvas;
    canvas.renderMode = agora::media::base::RENDER_MODE_FIT;
    canvas.uid = userId;
    canvas.view = hwnd;
    int ret = m_lpAgoraEngine->setupRemoteVideo(canvas);
    if (hwnd) {
        
    }
    
}

int Rtc::send(unsigned char* buff, int len)
{
    Sleep(100);
    int ret = m_lpAgoraEngine->sendStreamMessage(streamId_, (const char*)buff, len);
    return ret;
}

void Rtc::SetRenderer(SDL_Renderer* render)
{
    SDL_LockMutex(renderMutex);
    renderer = render;
    if (render == nullptr) {
        SDL_FreeSurface(surface);
        SDL_DestroyTexture(texture);
        SDL_DestroyTexture(textureInfo);
        surface = nullptr;
        texture = nullptr;
        textureInfo = nullptr;
        width = 0;
        height = 0;
    }
    SDL_UnlockMutex(renderMutex); 
}

bool Rtc::onRenderVideoFrame(const char* channelId, agora::rtc::uid_t remoteUid, VideoFrame& videoFrame)
{
    SDL_LockMutex(renderMutex);
    if (!renderer) {
        goto End;
        return true;
    }

    if (width != videoFrame.width || height != videoFrame.height) {
        if (texture)
            SDL_DestroyTexture(texture);
        width  = videoFrame.width;
        height = videoFrame.height;
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
    }

    if (texture) {
        if (currentRemoteFPS != remoteFPS || currentRemoteVideoBitrate != remoteVideoBitrate ||
            currentRxAudioKBitRate != rxAudioKBitRate || currentLastmileDely != lastmileDely ) {
            char szText[100] = { 0 };
            currentRemoteFPS = remoteFPS;
            currentRemoteVideoBitrate = remoteVideoBitrate;
            currentRemoteLostFrame = remoteLostFrame;
            currentLastmileDely = lastmileDely;
            currentRxAudioKBitRate = rxAudioKBitRate;
            sprintf(szText, "video: fps=%d, bitrate=%dKbps; audio:  bitrate=%d; lastMileDelay:%d", 
                currentRemoteFPS, currentRemoteVideoBitrate,  currentRxAudioKBitRate, currentLastmileDely);
            if (font)
                surface = TTF_RenderText_Blended(font, szText, color);
            if (surface)
                textureInfo = SDL_CreateTextureFromSurface(renderer, surface);
        }
        // D3D11DeviceContext::Map, write to buffer
        SDL_UpdateYUVTexture(texture, nullptr, videoFrame.yBuffer, videoFrame.yStride, videoFrame.uBuffer, videoFrame.uStride, videoFrame.vBuffer, videoFrame.vStride);

        //D3D11DeviceContext::Map, read from buffer
        //SDL_RenderReadPixels(renderer, nullptr, SDL_PIXELFORMAT_IYUV, );
        //D3D11_SetCopyState(renderer, cmd, NULL);
        //D3D11_DrawPrimitives(renderer, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, start, 4);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        if (textureInfo) {
            SDL_RenderCopy(renderer, textureInfo, nullptr, &dstRect);
        }
        
        SDL_RenderPresent(renderer);
    }
End:
    SDL_UnlockMutex(renderMutex);
    return true;
}

