//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "transfer_factory.h"

#include "http_data_request.h"
#include "rtm_data_receiver.h"

#include "utils/log/log.h"
#include "utils/refcountedobject.h"

static const char* const MODULE_NAME = "[RTE.TF]";

namespace agora {
namespace rte {

agora_refptr<IRteDataReceiver> TransferFactory::CreateRteDataReceiver(
    DataReceiveType type) {
  switch (type) {
    case DATA_RECEIVER_RTM:
      return new RefCountedObject<RtmRteDataReceiver>();
    default:
      LOG_ERR_AND_RET_NULL("invalid data receiver type: %d", type);
  }
}

agora_refptr<ISceneDataReceiver> TransferFactory::CreateSceneDataReceiver(
    DataReceiveType type, agora_refptr<IRteDataReceiver> rte_receiver) {
  switch (type) {
    case DATA_RECEIVER_RTM:
      return new RefCountedObject<RtmSceneDataReceiver>(rte_receiver);
    default:
      LOG_ERR_AND_RET_NULL("invalid data receiver type: %d", type);
  }
}

agora_refptr<IDataRequest> TransferFactory::CreateDataRequest(
    DataRequestType req_type, ApiType api_type, utils::worker_type worker) {
  switch (req_type) {
    case DATA_REQUSET_HTTPS:
      return TransferFactory::CreateHttpDataRequest(api_type, worker);
    default:
      LOG_ERR_AND_RET_NULL("invalid data request type: %d", req_type);
  }
}

agora_refptr<IDataRequest> TransferFactory::CreateHttpDataRequest(
    ApiType api_type, utils::worker_type worker) {
  switch (api_type) {
    case API_RTE_LOGIN:
      return new RefCountedObject<HttpApi<API_RTE_LOGIN>>(worker);
    case API_SCENE_ENTER:
      return new RefCountedObject<HttpApi<API_SCENE_ENTER>>(worker);
    case API_SCENE_SNAPSHOT:
      return new RefCountedObject<HttpApi<API_SCENE_SNAPSHOT>>(worker);
    case API_SCENE_SEQUENCE:
      return new RefCountedObject<HttpApi<API_SCENE_SEQUENCE>>(worker);
    case API_SEND_PEER_MESSAGE_TO_REMOTE_USER:
      return new RefCountedObject<
          HttpApi<API_SEND_PEER_MESSAGE_TO_REMOTE_USER>>(worker);
    case API_SEND_SCENE_MESSAGE_TO_ALL_REMOTE_USERS:
      return new RefCountedObject<
          HttpApi<API_SEND_SCENE_MESSAGE_TO_ALL_REMOTE_USERS>>(worker);
    case API_PUBLISH_TRACK:
      return new RefCountedObject<HttpApi<API_PUBLISH_TRACK>>(worker);
    case API_UNPUBLISH_TRACK:
      return new RefCountedObject<HttpApi<API_UNPUBLISH_TRACK>>(worker);
    case API_SET_USER_PROPERTIES:
      return new RefCountedObject<HttpApi<API_SET_USER_PROPERTIES>>(worker);
    case API_SET_SCENE_PROPERTIES:
      return new RefCountedObject<HttpApi<API_SET_SCENE_PROPERTIES>>(worker);
    case API_SEND_PEER_CHAT_MESSAGE_TO_REMOTE_USER:
      return new RefCountedObject<
          HttpApi<API_SEND_PEER_CHAT_MESSAGE_TO_REMOTE_USER>>(worker);
    case API_SEND_CHAT_MESSAGE_TO_ALL_REMOTE_USER:
      return new RefCountedObject<
          HttpApi<API_SEND_CHAT_MESSAGE_TO_ALL_REMOTE_USER>>(worker);
    case API_UPDATE_COURSE_STATE:
      return new RefCountedObject<HttpApi<API_UPDATE_COURSE_STATE>>(worker);
    case API_ALLOW_STUDENT_CHAT:
      return new RefCountedObject<HttpApi<API_ALLOW_STUDENT_CHAT>>(worker);
    default:
      LOG_ERR_AND_RET_NULL("invalid API type: %d", api_type);
  }
}

}  // namespace rte
}  // namespace agora
