//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include "agora_base.h"
#include "agora_service.h"
#include "agora_rtc_conn.h"

#ifdef __cplusplus
extern "C"{
#endif

/*
 * @ANNOTATION:TYPE:OBSERVER
 */
typedef struct _rtmp_streaming_observer
{
    void (*on_rtmp_streaming_state_changed)(AGORA_HANDLE agora_rtmp_streaming_service, const char* url, int state,
                                           int err_code);
    
    void (*on_stream_published)(AGORA_HANDLE agora_rtmp_streaming_service, const char* url, int error);
  
    void (*on_stream_unpublished)(AGORA_HANDLE agora_rtmp_streaming_service, const char* url);
    void (*on_transcoding_updated)(AGORA_HANDLE agora_rtmp_streaming_service);

}rtmp_streaming_observer;


/**
 * @ANNOTATION:GROUP:agora_rtmp_streaming_service
 */
AGORA_API_C_INT agora_rtmp_streaming_service_add_publish_stream_url(AGORA_HANDLE agora_rtmp_streaming_service, const char* url, int transcoding_enabled);
 
/**
 * @ANNOTATION:GROUP:agora_rtmp_streaming_service
 */
AGORA_API_C_INT agora_rtmp_streaming_service_remove_publish_stream_url(AGORA_HANDLE agora_rtmp_streaming_service, const char* url);

/**
 * @ANNOTATION:GROUP:agora_rtmp_streaming_service
 */
AGORA_API_C_INT agora_rtmp_streaming_service_set_live_transcoding(AGORA_HANDLE agora_rtmp_streaming_service, const live_transcoding* transcoding);
 
/**
 * @ANNOTATION:GROUP:agora_rtmp_streaming_service
 */
AGORA_API_C_INT agora_rtmp_streaming_service_register_observer(AGORA_HANDLE agora_rtmp_streaming_service, rtmp_streaming_observer* observer);
 

/**
 * @ANNOTATION:GROUP:agora_rtmp_streaming_service
 */
AGORA_API_C_INT agora_rtmp_streaming_service_unregister_observer(AGORA_HANDLE agora_rtmp_streaming_service, rtmp_streaming_observer* observer);



#ifdef __cplusplus
}
#endif
