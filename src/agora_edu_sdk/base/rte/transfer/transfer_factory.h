//
//  Agora Real-time Engagement
//
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include "internal/IAgoraRteTransferProtocol.h"

#include "transfer/http_data_request.h"
#include "utils/thread/thread_pool.h"

namespace agora {
namespace rte {

class TransferFactory {
 public:
  // receiver
  static agora_refptr<IRteDataReceiver> CreateRteDataReceiver(DataReceiveType type);

  static agora_refptr<ISceneDataReceiver> CreateSceneDataReceiver(
      DataReceiveType type, agora_refptr<IRteDataReceiver> rte_receiver);

  // request
  static agora_refptr<IDataRequest> CreateDataRequest(DataRequestType req_type, ApiType api_type,
                                                      utils::worker_type worker);

#ifdef FEATURE_ENABLE_UT_SUPPORT
 public:  // NOLINT
#else
 private:  // NOLINT
#endif  // FEATURE_ENABLE_UT_SUPPORT
  static agora_refptr<IDataRequest> CreateHttpDataRequest(ApiType api_type,
                                                          utils::worker_type worker);
};

}  // namespace rte
}  // namespace agora
