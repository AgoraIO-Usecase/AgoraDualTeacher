//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once


namespace agora { namespace signaling {

enum class SIGNALING_EVENT
{
	NATIVE_LOG = 100,
	ERROR_EVENT = 101,
	WARNING_EVENT = 102,
    
    API_CALL_EXECUTED = 1000,
    PEER_JOIN_CHANNEL = 1001,
    PEER_LEAVE_CHANNEL = 1002,
    CHANNEL_PEER_LIST_UPDATED = 1003,
    CHANNEL_ATTRIBUTES_UPDATED = 1004,
    CONNECTION_LOST = 1005,
    CONNECTION_INTERRUPTED = 1006,
    CONNECTION_RESTORED = 1007,
};

}}
