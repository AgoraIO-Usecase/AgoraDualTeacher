/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: rtmsdk_report_items.proto */

#ifndef PROTOBUF_C_rtmsdk_5freport_5fitems_2eproto__INCLUDED
#define PROTOBUF_C_rtmsdk_5freport_5fitems_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

typedef struct _Io__Agora__Pb__Rtmsdk__CommonIndex Io__Agora__Pb__Rtmsdk__CommonIndex;
typedef struct _Io__Agora__Pb__Rtmsdk__Session Io__Agora__Pb__Rtmsdk__Session;
typedef struct _Io__Agora__Pb__Rtmsdk__ApEvent Io__Agora__Pb__Rtmsdk__ApEvent;
typedef struct _Io__Agora__Pb__Rtmsdk__Link Io__Agora__Pb__Rtmsdk__Link;
typedef struct _Io__Agora__Pb__Rtmsdk__Logout Io__Agora__Pb__Rtmsdk__Logout;
typedef struct _Io__Agora__Pb__Rtmsdk__TxMessage Io__Agora__Pb__Rtmsdk__TxMessage;
typedef struct _Io__Agora__Pb__Rtmsdk__RxMessage Io__Agora__Pb__Rtmsdk__RxMessage;
typedef struct _Io__Agora__Pb__Rtmsdk__KickedOff Io__Agora__Pb__Rtmsdk__KickedOff;
typedef struct _Io__Agora__Pb__Rtmsdk__TxMessageRes Io__Agora__Pb__Rtmsdk__TxMessageRes;
typedef struct _Io__Agora__Pb__Rtmsdk__ChnJoin Io__Agora__Pb__Rtmsdk__ChnJoin;
typedef struct _Io__Agora__Pb__Rtmsdk__ChnJoinRes Io__Agora__Pb__Rtmsdk__ChnJoinRes;
typedef struct _Io__Agora__Pb__Rtmsdk__ChnLeave Io__Agora__Pb__Rtmsdk__ChnLeave;
typedef struct _Io__Agora__Pb__Rtmsdk__ChnGetMembers Io__Agora__Pb__Rtmsdk__ChnGetMembers;
typedef struct _Io__Agora__Pb__Rtmsdk__ChnGetMembersRes Io__Agora__Pb__Rtmsdk__ChnGetMembersRes;
typedef struct _Io__Agora__Pb__Rtmsdk__ChnMemberJoined Io__Agora__Pb__Rtmsdk__ChnMemberJoined;
typedef struct _Io__Agora__Pb__Rtmsdk__ChnMemberLeft Io__Agora__Pb__Rtmsdk__ChnMemberLeft;
typedef struct _Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus
    Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus;
typedef struct _Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes
    Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes;
typedef struct _Io__Agora__Pb__Rtmsdk__RenewToken Io__Agora__Pb__Rtmsdk__RenewToken;

/* --- enums --- */

/* --- messages --- */

struct _Io__Agora__Pb__Rtmsdk__CommonIndex {
  ProtobufCMessage base;
  char *index1;
  char *index2;
  char *index3;
};
#define IO__AGORA__PB__RTMSDK__COMMON_INDEX__INIT                             \
  {                                                                           \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__common_index__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string,       \
        (char *)protobuf_c_empty_string                                       \
  }

/*
 *id = 167
 */
struct _Io__Agora__Pb__Rtmsdk__Session {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  char *appid;
  char *ver;
  int32_t buildno;
  char *installid;
  char *localip;
  char *wanip;
  int32_t net1;
  int32_t netsubtype;
  char *ssid;
  char *bssid;
  int32_t rssi;
  int32_t os;
  char *did;
  char *lsid;
  char *fsid;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *token;
};
#define IO__AGORA__PB__RTMSDK__SESSION__INIT                                    \
  {                                                                             \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__session__descriptor)        \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0,   \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0,    \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string,       \
        (char *)protobuf_c_empty_string, 0, 0, (char *)protobuf_c_empty_string, \
        (char *)protobuf_c_empty_string, 0, 0, (char *)protobuf_c_empty_string, \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, NULL, \
        (char *)protobuf_c_empty_string                                         \
  }

/*
 *id = 168
 */
struct _Io__Agora__Pb__Rtmsdk__ApEvent {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *apaddr;
  size_t n_linkserverlist;
  char **linkserverlist;
  char *localwanip;
  int32_t errcode;
  int32_t servererrcode;
  char *isp;
  int64_t opid;
  int32_t envid;
  int32_t flag;
  char *area;
};
#define IO__AGORA__PB__RTMSDK__AP_EVENT__INIT                                            \
  {                                                                                      \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__ap_event__descriptor)                \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL,      \
        (char *)protobuf_c_empty_string, 0, NULL, (char *)protobuf_c_empty_string, 0, 0, \
        (char *)protobuf_c_empty_string, 0, 0, 0, (char *)protobuf_c_empty_string        \
  }

/*
 *id = 169
 */
struct _Io__Agora__Pb__Rtmsdk__Link {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  int32_t ec;
  int32_t sc;
  char *destserverip;
  char *ackedserverip;
  int32_t responsetime;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
};
#define IO__AGORA__PB__RTMSDK__LINK__INIT                                           \
  {                                                                                 \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__link__descriptor)               \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, 0, 0, \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, NULL   \
  }

/*
 *id = 170
 */
struct _Io__Agora__Pb__Rtmsdk__Logout {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
};
#define IO__AGORA__PB__RTMSDK__LOGOUT__INIT                                        \
  {                                                                                \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__logout__descriptor)            \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL \
  }

/*
 *id = 171
 */
struct _Io__Agora__Pb__Rtmsdk__TxMessage {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  int64_t insid;
  int64_t dialid;
  int64_t seq;
  char *srcid;
  char *dstid;
  int32_t dsttype;
  char *payload;
  int64_t messageid;
  protobuf_c_boolean isofflinemessage;
};
#define IO__AGORA__PB__RTMSDK__TX_MESSAGE__INIT                                              \
  {                                                                                          \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__tx_message__descriptor)                  \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, 0, 0, 0, \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0,                 \
        (char *)protobuf_c_empty_string, 0, 0                                                \
  }

/*
 *id = 172
 */
struct _Io__Agora__Pb__Rtmsdk__RxMessage {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  int64_t insid;
  int64_t dialid;
  int64_t seq;
  char *srcid;
  char *dstid;
  int32_t dsttype;
  char *payload;
  int64_t messageid;
  int64_t serverreceivedts;
  protobuf_c_boolean isofflinemessage;
};
#define IO__AGORA__PB__RTMSDK__RX_MESSAGE__INIT                                              \
  {                                                                                          \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__rx_message__descriptor)                  \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, 0, 0, 0, \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0,                 \
        (char *)protobuf_c_empty_string, 0, 0, 0                                             \
  }

/*
 *id = 173
 */
struct _Io__Agora__Pb__Rtmsdk__KickedOff {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  int32_t linkid;
  int32_t code;
  char *server;
  int32_t servercode;
};
#define IO__AGORA__PB__RTMSDK__KICKED_OFF__INIT                                           \
  {                                                                                       \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__kicked_off__descriptor)               \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, 0, 0, \
        (char *)protobuf_c_empty_string, 0                                                \
  }

/*
 *id = 174
 */
struct _Io__Agora__Pb__Rtmsdk__TxMessageRes {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  int64_t insid;
  int64_t dialid;
  int64_t seq;
  char *srcid;
  char *dstid;
  int32_t dsttype;
  int64_t messageid;
  int32_t err_code;
};
#define IO__AGORA__PB__RTMSDK__TX_MESSAGE_RES__INIT                                          \
  {                                                                                          \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__tx_message_res__descriptor)              \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, 0, 0, 0, \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, 0            \
  }

/*
 *id = 175
 */
struct _Io__Agora__Pb__Rtmsdk__ChnJoin {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *cname;
  int32_t errcode;
};
#define IO__AGORA__PB__RTMSDK__CHN_JOIN__INIT                                       \
  {                                                                                 \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__chn_join__descriptor)           \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, \
        (char *)protobuf_c_empty_string, 0                                          \
  }

/*
 *id = 176
 */
struct _Io__Agora__Pb__Rtmsdk__ChnJoinRes {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *cname;
  int32_t errcode;
  int32_t servererrcode;
};
#define IO__AGORA__PB__RTMSDK__CHN_JOIN_RES__INIT                                   \
  {                                                                                 \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__chn_join_res__descriptor)       \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, \
        (char *)protobuf_c_empty_string, 0, 0                                       \
  }

/*
 *id = 177
 */
struct _Io__Agora__Pb__Rtmsdk__ChnLeave {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *cname;
  int32_t errcode;
};
#define IO__AGORA__PB__RTMSDK__CHN_LEAVE__INIT                                      \
  {                                                                                 \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__chn_leave__descriptor)          \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, \
        (char *)protobuf_c_empty_string, 0                                          \
  }

/*
 *id = 178
 */
struct _Io__Agora__Pb__Rtmsdk__ChnGetMembers {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *cname;
  int32_t errcode;
};
#define IO__AGORA__PB__RTMSDK__CHN_GET_MEMBERS__INIT                                \
  {                                                                                 \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__chn_get_members__descriptor)    \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, \
        (char *)protobuf_c_empty_string, 0                                          \
  }

/*
 *id = 179
 */
struct _Io__Agora__Pb__Rtmsdk__ChnGetMembersRes {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *cname;
  int32_t errcode;
  int32_t size;
};
#define IO__AGORA__PB__RTMSDK__CHN_GET_MEMBERS_RES__INIT                             \
  {                                                                                  \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__chn_get_members_res__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL,  \
        (char *)protobuf_c_empty_string, 0, 0                                        \
  }

/*
 *id = 180
 */
struct _Io__Agora__Pb__Rtmsdk__ChnMemberJoined {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *cname;
  char *memberid;
};
#define IO__AGORA__PB__RTMSDK__CHN_MEMBER_JOINED__INIT                              \
  {                                                                                 \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__chn_member_joined__descriptor)  \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string            \
  }

/*
 *id = 181
 */
struct _Io__Agora__Pb__Rtmsdk__ChnMemberLeft {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *cname;
  char *memberid;
};
#define IO__AGORA__PB__RTMSDK__CHN_MEMBER_LEFT__INIT                                \
  {                                                                                 \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__chn_member_left__descriptor)    \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string            \
  }

/*
 *id = 182
 */
struct _Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  int32_t peercount;
};
#define IO__AGORA__PB__RTMSDK__QUERY_PEERS_ONLINE_STATUS__INIT                             \
  {                                                                                        \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__query_peers_online_status__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, 0      \
  }

/*
 *id = 183
 */
struct _Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  int32_t peercount;
  int32_t errcode;
};
#define IO__AGORA__PB__RTMSDK__QUERY_PEERS_ONLINE_STATUS_RES__INIT                             \
  {                                                                                            \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__query_peers_online_status_res__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, 0, 0       \
  }

/*
 *id = 184
 */
struct _Io__Agora__Pb__Rtmsdk__RenewToken {
  ProtobufCMessage base;
  char *sid;
  char *userid;
  int64_t lts;
  int64_t elapse;
  Io__Agora__Pb__Rtmsdk__CommonIndex *index;
  char *cname;
  char *token;
};
#define IO__AGORA__PB__RTMSDK__RENEW_TOKEN__INIT                                    \
  {                                                                                 \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__rtmsdk__renew_token__descriptor)        \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, NULL, \
        (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string            \
  }

/* Io__Agora__Pb__Rtmsdk__CommonIndex methods */
void io__agora__pb__rtmsdk__common_index__init(Io__Agora__Pb__Rtmsdk__CommonIndex *message);
size_t io__agora__pb__rtmsdk__common_index__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__CommonIndex *message);
size_t io__agora__pb__rtmsdk__common_index__pack(const Io__Agora__Pb__Rtmsdk__CommonIndex *message,
                                                 uint8_t *out);
size_t io__agora__pb__rtmsdk__common_index__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__CommonIndex *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__CommonIndex *io__agora__pb__rtmsdk__common_index__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__common_index__free_unpacked(Io__Agora__Pb__Rtmsdk__CommonIndex *message,
                                                        ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__Session methods */
void io__agora__pb__rtmsdk__session__init(Io__Agora__Pb__Rtmsdk__Session *message);
size_t io__agora__pb__rtmsdk__session__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__Session *message);
size_t io__agora__pb__rtmsdk__session__pack(const Io__Agora__Pb__Rtmsdk__Session *message,
                                            uint8_t *out);
size_t io__agora__pb__rtmsdk__session__pack_to_buffer(const Io__Agora__Pb__Rtmsdk__Session *message,
                                                      ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__Session *io__agora__pb__rtmsdk__session__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__session__free_unpacked(Io__Agora__Pb__Rtmsdk__Session *message,
                                                   ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__ApEvent methods */
void io__agora__pb__rtmsdk__ap_event__init(Io__Agora__Pb__Rtmsdk__ApEvent *message);
size_t io__agora__pb__rtmsdk__ap_event__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__ApEvent *message);
size_t io__agora__pb__rtmsdk__ap_event__pack(const Io__Agora__Pb__Rtmsdk__ApEvent *message,
                                             uint8_t *out);
size_t io__agora__pb__rtmsdk__ap_event__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__ApEvent *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__ApEvent *io__agora__pb__rtmsdk__ap_event__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__ap_event__free_unpacked(Io__Agora__Pb__Rtmsdk__ApEvent *message,
                                                    ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__Link methods */
void io__agora__pb__rtmsdk__link__init(Io__Agora__Pb__Rtmsdk__Link *message);
size_t io__agora__pb__rtmsdk__link__get_packed_size(const Io__Agora__Pb__Rtmsdk__Link *message);
size_t io__agora__pb__rtmsdk__link__pack(const Io__Agora__Pb__Rtmsdk__Link *message, uint8_t *out);
size_t io__agora__pb__rtmsdk__link__pack_to_buffer(const Io__Agora__Pb__Rtmsdk__Link *message,
                                                   ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__Link *io__agora__pb__rtmsdk__link__unpack(ProtobufCAllocator *allocator,
                                                                 size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__link__free_unpacked(Io__Agora__Pb__Rtmsdk__Link *message,
                                                ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__Logout methods */
void io__agora__pb__rtmsdk__logout__init(Io__Agora__Pb__Rtmsdk__Logout *message);
size_t io__agora__pb__rtmsdk__logout__get_packed_size(const Io__Agora__Pb__Rtmsdk__Logout *message);
size_t io__agora__pb__rtmsdk__logout__pack(const Io__Agora__Pb__Rtmsdk__Logout *message,
                                           uint8_t *out);
size_t io__agora__pb__rtmsdk__logout__pack_to_buffer(const Io__Agora__Pb__Rtmsdk__Logout *message,
                                                     ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__Logout *io__agora__pb__rtmsdk__logout__unpack(ProtobufCAllocator *allocator,
                                                                     size_t len,
                                                                     const uint8_t *data);
void io__agora__pb__rtmsdk__logout__free_unpacked(Io__Agora__Pb__Rtmsdk__Logout *message,
                                                  ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__TxMessage methods */
void io__agora__pb__rtmsdk__tx_message__init(Io__Agora__Pb__Rtmsdk__TxMessage *message);
size_t io__agora__pb__rtmsdk__tx_message__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__TxMessage *message);
size_t io__agora__pb__rtmsdk__tx_message__pack(const Io__Agora__Pb__Rtmsdk__TxMessage *message,
                                               uint8_t *out);
size_t io__agora__pb__rtmsdk__tx_message__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__TxMessage *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__TxMessage *io__agora__pb__rtmsdk__tx_message__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__tx_message__free_unpacked(Io__Agora__Pb__Rtmsdk__TxMessage *message,
                                                      ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__RxMessage methods */
void io__agora__pb__rtmsdk__rx_message__init(Io__Agora__Pb__Rtmsdk__RxMessage *message);
size_t io__agora__pb__rtmsdk__rx_message__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__RxMessage *message);
size_t io__agora__pb__rtmsdk__rx_message__pack(const Io__Agora__Pb__Rtmsdk__RxMessage *message,
                                               uint8_t *out);
size_t io__agora__pb__rtmsdk__rx_message__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__RxMessage *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__RxMessage *io__agora__pb__rtmsdk__rx_message__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__rx_message__free_unpacked(Io__Agora__Pb__Rtmsdk__RxMessage *message,
                                                      ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__KickedOff methods */
void io__agora__pb__rtmsdk__kicked_off__init(Io__Agora__Pb__Rtmsdk__KickedOff *message);
size_t io__agora__pb__rtmsdk__kicked_off__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__KickedOff *message);
size_t io__agora__pb__rtmsdk__kicked_off__pack(const Io__Agora__Pb__Rtmsdk__KickedOff *message,
                                               uint8_t *out);
size_t io__agora__pb__rtmsdk__kicked_off__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__KickedOff *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__KickedOff *io__agora__pb__rtmsdk__kicked_off__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__kicked_off__free_unpacked(Io__Agora__Pb__Rtmsdk__KickedOff *message,
                                                      ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__TxMessageRes methods */
void io__agora__pb__rtmsdk__tx_message_res__init(Io__Agora__Pb__Rtmsdk__TxMessageRes *message);
size_t io__agora__pb__rtmsdk__tx_message_res__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__TxMessageRes *message);
size_t io__agora__pb__rtmsdk__tx_message_res__pack(
    const Io__Agora__Pb__Rtmsdk__TxMessageRes *message, uint8_t *out);
size_t io__agora__pb__rtmsdk__tx_message_res__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__TxMessageRes *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__TxMessageRes *io__agora__pb__rtmsdk__tx_message_res__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__tx_message_res__free_unpacked(
    Io__Agora__Pb__Rtmsdk__TxMessageRes *message, ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__ChnJoin methods */
void io__agora__pb__rtmsdk__chn_join__init(Io__Agora__Pb__Rtmsdk__ChnJoin *message);
size_t io__agora__pb__rtmsdk__chn_join__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__ChnJoin *message);
size_t io__agora__pb__rtmsdk__chn_join__pack(const Io__Agora__Pb__Rtmsdk__ChnJoin *message,
                                             uint8_t *out);
size_t io__agora__pb__rtmsdk__chn_join__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__ChnJoin *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__ChnJoin *io__agora__pb__rtmsdk__chn_join__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__chn_join__free_unpacked(Io__Agora__Pb__Rtmsdk__ChnJoin *message,
                                                    ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__ChnJoinRes methods */
void io__agora__pb__rtmsdk__chn_join_res__init(Io__Agora__Pb__Rtmsdk__ChnJoinRes *message);
size_t io__agora__pb__rtmsdk__chn_join_res__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__ChnJoinRes *message);
size_t io__agora__pb__rtmsdk__chn_join_res__pack(const Io__Agora__Pb__Rtmsdk__ChnJoinRes *message,
                                                 uint8_t *out);
size_t io__agora__pb__rtmsdk__chn_join_res__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__ChnJoinRes *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__ChnJoinRes *io__agora__pb__rtmsdk__chn_join_res__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__chn_join_res__free_unpacked(Io__Agora__Pb__Rtmsdk__ChnJoinRes *message,
                                                        ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__ChnLeave methods */
void io__agora__pb__rtmsdk__chn_leave__init(Io__Agora__Pb__Rtmsdk__ChnLeave *message);
size_t io__agora__pb__rtmsdk__chn_leave__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__ChnLeave *message);
size_t io__agora__pb__rtmsdk__chn_leave__pack(const Io__Agora__Pb__Rtmsdk__ChnLeave *message,
                                              uint8_t *out);
size_t io__agora__pb__rtmsdk__chn_leave__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__ChnLeave *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__ChnLeave *io__agora__pb__rtmsdk__chn_leave__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__chn_leave__free_unpacked(Io__Agora__Pb__Rtmsdk__ChnLeave *message,
                                                     ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__ChnGetMembers methods */
void io__agora__pb__rtmsdk__chn_get_members__init(Io__Agora__Pb__Rtmsdk__ChnGetMembers *message);
size_t io__agora__pb__rtmsdk__chn_get_members__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__ChnGetMembers *message);
size_t io__agora__pb__rtmsdk__chn_get_members__pack(
    const Io__Agora__Pb__Rtmsdk__ChnGetMembers *message, uint8_t *out);
size_t io__agora__pb__rtmsdk__chn_get_members__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__ChnGetMembers *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__ChnGetMembers *io__agora__pb__rtmsdk__chn_get_members__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__chn_get_members__free_unpacked(
    Io__Agora__Pb__Rtmsdk__ChnGetMembers *message, ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__ChnGetMembersRes methods */
void io__agora__pb__rtmsdk__chn_get_members_res__init(
    Io__Agora__Pb__Rtmsdk__ChnGetMembersRes *message);
size_t io__agora__pb__rtmsdk__chn_get_members_res__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__ChnGetMembersRes *message);
size_t io__agora__pb__rtmsdk__chn_get_members_res__pack(
    const Io__Agora__Pb__Rtmsdk__ChnGetMembersRes *message, uint8_t *out);
size_t io__agora__pb__rtmsdk__chn_get_members_res__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__ChnGetMembersRes *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__ChnGetMembersRes *io__agora__pb__rtmsdk__chn_get_members_res__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__chn_get_members_res__free_unpacked(
    Io__Agora__Pb__Rtmsdk__ChnGetMembersRes *message, ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__ChnMemberJoined methods */
void io__agora__pb__rtmsdk__chn_member_joined__init(
    Io__Agora__Pb__Rtmsdk__ChnMemberJoined *message);
size_t io__agora__pb__rtmsdk__chn_member_joined__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__ChnMemberJoined *message);
size_t io__agora__pb__rtmsdk__chn_member_joined__pack(
    const Io__Agora__Pb__Rtmsdk__ChnMemberJoined *message, uint8_t *out);
size_t io__agora__pb__rtmsdk__chn_member_joined__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__ChnMemberJoined *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__ChnMemberJoined *io__agora__pb__rtmsdk__chn_member_joined__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__chn_member_joined__free_unpacked(
    Io__Agora__Pb__Rtmsdk__ChnMemberJoined *message, ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__ChnMemberLeft methods */
void io__agora__pb__rtmsdk__chn_member_left__init(Io__Agora__Pb__Rtmsdk__ChnMemberLeft *message);
size_t io__agora__pb__rtmsdk__chn_member_left__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__ChnMemberLeft *message);
size_t io__agora__pb__rtmsdk__chn_member_left__pack(
    const Io__Agora__Pb__Rtmsdk__ChnMemberLeft *message, uint8_t *out);
size_t io__agora__pb__rtmsdk__chn_member_left__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__ChnMemberLeft *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__ChnMemberLeft *io__agora__pb__rtmsdk__chn_member_left__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__chn_member_left__free_unpacked(
    Io__Agora__Pb__Rtmsdk__ChnMemberLeft *message, ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus methods */
void io__agora__pb__rtmsdk__query_peers_online_status__init(
    Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus *message);
size_t io__agora__pb__rtmsdk__query_peers_online_status__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus *message);
size_t io__agora__pb__rtmsdk__query_peers_online_status__pack(
    const Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus *message, uint8_t *out);
size_t io__agora__pb__rtmsdk__query_peers_online_status__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus *
io__agora__pb__rtmsdk__query_peers_online_status__unpack(ProtobufCAllocator *allocator, size_t len,
                                                         const uint8_t *data);
void io__agora__pb__rtmsdk__query_peers_online_status__free_unpacked(
    Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus *message, ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes methods */
void io__agora__pb__rtmsdk__query_peers_online_status_res__init(
    Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes *message);
size_t io__agora__pb__rtmsdk__query_peers_online_status_res__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes *message);
size_t io__agora__pb__rtmsdk__query_peers_online_status_res__pack(
    const Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes *message, uint8_t *out);
size_t io__agora__pb__rtmsdk__query_peers_online_status_res__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes *
io__agora__pb__rtmsdk__query_peers_online_status_res__unpack(ProtobufCAllocator *allocator,
                                                             size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__query_peers_online_status_res__free_unpacked(
    Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes *message, ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Rtmsdk__RenewToken methods */
void io__agora__pb__rtmsdk__renew_token__init(Io__Agora__Pb__Rtmsdk__RenewToken *message);
size_t io__agora__pb__rtmsdk__renew_token__get_packed_size(
    const Io__Agora__Pb__Rtmsdk__RenewToken *message);
size_t io__agora__pb__rtmsdk__renew_token__pack(const Io__Agora__Pb__Rtmsdk__RenewToken *message,
                                                uint8_t *out);
size_t io__agora__pb__rtmsdk__renew_token__pack_to_buffer(
    const Io__Agora__Pb__Rtmsdk__RenewToken *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Rtmsdk__RenewToken *io__agora__pb__rtmsdk__renew_token__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__rtmsdk__renew_token__free_unpacked(Io__Agora__Pb__Rtmsdk__RenewToken *message,
                                                       ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Io__Agora__Pb__Rtmsdk__CommonIndex_Closure)(
    const Io__Agora__Pb__Rtmsdk__CommonIndex *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__Session_Closure)(
    const Io__Agora__Pb__Rtmsdk__Session *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__ApEvent_Closure)(
    const Io__Agora__Pb__Rtmsdk__ApEvent *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__Link_Closure)(const Io__Agora__Pb__Rtmsdk__Link *message,
                                                    void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__Logout_Closure)(const Io__Agora__Pb__Rtmsdk__Logout *message,
                                                      void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__TxMessage_Closure)(
    const Io__Agora__Pb__Rtmsdk__TxMessage *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__RxMessage_Closure)(
    const Io__Agora__Pb__Rtmsdk__RxMessage *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__KickedOff_Closure)(
    const Io__Agora__Pb__Rtmsdk__KickedOff *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__TxMessageRes_Closure)(
    const Io__Agora__Pb__Rtmsdk__TxMessageRes *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__ChnJoin_Closure)(
    const Io__Agora__Pb__Rtmsdk__ChnJoin *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__ChnJoinRes_Closure)(
    const Io__Agora__Pb__Rtmsdk__ChnJoinRes *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__ChnLeave_Closure)(
    const Io__Agora__Pb__Rtmsdk__ChnLeave *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__ChnGetMembers_Closure)(
    const Io__Agora__Pb__Rtmsdk__ChnGetMembers *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__ChnGetMembersRes_Closure)(
    const Io__Agora__Pb__Rtmsdk__ChnGetMembersRes *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__ChnMemberJoined_Closure)(
    const Io__Agora__Pb__Rtmsdk__ChnMemberJoined *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__ChnMemberLeft_Closure)(
    const Io__Agora__Pb__Rtmsdk__ChnMemberLeft *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus_Closure)(
    const Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatus *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes_Closure)(
    const Io__Agora__Pb__Rtmsdk__QueryPeersOnlineStatusRes *message, void *closure_data);
typedef void (*Io__Agora__Pb__Rtmsdk__RenewToken_Closure)(
    const Io__Agora__Pb__Rtmsdk__RenewToken *message, void *closure_data);

/* --- services --- */

/* --- descriptors --- */

extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__common_index__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__session__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__ap_event__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__link__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__logout__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__tx_message__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__rx_message__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__kicked_off__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__tx_message_res__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__chn_join__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__chn_join_res__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__chn_leave__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__chn_get_members__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__chn_get_members_res__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__chn_member_joined__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__chn_member_left__descriptor;
extern const ProtobufCMessageDescriptor
    io__agora__pb__rtmsdk__query_peers_online_status__descriptor;
extern const ProtobufCMessageDescriptor
    io__agora__pb__rtmsdk__query_peers_online_status_res__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__rtmsdk__renew_token__descriptor;

PROTOBUF_C__END_DECLS

#endif /* PROTOBUF_C_rtmsdk_5freport_5fitems_2eproto__INCLUDED */
