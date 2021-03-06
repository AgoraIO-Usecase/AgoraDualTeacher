/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: cache_items.proto */

#ifndef PROTOBUF_C_cache_5fitems_2eproto__INCLUDED
#define PROTOBUF_C_cache_5fitems_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

typedef struct _Io__Agora__Pb__Cache__CacheDocument Io__Agora__Pb__Cache__CacheDocument;
typedef struct _Io__Agora__Pb__Cache__ReportCacheDocument Io__Agora__Pb__Cache__ReportCacheDocument;

/* --- enums --- */

/* --- messages --- */

struct _Io__Agora__Pb__Cache__CacheDocument {
  ProtobufCMessage base;
  /*
   * bssid or ip ==> dns name ==> ip list
   */
  ProtobufCBinaryData dnslist;
  char *lastsid;
  char *failedsid;
  ProtobufCBinaryData policy;
  char *installid;
  int32_t netengine;
  char *agorauniqueid;
  ProtobufCBinaryData loguploadedlist;
  char *udid;
  ProtobufCBinaryData storeparams;
};
#define IO__AGORA__PB__CACHE__CACHE_DOCUMENT__INIT                                            \
  {                                                                                           \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__cache__cache_document__descriptor)                \
    , {0, NULL}, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, {0, NULL}, \
        (char *)protobuf_c_empty_string, 0, (char *)protobuf_c_empty_string, {0, NULL},       \
        (char *)protobuf_c_empty_string, {                                                    \
      0, NULL                                                                                 \
    }                                                                                         \
  }

struct _Io__Agora__Pb__Cache__ReportCacheDocument {
  ProtobufCMessage base;
  /*
   * hash ==> ReportCacheItem
   */
  size_t n_reportcachelist;
  ProtobufCBinaryData *reportcachelist;
};
#define IO__AGORA__PB__CACHE__REPORT_CACHE_DOCUMENT__INIT                             \
  {                                                                                   \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__cache__report_cache_document__descriptor) \
    , 0, NULL                                                                         \
  }

/* Io__Agora__Pb__Cache__CacheDocument methods */
void io__agora__pb__cache__cache_document__init(Io__Agora__Pb__Cache__CacheDocument *message);
size_t io__agora__pb__cache__cache_document__get_packed_size(
    const Io__Agora__Pb__Cache__CacheDocument *message);
size_t io__agora__pb__cache__cache_document__pack(
    const Io__Agora__Pb__Cache__CacheDocument *message, uint8_t *out);
size_t io__agora__pb__cache__cache_document__pack_to_buffer(
    const Io__Agora__Pb__Cache__CacheDocument *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Cache__CacheDocument *io__agora__pb__cache__cache_document__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__cache__cache_document__free_unpacked(
    Io__Agora__Pb__Cache__CacheDocument *message, ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Cache__ReportCacheDocument methods */
void io__agora__pb__cache__report_cache_document__init(
    Io__Agora__Pb__Cache__ReportCacheDocument *message);
size_t io__agora__pb__cache__report_cache_document__get_packed_size(
    const Io__Agora__Pb__Cache__ReportCacheDocument *message);
size_t io__agora__pb__cache__report_cache_document__pack(
    const Io__Agora__Pb__Cache__ReportCacheDocument *message, uint8_t *out);
size_t io__agora__pb__cache__report_cache_document__pack_to_buffer(
    const Io__Agora__Pb__Cache__ReportCacheDocument *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Cache__ReportCacheDocument *io__agora__pb__cache__report_cache_document__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__cache__report_cache_document__free_unpacked(
    Io__Agora__Pb__Cache__ReportCacheDocument *message, ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Io__Agora__Pb__Cache__CacheDocument_Closure)(
    const Io__Agora__Pb__Cache__CacheDocument *message, void *closure_data);
typedef void (*Io__Agora__Pb__Cache__ReportCacheDocument_Closure)(
    const Io__Agora__Pb__Cache__ReportCacheDocument *message, void *closure_data);

/* --- services --- */

/* --- descriptors --- */

extern const ProtobufCMessageDescriptor io__agora__pb__cache__cache_document__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__cache__report_cache_document__descriptor;

PROTOBUF_C__END_DECLS

#endif /* PROTOBUF_C_cache_5fitems_2eproto__INCLUDED */
