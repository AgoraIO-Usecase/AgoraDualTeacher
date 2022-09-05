#include "./Rtm.h"
#include "IAgoraRtmService.h"
#include "JoypadController.h"

using namespace agora;
using namespace agora::rtm;

static JoypadController joyController;

Rtm& Rtm::GetRtmEngine()
{
    static Rtm rtm;
    return rtm;
}

bool Rtm::init(const char *appid)
{
    rtmService = createRtmService();
    if (rtmService == nullptr)
    {
        return false;
    }
   
    int ret = rtmService->initialize(appid, &rtmEventHandler);
    if (ret != 0)
    {
       
        return false;
    }
    
    ret = rtmService->setParameters("{\"rtm.peer.msg_qps_limit\":240}");
    if (ret != 0) {
    
    }
    return true;
}

bool Rtm::enter(const char *token, const char *userid)
{
    if (blogin) return true;
    blogin = true;
    int ret = rtmService->login(nullptr, userid);

    if (ret != 0)
    {
        return false;
    }
   
    return true;
}

void Rtm::leave()
{
    if (rtmService) {
        
        int ret = rtmService->logout();
        blogin = false;
        if (ret != 0)
        {
            return;
        }
    }
}

void Rtm::uninit()
{
    if (rtmService) {
        rtmService->release();
        rtmService = nullptr;
    }
  //  app->onRtmUninit();
}

bool Rtm::send(unsigned char* buff, int len, const char* peerId)
{
    if (!rtmService || len == 0 || !rtmEventHandler.IsLogined())
        return false;
    IMessage* message = rtmService->createMessage(buff,len);
    SendMessageOptions op;
    op.enableHistoricalMessaging = false;
    op.enableOfflineMessaging = false;
    int ret = rtmService->sendMessageToPeer(peerId, message, op);
    return 0 == ret;
}

bool Rtm::joinRtmChannel(const char* channelId) {
    if (!rtmService) {
        return false;
    }
    rtmChannel = rtmService->createChannel(channelId, &rtmEventHandler);
    if (!rtmChannel) {
        return false;
    }
    return 0 == rtmChannel->join();
}

bool Rtm::leaveRtmChannel()
{
    if (!rtmService || !rtmChannel) {
        return false;
    }
    return 0 == rtmChannel->leave();
}

bool Rtm::getMembers()
{
    return 0 == rtmChannel->getMembers();
}

bool Rtm::sendRtmChannelMessage(const uint8_t* msg, int len)
{
    if (!rtmService || !rtmChannel)
        return false;

    IMessage* message = rtmService->createMessage(msg, len);
    SendMessageOptions messageOptions;
    return 0 == rtmChannel->sendMessage(message, messageOptions);
}

Rtm::Rtm() 
    : rtmService(nullptr),
    rtmEventHandler(&joyController)
{
}

Rtm::~Rtm()
{
    if (rtmService)
    {
        uninit();
    }
}

bool Rtm::Dispatch(SDL_Event* event)
{
    return rtmEventHandler.Dispatch(event);
}

void Rtm::DispatchKBMessage(void* param)
{
    rtmEventHandler.DispatchKBMessage(param);
}