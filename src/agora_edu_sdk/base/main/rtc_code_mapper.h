/*
* Copyright (c) 2017 Agora.io
* All rights reserved.
* Proprietary and Confidential - Agora.io
*/
/*
* Yongli Wang, 2017-10
*/

#ifndef AGORA_RTC_CODE_MAPPER_H
#define AGORA_RTC_CODE_MAPPER_H

#include "IAgoraRtcEngine.h"
#include <unordered_map>
namespace agora{
    namespace rtc{

class AgoraCodeMapper
{
public:
    static bool getMappedErrorCode(ERROR_CODE_TYPE err, ERROR_CODE_TYPE& errorCode);
    static bool getMappedWarningCode(WARN_CODE_TYPE warn, WARN_CODE_TYPE& warnCode);
private:
    using error_map_type = std::unordered_map<ERROR_CODE_TYPE, ERROR_CODE_TYPE, std::hash<std::underlying_type<ERROR_CODE_TYPE>::type>>;
    using warn_map_type = std::unordered_map<WARN_CODE_TYPE, WARN_CODE_TYPE, std::hash<std::underlying_type<WARN_CODE_TYPE>::type>>;
    static const error_map_type s_errMap;
    static const warn_map_type s_warnMap;
};
    }
}
#endif

