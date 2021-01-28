//
//  Agora SDK
//
//  Copyright (c) 2018 Agora.io. All rights reserved.
//

#include "device_profile.h"

#include <stdlib.h>
#include <string.h>
#if defined(WEBRTC_IOS)
#include <TargetConditionals.h>
#endif

namespace agora {
namespace rtc {

struct profile_t {
  const char* deviceId;
  const char* profile;
};

// Manufacture/Model/Product/Device/Sdk Version/OS Version
static const profile_t g_profiles[] = {
#if defined(WEBRTC_ANDROID)
    // ---------------------------------------------------------------NEXUS
    // START------------------------------------------------------
    {"lge/nexus 5",
     "{\"audioEngine\":{\"magicId\":1, \"useOpensl\":false, \"playoutBufferLength\":80,  "
     "\"recordingDevice\":7, \"AESforSpeaker\": false, \"bssOn\":false, \"targetAngle\":90.0, "
     "\"micSpacing\":13.0}}"},
    {"lge/aosp on hammerhead",
     "{\"audioEngine\":{\"magicId\":1, \"useOpensl\":false, \"playoutBufferLength\":80,  "
     "\"recordingDevice\":7, \"AESforSpeaker\": false, \"bssOn\":false, \"targetAngle\":90.0, "
     "\"micSpacing\":13.0}}"},
    {"lge/nexus 4",
     "{\"audioEngine\":{\"magicId\":201, \"useOpensl\":false, \"playoutBufferLength\":80,  "
     "\"recordingDevice\":7, \"audioSampleRate\":16000, \"useBuiltinAEC\":true, "
     "\"AESforSpeaker\": false}}"},
    {"lge/nexus 5x",
     "{\"audioEngine\":{\"magicId\":202, \"useOpensl\":false, \"audioSampleRate\":16000,  "
     "\"recordingDevice\":7, \"AESforSpeaker\": false}}"},
    {"huawei/nexus 6p",
     "{\"audioEngine\":{\"magicId\":203, \"useOpensl\":false, \"audioMode\":3, "
     "\"AESforSpeaker\": false}}"},
    {"motorola/moto e (4)",
     "{\"audioEngine\":{\"magicId\":204, \"useOpensl\":false, \"AESforSpeaker\": true, "
     "\"recordingDevice\":7, \"audioSampleRate\":16000}}"},
    {"google/pixel 2",
     "{\"audioEngine\":{\"magicId\":205, \"useOpensl\":false, \"AESforSpeaker\": "
     "false,\"audioMode\":3,\"recordingDevice\":1, \"audioSampleRate\":16000}}"},

    // ---------------------------------------------------------------NEXUS
    // END------------------------------------------------------

    // -------------------------------------------------------------XIAOMI
    // BEGIN-----------------------------------------------------
    // ********devices using system APM solution *********
    // Mi5, Mi4, Mi4i, Mi4c (same delay offset works well) "/mi 4i"
    {"xiaomi/mi 4",
     "{\"audioEngine\":{\"magicId\":2,  \"useOpensl\":false,\"playoutBufferLength\":60 ,  "
     "\"recordingDevice\":7, \"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": "
     "false}}"},
    {"xiaomi/mi-4",
     "{\"audioEngine\":{\"magicId\":2,  \"useOpensl\":false,\"playoutBufferLength\":60 ,  "
     "\"recordingDevice\":7, \"audioSampleRate\":16000, \"javaOutMiniBufferMs\":200, "
     "\"javaInMiniBufferMs\":0, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"xiaomi/mi 5",
     "{\"audioEngine\":{\"magicId\":25,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": false, \"AESforSpeaker\": false}}"},
    {"xiaomi/mi-5",
     "{\"audioEngine\":{\"magicId\":25,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Mi3 CT,CU
    {"xiaomi/mi 3w",
     "{\"audioEngine\":{\"magicId\":3, \"useOpensl\":false,\"recordingDevice\":7,  "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"xiaomi/mi 3c",
     "{\"audioEngine\":{\"magicId\":3, \"useOpensl\":false, \"recordingDevice\":7,  "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Mi3 CMCC
    {"xiaomi/mi 3",
     "{\"audioEngine\":{\"magicId\":4,  \"useOpensl\":false, \"recordingDevice\":7,  "
     "\"audioSampleRate\":16000, \"hardwareAAC\": false, \"AESforSpeaker\": false}}"},
    // MiNote
    {"xiaomi/mi note",
     "{\"audioEngine\":{\"magicId\":5, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // MiPad
    {"xiaomi/mi pad",
     "{\"audioEngine\":{\"magicId\":16, \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hardwareAAC\": false, \"AESforSpeaker\": false}}"},
    // HMNote 1 LTE TD/W
    {"xiaomi/hm note 1lte",
     "{\"audioEngine\":{\"magicId\":15, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"playoutBufferLength\":120,  \"audioSampleRate\":16000, \"hwAPM\": true, "
     "\"AESforSpeaker\": false}}"},
    {"xiaomi/mi note pro",
     "{\"audioEngine\":{\"magicId\":22, \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"xiaomi/mi max",
     "{\"audioEngine\":{\"magicId\":26, \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},

    // *******rest Mi series***************
    // MI2S, 2SC, 2A
    {"xiaomi/mi 2s",
     "{\"audioEngine\":{\"magicId\":6, \"useOpensl\":false, \"recordingDevice\":7,  "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // MI2 MI2C
    {"xiaomi/mi 2",
     "{\"audioEngine\":{\"magicId\":7, \"useOpensl\":false, \"recordingDevice\":7,   "
     "\"audioSampleRate\":16000,  \"AESforSpeaker\": false}}"},
    // Mi2A
    {"xiaomi/mi 2a",
     "{\"audioEngine\":{\"magicId\":18, \"useOpensl\":false,  \"agcOn\":false,  "
     "\"recordingDevice\":7, \"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": "
     "false}}"},
    // Mi2C
    {"xiaomi/mi 2c",
     "{\"audioEngine\":{\"magicId\":19, \"useOpensl\":false,   \"recordingDevice\":7,   "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Mi1 plus/c1  Mi1s / Mi1sc
    {"xiaomi/mi-one",
     "{\"audioEngine\":{\"magicId\":8, \"useOpensl\":false, \"agcOn\":false, "
     "\"recordingDevice\":7, \"audioMode\":2, \"audioSampleRate\":16000, \"AESforSpeaker\": "
     "false}}"},
    {"xiaomi/mi 1s",
     "{\"audioEngine\":{\"magicId\":8, \"useOpensl\":false,  \"agcOn\":false, "
     "\"recordingDevice\":7, \"audioMode\":2, \"audioSampleRate\":16000, \"AESforSpeaker\": "
     "false}}"},
    {"xiaomi/mi note 3", "{\"audioEngine\":{\"magicId\":5001}}"},

    // **********HM series*********************
    // HM1 2013022/2013023
    {"xiaomi/2013",
     "{\"audioEngine\":{\"magicId\":9, \"useOpensl\":false, \"playoutBufferLength\":100, "
     "\"recordingDevice\":1, \"audioMode\":0, \"AESforSpeaker\": false}}"},  // OpenSL, mode
                                                                             // normal
                                                                             // HM 1S C/W
    {"xiaomi/hm 1s",
     "{\"audioEngine\":{\"magicId\":10, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"hwAPM\": true,\"AESforSpeaker\": false}}"},
    // HM1S LTE 2014501
    {"xiaomi/20145",
     "{\"audioEngine\":{\"magicId\":11,  \"useOpensl\":false, \"playoutBufferLength\":100, "
     "\"recordingDevice\":1, \"audioMode\":0, \"AESforSpeaker\": false}}"},  // mode normal
                                                                             // HM1S TD 2014011
    {"xiaomi/20140",
     "{\"audioEngine\":{\"magicId\":21,  \"useOpensl\":false,  \"playoutBufferLength\":100, "
     "\"recordingDevice\":1, \"audioMode\":0, \"AESforSpeaker\": false}}"},  // mode normal
                                                                             // HM2
    {"xiaomi/20148",
     "{\"audioEngine\":{\"magicId\":12,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"playoutBufferLength\":100, \"audioSampleRate\":16000,  \"hwAPM\": true,  "
     "\"AESforSpeaker\": false}}"},  // OpenSL is better
                                     // HM2A
    {"xiaomi/hm 2a",
     "{\"audioEngine\":{\"magicId\":13,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"playoutBufferLength\":100, \"AESforSpeaker\": false}}"},  // Java
                                                                  // HM3
    {"xiaomi/redmi 3",
     "{\"audioEngine\":{\"magicId\":24, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // HMNote 1 TD/W
    {"xiaomi/hm note 1",
     "{\"audioEngine\":{\"magicId\":14, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"AESforSpeaker\": true}}"},
    // HMNote 1s
    {"xiaomi/hm note 1s",
     "{\"audioEngine\":{\"magicId\":17,  \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"recordingDevice\":7,  \"AESforSpeaker\": false}}"},  // SW
                                                             // HMNote 2
    {"xiaomi/redmi note 2",
     "{\"audioEngine\":{\"magicId\":20,  \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},  // SW
    {"xiaomi/redmi note 3",
     "{\"audioEngine\":{\"magicId\":23,  \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},
    // Mi6,7,8
    {"xiaomi/mi 6",
     "{\"audioEngine\":{\"magicId\":27,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"javaOutMiniBufferMs\":200, "
     "\"javaInMiniBufferMs\":0,\"AESforSpeaker\": false}}"},
    {"xiaomi/mi-6",
     "{\"audioEngine\":{\"magicId\":27,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"xiaomi/mi 7",
     "{\"audioEngine\":{\"magicId\":28,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"xiaomi/mi-7",
     "{\"audioEngine\":{\"magicId\":28,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"xiaomi/mi 8",
     "{\"audioEngine\":{\"magicId\":28,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"xiaomi/mi-8",
     "{\"audioEngine\":{\"magicId\":28,  \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // unknown xiaomi  devices
    {"xiaomi/",
     "{\"audioEngine\":{\"magicId\":29,  \"useOpensl\":false,  \"recordingDevice\":7,  "
     "\"audioSampleRate\":16000,  \"AESforSpeaker\": false}}"},  // SW
                                                                 // -------------------------------------------------------------XIAOMI
                                                                 // END-----------------------------------------------------[2
                                                                 // -29]

    // -------------------------------------------------------------SAMSUNG BEGIN
    // ------------------------------------------------------ [30 - 45]
    // Note 2 N7100 N7102 N7105 N7108 N719
    {"samsung/gt-n71",
     "{\"audioEngine\":{\"magicId\":30, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hardwareAAC\": false, \"AESforSpeaker\": false}}"},
    {"samsung/sch-n71",
     "{\"audioEngine\":{\"magicId\":30, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hardwareAAC\": false, \"AESforSpeaker\": false}}"},
    {"samsung/sm-n71",
     "{\"audioEngine\":{\"magicId\":30, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hardwareAAC\": false, \"AESforSpeaker\": false}}"},
    // {"samsung/sm-n71", "{\"audioEngine\":{\"magicId\":30, \"useOpensl\":false,
    // \"recordingDevice\":6, \"audioMode\":0, \"AESforSpeaker\": false}}"},
    // Note 3 N9006 9002 9008 9009
    {"samsung/sm-n75",
     "{\"audioEngine\":{\"magicId\":31, \"useOpensl\":false,\"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sm-n90",
     "{\"audioEngine\":{\"magicId\":31,  \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Galaxy S3 US/CN
    {"samsung/sch-i53",
     "{\"audioEngine\":{\"magicId\":32, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sch-j",
     "{\"audioEngine\":{\"magicId\":32, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,  \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sch-l",
     "{\"audioEngine\":{\"magicId\":32, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,  \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/gt-i93",
     "{\"audioEngine\":{\"magicId\":32, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Galaxy S4 / S4 Mini (HW solution is controlled by built-in AEC)
    {"samsung/sch-i54",
     "{\"audioEngine\":{\"magicId\":33, \"useOpensl\":false, "
     "\"agcOn\":false,\"audioSampleRate\":16000, \"useBuiltinAEC\":true,  \"AESforSpeaker\": "
     "false}}"},
    {"samsung/sch-i95",
     "{\"audioEngine\":{\"magicId\":33, \"useOpensl\":false,  "
     "\"agcOn\":false,\"audioSampleRate\":16000, \"useBuiltinAEC\":true, \"AESforSpeaker\": "
     "false}}"},
    {"samsung/gt-i95",
     "{\"audioEngine\":{\"magicId\":33, \"useOpensl\":false,   "
     "\"agcOn\":false,\"audioSampleRate\":16000, \"useBuiltinAEC\":true,  \"AESforSpeaker\": "
     "false}}"},
    {"samsung/gt-i91",
     "{\"audioEngine\":{\"magicId\":33, \"useOpensl\":false, "
     "\"agcOn\":false,\"audioSampleRate\":16000, \"useBuiltinAEC\":true,  \"AESforSpeaker\": "
     "false}}"},
    // Galaxy S5 G9006, 9009 9008 9098, 900H
    {"samsung/sm-g90",
     "{\"audioEngine\":{\"magicId\":34,  \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Note 3 Neo (N750, 7502 7505 7507)
    {"samsung/sm-n70",
     "{\"audioEngine\":{\"magicId\":35,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Note 4 (N910)
    {"samsung/sm-n910",
     "{\"audioEngine\":{\"magicId\":36, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"AESforSpeaker\": false}}"},
    {"samsung/sm-n916",
     "{\"audioEngine\":{\"magicId\":36, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"AESforSpeaker\": false}}"},
    // Note Galaxy Edge (N915)
    {"samsung/sm-n915",
     "{\"audioEngine\":{\"magicId\":37,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Note Tablet (P60x, P90x)
    {"samsung/sm-p",
     "{\"audioEngine\":{\"magicId\":38,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/gt-n5",
     "{\"audioEngine\":{\"magicId\":38,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/gt-n70",
     "{\"audioEngine\":{\"magicId\":38,\"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/gt-n80",
     "{\"audioEngine\":{\"magicId\":38,\"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Galaxy Grand 2 (G7106, G7108) and more
    {"samsung/sm-g71",
     "{\"audioEngine\":{\"magicId\":39,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"AESforSpeaker\": false}}"},
    // Tab Tablet (T330 T530 etc.)
    {"samsung/sm-t",
     "{\"audioEngine\":{\"magicId\":40,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/gt-p",
     "{\"audioEngine\":{\"magicId\":40,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Galaxy S6 / S6 Edge
    {"samsung/sm-g92",
     "{\"audioEngine\":{\"magicId\":41,\"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sc-04",
     "{\"audioEngine\":{\"magicId\":41,\"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Galaxy A3/A5/A7 (started from Dec 2014)
    {"samsung/sm-a3",
     "{\"audioEngine\":{\"magicId\":42,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sm-a5",
     "{\"audioEngine\":{\"magicId\":42,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sm-a7",
     "{\"audioEngine\":{\"magicId\":42,\"useOpensl\":false,  \"audioSampleRate\":8000, "
     "\"audioMode\":2, \"hwAPM\": false, \"AESforSpeaker\": false}}"},
    {"samsung/sm-a8",
     "{\"audioEngine\":{\"magicId\":811,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Galaxy Win/Core, Grand Neo / S Advance / SII Plus etc, and other variants
    {"samsung/sm-j",
     "{\"audioEngine\":{\"magicId\":43,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sm-c",
     "{\"audioEngine\":{\"magicId\":43, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sm-e",
     "{\"audioEngine\":{\"magicId\":43, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/gt-i8",
     "{\"audioEngine\":{\"magicId\":43, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/gt-s",
     "{\"audioEngine\":{\"magicId\":43, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Japan versions , Note2 Note3 S3 S5 S6 etc.
    {"samsung/sm-j",
     "{\"audioEngine\":{\"magicId\":44,\"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sc-0",
     "{\"audioEngine\":{\"magicId\":44, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/scl2",
     "{\"audioEngine\":{\"magicId\":44, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Korea Versions, SII, SIII etc.
    {"samsung/shv",
     "{\"audioEngine\":{\"magicId\":44, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/shw",
     "{\"audioEngine\":{\"magicId\":44, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // US Versions, SIII, SIV etc.
    {"samsung/samsung-sgh-",
     "{\"audioEngine\":{\"magicId\":44,\"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true,\"AESforSpeaker\": false}}"},
    {"samsung/samsung-sm-",
     "{\"audioEngine\":{\"magicId\":44, \"useOpensl\":false,   \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,   \"hwAPM\": true,\"AESforSpeaker\": false}}"},
    {"samsung/sgh-",
     "{\"audioEngine\":{\"magicId\":44, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,  \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"samsung/sch-",
     "{\"audioEngine\":{\"magicId\":44,  \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true,  \"AESforSpeaker\": false}}"},
    {"samsung/sph-",
     "{\"audioEngine\":{\"magicId\":44, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,  \"hwAPM\": true, \"AESforSpeaker\": false}}"},

    // Galaxy S (use default sampling rate instead of 16KHz)
    {"samsung/gt-i9000",
     "{\"audioEngine\":{\"magicId\":801, \"useOpensl\":false,  \"agcOn\":false, "
     "\"recordingDevice\":6, \"audioMode\":0,\"useBuiltinAEC\":true,  \"AESforSpeaker\": "
     "false}}"},
    {"samsung/shw-m110s",
     "{\"audioEngine\":{\"magicId\":801, \"useOpensl\":false,  \"agcOn\":false, "
     "\"recordingDevice\":6, \"audioMode\":0,\"useBuiltinAEC\":true, \"AESforSpeaker\": "
     "false}}"},
    // GT-9100 Galaxy SII
    {"samsung/gt-i9100",
     "{\"audioEngine\":{\"magicId\":802, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // GT-I9082i Grand DUOS / corei
    {"samsung/gt-i8262",
     "{\"audioEngine\":{\"magicId\":803, \"useOpensl\":false,  \"agcOn\":false, "
     "\"audioSampleRate\":16000, \"useBuiltinAEC\":true, \"hwAPM\": true, \"hardwareMp3\": "
     "false, \"AESforSpeaker\": false}}"},
    // GT-S7582L Galaxy S Duos 2
    {"samsung/gt-s7582",
     "{\"audioEngine\":{\"magicId\":804, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // GT-S7562 Galaxy Trend Duos 8KHz bandwidth
    {"samsung/gt-s7562",
     "{\"audioEngine\":{\"magicId\":805, \"useOpensl\":false, \"useBuiltinAEC\":true, "
     "\"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // C7000
    {"samsung/sm-c7000",
     "{\"audioEngine\":{\"magicId\":806, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    // GT-i9200 galaxy mega
    {"samsung/gt-i9200",
     "{\"audioEngine\":{\"magicId\":807, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // S4 Canada version, SGH-i337m
    {"samsung/sgh-i337m",
     "{\"audioEngine\":{\"magicId\":808, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,  \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Note3 Intl version, builtin aec not working well
    {"samsung/sm-n900", "{\"audioEngine\":{\"magicId\":809, \"AESforSpeaker\": true}}"},
    // Galaxy S7
    {"samsung/sm-g93", "{\"audioEngine\":{\"magicId\":810}}"},

    // Low-end or unknown Devices, keep AEC
    {"samsung/",
     "{\"audioEngine\":{\"magicId\":45, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // -------------------------------------------------------------SAMSUNG END
    // ------------------------------------------------------ [30 - 45£¬800 - 850]

    // ---------------------------------------------------------------HUAWEI BEGIN
    // -------------------------------------------------------[46 - 60, 300 - 350]
    // Huawei Honor 3X
    {"huawei/huawei g750",
     "{\"audioEngine\":{\"magicId\":46,  \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"recordingDevice\":7, \"AESforSpeaker\": false}}"},
    // Huawei Mate 2
    {"huawei/huawei mt2",
     "{\"audioEngine\":{\"magicId\":47, \"hwAPM\": true, \"useOpensl\":true, "
     "\"recordingDevice\":7, \"AESforSpeaker\": true}}"},
    // Huawei Ascend G700
    {"huawei/huawei g700",
     "{\"audioEngine\":{\"magicId\":48, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    // Huawei Honor 2
    {"huawei/huawei u9508",
     "{\"audioEngine\":{\"magicId\":49, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    // Huawei Maimang 1/2  (A199, B199)
    {"huawei/huawei a199",
     "{\"audioEngine\":{ \"useOpensl\":false,  \"recordingDevice\":7, \"useBuiltinAEC\":true, "
     "\"AESforSpeaker\": false}}"},
    {"huawei/huawei b199",
     "{\"audioEngine\":{ \"useOpensl\":false,  \"recordingDevice\":7, \"useBuiltinAEC\":true, "
     "\"AESforSpeaker\": false}}"},
    // Huawei Maimang 3S (C199)
    {"huawei/huawei c199",
     "{\"audioEngine\":{\"magicId\":50,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"useBuiltinAEC\":true, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Huawei Honor 3C (H30-T10, H30-U10, Hol-U10)
    {"huawei/h30",
     "{\"audioEngine\":{\"magicId\":51, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"recordingDevice\":7, \"AESforSpeaker\": false}}"},
    {"huawei/honor h30",
     "{\"audioEngine\":{\"magicId\":51,  \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"recordingDevice\":7, \"AESforSpeaker\": false}}"},
    {"huawei/huawei h30",
     "{\"audioEngine\":{\"magicId\":51, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"recordingDevice\":7, \"AESforSpeaker\": false}}"},
    {"huawei/hol-u1",
     "{\"audioEngine\":{\"magicId\":51, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"recordingDevice\":7, \"AESforSpeaker\": false}}"},  // u10, u19
    {"huawei/hol-t00",
     "{\"audioEngine\":{\"magicId\":51,  \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"recordingDevice\":7, \"AESforSpeaker\": false}}"},
    {"huawei/huawei hol-u10",
     "{\"audioEngine\":{\"magicId\":51,  \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"recordingDevice\":7, \"AESforSpeaker\": false}}"},
    // Huawei Honor 6 (H60)
    {"huawei/h60",
     "{\"audioEngine\":{\"magicId\":52, \"useOpensl\":false, \"audioMode\":0, "
     "\"recordingDevice\":1, \"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},
    {"huawei/huawei h60",
     "{\"audioEngine\":{\"magicId\":52,  \"useOpensl\":false, \"audioMode\":0, "
     "\"recordingDevice\":1, \"useBuiltinAEC\":true, \"AESforSpeaker\": false}}"},
    // Huawei Mate 1
    {"huawei/huawei mt1",
     "{\"audioEngine\":{\"magicId\":53,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    // Huawei Mate 7
    {"huawei/huawei mt7",
     "{\"audioEngine\":{\"magicId\":54,\"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    // Huawei Ascend P1
    {"huawei/huawei p1",
     "{\"audioEngine\":{\"magicId\":55, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/p1",
     "{\"audioEngine\":{\"magicId\":55,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    // Huawei Ascend  P2
    {"huawei/huawei p2",
     "{\"audioEngine\":{\"magicId\":56,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/p2",
     "{\"audioEngine\":{\"magicId\":56, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    // Huawei Ascend P6
    {"huawei/huawei p6",
     "{\"audioEngine\":{\"magicId\":57, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    {"huawei/p6",
     "{\"audioEngine\":{\"magicId\":57,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    // Huawei Ascend  P7
    {"huawei/huawei p7",
     "{\"audioEngine\":{\"magicId\":58,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"huawei/p7",
     "{\"audioEngine\":{\"magicId\":58, \"useOpensl\":false, \"recordingDevice\":7, \"hwAPM\": "
     "true, \"AESforSpeaker\": false}}"},
    // Huawei Ascend  P8
    {"huawei/huawei p8",
     "{\"audioEngine\":{\"magicId\":59, \"useOpensl\":false, \"recordingDevice\":7, \"hwAPM\": "
     "true, \"AESforSpeaker\": false}}"},
    {"huawei/p8",
     "{\"audioEngine\":{\"magicId\":59,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"huawei/huawei gra-",
     "{\"audioEngine\":{\"magicId\":59,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"huawei/ale-",
     "{\"audioEngine\":{\"magicId\":59,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Huawei Honor 4X (Ch1-Cl10, Che2-TL00M, UL00, Chm-ul00)
    {"huawei/huawei che-",
     "{\"audioEngine\":{\"magicId\":60,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"huawei/huawei chm-",
     "{\"audioEngine\":{\"magicId\":60, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Huawei C8812, C8815 C8816 Low-end
    {"huawei/huawei c8",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/huawei t8",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/huawei g5",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/huawei y2",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/huawei y3",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/huawei y5",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/huawei y6",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/y5",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    {"huawei/y6",
     "{\"audioEngine\":{\"magicId\":300, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": true}}"},
    // Huawei Ascend G6
    {"huawei/huawei g6",
     "{\"audioEngine\":{\"magicId\":301, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    // Huawei Ascend G7
    {"huawei/huawei g7",
     "{\"audioEngine\":{\"magicId\":302, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    // Huawei MediaPad Tablet
    {"huawei/mediapad",
     "{\"audioEngine\":{\"magicId\":303,  \"useOpensl\":false, \"hwAPM\": true, "
     "\"AESforSpeaker\": false}}"},
    {"huawei/x1 7.0/mediapad/",
     "{\"audioEngine\":{\"magicId\":303,  \"useOpensl\":false, \"hwAPM\": true, "
     "\"AESforSpeaker\": false}}"},
    // Ascend G525
    {"huawei/huawei g525",
     "{\"audioEngine\":{\"magicId\":304,  \"useOpensl\":false, \"recordingDevice\":1, "
     "\"AESforSpeaker\": true}}"},
    // Huawei Honor 4A
    {"huawei/scl-",
     "{\"audioEngine\":{\"magicId\":305,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Huawei Honor 7
    {"huawei/plk-",
     "{\"audioEngine\":{\"magicId\":306,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Huawei Honor 6Plus
    {"huawei/pe-",
     "{\"audioEngine\":{\"magicId\":307,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Huawei Honor 5x
    {"huawei/kiw-",
     "{\"audioEngine\":{\"magicId\":308,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Huawei Honor 7i
    {"huawei/ath-",
     "{\"audioEngine\":{\"magicId\":309,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Huawei Mate 8
    {"huawei/huawei nxt-",
     "{\"audioEngine\":{\"magicId\":310,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    {"huawei/huawei mt8",
     "{\"audioEngine\":{\"magicId\":310,\"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    // Huawei Honor 5c
    {"huawei/nem-",
     "{\"audioEngine\":{\"magicId\":311,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    // Huawei NOVA
    {"huawei/huawei caz",
     "{\"audioEngine\":{\"magicId\":312,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"huawei/nce",
     "{\"audioEngine\":{\"magicId\":313,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": true}}"},
    {"huawei/bkl-al",
     "{\"audioEngine\":{\"magicId\":314, \"audioSampleRate\":16000, \"AESforSpeaker\":false}}"},
    {"huawei/stf-al",
     "{\"audioEngine\":{\"magicId\":315, \"audioSampleRate\":16000, \"AESforSpeaker\":false}}"},

    // unknown Devices, keep AEC
    {"huawei/",
     "{\"audioEngine\":{\"magicId\":350, \"useOpensl\":false,  "
     "\"recordingDevice\":7,\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},

    // ---------------------------------------------------------------HUAWEI END
    // -------------------------------------------------------[46 - 60, 300 - 350]

    // ---------------------ZTE Series ------------------- [61 -70]
    // ZTE Memo 5S
    {"zte/zte u5s",
     "{\"audioEngine\":{\"magicId\":61, \"useOpensl\":false, \"AESforSpeaker\": true}}"},
    // Nubia NX403
    {"nubia/nx403",
     "{\"audioEngine\":{\"magicId\":62, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},
    // ZTE Grand SII
    {"zte/zte grand s ii",
     "{\"audioEngine\":{\"magicId\":63, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    // ZTE Grand Memo II
    {"zte/zte m901", "{\"audioEngine\":{\"magicId\":64, \"useOpensl\":false}}"},
    {"zte/", "{\"audioEngine\":{\"magicId\":69, \"useOpensl\":false,  \"AESforSpeaker\": false}}"},
    // Nubia NX507
    {"nubia/nx507",
     "{\"audioEngine\":{\"magicId\":65, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    {"zte/zte n880g",
     "{\"audioEngine\":{\"magicId\":66, \"useOpensl\":false, \"recordingDevice\":1, "
     "\"audioSampleRate\":8000, \"AESforSpeaker\": true}}"},
    // Nubia
    {"nubia/",
     "{\"audioEngine\":{\"magicId\":70, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},

    // --------------------HTC Series ----------------- [71 - 80]
    // HTC Desire 816
    {"htc/htc_d816",
     "{\"audioEngine\":{\"magicId\":71, \"useOpensl\":false, \"recordingDevice\":6, "
     "\"AESforSpeaker\": false}}"},
    // HTC Desire 826
    {"htc/htc_d826",
     "{\"audioEngine\":{\"magicId\":72, \"useOpensl\":false, \"recordingDevice\":6, "
     "\"AESforSpeaker\": false}}"},
    // HTC One M8
    {"htc/htc_m8",
     "{\"audioEngine\":{\"magicId\":73, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},
    // HTC One M9
    {"htc/htc_m9",
     "{\"audioEngine\":{\"magicId\":74, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},
    // HTC sensation X1, x315e
    {"htc/htc sensation xl",
     "{\"audioEngine\":{\"magicId\":75, \"useOpensl\":false, \"recordingDevice\":6, "
     "\"AESforSpeaker\": false}}"},
    // HTC One
    {"htc/htc one",
     "{\"audioEngine\":{\"magicId\":76, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"htc/htl22",
     "{\"audioEngine\":{\"magicId\":76, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"hwAPM\": true,  \"AESforSpeaker\": false}}"},
    // HTC rest
    {"htc/",
     "{\"audioEngine\":{\"magicId\":77, \"useOpensl\":false, \"audioSampleRate\":16000,  "
     "\"AESforSpeaker\": false}}"},

    //  -------------------CoolPad Series -------------- [81- 90]
    // Coolpad 8720L/8730L/8713/8705/8702
    {"yulong/coolpad 87",
     "{\"audioEngine\":{\"magicId\":81, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},
    {"yulong/coolpad 86",
     "{\"audioEngine\":{\"magicId\":82, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": true}}"},
    {"yulong/coolpad",
     "{\"audioEngine\":{\"magicId\":83, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},
    {"coolpad",
     "{\"audioEngine\":{\"magicId\":83, \"useOpensl\":false,  \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},

    // Meizu MX
    {"meizu/m032",
     "{\"audioEngine\":{\"magicId\":90, \"useOpensl\":false, \"hardwareMp3\": false, "
     "\"AESforSpeaker\": true }}"},
    // Meizu MX2
    {"meizu/m0",
     "{\"audioEngine\":{\"magicId\":92, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hardwareMp3\": false, \"AESforSpeaker\": true}}"},
    // Meizu MX3
    {"meizu/mx3",
     "{\"audioEngine\":{\"magicId\":92, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hardwareMp3\": false, \"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"meizu/m35",
     "{\"audioEngine\":{\"magicId\":92, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hardwareMp3\": false, \"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Meizu MX4 / MX4 Pro
    {"meizu/mx4",
     "{\"audioEngine\":{\"magicId\":93, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hardwareMp3\": false, \"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Meizu MX5
    {"meizu/mx5",
     "{\"audioEngine\":{\"magicId\":94, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hardwareMp3\": false, \"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    // Meizu Meilan M1
    {"meizu/m1",
     "{\"audioEngine\":{\"magicId\":95, \"useOpensl\":false, \"hardwareMp3\": false, "
     "\"AESforSpeaker\": false }}"},
    // Meizu Meilan M1 Note
    {"meizu/m1 note",
     "{\"audioEngine\":{\"magicId\":96, \"useOpensl\":false, \"hardwareMp3\": false, "
     "\"AESforSpeaker\": false }}"},
    // Meizu Meilan M2, M2 Note
    {"meizu/m2",
     "{\"audioEngine\":{\"magicId\":97, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hardwareMp3\": false, \"AESforSpeaker\": false }}"},
    {"meizu/m578",
     "{\"audioEngine\":{\"magicId\":97, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hardwareMp3\": false, \"AESforSpeaker\": false }}"},
    // Meizu Meilan M3, M3 Note
    {"meizu/m3",
     "{\"audioEngine\":{\"magicId\":91, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hardwareMp3\": false, \"AESforSpeaker\": false }}"},
    // Meizu MX5 Pro has 8KHz low quality with HwAPM
    {"meizu/pro 5",
     "{\"audioEngine\":{\"magicId\":98, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"hwAPM\": true, \"hardwareMp3\": false, \"AESforSpeaker\": false }}"},
    // other meizu devices
    {"meizu/",
     "{\"audioEngine\":{\"magicId\":99, \"useOpensl\":false, \"hardwareMp3\": false, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false }}"},

    // SONY Xperia Z2
    {"sony/l50",
     "{\"audioEngine\":{\"magicId\":101, \"useOpensl\":false, \"hwAPM\": true, "
     "\"AESforSpeaker\": false}}"},
    // SONY Xperia Z1 / Z1 Mini
    {"sony/l3",
     "{\"audioEngine\":{\"magicId\":102,  \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"sony/so-04",
     "{\"audioEngine\":{\"magicId\":102,\"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"sony/m51",
     "{\"audioEngine\":{\"magicId\":102, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    // SONY Xperia Z3
    {"sony/l55",
     "{\"audioEngine\":{\"magicId\":103, \"useOpensl\":false, \"hwAPM\": true, "
     "\"AESforSpeaker\": false}}"},
    // SONY Xperia Z5, Z3+
    {"sony/e6",
     "{\"audioEngine\":{\"magicId\":107, \"useOpensl\":false, \"hwAPM\": true, "
     "\"AESforSpeaker\": false}}"},

    // Sony Xperia Arc,V,P,ion, TX etc
    {"sony/lt",
     "{\"audioEngine\":{\"magicId\":104, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    // SONY Xperia C3
    {"sony/s55",
     "{\"audioEngine\":{\"magicId\":105, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    // SONY Xperia T2
    {"sony/xm50h",
     "{\"audioEngine\":{\"magicId\":106, \"useOpensl\":false, \"AESforSpeaker\": false}}"},

    // ------------------MOTO Series -------------------[111 - 115]
    // Moto X
    {"motorola/xt105",
     "{\"audioEngine\":{\"magicId\":111, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hwAPM\": true, \"AESforSpeaker\": false, \"faultHwEncoder\": true}}"},
    {"motorola/xt1060",
     "{\"audioEngine\":{\"magicId\":111, \"useOpensl\":false, \"recordingDevice\":7,  "
     "\"hwAPM\": true, \"AESforSpeaker\": false, \"faultHwEncoder\": true}}"},
    // new Moto X
    {"motorola/xt109",
     "{\"audioEngine\":{\"magicId\":111,  \"useOpensl\":false, \"recordingDevice\":7, "
     "\"hwAPM\": true, \"AESforSpeaker\": false, \"faultHwEncoder\": true}}"},

    // Moto G
    {"motorola/xt102",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"motorola/xt103",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"motorola/xt104",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    // Moto G2
    {"motorola/xt1063",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"motorola/xt1064",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"motorola/xt1068",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"motorola/xt1069",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"motorola/xt107",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    // Moto Driod MAXX
    {"motorola/xt108",
     "{\"audioEngine\":{\"magicId\":112, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"motorola/motog3/",
     "{\"audioEngine\":{\"magicId\":113, \"useOpensl\":false, \"AESforSpeaker\": false}}"},
    {"motorola/motoe2/",
     "{\"audioEngine\":{\"magicId\":114, \"useOpensl\":false, \"hwAPM\": true,  "
     "\"AESforSpeaker\": false}}"},

    // Use AEC for Moto phones, including G3 etc
    {"motorola",
     "{\"audioEngine\":{\"magicId\":120, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},

    // ----------------BBK/VIVO Series ----------------[121 - 130]
    // source 7, mode 3 gives system AEC, NS, we also need to fs 16k to activate AGC...

    // Vivo X5S,X5M
    {"bbk/vivo x5",
     "{\"audioEngine\":{\"magicId\":121, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"vivo/vivo x5",
     "{\"audioEngine\":{\"magicId\":121, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},

    // Vivo Y13T, use AES in SPEAKER
    {"bbk/vivo y13t",
     "{\"audioEngine\":{\"magicId\":122, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},
    {"vivo/vivo y13t",
     "{\"audioEngine\":{\"magicId\":122, \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},

    // Vivo X3T, use AEC in SPEAKER, HW is no good
    {"bbk/vivo x3t",
     "{\"audioEngine\":{\"magicId\":123, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"vivo/vivo x3t",
     "{\"audioEngine\":{\"magicId\":123, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000,  \"AESforSpeaker\": false}}"},

    // Vivo X3L
    {"bbk/vivo x3l",
     "{\"audioEngine\":{\"magicId\":124, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"vivo/vivo x3l",
     "{\"audioEngine\":{\"magicId\":124, \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},

    // Vivo xplay (audio flinger issue when using source 7, use source 1 we still have built-in
    // aec)
    {"bbk/vivo xplay",
     "{\"audioEngine\":{\"magicId\":125, \"useOpensl\":false, \"recordingDevice\":1, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"vivo/vivo xplay",
     "{\"audioEngine\":{\"magicId\":125, \"useOpensl\":false, \"recordingDevice\":1, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"vivo/vivo xplay3",
     "{\"audioEngine\":{\"magicId\":128, \"useOpensl\":false, \"useBuiltinAEC\":true, "
     "\"AESforSpeaker\": false}}"},
    {"vivo/vivo xplay4",
     "{\"audioEngine\":{\"magicId\":128, \"useOpensl\":false, \"useBuiltinAEC\":true, "
     "\"AESforSpeaker\": false}}"},
    {"vivo/vivo xplay5",
     "{\"audioEngine\":{\"magicId\":128, \"useOpensl\":false, \"useBuiltinAEC\":true, "
     "\"AESforSpeaker\": false}}"},
    {"vivo/vivo xplay6",
     "{\"audioEngine\":{\"magicId\":128, \"useOpensl\":false, \"useBuiltinAEC\":true, "
     "\"AESforSpeaker\": false}}"},
    {"vivo/vivo xplay7",
     "{\"audioEngine\":{\"magicId\":128, \"useOpensl\":false, \"useBuiltinAEC\":true, "
     "\"AESforSpeaker\": false}}"},

    // vivo x6, x6s, x6plus
    {"bbk/vivo x6",
     "{\"audioEngine\":{\"magicId\":126, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},
    {"vivo/vivo x6",
     "{\"audioEngine\":{\"magicId\":126, \"useOpensl\":false, \"audioSampleRate\":16000, "
     "\"AESforSpeaker\": false}}"},

    // vivo x5pro d
    {"bbk/vivo x5pro",
     "{\"audioEngine\":{\"magicId\":127, \"audioSampleRate\":8000, \"AESforSpeaker\": true}}"},
    {"vivo/vivo x5pro",
     "{\"audioEngine\":{\"magicId\":127,  \"audioSampleRate\":8000, \"AESforSpeaker\": true}}"},

    // Vivo y35a and others
    {"vivo/vivo y",
     "{\"audioEngine\":{\"magicId\":129, \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"useBuiltinAEC\":true, \"AESforSpeaker\": false}}"},
    {"bbk/vivo y",
     "{\"audioEngine\":{\"magicId\":129, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"useBuiltinAEC\":true, \"AESforSpeaker\": false}}"},

    // Vivo Rest
    {"vivo/",
     "{\"audioEngine\":{\"magicId\":130, \"useOpensl\":false,  \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
    {"bbk/",
     "{\"audioEngine\":{\"magicId\":130, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},

    // -------------------Not Main-Stream ---------------
    // Smartisan T1, sm701, sm705
    {"smartisan/sm70",
     "{\"audioEngine\":{\"magicId\":131, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    {"smartisan/yq60",
     "{\"audioEngine\":{\"magicId\":132, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000,  \"AESforSpeaker\": false}}"},
    {"smartisan/sm90",
     "{\"audioEngine\":{\"magicId\":133, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000,   \"AESforSpeaker\": false}}"},
    {"smartisan/sm919",
     "{\"audioEngine\":{\"magicId\":133, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000,   \"AESforSpeaker\": false}}"},
    {"smartisan/",
     "{\"audioEngine\":{\"magicId\":134, \"useOpensl\":false, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000,  \"AESforSpeaker\": false}}"},

    // OnePlus A0001
    {"oneplus/a0001",
     "{\"audioEngine\":{\"magicId\":135, \"recordingDevice\":7, \"audioSampleRate\":16000,   "
     "\"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},
    {"oneplus/one a2001",
     "{\"audioEngine\":{\"magicId\":136,   \"recordingDevice\":7, \"audioSampleRate\":16000,   "
     "\"useBuiltinAEC\":true,  \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"oneplus/",
     "{\"audioEngine\":{\"magicId\":139, \"recordingDevice\":7, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},

    // QiKu
    {"qiku/8681",
     "{\"audioEngine\":{\"magicId\":140,  \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"qiku/", "{\"audioEngine\":{\"magicId\":141, \"AESforSpeaker\": false}}"},

    // Meitu (M4 has an echo issue)
    {"meitu/meitu m4",
     "{\"audioEngine\":{\"magicId\":145, \"audioSampleRate\":16000, \"recordingDevice\":7, "
     "\"AESforSpeaker\": false}}"},
    {"meitu",
     "{\"audioEngine\":{\"magicId\":149, \"audioSampleRate\":16000, \"recordingDevice\":7, "
     "\"useBuiltinAEC\":true, \"AESforSpeaker\": false}}"},

    // -------OPPO--------
    // use same strategy for OPPO and VIVO, all fs should be 16k
    {"oppo/r7",
     "{\"audioEngine\":{\"magicId\":150,  \"audioSampleRate\":16000,\"AESforSpeaker\": "
     "false}}"},
    // OPPO find 7 x9007 x9000
    {"oppo/x900",
     "{\"audioEngine\":{\"magicId\":151, \"recordingDevice\":1, \"audioMode\":0, "
     "\"audioSampleRate\":16000,\"AESforSpeaker\": false}}"},
    // OPPO find 5 x909t
    {"oppo/x909",
     "{\"audioEngine\":{\"magicId\":152, \"recordingDevice\":1, \"audioMode\":0, "
     "\"audioSampleRate\":16000,\"AESforSpeaker\": false}}"},
    // OPPO A37t
    {"oppo/oppo a37",
     "{\"audioEngine\":{\"magicId\":155, \"audioSampleRate\":16000,\"AESforSpeaker\": false}}"},
    // OPPO r9
    {"oppo/r9",
     "{\"audioEngine\":{\"magicId\":156,  \"audioSampleRate\":16000,\"AESforSpeaker\": "
     "false}}"},
    {"oppo/oppo r9",
     "{\"audioEngine\":{\"magicId\":156,  \"audioSampleRate\":16000,\"AESforSpeaker\": "
     "false}}"},
    {"oppo/oppo r11",
     "{\"audioEngine\":{\"magicId\":157, \"audioSampleRate\":16000,\"AESforSpeaker\": false}}"},
    // rest oppo
    {"oppo/",
     "{\"audioEngine\":{\"magicId\":165, \"recordingDevice\":7, "
     "\"audioSampleRate\":16000,\"AESforSpeaker\": false}}"},

    // Lenovo
    {"lenovo/lenovo a380t",
     "{\"audioEngine\":{\"magicId\":170, \"audioMode\":0,\"AESforSpeaker\": false}}"},
    {"lenovo_group_ltd/c100/blaze_tablet",
     "{\"audioEngine\":{\"magicId\":171,  \"AESforSpeaker\": false}}"},
    {"lenovo/lenovo a6000", "{\"audioEngine\":{\"magicId\":172, \"AESforSpeaker\": false}}"},
    {"lenovo/zuk",
     "{\"audioEngine\":{\"magicId\":173, \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"lenovo/moto",
     "{\"audioEngine\":{\"magicId\":174, \"hwAPM\": true,  \"AESforSpeaker\": false}}"},
    {"lenovo/lenovo a850",
     "{\"audioEngine\":{\"magicId\":175, \"audioSampleRate\":16000,\"AESforSpeaker\": false}}"},

    {"lenovo/",
     "{\"audioEngine\":{\"magicId\":179, \"recordingDevice\":7, \"audioSampleRate\":16000,  "
     "\"AESforSpeaker\": false}}"},

    // Letv
    {"letv/",
     "{\"audioEngine\":{\"magicId\":180, \"recordingDevice\":1, \"audioMode\":0, "
     "\"AESforSpeaker\": false}}"},
    {"lemobile/", "{\"audioEngine\":{\"magicId\":181, \"AESforSpeaker\": false}}"},

    // Gionee
    {"gionee/gn8001",
     "{\"audioEngine\":{\"magicId\":185, \"audioSampleRate\":16000, \"AESforSpeaker\": "
     "false}}"},
    {"gionee/gn3003",
     "{\"audioEngine\":{\"magicId\":186, \"audioSampleRate\":16000, \"AESforSpeaker\": "
     "false}}"},
    {"gionee/",
     "{\"audioEngine\":{\"magicId\":187, \"audioSampleRate\":16000,\"AESforSpeaker\": false}}"},

    // Asus
    {"asus/asus_t00j",
     "{\"audioEngine\":{\"magicId\":190, \"recordingDevice\":7, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},
    {"asus/",
     "{\"audioEngine\":{\"magicId\":195, \"recordingDevice\":7, \"audioSampleRate\":16000, "
     "\"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},

    // Underrun/ Overrun devices
    {"alps/", "{\"audioEngine\":{\"magicId\":900, \"AESforSpeaker\": true}}"},
    {"zlr/", "{\"audioEngine\":{\"magicId\":901, \"AESforSpeaker\": true}}"},

    // Yu
    {"yu/",
     "{\"audioEngine\":{\"magicId\":902, \"recordingDevice\":7, \"audioSampleRate\":16000,   "
     "\"AESforSpeaker\": false}}"},

    // lava
    {"lava/irisx8/",
     "{\"audioEngine\":{\"magicId\":906, \"recordingDevice\":7, \"audioSampleRate\":16000,   "
     "\"AESforSpeaker\": false}}"},
    {"lava/",
     "{\"audioEngine\":{\"magicId\":910, \"recordingDevice\":7, \"audioSampleRate\":16000,   "
     "\"AESforSpeaker\": false}}"},

    // xolo
    {"xolo/q1000s/",
     "{\"audioEngine\":{\"magicId\":911, \"recordingDevice\":7,  \"AESforSpeaker\": "
     "false}}"},  // have buit-in, but not good enough
    {"xolo/",
     "{\"audioEngine\":{\"magicId\":915, \"recordingDevice\":7,  \"AESforSpeaker\": false}}"},

    // micromax
    {"micromax/micromax a106/",
     "{\"audioEngine\":{\"magicId\":916, \"recordingDevice\":7, \"audioSampleRate\":16000,   "
     "\"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},
    {"alps/micromax a110/",
     "{\"audioEngine\":{\"magicId\":917, \"recordingDevice\":7, \"audioSampleRate\":16000,   "
     "\"useBuiltinAEC\":true,  \"AESforSpeaker\": false}}"},
    {"micromax/",
     "{\"audioEngine\":{\"magicId\":920, \"recordingDevice\":7, \"audioSampleRate\":16000,   "
     "\"useBuiltinAEC\":true, \"AESforSpeaker\": false}}"},

    // CVTE1
    {"mtk/mt5861",
     "{\"audioEngine\":{\"magicId\":990, \"useOpensl\":false,  \"recordingDevice\":1, "
     "\"AESforSpeaker\": false}}"},
    {"cvtouch",
     "{\"audioEngine\":{\"magicId\":990, \"useOpensl\":false,  \"recordingDevice\":1, "
     "\"AESforSpeaker\": false}}"},
    // Kindle
    {"amazon/kftt",
     "{\"audioEngine\":{\"magicId\":991, \"useOpensl\":false,  \"recordingDevice\":1, "
     "\"AESforSpeaker\": false}}"},
    // Doki (children watch)
    {"wherecom",
     "{\"audioEngine\":{\"magicId\":992, \"useOpensl\":false,  \"audioMode\":0,  "
     "\"AESforSpeaker\": false}}"},
    // ubtech development board
    {"ubtech/alpha2/",
     "{\"audioEngine\":{\"magicId\":993, \"recordingDevice\":1, \"audioSampleRate\":48000, "
     "\"AESforSpeaker\": false}}"},
    // shuzijiayuan
    {"szjy/", "{\"audioEngine\":{\"magicId\":994, \"hwAPM\":true, \"AESforSpeaker\": false}}"},
    // YeShen simulator
    {"yeshen/simulator",
     "{\"audioEngine\":{\"magicId\":995, \"useOpensl\":false, \"recordingDevice\":1, "
     "\"audioSampleRate\":16000, \"AESforSpeaker\": false}}"},
#elif defined(WEBRTC_IOS)
    // Profile for iOS devices
    // below iphone 4
    // {"iPhone/iPhone1", "{\"audioEngine\":{\"magicId\":1000, \"profileLowCPUDevice\":true}}"},
    {"iPhone/iPhone2", "{\"audioEngine\":{\"magicId\":1000, \"profileLowCPUDevice\":true}}"},
    // iPhone 4
    {"iPhone/iPhone3", "{\"audioEngine\":{\"magicId\":1001, \"profileLowCPUDevice\":true}}"},
    // iPhone 4S
    {"iPhone/iPhone4", "{\"audioEngine\":{\"magicId\":1002, \"profileLowCPUDevice\":true}}"},
    // iPhone 5/5C
    {"iPhone/iPhone5", "{\"audioEngine\":{\"magicId\":1003, \"profileLowCPUDevice\":true}}"},
    // iPhone 5S
    {"iPhone/iPhone6", "{\"audioEngine\":{\"magicId\":1004, \"profileLowCPUDevice\":true}}"},
    // iPhone 6/Plus
    {"iPhone/iPhone7", "{\"audioEngine\":{\"magicId\":1005}}"},
    // iPhone 6S/Plus/SE
    {"iPhone/iPhone8", "{\"audioEngine\":{\"magicId\":1006}}"},
    // iPhone 7/Plus
    {"iPhone/iPhone9", "{\"audioEngine\":{\"magicId\":1007}}"},
    // iPhone 8/Plus/X
    {"iPhone/iPhone10", "{\"audioEngine\":{\"magicId\":1008}}"},
    // iPhone Xs/Xs Max/Xr
    {"iPhone/iPhone11", "{\"audioEngine\":{\"magicId\":1009}}"},
    // rest iPhones
    {"iPhone/iPhone", "{\"audioEngine\":{\"magicId\":1099}}"},
    // iPad 1
    // iPad1,1: Wi-Fi (Original/1st Gen)
    // iPad1,1: Wi-Fi/3G/GPS (Original/1st Gen)
    {"iPad/iPad1", "{\"audioEngine\":{\"magicId\":1100, \"profileLowCPUDevice\":true}}"},
    // iPad 2
    // iPad2,1: iPad 2 (Wi-Fi Only)
    // iPad2,2: iPad 2 (Wi-Fi/GSM/GPS)
    // iPad2,3: iPad 2 (Wi-Fi/CDMA/GPS)
    // iPad2,4: iPad 2 (Wi-Fi Only, iPad2,4)
    {"iPad/iPad2,1", "{\"audioEngine\":{\"magicId\":1101, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad2,2", "{\"audioEngine\":{\"magicId\":1101, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad2,3", "{\"audioEngine\":{\"magicId\":1101, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad2,4", "{\"audioEngine\":{\"magicId\":1101, \"profileLowCPUDevice\":true}}"},
    // iPad Mini 1
    // iPad2,5: iPad mini (Wi-Fi Only/1st Gen)
    // iPad2,6: iPad mini (Wi-Fi/AT&T/GPS - 1st Gen)
    // iPad2,7: iPad mini (Wi-Fi/VZ & Sprint/GPS - 1st Gen)
    {"iPad/iPad2,5", "{\"audioEngine\":{\"magicId\":1102, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad2,6", "{\"audioEngine\":{\"magicId\":1102, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad2,7", "{\"audioEngine\":{\"magicId\":1102, \"profileLowCPUDevice\":true}}"},
    // iPad 3
    // iPad3,1: iPad 3rd Gen (Wi-Fi Only)
    // iPad3,2: iPad 3rd Gen (Wi-Fi/Cellular Verizon/GPS)
    // iPad3,3: iPad 3rd Gen (Wi-Fi/Cellular AT&T/GPS)
    {"iPad/iPad3,1", "{\"audioEngine\":{\"magicId\":1103, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad3,2", "{\"audioEngine\":{\"magicId\":1103, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad3,3", "{\"audioEngine\":{\"magicId\":1103, \"profileLowCPUDevice\":true}}"},
    // iPad 4
    // iPad3,4: iPad 4th Gen (Wi-Fi Only)
    // iPad3,5: iPad 4th Gen (Wi-Fi/AT&T/GPS)
    // iPad3,6: iPad 4th Gen (Wi-Fi/Verizon & Sprint/GPS)
    {"iPad/iPad3,4", "{\"audioEngine\":{\"magicId\":1104, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad3,5", "{\"audioEngine\":{\"magicId\":1104, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad3,6", "{\"audioEngine\":{\"magicId\":1104, \"profileLowCPUDevice\":true}}"},
    // iPad Air
    // iPad4,1: iPad Air (Wi-Fi Only)
    // iPad4,2: iPad Air (Wi-Fi/Cellular)
    // iPad4,3: iPad Air (Wi-Fi/TD-LTE - China)
    {"iPad/iPad4,1", "{\"audioEngine\":{\"magicId\":1105, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad4,2", "{\"audioEngine\":{\"magicId\":1105, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad4,3", "{\"audioEngine\":{\"magicId\":1105, \"profileLowCPUDevice\":true}}"},
    // iPad mini 2
    // iPad4,4: iPad mini 2 (Retina/2nd Gen, Wi-Fi Only)
    // iPad4,5: iPad mini 2 (Retina/2nd Gen, Wi-Fi/Cellular)
    // iPad4,6: iPad mini 2 (Retina/2nd Gen, China)
    {"iPad/iPad4,4", "{\"audioEngine\":{\"magicId\":1106, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad4,5", "{\"audioEngine\":{\"magicId\":1106, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad4,6", "{\"audioEngine\":{\"magicId\":1106, \"profileLowCPUDevice\":true}}"},
    // iPad mini 3
    // iPad4,7: iPad mini 3 (Wi-Fi Only)
    // iPad4,8: iPad mini 3 (Wi-Fi/Cellular)
    // iPad4,9: iPad mini 3 (Wi-Fi/Cellular, China)
    {"iPad/iPad4,7", "{\"audioEngine\":{\"magicId\":1107, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad4,8", "{\"audioEngine\":{\"magicId\":1107, \"profileLowCPUDevice\":true}}"},
    {"iPad/iPad4,9", "{\"audioEngine\":{\"magicId\":1107, \"profileLowCPUDevice\":true}}"},
    // iPad 5th Gen
    // iPad6,11: iPad 9.7" 5th Gen (Wi-Fi Only)
    // iPad6,12: iPad 9.7" 5th Gen (Wi-Fi/Cellular)
    {"iPad/iPad6,11", "{\"audioEngine\":{\"magicId\":1108}}"},
    {"iPad/iPad6,12", "{\"audioEngine\":{\"magicId\":1108}}"},
    // iPad mini 4
    // iPad5,1: iPad mini 4 (Wi-Fi Only)
    // iPad5,2: iPad mini 4 (Wi-Fi/Cellular)
    {"iPad/iPad5,1", "{\"audioEngine\":{\"magicId\":1109}}"},
    {"iPad/iPad5,2", "{\"audioEngine\":{\"magicId\":1109}}"},
    // iPad Air 2
    // iPad5,3: iPad Air 2 (Wi-Fi Only)
    // iPad5,4: iPad Air 2 (Wi-Fi/Cellular)
    {"iPad/iPad5,3", "{\"audioEngine\":{\"magicId\":1110}}"},
    {"iPad/iPad5,4", "{\"audioEngine\":{\"magicId\":1110}}"},
    // iPad Pro, 1st Gen
    // iPad6,3: iPad Pro 9.7" (Wi-Fi Only)
    // iPad6,4: iPad Pro 9.7" (Wi-Fi/Cellular)
    {"iPad/iPad6,3", "{\"audioEngine\":{\"magicId\":1111}}"},
    {"iPad/iPad6,4", "{\"audioEngine\":{\"magicId\":1111}}"},
    // iPad6,7: iPad Pro 12.9" (Wi-Fi Only)
    // iPad6,8: iPad Pro 12.9" (Wi-Fi/Cellular)
    {"iPad/iPad6,7", "{\"audioEngine\":{\"magicId\":1112}}"},
    {"iPad/iPad6,8", "{\"audioEngine\":{\"magicId\":1112}}"},
    // iPad Pro, 2nd Gen
    // iPad7,1: iPad Pro 12.9" (Wi-Fi Only - 2nd Gen)
    // iPad7,2: iPad Pro 12.9" (Wi-Fi/Cell - 2nd Gen)
    {"iPad/iPad7,1", "{\"audioEngine\":{\"magicId\":1113}}"},
    {"iPad/iPad7,2", "{\"audioEngine\":{\"magicId\":1113}}"},
    // iPad7,3: iPad Pro 10.5" (Wi-Fi Only)
    // iPad7,4: iPad Pro 10.5" (Wi-Fi/Cellular)
    {"iPad/iPad7,3", "{\"audioEngine\":{\"magicId\":1114}}"},
    {"iPad/iPad7,4", "{\"audioEngine\":{\"magicId\":1114}}"},
    // iPad7,5: iPad 9.7" 6th Gen (Wi-Fi Only)
    // iPad7,6: iPad 9.7" 6th Gen (Wi-Fi/Cellular)
    {"iPad/iPad7,5", "{\"audioEngine\":{\"magicId\":1115}}"},
    {"iPad/iPad7,6", "{\"audioEngine\":{\"magicId\":1115}}"},
    // iPad Pro, 3rd Gen
    // iPad8,1, iPad8,2**: iPad Pro 11" (Wi-Fi Only)
    // iPad8,3, iPad8,4**: iPad Pro 11" (Wi-Fi/Celluar - US/CA)
    // iPad8,3, iPad8,4**: iPad Pro 11" (Wi-Fi/Celluar - Global)
    // iPad8,3, iPad8,4**: iPad Pro 11" (Wi-Fi/Celluar - China)
    {"iPad/iPad8,1", "{\"audioEngine\":{\"magicId\":1116}}"},
    {"iPad/iPad8,2", "{\"audioEngine\":{\"magicId\":1116}}"},
    {"iPad/iPad8,3", "{\"audioEngine\":{\"magicId\":1116}}"},
    {"iPad/iPad8,4", "{\"audioEngine\":{\"magicId\":1116}}"},
    // iPad8,5, iPad8,6**: iPad Pro 12.9" (Wi-Fi Only - 3rd Gen)
    // iPad8,7, iPad8,8**: iPad Pro 12.9" (Wi-Fi+Cell US/CA - 3rd Gen)
    // iPad8,7, iPad8,8**: iPad Pro 12.9" (Wi-Fi+Cell Global - 3rd Gen)
    // iPad8,7, iPad8,8**: iPad Pro 12.9" (Wi-Fi+Cell China - 3rd Gen)
    {"iPad/iPad8,5", "{\"audioEngine\":{\"magicId\":1117}}"},
    {"iPad/iPad8,6", "{\"audioEngine\":{\"magicId\":1117}}"},
    {"iPad/iPad8,7", "{\"audioEngine\":{\"magicId\":1117}}"},
    {"iPad/iPad8,8", "{\"audioEngine\":{\"magicId\":1117}}"},
    // rest iPads
    {"iPad/iPad", "{\"audioEngine\":{\"magicId\":1199}}"},

    // low-end iPods
    {"iPod touch/iPod1", "{\"audioEngine\":{\"magicId\":1201, \"profileLowCPUDevice\":true}}"},
    {"iPod touch/iPod2", "{\"audioEngine\":{\"magicId\":1202, \"profileLowCPUDevice\":true}}"},
    {"iPod touch/iPod3", "{\"audioEngine\":{\"magicId\":1203, \"profileLowCPUDevice\":true}}"},
    {"iPod touch/iPod4", "{\"audioEngine\":{\"magicId\":1204, \"profileLowCPUDevice\":true}}"},
    // iPod touch 5
    {"iPod touch/iPod5,1", "{\"audioEngine\":{\"magicId\":1205, \"profileLowCPUDevice\":true}}"},
    // rest iPods
    {"iPod touch/iPod", "{\"audioEngine\":{\"magicId\":1206}}"},

#elif defined(WEBRTC_WIN)
    {"Windows", "{\"audioEngine\":{\"magicId\":3000,  \"hwAPM\": true, \"AESforSpeaker\": false}}"},
    {"Parallels Software", "{\"audioEngine\":{\"magicId\":3001,  \"AESforSpeaker\": false}}"},
    {"Xiaomi Inc/Mipad2", "{\"audioEngine\":{\"magicId\":3002,  \"AESforSpeaker\": false}}"},
    {"Microsoft Corporation/Surface",
     "{\"audioEngine\":{\"magicId\":3003,  \"AESforSpeaker\": false}}"},
    {"LENOVO", "{\"audioEngine\":{\"magicId\":3004,  \"AESforSpeaker\": false}}"},
    {"HCT/HCT",
     "{\"audioEngine\":{\"magicId\":3005,  \"AESforSpeaker\": false}}"},  // OEM, special
                                                                          // teaching device for
                                                                          // kindgarden
    {"Gigabyte Technology Co., Ltd./To be filled by O.E.M.",
     "{\"audioEngine\":{\"magicId\":3006,  \"AESforSpeaker\": false}}"},  // crazy teacher
                                                                          // special device
#elif defined(WEBRTC_MAC)
    {"MacBook", "{\"audioEngine\":{\"magicId\":4000, \"AESforSpeaker\": false}}"},
    {"Macmini", "{\"audioEngine\":{\"magicId\":4001, \"AESforSpeaker\": false}}"},
    {"MacBookPro8", "{\"audioEngine\":{\"magicId\":4002,  \"AESforSpeaker\": false}}"},
    {"MacBookPro7", "{\"audioEngine\":{\"magicId\":4002,  \"AESforSpeaker\": false}}"},
    {"MacBookPro6", "{\"audioEngine\":{\"magicId\":4002,  \"AESforSpeaker\": false}}"},
    {"MacBookPro5", "{\"audioEngine\":{\"magicId\":4002, \"AESforSpeaker\": false}}"},
    {"MacBookAir", "{\"audioEngine\":{\"magicId\":4003,  \"AESforSpeaker\": false}}"},
#endif
    {0, 0}};

const char* DeviceProfile::getDeviceProfile(const char* deviceId) {
  if (!deviceId || *deviceId == '\0') return NULL;
  int index = -1, len = 0;
  for (int i = 0; i < sizeof(g_profiles) / sizeof(g_profiles[0]); i++) {
    if (!g_profiles[i].deviceId) continue;
    int l = strlen(g_profiles[i].deviceId);
    if (!strncmp(g_profiles[i].deviceId, deviceId, l) && l > len) {
      index = i;
      len = l;
    }
  }
  if (index >= 0) {
    return g_profiles[index].profile;
  }
  return 0;
}

DeviceProfile::DeviceProfile(const char* deviceId) {
  const char* profile = getDeviceProfile(deviceId);
  if (profile) {
    commons::cjson::JsonWrapper json;
    json.parse(profile);

    magic_id_ = json.getObject("audioEngine").getIntValue("magicId", 0);
    low_cpu_device_ = json.getObject("audioEngine").getBooleanValue("profileLowCPUDevice", false);
  }
}

}  // namespace rtc
}  // namespace agora
