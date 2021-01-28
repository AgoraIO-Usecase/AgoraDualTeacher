//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//
#include "agora_observer_c.h"
#include "agora_receiver_c.h"
#include "agora_ref_ptr_holder.h"
#include "agora_utils_c.h"
#include "api2/IAgoraRtmService.h"
#include "api2/agora_rtm_service.h"
#include "base/agora_base.h"

namespace {

class CRtmServiceEventHandler : public agora::rtm::IRtmServiceEventHandler,
                                public agora::interop::CAgoraCallback<rtm_service_event_handler> {
 public:
  CRtmServiceEventHandler() = default;
  ~CRtmServiceEventHandler() override = default;

  void onLoginSuccess() override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_login_success) {
        p.second.on_login_success(p.first);
      }
    }
  }

  void onLoginFailure(agora::rtm::LOGIN_ERR_CODE errorCode) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_login_failure) {
        p.second.on_login_failure(p.first, errorCode);
      }
    }
  }

  void onLogout() override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_logout) {
        p.second.on_logout(p.first);
      }
    }
  }

  void onConnectionStateChanged(agora::rtm::CONNECTION_STATE state) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_connection_state_changed) {
        p.second.on_connection_state_changed(p.first, state);
      }
    }
  }

  void onSendMessageState(int64_t messageId, agora::rtm::PEER_MESSAGE_STATE state) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_send_message_state) {
        p.second.on_send_message_state(p.first, messageId, state);
      }
    }
  }

  void onMessageReceivedFromPeer(const char* peerId, const agora::rtm::IMessage* message) override {
    if (!message) {
      return;
    }
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_message_received_from_peer) {
        p.second.on_message_received_from_peer(p.first, peerId, &message);
      }
    }
  }
};

/*class CChannelEventHandler : public agora::rtm::IChannelEventHandler,
                             public agora::interop::CAgoraCallback<channel_event_handler> {
 public:
  CChannelEventHandler() = default;
  ~CChannelEventHandler() override = default;

  void onJoinSuccess() override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_join_success) {
        p.second.on_join_success(p.first);
      }
    }
  }

  void onJoinFailure(agora::rtm::JOIN_CHANNEL_ERROR errorCode) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_join_failure) {
        p.second.on_join_failure(p.first, errorCode);
      }
    }
  }

  void onLeave(agora::rtm::LEAVE_CHANNEL_REASON reason) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_leave) {
        p.second.on_leave(p.first, reason);
      }
    }
  }

  void onMessageReceived(const agora::rtm::IMessage* message) override {
    if (!message) {
      return;
    }
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_message_received) {
        p.second.on_message_received(p.first, &message);
      }
    }
  }

  void onSendMessageState(int64_t messageId, agora::rtm::CHANNEL_MESSAGE_STATE state) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_send_message_state) {
        p.second.on_send_message_state(p.first, messageId, state);
      }
    }
  }

  void onMemberJoined(agora::rtm::IChannelMember* member) override {
    if (!member) {
      return;
    }
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_member_joined) {
        p.second.on_member_joined(p.first, &member);
      }
    }
  }

  void onMemberLeft(agora::rtm::IChannelMember* member) override {
    if (!member) {
      return;
    }
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_member_left) {
        p.second.on_member_left(p.first, &member);
      }
    }
  }

  void onMembersGotten(agora::rtm::IChannelMember** members, int userCount) override {}

  void onAttributesUpdated(const agora::rtm::IChannelAttributes* attributes) override {
    if (!attributes) {
      return;
    }
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_attributes_updated) {
        p.second.on_attributes_updated(p.first, &attributes);
      }
    }
  }

  void onUpdateAttributesResponse(int64_t requestId, agora::rtm::RESPONSE_CODE resCode) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_update_attributes_response) {
        p.second.on_update_attributes_response(p.first, requestId, resCode);
      }
    }
  }

  void onAttributesDeleted(const agora::rtm::IChannelAttributes* attributes) override {
    if (!attributes) {
      return;
    }
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_attributes_deleted) {
        p.second.on_attributes_deleted(p.first, &attributes);
      }
    }
  }

  void onDeleteAttributesResponse(int64_t requestId, agora::rtm::RESPONSE_CODE resCode) override {
    auto obs = Clone();
    for (auto& p : obs) {
      if (p.second.on_delete_attributes_response) {
        p.second.on_delete_attributes_response(p.first, requestId, resCode);
      }
    }
  }
};*/

CRtmServiceEventHandler g_rtm_service_event_central_handler;
// CChannelEventHandler g_channel_event_central_handler;

}  // namespace

/**
 * The IMessage class.
 */
AGORA_API_C_HDL agora_rtm_message_create() { return agora::rtm::IMessage::createMessage(); }

AGORA_API_C_VOID agora_rtm_message_release(AGORA_HANDLE agora_rtm_message) {
  if (!agora_rtm_message) {
    return;
  }
  REINTER_CAST(message_ptr, agora::rtm::IMessage, agora_rtm_message);
  message_ptr->release();
  agora_rtm_message = nullptr;
}

AGORA_API_C int64_t AGORA_CALL_C agora_rtm_message_get_message_id(AGORA_HANDLE agora_rtm_message) {
  if (!agora_rtm_message) {
    return -1;
  }
  REINTER_CAST(message_ptr, agora::rtm::IMessage, agora_rtm_message);
  return message_ptr->getMessageId();
}

AGORA_API_C_INT agora_rtm_message_get_message_type(AGORA_HANDLE agora_rtm_message) {
  if (!agora_rtm_message) {
    return -1;
  }
  REINTER_CAST(message_ptr, agora::rtm::IMessage, agora_rtm_message);
  return message_ptr->getMessageType();
}

AGORA_API_C_VOID agora_rtm_message_set_text(AGORA_HANDLE agora_rtm_message, const char* str) {
  if (!agora_rtm_message) {
    return;
  }
  REINTER_CAST(message_ptr, agora::rtm::IMessage, agora_rtm_message);
  message_ptr->setText(str);
}

AGORA_API_C const char* AGORA_CALL_C agora_rtm_message_get_text(AGORA_HANDLE agora_rtm_message) {
  if (!agora_rtm_message) {
    return nullptr;
  }
  REINTER_CAST(message_ptr, agora::rtm::IMessage, agora_rtm_message);
  return message_ptr->getText();
}

/**
 * The IChannelMember class.
 */
/*AGORA_API_C const char* AGORA_CALL_C
agora_rtm_channel_member_get_member_id(AGORA_HANDLE agora_rtm_channel_member) {
  if (!agora_rtm_channel_member) {
    return nullptr;
  }
  REINTER_CAST(channel_member_ptr, agora::rtm::IChannelMember, agora_rtm_channel_member);
  return channel_member_ptr->getMemberId();
}*/

/*AGORA_API_C const char* AGORA_CALL_C
agora_rtm_channel_member_get_channel_id(AGORA_HANDLE agora_rtm_channel_member) {
  if (!agora_rtm_channel_member) {
    return nullptr;
  }
  REINTER_CAST(channel_member_ptr, agora::rtm::IChannelMember, agora_rtm_channel_member);
  return channel_member_ptr->getChannelId();
}*/

/*AGORA_API_C_VOID agora_rtm_channel_member_release(AGORA_HANDLE agora_rtm_channel_member) {
  if (!agora_rtm_channel_member) {
    return;
  }
  REINTER_CAST(channel_member_ptr, agora::rtm::IChannelMember, agora_rtm_channel_member);
  channel_member_ptr->release();
  agora_rtm_channel_member = nullptr;
}*/

/**
 * The IChannelAttributes class.
 */
/*AGORA_API_C_HDL agora_rtm_channel_attributes_create() {
  return agora::rtm::IChannelAttributes::createChannelAttributes();
}*/

/*AGORA_API_C_INT agora_rtm_channel_attributes_add_attribute(
    AGORA_HANDLE agora_rtm_channel_attributes, const char* key, const char* value) {
  if (!agora_rtm_channel_attributes) {
    return -1;
  }
  REINTER_CAST(channel_attribute_ptr, agora::rtm::IChannelAttributes, agora_rtm_channel_attributes);
  return channel_attribute_ptr->addAttribute(key, value);
}*/

/*AGORA_API_C_INT agora_rtm_channel_attributes_remove_attribute(
    AGORA_HANDLE agora_rtm_channel_attributes, const char* key) {
  if (!agora_rtm_channel_attributes) {
    return -1;
  }
  REINTER_CAST(channel_attribute_ptr, agora::rtm::IChannelAttributes, agora_rtm_channel_attributes);
  return channel_attribute_ptr->removeAttribute(key);
}*/

/*AGORA_API_C_INT agora_rtm_channel_attributes_get_attributes_size(
    AGORA_HANDLE agora_rtm_channel_attributes) {
  if (!agora_rtm_channel_attributes) {
    return -1;
  }
  REINTER_CAST(channel_attribute_ptr, agora::rtm::IChannelAttributes, agora_rtm_channel_attributes);
  return channel_attribute_ptr->getAttributesSize();
}*/

/*AGORA_API_C_VOID agora_rtm_channel_attributes_get_attributes(
    AGORA_HANDLE agora_rtm_channel_attributes, int size, char** key,
    char** value) {  // todo: discussion, how to traveral
  return;
}*/

/*AGORA_API_C const char* AGORA_CALL_C agora_rtm_channel_attributes_get_attribute_value(
    AGORA_HANDLE agora_rtm_channel_attributes, const char* key) {
  if (!agora_rtm_channel_attributes) {
    return nullptr;
  }
  REINTER_CAST(channel_attribute_ptr, agora::rtm::IChannelAttributes, agora_rtm_channel_attributes);
  return channel_attribute_ptr->getAttributeValue(key);
}*/

/*AGORA_API_C_INT agora_rtm_channel_attributes_release(AGORA_HANDLE agora_rtm_channel_attributes) {
  if (!agora_rtm_channel_attributes) {
    return -1;
  }
  REINTER_CAST(channel_attribute_ptr, agora::rtm::IChannelAttributes, agora_rtm_channel_attributes);
  return channel_attribute_ptr->release();
  agora_rtm_channel_attributes = nullptr;
}*/

/**
 * The IChannelEventHandler class. observor
 */

/**
 * The IChannel class.
 */
/*AGORA_API_C_VOID agora_rtm_channel_set_event_handler(AGORA_HANDLE agora_rtm_channel,
                                                     channel_event_handler* event_handler) {
  if (!agora_rtm_channel || !event_handler) {
    return;
  }
  g_channel_event_central_handler.Add(agora_rtm_channel, event_handler);
}*/

/*AGORA_API_C_INT agora_rtm_channel_join(AGORA_HANDLE agora_rtm_channel) {
  if (!agora_rtm_channel) {
    return -1;
  }
  REINTER_CAST(channel_ptr, agora::rtm::IChannel, agora_rtm_channel);
  return channel_ptr->join();
}*/

/*AGORA_API_C_INT agora_rtm_channel_leave(AGORA_HANDLE agora_rtm_channel) {
  if (!agora_rtm_channel) {
    return -1;
  }
  REINTER_CAST(channel_ptr, agora::rtm::IChannel, agora_rtm_channel);
  return channel_ptr->leave();
}*/

/*AGORA_API_C_INT agora_rtm_channel_send_message(AGORA_HANDLE agora_rtm_channel,
                                               AGORA_HANDLE agora_rtm_message) {
  if (!agora_rtm_channel || !agora_rtm_message) {
    return -1;
  }
  REINTER_CAST(channel_ptr, agora::rtm::IChannel, agora_rtm_channel);
  REINTER_CAST(message_ptr, agora::rtm::IMessage, agora_rtm_message);
  return channel_ptr->sendMessage(message_ptr);
}*/

/*AGORA_API_C_INT agora_rtm_channel_update_attributes(AGORA_HANDLE agora_rtm_channel,
                                                    AGORA_HANDLE agora_rtm_channel_attributes,
                                                    int64_t* request_id) {
  if (!agora_rtm_channel || !agora_rtm_channel_attributes || !request_id) {
    return -1;
  }
  REINTER_CAST(channel_ptr, agora::rtm::IChannel, agora_rtm_channel);
  REINTER_CAST(channel_attribute_ptr, agora::rtm::IChannelAttributes, agora_rtm_channel_attributes);
  return channel_ptr->updateAttributes(channel_attribute_ptr, *request_id);
}*/

/*AGORA_API_C_INT agora_rtm_channel_delete_attributes(AGORA_HANDLE agora_rtm_channel,
                                                    AGORA_HANDLE agora_rtm_channel_attributes,
                                                    int64_t* request_id) {
  if (!agora_rtm_channel || !agora_rtm_channel_attributes || !request_id) {
    return -1;
  }
  REINTER_CAST(channel_ptr, agora::rtm::IChannel, agora_rtm_channel);
  REINTER_CAST(channel_attribute_ptr, agora::rtm::IChannelAttributes, agora_rtm_channel_attributes);
  return channel_ptr->deleteAttributes(channel_attribute_ptr, *request_id);
}*/

/*AGORA_API_C const char* AGORA_CALL_C agora_rtm_channel_get_id(AGORA_HANDLE agora_rtm_channel) {
  if (!agora_rtm_channel) {
    return nullptr;
  }
  REINTER_CAST(channel_ptr, agora::rtm::IChannel, agora_rtm_channel);
  return channel_ptr->getId();
}*/

/*AGORA_API_C_INT agora_rtm_channel_release(AGORA_HANDLE agora_rtm_channel) {
  if (!agora_rtm_channel) {
    return -1;
  }
  REINTER_CAST(channel_ptr, agora::rtm::IChannel, agora_rtm_channel);
  agora_rtm_channel = nullptr;
  return channel_ptr->release();
}*/

/**
 * The IRtmService class.
 */

AGORA_API_C_INT agora_rtm_service_initialize(AGORA_HANDLE agora_rtm_service, const char* app_id,
                                             rtm_service_event_handler* event_handler) {
  if (!agora_rtm_service || !event_handler) {
    return -1;
  }
  REINTER_CAST(agora_rtm_service_ptr, agora::rtm::IRtmService, agora_rtm_service);
  REINTER_CAST(rtm_service_event_handler_ptr, agora::rtm::IRtmServiceEventHandler, event_handler);
  g_rtm_service_event_central_handler.Add(agora_rtm_service, event_handler);
  return agora_rtm_service_ptr->initialize(app_id, rtm_service_event_handler_ptr);
}

AGORA_API_C_VOID agora_rtm_service_unregister_observer(AGORA_HANDLE agora_rtm_service,
                                                       rtm_service_event_handler* event_handler) {
  if (!agora_rtm_service || !event_handler) {
    return;
  }
  g_rtm_service_event_central_handler.Remove(event_handler);
}

AGORA_API_C_INT agora_rtm_service_release(AGORA_HANDLE agora_rtm_service, int sync) {
  if (!agora_rtm_service) {
    return -1;
  }
  REINTER_CAST(agora_rtm_service_ptr, agora::rtm::IRtmService, agora_rtm_service);
  agora_rtm_service = nullptr;
  return agora_rtm_service_ptr->release(sync);
}

AGORA_API_C_INT agora_rtm_service_login(AGORA_HANDLE agora_rtm_service, const char* token,
                                        const char* user_id) {
  if (!agora_rtm_service) {
    return -1;
  }
  REINTER_CAST(agora_rtm_service_ptr, agora::rtm::IRtmService, agora_rtm_service);
  return agora_rtm_service_ptr->login(token, user_id);
}

AGORA_API_C_INT agora_rtm_service_logout(AGORA_HANDLE agora_rtm_service) {
  if (!agora_rtm_service) {
    return -1;
  }
  REINTER_CAST(agora_rtm_service_ptr, agora::rtm::IRtmService, agora_rtm_service);
  return agora_rtm_service_ptr->logout();
}

AGORA_API_C_INT agora_rtm_service_send_message_to_peer(AGORA_HANDLE agora_rtm_service,
                                                       const char* peer_id,
                                                       AGORA_HANDLE agora_rtm_message) {
  if (!agora_rtm_service || !agora_rtm_message) {
    return -1;
  }
  REINTER_CAST(agora_rtm_service_ptr, agora::rtm::IRtmService, agora_rtm_service);
  REINTER_CAST(message_ptr, agora::rtm::IMessage, agora_rtm_message);
  return agora_rtm_service_ptr->sendMessageToPeer(peer_id, message_ptr);
}

/*AGORA_API_C_HDL agora_rtm_service_create_channel(AGORA_HANDLE agora_rtm_service,
                                                 const char* channel_id,
                                                 channel_event_handler* eventHandler) {
  if (!agora_rtm_service) {
    return nullptr;
  }
  REINTER_CAST(agora_rtm_service_ptr, agora::rtm::IRtmService, agora_rtm_service);
  REINTER_CAST(channel_event_handler_ptr, agora::rtm::IChannelEventHandler, eventHandler);
  return agora_rtm_service_ptr->createChannel(channel_id, channel_event_handler_ptr);
}*/
