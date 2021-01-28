namespace cpp io.agora.argus

# uri range:
# encrypted item 900
# vosdk 1000 - 1999
# webrtcserver   2000 - 2999
# broadcast server 3000 - 3200
# vos server 4000 - 4999

// uri = 900
// 一个加密数据包，通过encType指定了加密所使用的算法
// payload 为经过该编码方式加密的payload
// key1, key2, key3, key4  为密钥， 通过密钥可以解码出payload的真实内容
// payload 为加密内容， 反解出该payload 得到的是另外一个uri<>900 的ReportItem的序列化后的值
struct EncryptedItem {
    1: optional i32 encType,
    2: optional binary key1,
    3: optional binary key2,
    4: optional binary key3,
    5: optional binary key4,
    10: optional binary payload
}

struct VosdkHeader {
  1: optional string sid,
  2: optional string cname,
  3: optional i64 cid,
  4: optional i64 lts,
  5: optional string ip,
  6: optional i64 uid,
  7: optional bool success,
  8: optional i64 elapse,
  9: optional i64 peer
}

struct VosdkCounterItem {
  1: optional string name,
  2: optional i64 lts,
  3: optional i32 value,
  4: optional i32 id,
  5: optional i32 tagErrorCode
}

//uri = 1000
struct VosdkCounter {
  1: optional VosdkHeader header,
  2: optional list<VosdkCounterItem> items
}

//uri = 1001
struct VosdkSession{
  1: optional VosdkHeader header,
  5: optional string vk,
  6: optional string ver,
  7: optional i32 net1,
  8: optional i32 net2,
  9: optional string localIp,
  10: optional string ssid,
  11: optional string bssid,
  12: optional i32 siglevel,
  13: optional i32 rssi,
  14: optional i32 os1,
  15: optional string did,
  16: optional i32 pnq,
  17: optional i32 lost,
  18: optional string info,
  19: optional string lsid,
  20: optional i32 channelMode,
  21: optional string cheVer,
  22: optional i32 sdkBuildNumber,
  23: optional i32 cheBuildNumber,
  24: optional string fsid,
  25: optional i32 channelProfile,
  26: optional i32 netSubType,
  27: optional i32 clientType, // 1: Normal; 2:PSTN, 3: Recording
  28: optional i32 appCategory,
  29: optional i32 clientRole,
  30: optional string installId,
  31: optional string stringUid,
  32: optional i32 jitter,
  33: optional string verExtraInfo,
  34: optional string cpuid,
  35: optional string configServiceVersion,
  36: optional string serviceId,
  37: optional i32 configElapsed,
  38: optional bool isABTestSuccess
}



//uri = 1002
struct VosdkVocs {
    1: optional VosdkHeader header,
    5: optional i32 ec,
    6: optional i32 sc,
    7: optional string serverIp,
    8: optional bool firstSuccess,
    9: optional i32 responseTime,
    10: optional list<string> serverIpList,
    11: optional string ssid,
    12: optional string bssid,
    13: optional string localWanIp,
    14: optional string ispName,
    15: optional bool minorIsp

}

//uri = 1003
struct VosdkVos {
    1: optional VosdkHeader header,
    5: optional i32 ec,
    6: optional i32 sc,
    7: optional string serverIp,
    8: optional list<string> vosList,
    9: optional bool firstSuccess,
    10: optional i32 channelCount,
    11: optional i32 responseTime,
    12: optional string ackedLoginServerIp
}

//uri = 1004
struct VosdkChan {
    1: optional VosdkHeader header,
    5: optional i32 chm
}

//uri = 1005
struct VosdkQuit {
    1: optional VosdkHeader header
    2: optional i32 dnsParsedTime
}

//uri = 1006
struct VosdkPeer {
    1: optional VosdkHeader header,
    2: optional i64 peerUid
}

//uri = 1007
struct VosdkViLocalFrame {
    1: optional VosdkHeader header,
    2: optional i32 height,
    3: optional i32 width
}

//uri = 1008
struct VosdkViRemoteFrame{
    1: optional VosdkHeader header,
    2: optional i64 peerUid,
    3: optional i32 height,
    4: optional i32 width,
    5: optional string codec
}

//uri = 1009
struct VosdkRating {
    1: optional VosdkHeader header,
    2: optional string vk,
    3: optional i32 rating,
    4: optional string description
}

//uri = 1010
struct VosdkACodec {
    1: optional VosdkHeader header,
    2: optional string codec,
    3: optional i32 frames,
    4: optional i32 interleaves
}

//uri = 1011
struct VosdkNetOb {
    1: optional VosdkHeader header,
    2: optional binary payload
}

//uri = 1026
struct VosdkDeviceStatChange {
    1: optional VosdkHeader header,
    2: optional i32 deviceType,
  	3: optional i32 StateType,
    4: optional string deviceId,
    5: optional string deviceName
}

struct VosdkCameraInfoItem {
  1: optional string friendName,
  2: optional string deviceId,
  3: optional bool bUse
}

//uri = 1030
struct VosdkCameraInfos {
  1: optional VosdkHeader header,
  2: optional list<VosdkCameraInfoItem> items
}

//WebRTC Browser to server
//uri = 2000
struct Vosb2s {
  1: optional i32 vid,
  2: optional i64 cid,
  3: optional i64 uid,
  4: optional i32 vos_id,
  5: optional i32 source_ip,
  6: optional i16 source_port,
  7: optional i32 dest_ip,
  8: optional i16 dest_port,
  9: optional i16 delay,
  10: optional i16 jitter100,
  11: optional i16 jitter95,
  12: optional i16 jitter90,
  13: optional i16 lost_ratio,
  14: optional i16 lost_ratio2,
  15: optional i16 lost_ratio3,
  16: optional i64 lts
}

//WebRTC Browser to Gateway
//uri = 2001
struct WrtcBrowser2Gateway {
  1: optional i32 vid,
  2: optional i64 cid,
  3: optional i64 uid,
  4: optional i32 vos_id,
  5: optional i32 source_ip,
  6: optional i16 source_port,
  7: optional i32 dest_ip,
  8: optional i16 dest_port,
  9: optional i16 delay,
  10: optional i16 jitter100,
  11: optional i16 jitter95,
  12: optional i16 jitter90,
  13: optional i16 lost_ratio,
  14: optional i16 lost_ratio2,
  15: optional i16 lost_ratio3,
  16: optional i64 lts
}

//WebRTC Browser Initial Profile
//uri = 2002
struct WrtcProfile {
  1: optional VosdkHeader header,
  2: optional i16 framerate,
  3: optional i32 bitrate,
  4: optional i32 width,
  5: optional i32 height
}

//WebRTC Browser Publish Stream to Gateway
//uri = 2003
struct WrtcPubStream {
 1: optional VosdkHeader header,
 2: optional bool audio,
 3: optional bool video
}

//WebRTC Browser Unpublish Stream
//uri = 2004
struct WrtcUnpubStream {
  1: optional VosdkHeader header
}

//WebRTC Browser Subscribe Stream
//uri = 2005
struct WrtcSubStream {
  1: optional VosdkHeader header,
  2: optional i64 peerUid
}

//WebRTC Browser Unsubscribe Stream
//uri = 2006
struct WrtcUnsubStream {
  1: optional VosdkHeader header,
  2: optional i64 peerUid
}

//uri = 1012,   the event of start stun, received stun request
struct VosdkP2PStartStun {
    1: optional VosdkHeader header,
    2: optional i64 peerUid,   // peer user id, generated on ice server
    3: optional i64 openTs,// open p2p ts
    4: optional i64 joinIceTs// join ice ts
}

//uri = 1013, the event of beginning to send data
struct VosdkP2PSendDataBeginning {
    1: optional VosdkHeader header,
    2: optional i64 peerUid,    // peer user id, generated on ice server
    3: optional i64 openTs,// open p2p ts
    4: optional i64 joinIceTs,// join ice ts
    5: optional i64 startStunTs,// start stun ts
    6: optional i64 succStunTs// succ stun ts
}

//uri = 1014,   the event of start stun, received stun request
struct VosdkP2PJoinIce {
    1: optional VosdkHeader header,
    2: optional i64 peerUid,    // peer user id, generated on ice server
    3: optional i64 openTs,     // open p2p ts
    4: optional map<string, string> externalAddresses,    // ice server address -> external address
    5: optional list<string> localAddress

}

//uri = 1015, the event of beginning to send data
struct VosdkP2PSuccStun{
    1: optional VosdkHeader header,
    2: optional i64 peerUid,    // peer user id, generated on ice server
    3: optional i64 openTs,// open p2p ts
    4: optional i64 joinIceTs,// join ice ts
    5: optional i64 startStunTs,// start stun ts
    6: optional i32 peerIP,// peer ip
    7: optional i16 peerPort,// peer port
    8: optional string peerAddr// peer ip:port
}

//uri = 1016, the event of beginning p2p
struct VosdkP2POpen{
    1: optional VosdkHeader header
}

//obsoleted
struct VosdkErrorCodeItem {
  1: i64 lts,
  2: i32 error
}

//obsoleted
//uri = 1017, SDK error codes
struct VosdkErrorCode{
    1: optional VosdkHeader header,
    2: optional list<VosdkErrorCodeItem> errorList
}

//uri = 1018, p2p switch state
struct VosdkP2PSwitch{
    1: optional VosdkHeader header,
    2: optional i16 status,//1 p2p start 0 p2p end
    3: optional i64 reportTs//
}

//uri = 1019, p2p report stun stat
struct VosdkP2PStunStat{
    1: optional VosdkHeader header,
    2: optional i16 code, //p2p stun code
    3: optional string stunId, //stun id
    4: optional string resource, //
    5: optional string fromId, //from id
    6: optional string toId, //to id
    7: optional string sourceIp, //from ip
    8: optional string destIp //to ip
}

//uri = 1020, network change information
struct VosdkNetworkInformation{
    1: optional VosdkHeader header,
    2: optional i32 networkType,
    3: optional i32 networkSubType,
    4: optional string localIp,
    5: optional string ssid,
    6: optional string bssid,
    7: optional i32 siglevel,
    8: optional i32 rssi
}

//uri = 1021
struct VosdkNetOb2 {
    1: optional VosdkHeader header,
    2: optional binary payload
}

//uri = 1022
struct VosdkNetOb3 {
    1: optional VosdkHeader header,
    2: optional list<byte> payload
}

//uri = 1023
struct VosdkNetOb4 {
    1: optional VosdkHeader header,
    2: optional list<byte> payload
}

//uri = 1024
struct VosdkViRemoteFrameDecoded{
    1: optional VosdkHeader header,
    2: optional i64 peerUid,
    3: optional i32 height,
    4: optional i32 width
}

//uri = 1025,   the event of video stream switch
struct VosdkSwitchVideoStream {
    1: optional VosdkHeader header,
    2: optional i16 eventType,   // begin, end, timeout
    3: optional i16 expectedStream,  // expected stream type
    4: optional i32 requestId,  // request id of switch
    5: optional i64 beginTs,   // optional 
    6: optional i64 endTs  // optional 
}


// uri = 1027
struct VosdkLbes {
    1: optional VosdkHeader header,
    2: optional i16 lbesUri,
    3: optional string url,
    4: optional string payload,
    5: optional i16 server_code,
    6: optional i16 code,
    7: optional string traceId,
}

// uri = 1031
struct VosdkMaxVideoPayloadSet{
    1: optional VosdkHeader header,
    2: optional i32 maxPayload,
}

// uri = 1032 vosdkFirstAudioPacketSent
struct VosdkFirstAudioPacketSent {
    1: optional VosdkHeader header,
    2: optional i32 codec,
}

// uri = 1033 vosdkFirstAudioPacketReceived
struct VosdkFirstAudioPacketReceived {
    1: optional VosdkHeader header,
    2: optional i32 codec,
}

// uri = 1034 vosdkAudioSendingStopped
struct VosdkAudioSendingStopped {
    1: optional VosdkHeader header,
    2: optional string reason,
}

// uri = 1035 vosdkAudioDisabled
struct VosdkAudioDisabled {
    1: optional VosdkHeader header,
}

// uri = 1036 vosdkAudioEnabled
struct VosdkAudioEnabled {
    1: optional VosdkHeader header,
}

// uri = 1037 vosdkFirstVideoPacketSent
struct VosdkFirstVideoPacketSent {
    1: optional VosdkHeader header,
    2: optional i32 codec,
}

// uri = 1038 vosdkFirstVideoPacketReceived
struct VosdkFirstVideoPacketReceived {
    1: optional VosdkHeader header,
    2: optional i32 codec,
}

// uri = 1039 vosdkFirstVideoFrameDecoded
struct VosdkFirstVideoFrameDecoded {
    1: optional VosdkHeader header,
    2: optional i32 width,
    3: optional i32 height,
}

// uri = 1040 vosdkFirstVideoFrameDrawed
struct VosdkFirstVideoFrameDrawed {
    1: optional VosdkHeader header,
    2: optional i32 width,
    3: optional i32 height,
}

// uri = 1041 vosdkVideoSendingStopped
struct VosdkVideoSendingStopped {
    1: optional VosdkHeader header,
    2: optional string reason,
}

// uri = 1042 vosdkVideoDisabled
struct VosdkVideoDisabled {
    1: optional VosdkHeader header,
}

// uri = 1043 vosdkVideoEnabled
struct VosdkVideoEnabled {
    1: optional VosdkHeader header,
}

// uri = 1044 vosdkVideoStreamSelected
struct VosdkVideoStreamSelected {
    1: optional VosdkHeader header,
    2: optional i32 streamType,
}

// uri = 1045 vosdkVideoStreamChangeRequest
struct VosdkVideoStreamChangeRequest {
    1: optional VosdkHeader header,
    2: optional i32 streamType,
}

// uri = 1046 vosdkFirstDataPacketSent
struct VosdkFirstDataPacketSent {
    1: optional VosdkHeader header,
    2: optional i32 transportType,
}

// uri = 1047 vosdkFirstDataPacketReceived
struct VosdkFirstDataPacketReceived {
    1: optional VosdkHeader header,
    2: optional i32 transportType,
}

// uri = 1048 vosdkError
struct VosdkError {
    1: optional VosdkHeader header,
    2: optional i32 errorNo,
    3: optional string description,
}

// uri = 1049 vosdkPeerOnlineStatus
struct VosdkPeerOnlineStatus {
    1: optional VosdkHeader header,
}

// uri = 1050 vosdkPeerOfflineStatus
struct VosdkPeerOfflineStatus {
    1: optional VosdkHeader header,
    2: optional string reason,
}

// uri = 1051 vosdkAudioMutePeerStatus
struct VosdkAudioMutePeerStatus {
    1: optional VosdkHeader header,
    2: optional bool muted,
}

// uri = 1052 vosdkVideoMutePeerStatus
struct VosdkVideoMutePeerStatus {
    1: optional VosdkHeader header,
    2: optional bool muted,
}

// uri = 1053 vosdkAudioMuteAllStatus
struct VosdkAudioMuteAllStatus {
    1: optional VosdkHeader header,
    2: optional bool muted,
}

// uri = 1054 vosdkVideoMuteAllStatus
struct VosdkVideoMuteAllStatus {
    1: optional VosdkHeader header,
    2: optional bool muted,
}

// uri = 1055 vosdkDefaultPeerStatus
struct VosdkDefaultPeerStatus {
    1: optional VosdkHeader header,
    2: optional i32 streamType,
}

// uri = 1056
struct VosdkP2PStunLoginSuccess {
    1: optional VosdkHeader header,
    2: optional string serverIp,
}

// uri = 1057
struct VosdkP2PStunLoginFailed {
    1: optional VosdkHeader header,
    2: optional string serverIp,
    3: optional i32 code,
}

// uri = 1058
struct VosdkP2PPeerTryTouch {
    1: optional VosdkHeader header,
    2: optional string peerLanIp,
    3: optional string peerWanIp,
}

// uri = 1059
struct VosdkP2PPeerConnected {
    1: optional VosdkHeader header,
    2: optional string peerIp,
}

// uri = 1060
struct VosdkP2PPeerDisconnected {
    1: optional VosdkHeader header,
    2: optional string reason,
}

// uri = 1061
struct VosdkP2PStart {
    1: optional VosdkHeader header,
    2: optional i32 threshold,
    3: optional string label,
}

// uri = 1062
struct VosdkP2PStop {
    1: optional VosdkHeader header,
    2: optional string reason,
}

//uri = 1063
struct VosdkAPEvent {
    1: optional VosdkHeader header,
    5: optional i32 ec,
    6: optional i32 sc,
    7: optional string serverIp,
    8: optional bool firstSuccess,
    9: optional i32 responseTime,
    10: optional list<string> serverIpList,
    11: optional string ssid,
    12: optional string bssid,
    13: optional string localWanIp,
    14: optional string ispName,
    15: optional bool minorIsp,
    16: optional i32 flag,
    17: optional string serviceName,
    18: optional string detail,
}

// uri = 1064
struct VosdkReportStats {
    1: optional VosdkHeader header,
    2: optional i32 allTotalTxPackets,
    3: optional i32 allTotalAckedPackets,
    4: optional i32 allValidTxPackets,
    5: optional i32 allValidAckedPackets,
    6: optional i32 counterTotalTxPackets,
    7: optional i32 counterTotalAckedPackets,
    8: optional i32 counterValidTxPackets,
    9: optional i32 counterValidAckedPackets,
    10: optional i32 eventTotalTxPackets,
    11: optional i32 eventTotalAckedPackets,
    12: optional i32 eventValidTxPackets,
    13: optional i32 eventValidAckedPackets,
}

//uri = 1081
struct VosdkAPWorkerEvent {
    1: optional VosdkHeader header,
    2: optional i32 ec,
    3: optional i32 sc,
    4: optional string serverIp,
    5: optional bool firstSuccess,
    6: optional i32 responseTime,
    7: optional string serviceName,
    8: optional string response_detail,
}


// uri = 1084
struct VosdkWorkerEvent {
    1: optional VosdkHeader header,
    2: optional string command,
    3: optional string actionType,
    4: optional string url,
    5: optional string payload,
    6: optional i16 server_code,
    7: optional i16 code,
    8: optional string traceId,
    9: optional i32 workerType,
    10: optional i32 responseTime
}

struct VosdkRecordingMixModeProperty {
  1: bool mixMode,
  2: bool mixedVidoAudioMode,
  3: i32 mixHigh,
  4: i32 mixLow,
  5: i32 mixFps,
  6: i32 mixKbps,
}

// uri = 1065
struct VosdkRecordingJoin {
  1: optional VosdkHeader header,
  2: optional VosdkRecordingMixModeProperty property,
  3: optional i32 minUdpPort,
  4: optional i32 maxUdpPort,
  5: optional i32 decodeAudioType,
  6: optional i32 decodeVideoType,
  7: optional i32 liveMode,
  8: optional i64 idle,
  9: optional i32 audioOnly,
  10: optional i32 videoOnly,// new insert
  11: optional i32 syslogFacility,
  12: optional i32 streamType,
  13: optional i32 triggerMode,
  14: optional i32 language,
  15: optional map<string, i64> res15Fields, //fixed reserve 15 fields
}

// uri = 1066
struct VosdkRecordingLeave {
  1: optional VosdkHeader header,
  2: optional i32 leavePathCode,
  3: optional map<string, i64> res5Fields,//fixed reserve 5 fields
}

struct VosdkPrivilegeExpireInfo {
  1: optional i32 privilege,
  2: optional i32 remainingTime,
  3: optional i64 expireTs
}

// uri = 1067
struct VosdkPrivilegeWillExpire {
  1: optional VosdkHeader header,
  2: optional string token,
  3: optional list<VosdkPrivilegeExpireInfo> privilegeExpireInfos
}

// uri = 1068
struct VosdkRenewToken {
  1: optional VosdkHeader header,
  2: optional string token
}

// uri = 1069
struct VosdkRenewTokenRes {
  1: optional VosdkHeader header,
  2: optional i32 res_code
}

// uri = 1070
struct VosdkSignalingMsgStat {
    1: optional VosdkHeader header,
    2: optional i32 txMsgCount,
    3: optional i32 rxClientMsgCount,
    4: optional i32 rxSignalingMsgCount,
    5: optional i32 rxDualMsgCount
}

// uri = 1072
struct VosdkLocalFallbackStatus {
    1: optional VosdkHeader header,
    2: optional i32 status,   // 0 - video high, 1 - video low, 2 - video muted
}


// uri = 1073
struct VosdkRemoteFallbackStatus {
    1: optional VosdkHeader header,
    2: optional i32 src, // 0 - video high, 1 - video low, 2 - video muted
    3: optional i32 dst,  // 0 - video high, 1 - video low, 2 - video muted
}

// uri = 1074
struct VosdkVideoBandwidthAggressiveLevel {
    1: optional VosdkHeader header,
    2: optional i32 level,
}

// uri = 1075
struct VosdkAppSetMinPlayoutDelay {
    1: optional VosdkHeader header,
    2: optional i32 playoutDelay
}

// uri = 1076
struct VosdkAppSetStartVideoBitRate {
    1: optional VosdkHeader header,
    2: optional i32 startVideoBitRate
}

// uri = 1077
struct VosdkSendVideoPaced {
    1: optional VosdkHeader header,
    2: optional bool isEnabled
}

// uri = 1078
struct VosdkApiExec {
    1: optional VosdkHeader header,
    2: optional string apiPayload,
    3: optional bool isCache,
    4: optional bool isNotificationSuppressed
}

// uri = 1080
struct VosdkABTest {
    1: optional VosdkHeader header,
    2: optional string feature,
    3: optional string tag,
    4: optional string params
}

// uri = 1082
struct VosdkVideoInitialOptions {
    1: optional VosdkHeader header,
    2: optional bool isSendVideoPacedEnabled,
    3: optional bool isVideoFecEnabled,
    4: optional i32 videoFecMethod,
    5: optional i32 localFallbackOption,
    6: optional i32 remoteFallbackOption
}

// uri = 1085
struct VosdkVqcStat {
    1: optional VosdkHeader header,
    2: optional i32 totalFrames,
    3: optional i32 averageScore,
    4: optional i32 llRatio,
    5: optional i32 hhRatio
}

//uri = 2101
struct WhiteBoardJoinCenter {
    1: optional VosdkHeader header,
    2: optional string cname
    3: optional string uid
    4: optional i32 vid
    5: optional i32 code
    6: optional string server
    7: optional string uip
}

//uri = 2102
struct WhiteBoardJoinEdge {
    1: optional VosdkHeader header,
    2: optional string cname
    3: optional string uid
    4: optional i32 vid
    5: optional i32 code
    6: optional string mode
    7: optional string role
    8: optional string uip
}

//uri = 2103
struct WhiteBoardJoinWebSocket {
    1: optional VosdkHeader header,
    2: optional string cname
    3: optional string uid
    4: optional i32 vid
    5: optional string uip
}

//uri = 2104
struct WhiteBoardUploadFile {
    1: optional VosdkHeader header,
    2: optional string cname
    3: optional string uid
    4: optional string filename
    5: optional i32 code
    6: optional i32 elapse
    7: optional string uip
}

//uri = 2105
struct WhiteBoardQuitWebSocket {
    1: optional VosdkHeader header,
    2: optional string cname
    3: optional string uid
    4: optional i32 vid
    5: optional string uip
}

//Broadcasting Server


//uri = 3000
// CDN dispatcher  的推流上报事件
struct BCPushEvent {
    1: optional i64 ts,
    2: optional i32 vid,
    3: optional i32 cdnId,
    4: optional string channelName,
    5: optional string dispatcherIp,
    6: optional string streamId,
    7: optional i32 status, // 1 - start, 2 - stop, 3 - error
    8: optional i32 errorCode
}

//uri = 3001
//CDN 推流打点采样
struct BCPeriodicUsage {
    1: optional i64 ts,
    2: optional i32 vid,
    3: optional i32 cdnId,
    4: optional string channelName,
    5: optional i32 trafficAgoraKb,  //从agora过来的流量
    6: optional i32 trafficCDNKb    //发到CDN的流量
}

//uri = 5000
// 通用链路跟踪span事件
// traceId/id/parentid: long => Hex String
struct TrackSpan {
  1: string traceId,
  2: string id,
  3: optional string parentId,
  4: string spanName,
  5: list<Annotation> annotations,
  6: optional list<BinaryAnnotation> binaryAnnotations,
  7: optional bool debug = 0,
  8: optional i64 timestamp,
  9: optional i64 duration,
  10: optional i64 traceIdHigh
}

struct Endpoint {
  1: i32 ipv4,
  2: i16 port,
  3: string serviceName
}

struct Annotation {
  1: i64 timestamp,
  2: string value,
  3: optional Endpoint endpoint
}

struct BinaryAnnotation {
  1: string key,
  2: string value,
  3: optional Endpoint endpoint
}

//VOS server 4000 - 4999

//uri = 4000
// 数据源 VOS
// 上报周期: 每个用户 每10s上报1条
// 上报内容： 用户当前频道内的状态， 如果用户在频道内， 需要定期上报用户当前的状态。
struct VOSClientPeriodicState {
    1: optional i64 ts,
    2: optional i64 cid,
    3: optional i64 uid,
    4: optional string sid,
    5: optional i32 vosId, 
    6: optional i32 state   // bitwise:  0 - not sending audio & video,  1 - only sending audio, 2 - only sending video, 3 - both audio and video
}

//属性名命名规则: namespace + 首字母大写的name:
//vosB2s
//vosdkRating
struct ReportItem {
  1: i32 uri,

  900: optional EncryptedItem encryptedItem,
  1000: optional VosdkCounter vosdkCounter,
  1001: optional VosdkSession vosdkSession,
  1002: optional VosdkVocs vosdkVocs,
  1003: optional VosdkVos vosdkVos,
  1004: optional VosdkChan vosdkChan,
  1005: optional VosdkQuit vosdkQuit,
  1006: optional VosdkPeer vosdkPeer,
  1007: optional VosdkViLocalFrame vosdkViLocalFrame,
  1008: optional VosdkViRemoteFrame vosdkViRemoteFrame,
  1009: optional VosdkRating vosdkRating,
  1010: optional VosdkACodec vosdkACodec,
  1011: optional VosdkNetOb vosdkNetOb,
  1012: optional VosdkP2PStartStun vosdkStartStun,
  1013: optional VosdkP2PSendDataBeginning vosdkSendDataBeginning,
  1014: optional VosdkP2PJoinIce vosdkJoinIce,
  1015: optional VosdkP2PSuccStun vosdkSuccStun,
  1016: optional VosdkP2POpen vosdkP2POpen,
  1017: optional VosdkErrorCode vosdkErrorCode,
  1018: optional VosdkP2PSwitch vosdkP2PSwitch,
  1019: optional VosdkP2PStunStat vosdkP2PStunStat,
  1020: optional VosdkNetworkInformation vosdkNetworkInformation,
  1021: optional VosdkNetOb2 vosdkNetOb2,
  1022: optional VosdkNetOb3 vosdkNetOb3,
  1023: optional VosdkNetOb4 vosdkNetOb4,
  1024: optional VosdkViRemoteFrameDecoded vosdkViRemoteFrameDecoded,
  1025: optional VosdkSwitchVideoStream vosdkSwitchVideoStream,
  1026: optional VosdkDeviceStatChange  vosdkDeviceStatChange,
  1027: optional VosdkLbes  vosdkLbes,
  1030: optional VosdkCameraInfos vosdkCameraInfos,
  1031: optional VosdkMaxVideoPayloadSet vosdkMaxVideoPayloadSet,
  1032: optional VosdkFirstAudioPacketSent vosdkFirstAudioPacketSent,
  1033: optional VosdkFirstAudioPacketReceived vosdkFirstAudioPacketReceived,
  1034: optional VosdkAudioSendingStopped vosdkAudioSendingStopped,
  1035: optional VosdkAudioDisabled vosdkAudioDisabled,
  1036: optional VosdkAudioEnabled vosdkAudioEnabled,
  1037: optional VosdkFirstVideoPacketSent vosdkFirstVideoPacketSent,
  1038: optional VosdkFirstVideoPacketReceived vosdkFirstVideoPacketReceived,
  1039: optional VosdkFirstVideoFrameDecoded vosdkFirstVideoFrameDecoded,
  1040: optional VosdkFirstVideoFrameDrawed vosdkFirstVideoFrameDrawed,
  1041: optional VosdkVideoSendingStopped vosdkVideoSendingStopped,
  1042: optional VosdkVideoDisabled vosdkVideoDisabled,
  1043: optional VosdkVideoEnabled vosdkVideoEnabled,
  1044: optional VosdkVideoStreamSelected vosdkVideoStreamSelected,
  1045: optional VosdkVideoStreamChangeRequest vosdkVideoStreamChangeRequest,
  1046: optional VosdkFirstDataPacketSent vosdkFirstDataPacketSent,
  1047: optional VosdkFirstDataPacketReceived vosdkFirstDataPacketReceived,
  1048: optional VosdkError vosdkError,
  1049: optional VosdkPeerOnlineStatus vosdkPeerOnlineStatus,
  1050: optional VosdkPeerOfflineStatus vosdkPeerOfflineStatus,
  1051: optional VosdkAudioMutePeerStatus vosdkAudioMutePeerStatus,
  1052: optional VosdkVideoMutePeerStatus vosdkVideoMutePeerStatus,
  1053: optional VosdkAudioMuteAllStatus vosdkAudioMuteAllStatus,
  1054: optional VosdkVideoMuteAllStatus vosdkVideoMuteAllStatus,
  1055: optional VosdkDefaultPeerStatus vosdkDefaultPeerStatus,
  1056: optional VosdkP2PStunLoginSuccess vosdkP2PStunLoginSuccess,
  1057: optional VosdkP2PStunLoginFailed vosdkP2PStunLoginFailed,
  1058: optional VosdkP2PPeerTryTouch vosdkP2PPeerTryTouch,
  1059: optional VosdkP2PPeerConnected vosdkP2PPeerConnected,
  1060: optional VosdkP2PPeerDisconnected vosdkP2PPeerDisconnected,
  1061: optional VosdkP2PStart vosdkP2PStart,
  1062: optional VosdkP2PStop vosdkP2PStop,
  1063: optional VosdkAPEvent vosdkAPEvent,
  1064: optional VosdkReportStats vosdkReportStats,
  1065: optional VosdkRecordingJoin vosdkRecordingJoin,
  1066: optional VosdkRecordingLeave vosdkRecordingLeave,
  1067: optional VosdkPrivilegeWillExpire vosdkPrivilegeWillExpire,
  1068: optional VosdkRenewToken vosdkRenewToken,
  1069: optional VosdkRenewTokenRes vosdkRenewTokenRes,
  1070: optional VosdkSignalingMsgStat vosdkSignalingMsgStat,
  1072: optional VosdkLocalFallbackStatus vosdkLocalFallbackStatus,
  1073: optional VosdkRemoteFallbackStatus vosdkRemoteFallbackStatus,
  1074: optional VosdkVideoBandwidthAggressiveLevel vosdkVideoBandwidthAggressiveLevel,
  1075: optional VosdkAppSetMinPlayoutDelay vosdkAppSetMinPlayoutDelay,
  1076: optional VosdkAppSetStartVideoBitRate vosdkAppSetStartVideoBitRate,
  1077: optional VosdkSendVideoPaced vosdkSendVideoPaced,
  1078: optional VosdkApiExec vosdkApiExec,
  1080: optional VosdkABTest vosdkABTest,
  1081: optional VosdkAPWorkerEvent vosdkAPWorkerEvent,
  1082: optional VosdkVideoInitialOptions vosdkVideoInitialOptions,
  1084: optional VosdkWorkerEvent vosdkWorkerEvent,
  1085: optional VosdkVqcStat vosdkVqcStat,

  2000: optional Vosb2s vosB2s,
  2001: optional WrtcBrowser2Gateway wrtcBrowser2Gateway,
  2002: optional WrtcProfile wrtcProfile,
  2003: optional WrtcPubStream wrtcPubStream,
  2004: optional WrtcUnpubStream wrtcUnpubStream,
  2005: optional WrtcSubStream wrtcSubStream,
  2006: optional WrtcUnsubStream wrtcUnsubStream,
  2101: optional WhiteBoardJoinCenter  whiteBoardJoinCenter,
  2102: optional WhiteBoardJoinEdge  whiteBoardJoinEdge,
  2103: optional WhiteBoardJoinWebSocket  whiteBoardJoinWebSocket,
  2104: optional WhiteBoardUploadFile  whiteBoardUploadFile,
  2105: optional WhiteBoardQuitWebSocket  whiteBoardQuitWebSocket,
  3000: optional BCPushEvent bcPushEvent,
  3001: optional BCPeriodicUsage bcPeriodicUsage,
  4000: optional VOSClientPeriodicState vosClientPeriodicState,
  5000: optional TrackSpan trackSpan
}
