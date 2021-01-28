//
//  signaling_protocol.h
//  mediasdk
//
//  Created by junhao wang on 04/08/2017.
//  Copyright © 2017 Agora. All rights reserved.
//

#pragma once

#ifndef signaling_protocol_h
#define signaling_protocol_h

#include <base/base_type.h>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "utils/packer/packer_type.h"
#include "utils/packer/packet.h"

// LBES: live broadcasting edge service

#define SIGNALING_GET_LBES_URI 1
#define SIGNALING_GET_LBES_RES_URI 2
#define SIGNALING_LBES_QUIT_URI 502
#define SIGNALING_LBES_QUIT_RES_URI 503
#define SIGNALING_LBES_PING_URI 504
#define SIGNALING_LBES_PONG_URI 505
#define SIGNALING_LBES_UNPUBLISH_URI 508
#define SIGNALING_LBES_UNPUBLISH_RES_URI 509
#define SIGNALING_LBES_PUBLISH_REQ_URI_v1 512
#define SIGNALING_LBES_PUBLISH_RES_URI_v1 513

#define SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI_v1 514
#define SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI_v1 515
#define SIGNALING_LBES_ADD_INJECT_STREAM_URL_REQ_URI 516
#define SIGNALING_LBES_ADD_INJECT_STREAM_URL_RES_URI 517
#define SIGNALING_LBES_REMOVE_INJECT_STREAM_URL_REQ_URI 518
#define SIGNALING_LBES_REMOVE_INJECT_STREAM_URL_RES_URI 519
#define SIGNALING_LBES_STREAM_INJECTED_STATUS_RES_URI 520

#define SIGNALING_LBES_PUBLISH_REQ_URI_v2 521
#define SIGNALING_LBES_PUBLISH_RES_URI_v2 522
#define SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI_v2 523
#define SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI_v2 524

#define SIGNALING_LBES_PUBLISH_REQ_URI_v3 525
#define SIGNALING_LBES_PUBLISH_RES_URI_v3 526
#define SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI_v3 527
#define SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI_v3 528

#define SIGNALING_LBES_PUBLISH_REQ_URI_v4 529
#define SIGNALING_LBES_PUBLISH_RES_URI_v4 530
#define SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI_v4 531
#define SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI_v4 532

#define SIGNALING_PEER_COMMAND_URI 600

static std::string LIVE_STREAM_LOGIN = "LoginLiveStream";

static std::string LIVE_STREAM_TASK_PUBLISH_STREAM = "PublishStream";
static std::string LIVE_STREAM_TASK_DESTROY_LIVE_STREAM = "DestroyLiveStream";
static std::string LIVE_STREAM_TASK_UNPUBLISH_STREAM = "UnpublishStream";
static std::string LIVE_STREAM_TASK_UPDATE_TRANSCODING = "UpdateTranscoding";
static std::string LIVE_STREAM_TASK_PUBLISH_STREAM_STATUS = "PublishStreamStatus";

static std::string LIVE_STREAM_TASK_ADD_INJECT_STREAM = "InjectStream";
static std::string LIVE_STREAM_TASK_REMOVE_INJECT_STREAM = "UninjectStream";

static std::string LIVE_STREAM_TASK_TYPE_REQ = "request";
static std::string LIVE_STREAM_TASK_TYPE_RES = "response";
static std::string LIVE_STREAM_TASK_TYPE_STATUS = "status";
#define SIGNALING_CLIENT_MESSAGE_URI 1001
#define SIGNALING_CLIENT_MESSAGE2_URI 1000

namespace agora {
using namespace commons;
namespace rtc {
namespace protocol {
#define name_to_str(name_31415926) (#name_31415926)

enum { SIGNALING_SERVER_TYPE = 20 };

DECLARE_PACKABLE_5(lbec_detail, std::string, vosIp, std::string, signalingSdkVersion, std::string,
                   sdkVersion, uint32_t, clientRole, std::string, appId);

DECLARE_PACKET_8(PGetLbesReq, SIGNALING_SERVER_TYPE, SIGNALING_GET_LBES_URI, std::string, sid,
                 seq_t, seq, uint64_t, ts, std::string, cname, uid_t, uid, std::string, url,
                 int32_t, lbecNumber, lbec_detail, detail);

DECLARE_PACKET_11(PGetLbesRes, SIGNALING_SERVER_TYPE, SIGNALING_GET_LBES_RES_URI, std::string, sid,
                  seq_t, seq, uint32_t, code, uint64_t, server_ts, std::string, cname, uid_t, uid,
                  vid_t, vid, int32_t, lbecNumber, std::string, lbesToken, std::vector<std::string>,
                  addresses, std::vector<std::string>, ipAddresses);

DECLARE_PACKET_7(PQuitLbesReq, SIGNALING_SERVER_TYPE, SIGNALING_LBES_QUIT_URI, std::string, command,
                 std::string, sid, seq_t, seq, uint64_t, ts, std::string, cname, uid_t, uid, vid_t,
                 vid);
DECLARE_PACKET_8(PQuitLbesRes, SIGNALING_SERVER_TYPE, SIGNALING_LBES_QUIT_RES_URI, std::string,
                 command, std::string, sid, seq_t, seq, uint32_t, code, uint64_t, server_ts,
                 std::string, cname, uid_t, uid, uint32_t, vid);

DECLARE_PACKET_6(PLbesPing, SIGNALING_SERVER_TYPE, SIGNALING_LBES_PING_URI, std::string, sid, seq_t,
                 seq, uint64_t, ts, std::string, cname, uid_t, uid, vid_t, vid);
DECLARE_PACKET_7(PLbesPong, SIGNALING_SERVER_TYPE, SIGNALING_LBES_PONG_URI, std::string, sid, seq_t,
                 seq, uint32_t, code, uint64_t, server_ts, std::string, cname, uid_t, uid, uint32_t,
                 vid);

DECLARE_PACKABLE_7(transcoding_user, uid_t, uid, int32_t, x, int32_t, y, int32_t, width, int32_t,
                   height, int32_t, zOrder, double, alpha);

/*
DECLARE_PACKABLE_11(transcoding_info, int32_t, width, int32_t, height, int32_t, gop, int32_t,
sampleRate, int32_t, framerate, int32_t, bitrate, bool, lowLatency, int32_t, videoCodecProfile,
uint32_t, backgroundColor, std::string, userConfigExtraInfo, std::vector<transcoding_user>,
userConfigs);
*/

DECLARE_PACKABLE_13(transcoding_info, int32_t, width, int32_t, height, int32_t, videoGop, int32_t,
                    videoFramerate, int32_t, videoCodecProfile, int32_t, videoBitrate, bool,
                    lowLatency, int32_t, audioSampleRate, int32_t, audioBitrate, int32_t,
                    audioChannels, uint32_t, backgroundColor, std::string, userConfigExtraInfo,
                    std::vector<transcoding_user>, userConfigs);

//            DECLARE_PACKET_9(PLbesPublishReq_v1, SIGNALING_SERVER_TYPE,
//            SIGNALING_LBES_PUBLISH_REQ_URI_v1, seq_t, seq, std::string, sid, vid_t, vid,
//            std::string, cname, uid_t, uid, uint64_t, ts, std::string, lbesToken, std::string,
//            url, std::unique_ptr<transcoding_info>, transcodingConfig);
//            DECLARE_PACKET_9(PLbesPublishRes_v1, SIGNALING_SERVER_TYPE,
//            SIGNALING_LBES_PUBLISH_RES_URI_v1, seq_t, seq, std::string, sid, uint32_t, vid,
//            std::string, cname,uid_t, uid, uint64_t, server_ts, std::string, url, uint32_t, code,
//            std::string, reason);

DECLARE_PACKET_8(PLbesUnpublishReq, SIGNALING_SERVER_TYPE, SIGNALING_LBES_UNPUBLISH_URI,
                 std::string, command, std::string, sid, seq_t, seq, uint64_t, ts, std::string,
                 cname, uid_t, uid, vid_t, vid, std::string, url);
DECLARE_PACKET_10(PLbesUnpublishRes, SIGNALING_SERVER_TYPE, SIGNALING_LBES_UNPUBLISH_RES_URI,
                  std::string, command, std::string, sid, seq_t, seq, uint32_t, code, uint64_t,
                  server_ts, std::string, cname, uid_t, uid, uint32_t, vid, std::string, url,
                  std::string, reason);

//            DECLARE_PACKET_7(PLbesTranscodingUpdateReq_v1, SIGNALING_SERVER_TYPE,
//            SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI_v1, seq_t, seq, std::string, sid, vid_t,
//            vid, std::string, cname, uid_t, uid, uint64_t, ts, std::unique_ptr<transcoding_info>,
//            transcodingConfig); DECLARE_PACKET_8(PLbesTranscodingUpdateRes_v1,
//            SIGNALING_SERVER_TYPE, SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI_v1, seq_t, seq,
//            std::string, sid, uint32_t, vid, std::string, cname, uid_t, uid, uint64_t, server_ts,
//            uint32_t, code, std::string, reason);

DECLARE_PACKET_5(PPeerCommandMessage, SIGNALING_SERVER_TYPE, SIGNALING_PEER_COMMAND_URI, int32_t,
                 command, seq_t, seq, uint64_t, ts, uid_t, uid, uint32_t, response);

DECLARE_PACKABLE_8(PInjectStreamConfig, int32_t, width, int32_t, height, int32_t, videoGop, int32_t,
                   videoFramerate, int32_t, videoBitrate, int32_t, audioSampleRate, int32_t,
                   audioBitrate, int32_t, audioChannels);
DECLARE_PACKET_10(PLbesAddInjectStreamUrlReq, SIGNALING_SERVER_TYPE,
                  SIGNALING_LBES_ADD_INJECT_STREAM_URL_REQ_URI, seq_t, seq, std::string, command,
                  std::string, sid, vid_t, vid, std::string, cname, uid_t, uid, uint64_t, ts,
                  std::string, lbesToken, std::string, url, PInjectStreamConfig, transcodingConfig);
DECLARE_PACKET_11(PLbesAddInjectStreamUrlRes, SIGNALING_SERVER_TYPE,
                  SIGNALING_LBES_ADD_INJECT_STREAM_URL_RES_URI, seq_t, seq, std::string, command,
                  std::string, sid, uint32_t, vid, std::string, cname, uid_t, uid, uint64_t,
                  server_ts, std::string, url, uint32_t, code, std::string, reason, uid_t,
                  inject_uid);
DECLARE_PACKET_8(PLbesRemoveInjectStreamUrlReq, SIGNALING_SERVER_TYPE,
                 SIGNALING_LBES_REMOVE_INJECT_STREAM_URL_REQ_URI, seq_t, seq, std::string, command,
                 std::string, sid, vid_t, vid, std::string, cname, uid_t, uid, uint64_t, ts,
                 std::string, url);
DECLARE_PACKET_11(PLbesRemoveInjectStreamUrlRes, SIGNALING_SERVER_TYPE,
                  SIGNALING_LBES_REMOVE_INJECT_STREAM_URL_RES_URI, seq_t, seq, std::string, command,
                  std::string, sid, uint32_t, vid, std::string, cname, uid_t, uid, uint64_t,
                  server_ts, uint32_t, code, std::string, reason, std::string, url, uid_t,
                  inject_uid);
DECLARE_PACKET_11(PLbesStreamInjectedStatusRes, SIGNALING_SERVER_TYPE,
                  SIGNALING_LBES_STREAM_INJECTED_STATUS_RES_URI, seq_t, seq, std::string, command,
                  std::string, sid, uint32_t, vid, std::string, cname, uid_t, uid, uint64_t,
                  server_ts, std::string, url, uint32_t, code, std::string, reason, uid_t,
                  inject_uid);

//            DECLARE_PACKABLE_8(PTranscodingUserConfig_v2, uid_t, uid, int32_t, x, int32_t, y,
//            int32_t, width, int32_t, height, int32_t, zOrder, double, alpha, int32_t,
//            audioChannel); DECLARE_PACKABLE_13(PTranscodingConfig_v2, int32_t, width, int32_t,
//            height, int32_t, videoGop, int32_t, videoFramerate, int32_t, videoCodecProfile,
//            int32_t, videoBitrate, bool, lowLatency, int32_t, audioSampleRate, int32_t,
//            audioBitrate, int32_t, audioChannels, uint32_t, backgroundColor, std::string,
//            userConfigExtraInfo, std::vector<PTranscodingUserConfig_v2>, userConfigs);
//            DECLARE_PACKET_9(PLbesPublishReq_v2, SIGNALING_SERVER_TYPE,
//            SIGNALING_LBES_PUBLISH_REQ_URI_v2, seq_t, seq, std::string, sid, vid_t, vid,
//            std::string, cname, uid_t, uid, uint64_t, ts, std::string, lbesToken, std::string,
//            url, std::unique_ptr<PTranscodingConfig_v2>, transcodingConfig);
//            DECLARE_PACKET_9(PLbesPublishRes_v2, SIGNALING_SERVER_TYPE,
//            SIGNALING_LBES_PUBLISH_RES_URI_v2, seq_t, seq, std::string, sid, uint32_t, vid,
//            std::string, cname,uid_t, uid, uint64_t, server_ts, std::string, url, uint32_t, code,
//            std::string, reason); DECLARE_PACKET_7(PLbesTranscodingUpdateReq_v2,
//            SIGNALING_SERVER_TYPE, SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI_v2, seq_t, seq,
//            std::string, sid, vid_t, vid, std::string, cname, uid_t, uid, uint64_t, ts,
//            std::unique_ptr<PTranscodingConfig_v2>, transcodingConfig);
//            DECLARE_PACKET_8(PLbesTranscodingUpdateRes_v2, SIGNALING_SERVER_TYPE,
//            SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI_v2, seq_t, seq, std::string, sid, uint32_t,
//            vid, std::string, cname, uid_t, uid, uint64_t, server_ts, uint32_t, code, std::string,
//            reason);

DECLARE_PACKABLE_7(PLbesImage, std::string, url, int, x, int, y, int, width, int, height, int,
                   zOrder, double, alpha);
DECLARE_PACKABLE_8(PTranscodingUserConfig_v3, uid_t, uid, int32_t, x, int32_t, y, int32_t, width,
                   int32_t, height, int32_t, zOrder, double, alpha, int32_t, audioChannel);
DECLARE_PACKABLE_16(PTranscodingConfig_v4, int32_t, width, int32_t, height, int32_t, videoGop,
                    int32_t, videoFramerate, int32_t, videoCodecProfile, int32_t, videoBitrate,
                    bool, lowLatency, int32_t, audioSampleRate, int32_t, audioBitrate, int32_t,
                    audioChannels, int32_t, audioCodecProfile, uint32_t, backgroundColor,
                    std::string, userConfigExtraInfo, std::string, metadata,
                    std::vector<PTranscodingUserConfig_v3>, userConfigs, std::vector<PLbesImage>,
                    images);
DECLARE_PACKET_11(PLbesPublishReq_v4, SIGNALING_SERVER_TYPE, SIGNALING_LBES_PUBLISH_REQ_URI_v4,
                  seq_t, seq, std::string, command, std::string, sid, vid_t, vid, std::string,
                  cname, uid_t, uid, uint64_t, ts, std::string, lbesToken, std::string, sdkVersion,
                  std::string, url, std::unique_ptr<PTranscodingConfig_v4>, transcodingConfig);

DECLARE_PACKET_10(PLbesPublishRes_v4, SIGNALING_SERVER_TYPE, SIGNALING_LBES_PUBLISH_RES_URI_v4,
                  seq_t, seq, std::string, command, std::string, sid, uint32_t, vid, std::string,
                  cname, uid_t, uid, uint64_t, server_ts, std::string, url, uint32_t, code,
                  std::string, reason);

DECLARE_PACKET_8(PLbesTranscodingUpdateReq_v4, SIGNALING_SERVER_TYPE,
                 SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI_v4, seq_t, seq, std::string, command,
                 std::string, sid, vid_t, vid, std::string, cname, uid_t, uid, uint64_t, ts,
                 std::unique_ptr<PTranscodingConfig_v4>, transcodingConfig);
DECLARE_PACKET_9(PLbesTranscodingUpdateRes_v4, SIGNALING_SERVER_TYPE,
                 SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI_v4, seq_t, seq, std::string, command,
                 std::string, sid, uint32_t, vid, std::string, cname, uid_t, uid, uint64_t,
                 server_ts, uint32_t, code, std::string, reason);

//            DECLARE_PACKET_2(PSignalingClientMessage, SIGNALING_SERVER_TYPE,
//            SIGNALING_CLIENT_MESSAGE_URI, std::string, sigAccount, std::string, payload);

DECLARE_PACKABLE_8(account_req_detail, std::string, command, std::string, sid, std::string, appId,
                   std::string, cname, std::string, uid, int32_t, seq, std::string, version,
                   int64_t, ts);
//            DECLARE_PACKABLE_3(server_account, std::string, streamAccount, std::string,
//            streamServerAddress, int32_t, port); DECLARE_PACKABLE_6(account_res_detail, int32_t,
//            code, std::string, sid, std::string, reason, std::string, streamToken, uint32_t, vid,
//            std::vector<server_account>, servers);

DECLARE_PACKET_1(PSignalingClientMessage2, SIGNALING_SERVER_TYPE, SIGNALING_CLIENT_MESSAGE2_URI,
                 std::string, payload);

typedef PLbesPublishReq_v4 PLbesPublishReq;
typedef PLbesPublishRes_v4 PLbesPublishRes;

typedef PTranscodingUserConfig_v3 PTranscodingUserConfig;
typedef PTranscodingConfig_v4 PTranscodingConfig;
typedef PLbesTranscodingUpdateReq_v4 PLbesTranscodingUpdateReq;
typedef PLbesTranscodingUpdateRes_v4 PLbesTranscodingUpdateRes;

#define SIGNALING_LBES_PUBLISH_REQ_URI SIGNALING_LBES_PUBLISH_REQ_URI_v4
#define SIGNALING_LBES_PUBLISH_RES_URI SIGNALING_LBES_PUBLISH_RES_URI_v4
#define SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI SIGNALING_LBES_TRANSCODING_UPDATE_REQ_URI_v4
#define SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI SIGNALING_LBES_TRANSCODING_UPDATE_RES_URI_v4
}  // namespace protocol
}  // namespace rtc
}  // namespace agora
#endif /* signaling_protocol_h */
