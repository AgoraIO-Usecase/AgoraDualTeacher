//
//  Agora C SDK
//
//  Created by Hugo Chan in 2020.7
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include <stdint.h>
#include "agora_api.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus



/**
 * @ANNOTATION:GROUP:agora_parameter
 */
AGORA_API_C_INT agora_parameter_set_int(AGORA_HANDLE agora_parameter, const char* key, int value);

/**
 * @ANNOTATION:GROUP:agora_parameter
 */
AGORA_API_C_INT agora_parameter_set_uint(AGORA_HANDLE agora_parameter, const char* key, unsigned int value);

/**
 * @ANNOTATION:GROUP:agora_parameter
 */
AGORA_API_C_INT agora_parameter_set_number(AGORA_HANDLE agora_parameter, const char* key, double value);

/**
 * @ANNOTATION:GROUP:agora_parameter
 */
AGORA_API_C_INT agora_parameter_set_string(AGORA_HANDLE agora_parameter, const char* key, const char* value);

/**
 * @ANNOTATION:GROUP:agora_parameter
 */
AGORA_API_C_INT agora_parameter_set_array(AGORA_HANDLE agora_parameter, const char* key, const char* json_src);

/**
 * @ANNOTATION:GROUP:agora_parameter
 */
AGORA_API_C_INT agora_parameter_set_parameters(AGORA_HANDLE agora_parameter, const char* json_src);

/**
 * @ANNOTATION:GROUP:agora_parameter
 * @ANNOTATION:OUT:value
 */
AGORA_API_C_INT agora_parameter_get_int(AGORA_HANDLE agora_parameter, const char* key, int* value);

/**
 * @ANNOTATION:GROUP:agora_parameter
 * @ANNOTATION:OUT:value
 */
AGORA_API_C_INT agora_parameter_get_uint(AGORA_HANDLE agora_parameter, const char* key, unsigned int* value);

/**
 * @ANNOTATION:GROUP:agora_parameter
 * @ANNOTATION:OUT:value
 */
AGORA_API_C_INT agora_parameter_get_number(AGORA_HANDLE agora_parameter, const char* key, double* value);

/**
 * @ANNOTATION:GROUP:agora_parameter
 * @ANNOTATION:OUT:value:value_size
 */
AGORA_API_C_INT agora_parameter_get_string(AGORA_HANDLE agora_parameter, const char* key, char* value, uint32_t* value_size);

/**
 * @ANNOTATION:GROUP:agora_parameter
 * @ANNOTATION:OUT:value:value_size
 */
AGORA_API_C_INT agora_parameter_get_array(AGORA_HANDLE agora_parameter, const char* key, const char* json_src, char* value, uint32_t* value_size);

// TODO: inconsistent with get array?
/**
 * @ANNOTATION:GROUP:agora_parameter
 * @ANNOTATION:OUT:value:value_size
 */
AGORA_API_C_INT agora_parameter_get_object(AGORA_HANDLE agora_parameter, const char* key, char* value, uint32_t* value_size);

/**
 * @ANNOTATION:GROUP:agora_parameter
 * @ANNOTATION:OUT:value:value_size
 */
AGORA_API_C_INT agora_parameter_convert_path(AGORA_HANDLE agora_parameter, const char* file_path, char* value, uint32_t* value_size);



#ifdef __cplusplus
}
#endif // __cplusplus