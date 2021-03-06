/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: counter.proto */

#ifndef PROTOBUF_C_counter_2eproto__INCLUDED
#define PROTOBUF_C_counter_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003001 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif

typedef struct _Io__Agora__Pb__Counter__Counter Io__Agora__Pb__Counter__Counter;
typedef struct _Io__Agora__Pb__Counter__CounterItem Io__Agora__Pb__Counter__CounterItem;

/* --- enums --- */

/* --- messages --- */

/*
 * id = 1
 */
struct _Io__Agora__Pb__Counter__Counter {
  ProtobufCMessage base;
  char *sid;
  int64_t peer;
  size_t n_items;
  Io__Agora__Pb__Counter__CounterItem **items;
};
#define IO__AGORA__PB__COUNTER__COUNTER__INIT                             \
  {                                                                       \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__counter__counter__descriptor) \
    , (char *)protobuf_c_empty_string, 0, 0, NULL                         \
  }

struct _Io__Agora__Pb__Counter__CounterItem {
  ProtobufCMessage base;
  int64_t lts;
  int32_t id;
  int32_t value;
  /*
   * 6 indicates interval of 6 seconds, 2 for 2 seconds
   */
  int32_t tagerrorcode;
  uint32_t streamid;
};
#define IO__AGORA__PB__COUNTER__COUNTER_ITEM__INIT                             \
  {                                                                            \
    PROTOBUF_C_MESSAGE_INIT(&io__agora__pb__counter__counter_item__descriptor) \
    , 0, 0, 0, 0, 0                                                            \
  }

/* Io__Agora__Pb__Counter__Counter methods */
void io__agora__pb__counter__counter__init(Io__Agora__Pb__Counter__Counter *message);
size_t io__agora__pb__counter__counter__get_packed_size(
    const Io__Agora__Pb__Counter__Counter *message);
size_t io__agora__pb__counter__counter__pack(const Io__Agora__Pb__Counter__Counter *message,
                                             uint8_t *out);
size_t io__agora__pb__counter__counter__pack_to_buffer(
    const Io__Agora__Pb__Counter__Counter *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Counter__Counter *io__agora__pb__counter__counter__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__counter__counter__free_unpacked(Io__Agora__Pb__Counter__Counter *message,
                                                    ProtobufCAllocator *allocator);
/* Io__Agora__Pb__Counter__CounterItem methods */
void io__agora__pb__counter__counter_item__init(Io__Agora__Pb__Counter__CounterItem *message);
size_t io__agora__pb__counter__counter_item__get_packed_size(
    const Io__Agora__Pb__Counter__CounterItem *message);
size_t io__agora__pb__counter__counter_item__pack(
    const Io__Agora__Pb__Counter__CounterItem *message, uint8_t *out);
size_t io__agora__pb__counter__counter_item__pack_to_buffer(
    const Io__Agora__Pb__Counter__CounterItem *message, ProtobufCBuffer *buffer);
Io__Agora__Pb__Counter__CounterItem *io__agora__pb__counter__counter_item__unpack(
    ProtobufCAllocator *allocator, size_t len, const uint8_t *data);
void io__agora__pb__counter__counter_item__free_unpacked(
    Io__Agora__Pb__Counter__CounterItem *message, ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*Io__Agora__Pb__Counter__Counter_Closure)(
    const Io__Agora__Pb__Counter__Counter *message, void *closure_data);
typedef void (*Io__Agora__Pb__Counter__CounterItem_Closure)(
    const Io__Agora__Pb__Counter__CounterItem *message, void *closure_data);

/* --- services --- */

/* --- descriptors --- */

extern const ProtobufCMessageDescriptor io__agora__pb__counter__counter__descriptor;
extern const ProtobufCMessageDescriptor io__agora__pb__counter__counter_item__descriptor;

PROTOBUF_C__END_DECLS

#endif /* PROTOBUF_C_counter_2eproto__INCLUDED */
