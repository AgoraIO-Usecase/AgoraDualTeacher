//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once
#include "agora_base.h"
#include "agora_media_base.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/*
 * @ANNOTATION:TYPE:OBSERVER
 */
typedef struct _agora_media_player_source_observer {

    void (*on_player_source_state_changed)(AGORA_HANDLE agora_media_player_source, int state,
                                           int ec);
    void (*on_position_changed)(AGORA_HANDLE agora_media_player_source, int64_t position);

    void (*on_player_event)(AGORA_HANDLE agora_media_player_source, int event);

    void (*on_meta_data)(AGORA_HANDLE agora_media_player_source, const void* data, int length);

    void (*on_completed)(AGORA_HANDLE agora_media_player_source);

}agora_media_player_source_observer;

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_get_source_id(AGORA_HANDLE agora_media_player_source);  

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_open(AGORA_HANDLE agora_media_player_source, const char* url, int64_t start_pos);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_play(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_pause(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_stop(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_resume(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_seek(AGORA_HANDLE agora_media_player_source, int64_t new_pos);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C int64_t AGORA_CALL_C agora_media_player_source_get_duration(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C int64_t AGORA_CALL_C agora_media_player_source_get_play_position(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C int64_t AGORA_CALL_C agora_media_player_source_get_stream_count(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C media_stream_info* AGORA_CALL_C agora_media_player_source_get_stream_info(AGORA_HANDLE agora_media_player_source, int64_t index);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_VOID agora_media_player_source_destroy_stream_info(AGORA_HANDLE agora_media_player_source, media_stream_info* info);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_set_loop_count(AGORA_HANDLE agora_media_player_source, int loop_count);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_change_playback_speed(AGORA_HANDLE agora_media_player_source, int speed);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_select_audio_track(AGORA_HANDLE agora_media_player_source, int index);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_set_player_option(AGORA_HANDLE agora_media_player_source, const char* key, int value);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_take_screenshot(AGORA_HANDLE agora_media_player_source, const char* filename);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_select_internal_subtitle(AGORA_HANDLE agora_media_player_source, int index);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_set_external_subtitle(AGORA_HANDLE agora_media_player_source, const char* url);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_get_state(AGORA_HANDLE agora_media_player_source);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_register_player_source_observer(AGORA_HANDLE agora_media_player_source, agora_media_player_source_observer* observer);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_unregister_player_source_observer(AGORA_HANDLE agora_media_player_source, agora_media_player_source_observer* observer);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_register_audio_frame_observer(AGORA_HANDLE agora_media_player_source, audio_frame_observer* observer);

/**
 * @ANNOTATION:GROUP:agora_media_player_source
 */
AGORA_API_C_INT agora_media_player_source_unregister_audio_frame_observer(AGORA_HANDLE agora_media_player_source, audio_frame_observer* observer);


#ifdef __cplusplus
}
#endif // __cplusplus
