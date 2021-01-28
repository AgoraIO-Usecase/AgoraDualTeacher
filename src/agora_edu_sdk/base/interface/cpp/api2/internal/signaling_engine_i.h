//
//  Agora Media SDK
//
//  Created by Sting Feng in 2015-05.
//  Copyright (c) 2015 Agora IO. All rights reserved.
//
#pragma once
#include <functional>
#include <string>
#include <IAgoraSignalingEngine.h>
#include <signaling/signaling_event.h>

namespace agora {
    namespace commons {
        namespace cjson {
            class JsonWrapper;
        }
    }
    typedef agora::commons::cjson::JsonWrapper any_document_t;

namespace signaling {

class ISignalingEngineEventHandlerEx : public ISignalingEngineEventHandler
{
public:
    virtual bool onEvent(SIGNALING_EVENT evt, std::string* payload) {
        (void)evt;
        (void)payload;

        /* return false to indicate this event is not handled */
        return false;
    }
};

class ISignalingEngineEx : public ISignalingEngine
{
public:
    virtual int setParameters(const char* parameters) = 0;
    virtual int getParameters(const char* key, any_document_t& result) = 0;
};

}}
