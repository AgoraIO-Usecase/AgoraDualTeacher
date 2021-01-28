//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#pragma once

#include <stdint.h>
#include "agora_base.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/**
 * @ANNOTATION:GROUP:global
 * @ANNOTATION:CTOR:agora_rtm_message
 */
AGORA_API_C_HDL agora_rtm_message_create();

/**
 * @ANNOTATION:GROUP:agora_rtm_message
 * @ANNOTATION:DTOR:agora_rtm_message
 */
AGORA_API_C_VOID agora_rtm_message_release(AGORA_HANDLE agora_rtm_message);

/**
 * @ANNOTATION:GROUP:agora_rtm_message
 */
AGORA_API_C int64_t AGORA_CALL_C agora_rtm_message_get_message_id(AGORA_HANDLE agora_rtm_message);

/**
 * @ANNOTATION:GROUP:agora_rtm_message
 */
AGORA_API_C_INT agora_rtm_message_get_message_type(AGORA_HANDLE agora_rtm_message);

/**
 * @ANNOTATION:GROUP:agora_rtm_message
 */
AGORA_API_C_VOID agora_rtm_message_set_text(AGORA_HANDLE agora_rtm_message, const char *str);

/**
 * @ANNOTATION:GROUP:agora_rtm_message
 */
AGORA_API_C const char* AGORA_CALL_C agora_rtm_message_get_text(AGORA_HANDLE agora_rtm_message);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_member
 */
AGORA_API_C const char* AGORA_CALL_C agora_rtm_channel_member_get_member_id(AGORA_HANDLE agora_rtm_channel_member);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_member
 */
AGORA_API_C const char* AGORA_CALL_C agora_rtm_channel_member_get_channel_id(AGORA_HANDLE agora_rtm_channel_member);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_member
 */
AGORA_API_C_VOID agora_rtm_channel_member_release(AGORA_HANDLE agora_rtm_channel_member);

/**
 * @ANNOTATION:GROUP:global
 * @ANNOTATION:CTOR:agora_rtm_channel_attributes
 */
AGORA_API_C_HDL agora_rtm_channel_attributes_create();

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_attributes
 */
AGORA_API_C_INT agora_rtm_channel_attributes_release(AGORA_HANDLE agora_rtm_channel_attributes);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_attributes
 */
AGORA_API_C_INT agora_rtm_channel_attributes_add_attribute(AGORA_HANDLE agora_rtm_channel_attributes, const char *key, const char *value);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_attributes
 */
AGORA_API_C_INT agora_rtm_channel_attributes_remove_attribute(AGORA_HANDLE agora_rtm_channel_attributes, const char *key);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_attributes
 */
AGORA_API_C_INT agora_rtm_channel_attributes_get_attributes_size(AGORA_HANDLE agora_rtm_channel_attributes);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_attributes
 */
AGORA_API_C_VOID agora_rtm_channel_attributes_get_attributes(AGORA_HANDLE agora_rtm_channel_attributes, int size, char **key,
                            char **value);  // todo: discussion, how to traveral

/**
 * @ANNOTATION:GROUP:agora_rtm_channel_attributes
 */
AGORA_API_C const char* AGORA_CALL_C agora_rtm_channel_attributes_get_attribute_value(AGORA_HANDLE agora_rtm_channel_attributes, const char *key);



/**
 * The IChannelEventHandler class.
 * @ANNOTATION:TYPE:OBSERVER
 */
typedef struct _channel_event_handler {
  void (*on_join_success)(AGORA_HANDLE agora_rtm_channel);
  void (*on_join_failure)(AGORA_HANDLE agora_rtm_channel, int error_code);
  void (*on_leave)(AGORA_HANDLE agora_rtm_channel, int reason);
  void (*on_message_received)(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_message);
  void (*on_send_message_state)(AGORA_HANDLE agora_rtm_channel, int64_t message_id, int state);
  void (*on_member_joined)(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_channel_member);
  void (*on_member_left)(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_channel_member);
  void (*on_members_gotten)(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_channel_member, int user_count);
  void (*on_attributes_updated)(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_channel_attributes);
  void (*on_update_attributes_response)(AGORA_HANDLE agora_rtm_channel, int64_t request_id, int res_code);
  void (*on_attributes_deleted)(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_channel_attributes);
  void (*on_delete_attributes_response)(AGORA_HANDLE agora_rtm_channel, int64_t request_id, int res_code);
}channel_event_handler;


/**
 * @ANNOTATION:GROUP:agora_rtm_channel
 */
AGORA_API_C_VOID agora_rtm_channel_set_event_handler(AGORA_HANDLE agora_rtm_channel, channel_event_handler *event_handler);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel
 */
AGORA_API_C_INT agora_rtm_channel_join(AGORA_HANDLE agora_rtm_channel);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel
 */
AGORA_API_C_INT agora_rtm_channel_leave(AGORA_HANDLE agora_rtm_channel);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel
 */
AGORA_API_C_INT agora_rtm_channel_send_message(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_message);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel
 */
AGORA_API_C_INT agora_rtm_channel_update_attributes(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_channel_attributes, int64_t* request_id);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel
 */
AGORA_API_C_INT agora_rtm_channel_delete_attributes(AGORA_HANDLE agora_rtm_channel, AGORA_HANDLE agora_rtm_channel_attributes, int64_t* request_id);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel
 */
AGORA_API_C const char* AGORA_CALL_C agora_rtm_channel_get_id(AGORA_HANDLE agora_rtm_channel);

/**
 * @ANNOTATION:GROUP:agora_rtm_channel
 */
AGORA_API_C_INT agora_rtm_channel_release(AGORA_HANDLE agora_rtm_channel);

/**
 * The IRtmServiceEventHandler class.
 * @ANNOTATION:TYPE:OBSERVER
 */
typedef struct _rtm_service_event_handler {

  void (*on_login_success)(AGORA_HANDLE agora_rtm_service);
  void (*on_login_failure)(AGORA_HANDLE agora_rtm_service, int error_code);
  void (*on_logout)(AGORA_HANDLE agora_rtm_service);
  void (*on_connection_state_changed)(AGORA_HANDLE agora_rtm_service, int state);
  void (*on_send_message_state)(AGORA_HANDLE agora_rtm_service, int64_t message_id, int state);
  void (*on_message_received_from_peer)(AGORA_HANDLE agora_rtm_service, const char *peer_id, AGORA_HANDLE agora_rtm_message);
}rtm_service_event_handler;

/**
 * @ANNOTATION:GROUP:agora_rtm_service
 */
AGORA_API_C_INT agora_rtm_service_initialize(AGORA_HANDLE agora_rtm_service, const char *app_id, rtm_service_event_handler *event_handler);

/**
 * @ANNOTATION:GROUP:agora_rtm_service
 */
AGORA_API_C_VOID agora_rtm_service_unregister_observer(AGORA_HANDLE agora_rtm_service, rtm_service_event_handler *event_handler);

/**
 * @ANNOTATION:GROUP:agora_rtm_service
 */
AGORA_API_C_INT agora_rtm_service_release(AGORA_HANDLE agora_rtm_service, int sync);

/**
 * @ANNOTATION:GROUP:agora_rtm_service
 */
AGORA_API_C_INT agora_rtm_service_login(AGORA_HANDLE agora_rtm_service, const char *token, const char *user_id);

/**
 * @ANNOTATION:GROUP:agora_rtm_service
 */
AGORA_API_C_INT agora_rtm_service_logout(AGORA_HANDLE agora_rtm_service);

/**
 * @ANNOTATION:GROUP:agora_rtm_service
 */
AGORA_API_C_INT agora_rtm_service_send_message_to_peer(AGORA_HANDLE agora_rtm_service, const char *peer_id, AGORA_HANDLE agora_rtm_message);

/**
 * @ANNOTATION:GROUP:agora_rtm_service
 */
AGORA_API_C_HDL agora_rtm_service_create_channel(AGORA_HANDLE agora_rtm_service, const char *channel_id, channel_event_handler *event_handler);

#ifdef __cplusplus
}
#endif  // __cplusplus



