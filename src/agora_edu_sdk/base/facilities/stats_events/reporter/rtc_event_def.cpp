//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//
#include "rtc_event_def.h"

#include "facilities/argus/protobuf-c/gen-cpp/counter.pb-c.h"
#include "facilities/argus/protobuf-c/gen-cpp/message.pb-c.h"
#include "facilities/argus/protobuf-c/gen-cpp/rtmsdk_report_items.pb-c.h"
#include "facilities/argus/protobuf-c/gen-cpp/vosdk_report_items.pb-c.h"
#include "facilities/argus/protobuf-c/protobuf_wrapper.h"
#include "utils/tools/sysinfo.h"

namespace agora {
namespace rtc {

#define PACK_HEADER()                      \
  event.setString("sid", this->sid);       \
  event.setString("ip", this->ip);         \
  event.instance()->lts = this->lts;       \
  event.instance()->elapse = this->elapse; \
  event.setString("cname", this->cname);   \
  event.instance()->cid = this->cid;       \
  event.instance()->uid = this->uid;

#define EVENT_PACK_PRE_ACTION_WITHOUT_HEADER(MsgName, MsgVarName)   \
  base::ProtobufInstance<PROTOBUF_ADD_PREFIX_VOSDK(MsgName)> event( \
      PROTOBUF_FUNCTION_VOSDK(MsgVarName));

#define EVENT_PACK_PRE_ACTION(MsgName, MsgVarName)          \
  EVENT_PACK_PRE_ACTION_WITHOUT_HEADER(MsgName, MsgVarName) \
  PACK_HEADER()

#define EVENT_PACK_POST_ACTION()                                                      \
  std::string packedContent;                                                          \
  event.PackInstance(packedContent);                                                  \
  base::ProtobufInstance<PROTOBUF_ADD_PREFIX_MESSAGE(Message)> msg(                   \
      PROTOBUF_FUNCTION_MESSAGE(message));                                            \
  msg.instance()->id = static_cast<int32_t>(this->id);                                \
  base::ProtobufWrapper::TransformStringToBytes(&msg.instance()->msg, packedContent); \
  msg.PackInstance(packedContent);                                                    \
  return packedContent;

std::string CustomReportEvent::pack() {
  base::ProtobufInstance<PROTOBUF_ADD_PREFIX_VOSDK(UserAnalytics)> event(
      PROTOBUF_FUNCTION_VOSDK(user_analytics));

  event.setString("sid", this->sid);
  event.instance()->lts = this->lts;
  event.setString("reportid", this->reportid);
  event.setString("category", this->category);
  event.setString("event", this->event);
  event.setString("label", this->label);
  event.instance()->value = this->value;

  EVENT_PACK_POST_ACTION();
}

std::string JoinChannelEvent::pack() {
  EVENT_PACK_PRE_ACTION(Session, session);

  event.setString("vk", this->vk);
  event.setString("verextrainfo", this->verExtraInfo);
  event.setString("ver", this->ver);
  event.instance()->sdkbuildnumber = this->sdkBuildNumber;
  event.instance()->chebuildnumber = this->cheBuildNumber;
  event.instance()->net1 = this->net1;
  event.instance()->netsubtype = this->netSubType;
  event.instance()->rssi = this->rssi;
  event.instance()->siglevel = this->siglevel;
  event.instance()->os1 = this->os1;
  event.setString("did", this->did);
  event.instance()->pnq = this->pnq;
  event.instance()->lost = this->lost;
  event.instance()->jitter = this->jitter;
  event.setString("info", this->info);
  event.instance()->channelmode = this->channelMode;
  event.instance()->clienttype = this->clientType;
  event.instance()->clientrole = this->clientRole;
  event.instance()->channelprofile = this->channelProfile;
  event.setString("lsid", this->lsid);
  event.setString("fsid", this->fsid);
  event.setString("installid", this->installId);
  event.setString("configserviceversion", this->configServiceVersion);

  EVENT_PACK_POST_ACTION();
}

std::string QuitEvent::pack() {
  EVENT_PACK_PRE_ACTION(Quit, quit);

  event.instance()->dnsparsedtime = this->dnsParsedTime;

  EVENT_PACK_POST_ACTION();
}

std::string VocsEvent::pack() {
  EVENT_PACK_PRE_ACTION(Vocs, vocs);

  event.instance()->sc = this->sc;
  event.instance()->elapse = this->elapse;
  event.instance()->responsetime = this->responseTime;
  event.instance()->ec = this->ec;
  event.instance()->success = this->success;
  event.instance()->firstsuccess = this->firstSuccess;
  event.setString("serverIp", this->serverIp);
  std::vector<std::string>* server_ip_list_string = event.getStringList("serverIpList");
  if (server_ip_list_string) {
    for (const auto& ip : this->serverIpList) {
      server_ip_list_string->push_back(ip);
    }
  }
  event.setString("ispName", this->ispName);
  event.instance()->minorisp = this->minorIsp;

  EVENT_PACK_POST_ACTION();
}

std::string VosEvent::pack() {
  EVENT_PACK_PRE_ACTION(Vos, vos);

  event.instance()->responsetime = this->elapse;
  event.instance()->ec = this->ec;
  event.instance()->success = this->success;
  event.instance()->firstsuccess = true;
  event.instance()->sc = this->sc;
  event.setString("serverip", this->serverIp);
  event.setString("ackedloginserverip", this->ackedLoginServerIp);
  event.instance()->channelcount = this->channelCount;
  event.setString("wanIp", this->wanIp);

  EVENT_PACK_POST_ACTION();
}

std::string FirstAudioPacketSentEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstAudioPacketSent, first_audio_packet_sent);

  event.instance()->codec = this->codec;

  EVENT_PACK_POST_ACTION();
}

std::string FirstAudioPacketReceivedEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstAudioPacketReceived, first_audio_packet_received);

  event.instance()->codec = this->codec;

  EVENT_PACK_POST_ACTION();
}

std::string AudioSendingStoppedEvent::pack() {
  EVENT_PACK_PRE_ACTION(AudioSendingStopped, audio_sending_stopped);

  event.setString("reason", this->reason);

  EVENT_PACK_POST_ACTION();
}

std::string AudioDisabledEvent::pack() {
  EVENT_PACK_PRE_ACTION(AudioDisabled, audio_disabled);

  EVENT_PACK_POST_ACTION();
}

std::string AudioEnabledEvent::pack() {
  EVENT_PACK_PRE_ACTION(AudioEnabled, audio_enabled);

  EVENT_PACK_POST_ACTION();
}

std::string FirstVideoPacketSentEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstVideoPacketSent, first_video_packet_sent);

  event.instance()->codec = this->codec;

  EVENT_PACK_POST_ACTION();
}

std::string FirstVideoPacketReceivedEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstVideoPacketReceived, first_video_packet_received);

  event.instance()->peer = this->peer;
  event.instance()->codec = this->codec;

  EVENT_PACK_POST_ACTION();
}

std::string VideoSendingStoppedEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoSendingStopped, video_sending_stopped);

  event.setString("reason", this->reason);

  EVENT_PACK_POST_ACTION();
}

std::string VideoDisabledEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoDisabled, video_disabled);

  EVENT_PACK_POST_ACTION();
}

std::string VideoEnabledEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoEnabled, video_enabled);

  EVENT_PACK_POST_ACTION();
}

std::string PeerOnlineStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(PeerOnlineStatus, peer_online_status);

  EVENT_PACK_POST_ACTION();
}

std::string PeerOfflineStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(PeerOfflineStatus, peer_offline_status);

  event.setString("reason", this->reason);

  EVENT_PACK_POST_ACTION();
}

std::string AudioMutePeerStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(AudioMutePeerStatus, audio_mute_peer_status);

  event.instance()->muted = this->muted;

  EVENT_PACK_POST_ACTION();
}

std::string VideoMutePeerStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoMutePeerStatus, video_mute_peer_status);

  event.instance()->muted = this->muted;

  EVENT_PACK_POST_ACTION();
}

std::string AudioMuteAllStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(AudioMuteAllStatus, audio_mute_all_status);

  event.instance()->muted = this->muted;

  EVENT_PACK_POST_ACTION();
}

std::string VideoMuteAllStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoMuteAllStatus, video_mute_all_status);

  event.instance()->muted = this->muted;

  EVENT_PACK_POST_ACTION();
}

std::string RenewTokenEvent::pack() {
  EVENT_PACK_PRE_ACTION(RenewToken, renew_token);

  event.setString("token", this->token);

  EVENT_PACK_POST_ACTION();
}

std::string RenewTokenResEvent::pack() {
  EVENT_PACK_PRE_ACTION(RenewTokenRes, renew_token_res);

  event.instance()->res_code = this->res_code;

  EVENT_PACK_POST_ACTION();
}

std::string P2pStunLoginSuccessEvent::pack() {
  EVENT_PACK_PRE_ACTION(P2pStunLoginSuccess, p2p_stun_login_success);

  event.setString("serverip", this->serverIp);
  event.instance()->vid = this->vid;

  EVENT_PACK_POST_ACTION();
}

std::string P2pStunLoginFailedEvent::pack() {
  EVENT_PACK_PRE_ACTION(P2pStunLoginFailed, p2p_stun_login_failed);

  event.setString("serverip", this->serverIp);
  event.instance()->vid = this->vid;
  event.instance()->code = this->code;

  EVENT_PACK_POST_ACTION();
}

std::string P2pPeerTryTouchEvent::pack() {
  EVENT_PACK_PRE_ACTION(P2pPeerTryTouch, p2p_peer_try_touch);

  event.setString("peerlanip", this->peerLanIp);
  event.setString("peerwanip", this->peerWanIp);

  EVENT_PACK_POST_ACTION();
}

std::string P2pPeerConnectedEvent::pack() {
  EVENT_PACK_PRE_ACTION(P2pPeerConnected, p2p_peer_connected);

  event.setString("peerip", this->peerIp);

  EVENT_PACK_POST_ACTION();
}

std::string P2pPeerDisconnectedEvent::pack() {
  EVENT_PACK_PRE_ACTION(P2pPeerDisconnected, p2p_peer_disconnected);

  event.setString("reason", this->reason);

  EVENT_PACK_POST_ACTION();
}

std::string P2pStartEvent::pack() {
  EVENT_PACK_PRE_ACTION(P2pStart, p2p_start);

  event.instance()->threshold = this->threshold;
  event.setString("label", this->label);

  EVENT_PACK_POST_ACTION();
}

std::string P2pStopEvent::pack() {
  EVENT_PACK_PRE_ACTION(P2pStop, p2p_stop);

  event.setString("reason", this->reason);

  EVENT_PACK_POST_ACTION();
}

std::string ViLocalFrameEvent::pack() {
  EVENT_PACK_PRE_ACTION(ViLocalFrame, vi_local_frame);

  event.instance()->height = this->height;
  event.instance()->width = this->width;

  EVENT_PACK_POST_ACTION();
}

std::string ViRemoteFrameEvent::pack() {
  EVENT_PACK_PRE_ACTION(ViRemoteFrame, vi_remote_frame);

  event.instance()->peeruid = this->uid;
  event.instance()->width = this->width;
  event.instance()->height = this->height;

  EVENT_PACK_POST_ACTION();
}

std::string RatingEvent::pack() {
  EVENT_PACK_PRE_ACTION(Rating, rating);

  event.setString("vk", this->vk);
  event.instance()->rating = this->rating;
  event.setString("description", this->description);

  EVENT_PACK_POST_ACTION();
}

std::string ACodecEvent::pack() {
  EVENT_PACK_PRE_ACTION(ACodec, acodec);

  event.setString("codec", this->codec);
  event.instance()->frames = this->frames;
  event.instance()->interleaves = this->interleaves;

  EVENT_PACK_POST_ACTION();
}

std::string PeerEvent::pack() {
  EVENT_PACK_PRE_ACTION(Peer, peer);

  event.instance()->peeruid = this->peerUid;

  EVENT_PACK_POST_ACTION();
}

std::string VideoBandwidthAggressiveLevelEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoBandwidthAggressiveLevel, video_bandwidth_aggressive_level);

  event.instance()->level = this->level;

  EVENT_PACK_POST_ACTION();
}

std::string AppSetMinPlayoutDelayEvent::pack() {
  EVENT_PACK_PRE_ACTION(AppSetMinPlayoutDelay, app_set_min_playout_delay);

  event.instance()->playoutdelay = this->playoutDelay;

  EVENT_PACK_POST_ACTION();
}

std::string AppSetVideoStartBitRateEvent::pack() {
  EVENT_PACK_PRE_ACTION(AppSetVideoStartBitRate, app_set_video_start_bit_rate);

  event.instance()->startvideobitrate = this->startVideoBitRate;

  EVENT_PACK_POST_ACTION();
}

std::string SendVideoPacedEvent::pack() {
  EVENT_PACK_PRE_ACTION(SendVideoPaced, send_video_paced);

  event.instance()->isenabled = this->isEnabled;

  EVENT_PACK_POST_ACTION();
}

std::string ABTestEvent::pack() {
  EVENT_PACK_PRE_ACTION(ABTest, abtest);

  event.setString("feature", this->feature);
  event.setString("tag", this->tag);
  event.setString("params", this->params);

  EVENT_PACK_POST_ACTION();
}

std::string VideoInitialOptionsEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoInitialOptions, video_initial_options);

  event.instance()->issendvideopacedenabled = this->isSendVideoPacedEnabled;
  event.instance()->isvideofecenabled = this->isVideoFecEnabled;
  event.instance()->videofecmethod = this->videoFecMethod;
  event.instance()->localfallbackoption = this->localFallbackOption;
  event.instance()->remotefallbackoption = this->remoteFallbackOption;

  EVENT_PACK_POST_ACTION();
}

std::string VqcStatEvent::pack() {
  EVENT_PACK_PRE_ACTION(VqcStat, vqc_stat);

  event.instance()->totalframes = this->totalFrames;
  event.instance()->averagescore = this->averageScore;
  event.instance()->llratio = this->llRatio;
  event.instance()->hhratio = this->hhRatio;

  EVENT_PACK_POST_ACTION();
}

std::string NetworkInformationEvent::pack() {
  EVENT_PACK_PRE_ACTION(NetworkInformation, network_information);

  event.instance()->networktype = this->networkType;
  event.instance()->networksubtype = this->networkSubType;
  event.instance()->rssi = this->rssi;
  event.instance()->siglevel = this->siglevel;

  EVENT_PACK_POST_ACTION();
}

std::string SwitchVideoStreamEvent::pack() {
  EVENT_PACK_PRE_ACTION(SwitchVideoStream, switch_video_stream);

  event.instance()->eventtype = this->eventType;
  event.instance()->expectedstream = this->expectedStream;
  event.instance()->requestid = this->requestId;
  event.instance()->begints = this->beginTs;
  event.instance()->endts = this->endTs;

  EVENT_PACK_POST_ACTION();
}

std::string DeviceStatChangeEvent::pack() {
  EVENT_PACK_PRE_ACTION(DeviceStatChange, device_stat_change);

  event.instance()->statetype = this->StateType;
  event.instance()->devicetype = this->deviceType;
  event.setString("devicename", this->deviceName);
  event.setString("deviceid", this->deviceId);

  EVENT_PACK_POST_ACTION();
}

std::string MaxVideoPayloadSetEvent::pack() {
  EVENT_PACK_PRE_ACTION(MaxVideoPayloadSet, max_video_payload_set);

  event.instance()->maxpayload = this->maxPayload;

  EVENT_PACK_POST_ACTION();
}

std::string FirstVideoFrameDecodedEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstVideoFrameDecoded, first_video_frame_decoded);

  event.instance()->width = this->width;
  event.instance()->height = this->height;
  event.instance()->peer = this->peer;

  EVENT_PACK_POST_ACTION();
}

std::string FirstVideoFrameDrawedEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstVideoFrameDrawed, first_video_frame_drawed);

  event.instance()->width = this->width;
  event.instance()->height = this->height;
  event.instance()->peer = this->peer;

  EVENT_PACK_POST_ACTION();
}

std::string VideoStreamSelectedEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoStreamSelected, video_stream_selected);

  event.instance()->streamtype = this->streamType;

  EVENT_PACK_POST_ACTION();
}

std::string VideoStreamChangeRequestEvent::pack() {
  EVENT_PACK_PRE_ACTION(VideoStreamChangeRequest, video_stream_change_request);

  event.instance()->streamtype = this->streamType;

  EVENT_PACK_POST_ACTION();
}

std::string FirstDataPacketSentEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstDataPacketSent, first_data_packet_sent);

  event.instance()->transporttype = this->transportType;

  EVENT_PACK_POST_ACTION();
}

std::string FirstDataPacketReceivedEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstDataPacketReceived, first_data_packet_received);

  event.instance()->transporttype = this->transportType;

  EVENT_PACK_POST_ACTION();
}

std::string ErrorEvent::pack() {
  EVENT_PACK_PRE_ACTION(Error, error);

  event.instance()->errorno = this->errorNo;
  event.setString("description", this->description);

  EVENT_PACK_POST_ACTION();
}

std::string DefaultPeerStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(DefaultPeerStatus, default_peer_status);

  event.instance()->streamtype = streamType;

  EVENT_PACK_POST_ACTION();
}

std::string APEventEvent::pack() {
  EVENT_PACK_PRE_ACTION(APEvent, apevent);

  event.instance()->sc = this->sc;
  event.instance()->ec = this->ec;
  event.instance()->success = this->success;
  event.instance()->firstsuccess = this->firstSuccess;

  std::vector<std::string>* serveriplist_vector = event.getStringList("serveriplist");
  event.setString("serverip", this->serverIp);
  if (serveriplist_vector) {
    for (auto& ip : serverIpList) {
      serveriplist_vector->push_back(ip);
    }
  }

  event.instance()->flag = this->flag;
  event.instance()->connecttype = this->connectType;

  EVENT_PACK_POST_ACTION();
}

std::string ReportStatsEvent::pack() {
  EVENT_PACK_PRE_ACTION(ReportStats, report_stats);

  event.instance()->alltotaltxpackets = this->allTotalTxPackets;
  event.instance()->alltotalackedpackets = this->allTotalAckedPackets;
  event.instance()->allvalidtxpackets = this->allValidTxPackets;
  event.instance()->allvalidackedpackets = this->allValidAckedPackets;
  event.instance()->countertotaltxpackets = this->counterTotalTxPackets;
  event.instance()->countertotalackedpackets = this->counterTotalAckedPackets;
  event.instance()->countervalidtxpackets = this->counterValidTxPackets;
  event.instance()->countervalidackedpackets = this->counterValidAckedPackets;
  event.instance()->eventtotaltxpackets = this->eventTotalTxPackets;
  event.instance()->eventtotalackedpackets = this->eventTotalAckedPackets;
  event.instance()->eventvalidtxpackets = this->eventValidTxPackets;
  event.instance()->eventvalidackedpackets = this->eventValidAckedPackets;

  EVENT_PACK_POST_ACTION();
}

std::string PPrivilegeWillExpireEvent::pack() {
  EVENT_PACK_PRE_ACTION(PPrivilegeWillExpire, pprivilege_will_expire);

  event.setString("token", this->token);
  std::list<base::ProtobufInstance<PROTOBUF_ADD_PREFIX_VOSDK(PrivilegeExpireInfo)>> item_list;
  for (auto& it : privilegeExpireInfos) {
    item_list.emplace_back(PROTOBUF_FUNCTION_VOSDK(privilege_expire_info));
    auto& privilege_item = item_list.back();
    privilege_item.instance()->privilege = it.privilege;
    privilege_item.instance()->remainingtime = it.remainingTime;
    privilege_item.instance()->expirets = it.expireTs;
    privilege_item.PrePack();
  }
  int size = item_list.size();
  if (size > 0) {
    event.instance()->n_privilegeexpireinfos = size;
    event.instance()->privilegeexpireinfos =
        (PROTOBUF_ADD_PREFIX_VOSDK(PrivilegeExpireInfo)**)malloc(
            sizeof(PROTOBUF_ADD_PREFIX_VOSDK(PrivilegeExpireInfo)*) * size);

    auto iter = item_list.begin();
    for (int i = 0; i < size && iter != item_list.end(); ++i, ++iter) {
      event.instance()->privilegeexpireinfos[i] = iter->release();
    }
  }

  EVENT_PACK_POST_ACTION();
}

std::string LocalFallbackStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(LocalFallbackStatus, local_fallback_status);

  event.instance()->status = this->status;

  EVENT_PACK_POST_ACTION();
}

std::string RemoteFallbackStatusEvent::pack() {
  EVENT_PACK_PRE_ACTION(RemoteFallbackStatus, remote_fallback_status);

  event.instance()->src = this->src;
  event.instance()->dst = this->dst;

  EVENT_PACK_POST_ACTION();
}

std::string WorkerEventEvent::pack() {
  EVENT_PACK_PRE_ACTION(WorkerEvent, worker_event);

  event.setString("command", this->command);
  event.setString("action_type", this->actionType);
  event.instance()->workertype = this->workerType;
  event.setString("url", this->url);
  event.setString("payload", this->payload);
  event.instance()->server_code = this->server_code;
  event.instance()->code = this->code;
  event.instance()->elapse = this->elapse;

  EVENT_PACK_POST_ACTION();
}

std::string APWorkerEventEvent::pack() {
  EVENT_PACK_PRE_ACTION(APWorkerEvent, apworker_event);

  event.instance()->sc = this->sc;
  event.instance()->responsetime = this->responseTime;
  event.instance()->ec = this->ec;
  event.instance()->success = this->success;
  event.instance()->firstsuccess = true;
  event.setString("serverip", this->serverIp);
  event.setString("servicename", this->serviceName);
  event.setString("response_detail", this->response_detail);

  EVENT_PACK_POST_ACTION();
}

std::string FirstJoinVosSuccessEvent::pack() {
  EVENT_PACK_PRE_ACTION(FirstJoinVosSuccess, first_join_vos_success);

  event.setString("serverip", this->serverIp);
  event.instance()->responsetime = this->responseTime;
  event.setString("ackedloginserverip", this->ackedLoginServerIp);
  event.setString("wanIp", this->wanIp);
  event.setString("configserviceversion", this->configServiceVersion);
  event.instance()->configelapsed = this->configElapsed;
  event.instance()->isabtestsuccess = this->isABTestSuccess;

  EVENT_PACK_POST_ACTION();
}

std::string TrackSpanEvent::pack() {
  EVENT_PACK_PRE_ACTION_WITHOUT_HEADER(TrackSpan, track_span);

  event.setString("traceid", this->traceId);
  event.setString("id", this->strId);
  event.setString("spanname", this->spanName);

  int32_t index = 0;
  event.instance()->n_annotations = this->annotations.size();
  for (auto& annotation : this->annotations) {
    base::ProtobufInstance<PROTOBUF_ADD_PREFIX_VOSDK(Annotation)> annot(
        PROTOBUF_FUNCTION_VOSDK(annotation));
    annot.instance()->timestamp = annotation.timestamp;
    annot.setString("value", annotation.value);

    base::ProtobufInstance<PROTOBUF_ADD_PREFIX_VOSDK(Endpoint)> endpoint(
        PROTOBUF_FUNCTION_VOSDK(endpoint));
    endpoint.instance()->ipv4 = annotation.endpoint.ipv4;
    endpoint.instance()->port = annotation.endpoint.port;
    endpoint.setString("servicename", annotation.endpoint.serviceName);
    endpoint.PrePack();
    annot.instance()->endpoint = endpoint.release();
    annot.PrePack();

    event.instance()->annotations = (PROTOBUF_ADD_PREFIX_VOSDK(Annotation)**)malloc(
        sizeof(PROTOBUF_ADD_PREFIX_VOSDK(Annotation)*));
    event.instance()->annotations[index++] = annot.release();
  }

  EVENT_PACK_POST_ACTION();
}

#define RTM_EVENT_PACK_PRE_ACTION(MsgName, MsgVarName)                         \
  base::ProtobufInstance<PROTOBUF_ADD_PREFIX_RTMSDK(MsgName)> event(           \
      PROTOBUF_FUNCTION_RTMSDK(MsgVarName));                                   \
  event.setString("sid", this->sid);                                           \
  if (!this->userId.empty()) {                                                 \
    event.setString("userid", this->userId);                                   \
  }                                                                            \
  event.instance()->lts = this->lts;                                           \
  event.instance()->elapse = this->elapse;                                     \
  base::ProtobufInstance<PROTOBUF_ADD_PREFIX_RTMSDK(CommonIndex)> commonIndex( \
      PROTOBUF_FUNCTION_RTMSDK(common_index));                                 \
  commonIndex.setString("index1", this->index1);                               \
  commonIndex.PrePack();                                                       \
  event.instance()->index = commonIndex.release();

std::string RtmSessionEvent::pack() {
  RTM_EVENT_PACK_PRE_ACTION(Session, session);

  event.setString("appid", this->appId);
  event.setString("ver", this->ver);

  event.instance()->buildno = this->buildno;
  event.instance()->os = this->os;

  if (!this->token.empty()) {
    event.setString("token", this->token);
  }

  EVENT_PACK_POST_ACTION();
}

std::string RtmLogoutEvent::pack() {
  RTM_EVENT_PACK_PRE_ACTION(Logout, logout);

  EVENT_PACK_POST_ACTION();
}

std::string RtmApEvent::pack() {
  RTM_EVENT_PACK_PRE_ACTION(ApEvent, ap_event);

  if (!this->apAddr.empty()) {
    event.setString("ap.addr", this->apAddr);
  }

  if (!this->linkServerList.empty()) {
    std::vector<std::string>* lst = event.getStringList("linkserverlist");
    for (const auto& addr : this->linkServerList) {
      lst->push_back(addr);
    }
  }

  if (!this->localWanIp.empty()) {
    event.setString("localwanip", this->localWanIp);
  }

  event.instance()->errcode = this->errCode;
  event.instance()->servererrcode = this->serverErrCode;
  event.instance()->opid = this->opId;
  event.instance()->flag = this->flag;
  if (!this->isp.empty()) {
    event.setString("isp", this->isp);
  }

  EVENT_PACK_POST_ACTION();
}

std::string RtmLinkEvent::pack() {
  RTM_EVENT_PACK_PRE_ACTION(Link, link);

  event.instance()->ec = this->ec;
  event.instance()->sc = this->sc;
  event.setString("destserverip", this->destServerIp);
  event.setString("ackedserverip", this->ackedServerIp);
  event.instance()->responsetime = this->responseTime;

  EVENT_PACK_POST_ACTION();
}

std::string RtmRxMessageEvent::pack() {
  RTM_EVENT_PACK_PRE_ACTION(RxMessage, rx_message);

  event.instance()->insid = this->insId;
  event.instance()->dialid = this->dialId;
  event.instance()->seq = this->seq;
  event.instance()->dsttype = this->dstType;
  event.instance()->messageid = this->messageId;
  event.setString("srcid", this->srcId);
  event.setString("dstid", this->dstId);
  event.setString("payload", this->payload);

  EVENT_PACK_POST_ACTION();
}

std::string RtmTxMessageEvent::pack() {
  RTM_EVENT_PACK_PRE_ACTION(TxMessage, tx_message);

  event.instance()->insid = this->insId;
  event.instance()->dialid = this->dialId;
  event.instance()->seq = this->seq;
  event.instance()->dsttype = this->dstType;
  event.instance()->messageid = this->messageId;
  event.setString("srcid", this->srcId);
  event.setString("dstid", this->dstId);
  event.setString("payload", this->payload);

  EVENT_PACK_POST_ACTION();
}

std::string RtmTxMessageResEvent::pack() {
  RTM_EVENT_PACK_PRE_ACTION(TxMessageRes, tx_message_res);

  EVENT_PACK_POST_ACTION();
}

std::string JoinChannelTimeoutEvent::pack() {
  EVENT_PACK_PRE_ACTION(JoinChannelTimeout, join_channel_timeout);

  event.instance()->timeout = this->timeout;
  EVENT_PACK_POST_ACTION();
}

#define FILL_FIRST_FRAME_DECODED_INFO()                              \
  event.instance()->uid = this->uid;                                 \
  event.instance()->availablepublish = this->availablePublish;       \
  event.instance()->peerpublishduration = this->peerPublishDuration; \
  event.instance()->joinchannelsuccesselapse = this->joinChannelSuccessElapse

std::string PeerFirstVideoFrameDecodedEvent::pack() {
  EVENT_PACK_PRE_ACTION(PeerFirstVideoFrameDecoded, peer_first_video_frame_decoded);

  FILL_FIRST_FRAME_DECODED_INFO();

  event.instance()->firstdrawnelapse = this->firstDrawnElapse;

  EVENT_PACK_POST_ACTION();
}

std::string PeerFirstAudioFrameDecodedEvent::pack() {
  EVENT_PACK_PRE_ACTION(PeerFirstAudioFrameDecoded, peer_first_audio_frame_decoded);

  FILL_FIRST_FRAME_DECODED_INFO();

  event.instance()->firstdrawnelapse = this->firstDrawnElapse;

  EVENT_PACK_POST_ACTION();
}

std::string PeerFirstVideoFrameDecodedTimeoutEvent::pack() {
  EVENT_PACK_PRE_ACTION(PeerFirstVideoFrameDecodedTimeout, peer_first_video_frame_decoded_timeout);

  FILL_FIRST_FRAME_DECODED_INFO();

  event.instance()->timeout = this->timeout;

  EVENT_PACK_POST_ACTION();
}

std::string PeerFirstAudioFrameDecodedTimeoutEvent::pack() {
  EVENT_PACK_PRE_ACTION(PeerFirstAudioFrameDecodedTimeout, peer_first_audio_frame_decoded_timeout);

  FILL_FIRST_FRAME_DECODED_INFO();

  event.instance()->timeout = this->timeout;

  EVENT_PACK_POST_ACTION();
}

std::string CrashEvent::pack() {
  EVENT_PACK_PRE_ACTION(CrashEvent, crash_event);

  event.instance()->crashver = 1;
  event.instance()->dmptype = 0;

  event.instance()->lstlts = info.crash_ctx.crashTs;
  event.instance()->lstcrashaddr = info.crash_ctx.crashAddr;
  event.instance()->lstldbegin = info.crash_ctx.loadAddrBegin;
  event.instance()->lstldend = info.crash_ctx.loadAddrEnd;
  event.instance()->isdumpfile = info.crash_ctx.isDumpFile;
  event.setString("lstcrashuid", info.crash_ctx.crashId);

  event.instance()->lstnetwork = info.call_ctx.channelInfo.networkType;
  event.instance()->lstclientrole = info.call_ctx.channelInfo.clientRole;
  event.instance()->lstbuildno = info.call_ctx.buildNo;
  event.setString("lstserviceid", info.call_ctx.serviceId);
  event.setString("lstsessionid", info.call_ctx.channelInfo.sessionId);
  event.setString("lstchannelname", info.call_ctx.channelInfo.channelName);
  event.setString("lstsdkver", info.call_ctx.sdkVersion);
  event.setString("deviceid", info.call_ctx.deviceId);
  event.setString("appid", info.call_ctx.appId);

  event.instance()->os = static_cast<int32_t>(utils::get_platform_type());
  event.instance()->cpuarch = static_cast<int32_t>(utils::get_cpu_arch_type());

  EVENT_PACK_POST_ACTION();
}

}  // namespace rtc
}  // namespace agora
