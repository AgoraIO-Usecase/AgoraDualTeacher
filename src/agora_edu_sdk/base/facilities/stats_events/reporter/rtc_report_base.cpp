//
//  Agora Media SDK
//
//  Created by Xiaosen Wang in 2020-03.
//  Copyright (c) 2020 Agora IO. All rights reserved.
//

#include "rtc_report_base.h"

#include <string.h>

#include "utils/log/log.h"
#include "utils/tools/json_wrapper.h"

const char MODULE_NAME[] = "[RRB]";

namespace agora {
namespace rtc {

const int32_t kAllEventCounterID = -1;
const int32_t kExternalCounterIdStart = 3000;
const int32_t kExternalCounterIdEnd = 4000;
const uint32_t kDefaultReportCount = 1;
const uint32_t kDefaultReportInterval = 6;
const uint32_t kDefaultAudioReportInterval = 2;
const uint32_t kReportDisabledCount = 0;
const uint32_t kReportNoLimitInterval = 0;
const uint32_t kReportAcceptableDetla = 100;

static const uint32_t kEventReportRetryCount = 5;
static const int32_t kMinReportCount = 0;
static const int32_t kMaxReportCount = 10000;  // TODO(xwang): update this value when confirmed
static const int32_t kMinReportInterval = 0;
static const int32_t kMaxReportInterval = 10000;  // TODO(xwang): update this value when confirmed

const std::unordered_map<ReportItemType, EventProperty> kEventConfigPropertyMap = {
    {ReportItemType::CustomReportEvent, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::Session, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::VosdkVideoInitialOptions, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::Quit, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::MaxVideoPayloadSet, {ReportLevel::Normal, kEventReportRetryCount}},
    {ReportItemType::TracerAPEvent, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::APWorkerEvent, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::Vocs, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::FirstJoinVosSuccess, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::Vos, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::Peer, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::ViLocalFrame, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::ViRemoteFrame, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerFirstAudioPacketSent, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerFirstAudioPacketReceived, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerAudioSendingStopped, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerAudioDisabled, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerAudioEnabled, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerFirstVideoPacketSent, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerFirstVideoPacketReceived, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerFirstVideoFrameDecoded, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerFirstVideoFrameDrawed, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerVideoSendingStopped, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerVideoDisabled, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerVideoEnabled, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerVideoStreamSelected, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerVideoStreamChangeRequest, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerFirstDataPacketSent, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerFirstDataPacketReceived, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerErrorCallback, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerPeerOnline, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerPeerOffline, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerAudioMutePeerStatusChanged, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerVideoMutePeerStatusChanged, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::VosdkRemoteFallbackChanged, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerAudioMuteAllStatusChanged, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerVideoMuteAllStatusChanged, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerDefaultPeerStatus, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerP2PStunLoginSuccess, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerP2PStunLoginFailed, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerP2PPeerTryTouch, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerP2PPeerConnected, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerP2PPeerDisconnected, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerP2PStart, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerP2PStop, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::VosdkRecordingJoin, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::VosdkRecordingLeave, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::VosdkVideoBandwidthAggressiveLevel,
     {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerLocalFallbackStatus, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerAppSetMinPlayoutDelay, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerAppSetVideoStartBitRate, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::VosdkVqcStat, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::ACodec, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::NetworkInformation, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::Rating, {ReportLevel::Normal, kEventReportRetryCount}},
    {ReportItemType::TracerAPEvent, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::DeviceStatChange, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TracerReportStats, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::SwitchVideoStream, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::PPrivilegeWillExpire, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::RenewToken, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::RenewTokenRes, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::VosdkSendVideoPaced, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::VosdkABTest, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::WorkerEvent, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::TrackSpan, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::RtmLogin, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::RtmLogout, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::RtmApEvent, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::RtmLink, {ReportLevel::Critical, kEventReportRetryCount}},
    {ReportItemType::RtmRxMessage, {ReportLevel::Normal, kEventReportRetryCount}},
    {ReportItemType::RtmTxMessage, {ReportLevel::Normal, kEventReportRetryCount}},
    {ReportItemType::RtmTxMessageRes, {ReportLevel::Normal, kEventReportRetryCount}},
    {ReportItemType::PeerFirstAudioFrameDecoded, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::PeerFirstVideoFrameDecoded, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::PeerFirstAudioFrameDecodedTimeout,
     {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::PeerFirstVideoFrameDecodedTimeout,
     {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::JoinChannelTimeout, {ReportLevel::High, kEventReportRetryCount}},
    {ReportItemType::CrashEvent, {ReportLevel::High, kEventReportRetryCount}}};

const std::unordered_map<int32_t, CounterProperty> kPeerCounterPropertyMap = {
    {(int32_t)VIDEO_COUNTER_CAPTURE_RESOLUTION_WIDTH, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_CAPTURE_RESOLUTION_HEIGHT, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_CAPTURE_FRAME_RATE, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_ENCODER_IN_FRAME, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_ENCODER_OUT_FRAME, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_ENCODER_FAILED_FRAME, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_ENCODER_SKIP_FRAME, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_HIGH_BITRATE, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_HIGH_FRAME_RATE, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_RENDER_IN_FRAMES, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_RENDER_OUT_FRAMES, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_HIGH_WIDTH, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_LOCAL_HIGH_HEIGHT, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_LOCAL_HIGH_QP, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_LOW_BITRATE, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_LOW_FRAME_RATE, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_SENT_RTT, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_SENT_LOSS, {ReportLevel::High, CounterAlgo::Max}},
    {(int32_t)VIDEO_COUNTER_LOCAL_TARGET_BITRATE, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_VIDEO_HARDWARE_ENCODER, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_BITRATE, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_FRAME_RATE, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_WIDTH, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_HEIGHT, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_DELAY, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_LOW_BITRATE, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_LOW_FRAME_RATE, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_LOW_WIDTH, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_LOW_HEIGHT, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_RENDER_TYPE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_DECODE_TIME, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_N_SDK_R_VOS_T_V_LOSS, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_N_SDK_T_VOS_R_V_LOSS, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_ENCODER_KEY_FRAME_NUM, {ReportLevel::High, CounterAlgo::Added}},
    {(int32_t)VIDEO_COUNTER_CAPTURE_COEF_VARIATION, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_CAPTURE_COEF_UNIFORMILITY, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_CAPTURE_TARGET_FPS, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_CAPTURE_REAL_FPS, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_SENDTOENC_COEF_UNIFORMILITY, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_SEND_KEY_FRAME_NUM, {ReportLevel::High, CounterAlgo::Added}},
    {(int32_t)VIDEO_COUNTER_SEND_LOST, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_SEND_RTT, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_SEND_JITTER, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_LOSS_AFTER_FEC, {ReportLevel::High, CounterAlgo::Max}},
    {(int32_t)VIDEO_COUNTER_REMOTE_FLAGS, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_DECODE_FAILED_FRAMES, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_DECODE_REJECTED_FRAMES,
     {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_DECODER_IN_FRAMES, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_DECODER_OUT_FRAMES, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_DECODE_BACKGROUND_DROPED_FRAMES,
     {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_ENCODE_TIME, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_FEC_LEVEL, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_LOCAL_ESTIMATED_BANDWIDTH, {ReportLevel::Low, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_MAX_FRAME_OUT_INTERVAL,
     {ReportLevel::Debug, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_MAX_RENDER_INTERVAL, {ReportLevel::Debug, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_REMOTE_DECODE_QP, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_CHAT_ENGINE_STAT, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_LOCAL_CAMERO_OPEN_STATUS, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_LOCAL_CAPTURE_TYPE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_LOCAL_RENDER_TYPE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_LOCAL_RENDER_BUFFER_SIZE, {ReportLevel::High, CounterAlgo::Max}},
    {(int32_t)VIDEO_COUNTER_LOCAL_RENDER_MEAN_FPS, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)VIDEO_COUNTER_REMOTE_RENDER_MEAN_FPS, {ReportLevel::High, CounterAlgo::Latest}},

    {(int32_t)AUDIO_COUNTER_AEC_DELAY, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_AEC_ERL, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_AEC_ERLE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_NEAROUT_SIGNAL_LEVEL, {ReportLevel::Critical, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_HOWLING_STATE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_RECORD_FREQUENCY, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)AUDIO_COUNTER_PLAYBACK_FREQUENCY, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)AUDIO_COUNTER_OUTPUT_ROUTE, {ReportLevel::MoreHigh, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_SEND_BITRATE, {ReportLevel::MoreHigh, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_RECEIVE_BITRATE, {ReportLevel::MoreHigh, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_AUDIO_CODEC, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)AUDIO_COUNTER_NEARIN_SIGNAL_LEVEL, {ReportLevel::Critical, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_FARIN_SIGNAL_LEVEL, {ReportLevel::Critical, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_ENCODEIN_SIGNAL_LEVEL, {ReportLevel::MoreHigh, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_ATTRIBUTE_BITS, {ReportLevel::MoreHigh, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_AUDIO_ADM, {ReportLevel::MoreHigh, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_AUDIO_PROFILE, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)AUDIO_COUNTER_N_SDK_R_VOS_T_A_LOSS, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)AUDIO_COUNTER_N_SDK_T_VOS_R_A_LOSS, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)AUDIO_COUNTER_AUDIO_EFFECT_TYPE, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)AUDIO_COUNTER_RECEIVE_JITTER_TO_USER, {ReportLevel::MoreHigh, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_SEND_FRACTION_LOST, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_SEND_RTT_MS, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)AUDIO_COUNTER_SEND_JITTER_MS, {ReportLevel::High, CounterAlgo::Latest}},

    // Freeze Count/Time
    {(int32_t)COMMUNICATION_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)COMMUNICATION_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)COMMUNICATION_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)COMMUNICATION_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)COMMUNICATION_INDICATOR_TYPE::VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)COMMUNICATION_INDICATOR_TYPE::VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_80MS_TOTAL_FROZEN_TIME,
     {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)BROADCASTER_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FROZEN_RATE,
     {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::VIDEO_COUNTER_LOCAL_UPLINK_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_500MS_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_200MS_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_80MS_TOTAL_FROZEN_TIME,
     {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)AUDIENCE_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FROZEN_RATE,
     {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)SERVER_SDK_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)SERVER_SDK_INDICATOR_TYPE::VIDEO_COUNTER_REMOTE_RENDER_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)SERVER_SDK_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FREEZE_COUNT,
     {ReportLevel::Critical, CounterAlgo::Added}},
    {(int32_t)SERVER_SDK_INDICATOR_TYPE::AUDIO_COUNTER_REMOTE_RENDER_FREEZE_TIME,
     {ReportLevel::Critical, CounterAlgo::Added}},

    {(int32_t)VIDEO_COUNTER_WEB_AGENT_DELAY, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_WEB_AGENT_RENDERED_FRAME_RATE,
     {ReportLevel::Low, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_WEB_AGENT_SKIPPED_FRAMES, {ReportLevel::Low, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_WEB_AGENT_SENT_FRAMES, {ReportLevel::Normal, CounterAlgo::Average}},

    {(int32_t)DATA_STREAM_COUNTER_BITRATE, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)DATA_STREAM_COUNTER_PACKETRATE, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)DATA_STREAM_COUNTER_DELAY, {ReportLevel::Low, CounterAlgo::Average}},
    {(int32_t)DATA_STREAM_COUNTER_LOST, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)DATA_STREAM_COUNTER_ERROR_CODE, {ReportLevel::Normal, CounterAlgo::Average}},
    {(int32_t)DATA_STREAM_COUNTER_MISSED, {ReportLevel::Low, CounterAlgo::Average}},
    {(int32_t)VIDEO_COUNTER_LOCAL_VIDEO_REXFER_BITRATE, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)DATA_STREAM_COUNTER_CACHED, {ReportLevel::Low, CounterAlgo::Average}}};

const std::unordered_map<int32_t, CounterProperty> kEventCounterPropertyMap = {
    {(int32_t)A_AEC_HNL1, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)A_ADM_PLAYOUT_GLITCH, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)A_ENGINE_RESTART, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)A_APM_BUFFER, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)A_AEC_DELAY, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)A_AEC_ERL, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)A_AEC_ERLE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)A_HOWLING_STATE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)A_AUDIO_MAX_TX_INTERVAL, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)A_VIDEO_MAX_TX_INTERVAL, {ReportLevel::Debug, CounterAlgo::Latest}},

    {(int32_t)A_AUDIO_ENGINE_STAT2, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},

    {(int32_t)A_MAGIC_ID, {ReportLevel::Normal, CounterAlgo::Latest}},

    {(int32_t)S_LAST_ERROR, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)A_TX_RATE, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)A_RX_RATE, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)A_SEND_CODEC_TYPE, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)A_FRAMES_PER_PACKET, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)A_INTERLEAVES_PER_PACKET, {ReportLevel::Normal, CounterAlgo::Latest}},

    {(int32_t)S_MUTE_STATUS, {ReportLevel::Critical, CounterAlgo::Latest}},
    {(int32_t)S_CPU_APP, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)S_CPU_TOTAL, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)S_ACTIVE_CORE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_CURRENT_CORE_FREQUENCY, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_IN_TASK_PICKUP_COUNT, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_IN_TASK_PICKUP_TIME, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)S_IN_TASK_QUEUE_SIZE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_TIMER_ACCURACY, {ReportLevel::Normal, CounterAlgo::Latest}},

    {(int32_t)N_TX_RATE, {ReportLevel::Critical, CounterAlgo::Latest}},
    {(int32_t)N_RX_RATE, {ReportLevel::Critical, CounterAlgo::Latest}},
    {(int32_t)N_TX_BYTES, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)N_RX_BYTES, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)N_TX_PACKET_RATE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)N_RX_PACKET_RATE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)N_P2P_SENT_RATE, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)N_P2P_PEER_DELAY, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)N_NET_CHANGED, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)N_BROADCAST_TX_RATE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)N_BROADCAST_RX_RATE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)N_REPORT_TX_RATE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)N_ONLINE_PEERS, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)N_SUBSCRIBE_STREAM_TYPE, {ReportLevel::High, CounterAlgo::Latest}},

    // trace counters
    // local audio
    {(int32_t)T_LA_ENCODER_TX_FRAMES, {ReportLevel::Low, CounterAlgo::Latest}},
    // remote audio
    {(int32_t)T_RA_FEC_SAVED_LOSS_RATE, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)T_RA_NETEQ_RX_FRAMES, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)T_RA_RX_EXPIRED_FRAMES, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)T_RA_DECODER_OUT_FRAMES, {ReportLevel::Low, CounterAlgo::Latest}},
    // local video
    {(int32_t)T_LV_CAPTURE_WIDTH, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)T_LV_CAPTURE_HEIGHT, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)T_LV_CAPTURE_FRAMES, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)T_LV_ENCODER_IN_FRAMES, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)T_LV_ENCODER_OUT_FRAMES, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)T_LV_ENCODER_FAILED_FRAMES, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)T_LV_ENCODER_SKIP_FRAMES, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)T_LV_TX_PACKETS_LOW, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)T_LV_TX_PACKETS_HIGH, {ReportLevel::Low, CounterAlgo::Latest}},
    // remote video
    {(int32_t)T_RV_RX_PACKETS, {ReportLevel::Normal, CounterAlgo::Latest}},
    // sdk audio
    {(int32_t)T_SDK_A_TX_PACKETS, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)T_SDK_A_TX_DTX_PACKETS, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)T_SDK_A_RX_PACKETS, {ReportLevel::Low, CounterAlgo::Latest}},
    // sdk video
    {(int32_t)T_SDK_V_TX_PACKETS, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)T_SDK_V_RX_PACKETS, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)A_VIDEO_DECODER_TIME, {ReportLevel::Low, CounterAlgo::Latest}},

    // sdk audio
    {(int32_t)T_SDK_A_TX_FRAME_RATE, {ReportLevel::Low, CounterAlgo::Latest}},
    // remote audio
    {(int32_t)T_RA_RX_PACKETS, {ReportLevel::Low, CounterAlgo::Latest}},

    {(int32_t)S_AUTIO_TX_TIME, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)S_AUDIO_RX_TIME, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)S_VIDEO_TX_TIME, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)S_VIDEO_RX_TIME, {ReportLevel::Low, CounterAlgo::Latest}},

#if TARGET_OS_IOS
    {(int32_t)A_AUDIO_SAMPLE_RATE, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)A_AUDIO_IO_BUFFER_DURATION, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)A_AUDIO_OUTPUT_LATENCY, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)A_AUDIO_INPUT_LATENCY, {ReportLevel::Low, CounterAlgo::Latest}},
#endif
    {(int32_t)A_AEC_FRAC, {ReportLevel::Low, CounterAlgo::Latest}},
    {(int32_t)A_AEC_QUALITY, {ReportLevel::Low, CounterAlgo::Latest}},

    // rtt
    {(int32_t)N_WAN_RTT, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)N_GATEWAY_RTT, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)A_TX_VIDEO_CODEC, {ReportLevel::Low, CounterAlgo::Latest}},

    // encryption
    {(int32_t)S_AUDIO_ENCRYPTION_TIME, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_AUDIO_DECRYPTION_TIME, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_VIDEO_ENCRYPTION_TIME, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_VIDEO_DECRYPTION_TIME, {ReportLevel::Normal, CounterAlgo::Latest}},

    // audio&video dropped packets
    {(int32_t)T_SDK_A_TX_DROPPED_PACKETS, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)T_SDK_A_RX_DROPPED_PACKETS, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)T_SDK_V_TX_DROPPED_PACKETS, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)T_SDK_V_RX_DROPPED_PACKETS, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_CLIENT_ROLE, {ReportLevel::High, CounterAlgo::Latest}},

    {(int32_t)S_TOTAL_CORE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)S_CPU_QUALITY, {ReportLevel::Low, CounterAlgo::Latest}},

    {(int32_t)V_HARDWARE_ENCODER, {ReportLevel::High, CounterAlgo::Latest}},

    {(int32_t)T_LV_ENCODER_DROPPED_FRAMES, {ReportLevel::Normal, CounterAlgo::Latest}},

    {(int32_t)A_BROADCASTER_RECV_RENDER_FREEZE_RATE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)A_BROADCASTER_RECV_RENDER_FREEZE_TIME, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)A_AUDIENCE_RECV_RENDER_FREEZE_RATE, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)A_AUDIENCE_RECV_RENDER_FREEZE_TIME, {ReportLevel::High, CounterAlgo::Latest}},

    {(int32_t)S_REPORT_CACHED_SIZE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_BATTERY_LIFE, {ReportLevel::Normal, CounterAlgo::Latest}},
    {(int32_t)S_LAST_ERROR_1, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)S_LAST_ERROR_2, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)A_ENCODEIN_SIGNAL_LEVEL, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)N_P2P_SEND_RATE, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)N_P2P_RECEIVE_RATE, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)N_P2P_TX_LOST, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)N_P2P_RX_LOST, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)A_AUDIO_ADM, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)A_AUDIO_PROFILE, {(ReportLevel::MoreHigh), CounterAlgo::Latest}},
    {(int32_t)S_DAMAGED_PACKETS, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)S_EXCEED_MTU_PACKETS, {ReportLevel::High, CounterAlgo::Latest}},
    {(int32_t)N_SDK_R_VOS_T_V_LOSS, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)N_SDK_T_VOS_R_V_LOSS, {ReportLevel::High, CounterAlgo::Average}},
    {(int32_t)N_SDK_R_VOS_T_A_LOSS, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)N_SDK_T_VOS_R_A_LOSS, {ReportLevel::MoreHigh, CounterAlgo::Max}},
    {(int32_t)kExternalCounterIdStart, {ReportLevel::Normal, CounterAlgo::Latest}}};

const std::set<AUDIO_COUNTER_TYPE> kAudioCounterCollection = {
    AUDIO_COUNTER_AEC_DELAY,
    AUDIO_COUNTER_AEC_ERL,
    AUDIO_COUNTER_AEC_ERLE,
    AUDIO_COUNTER_NEAROUT_SIGNAL_LEVEL,
    AUDIO_COUNTER_HOWLING_STATE,
    AUDIO_COUNTER_RECORD_FREQUENCY,
    AUDIO_COUNTER_PLAYBACK_FREQUENCY,
    AUDIO_COUNTER_OUTPUT_ROUTE,
    AUDIO_COUNTER_SEND_BITRATE,
    AUDIO_COUNTER_RECEIVE_BITRATE,
    AUDIO_COUNTER_AUDIO_CODEC,
    AUDIO_COUNTER_FRAMES_PER_PACKET,
    AUDIO_COUNTER_INTERLEAVES_PER_PACKET,
    AUDIO_COUNTER_NEARIN_SIGNAL_LEVEL,
    AUDIO_COUNTER_FARIN_SIGNAL_LEVEL,
    AUDIO_COUNTER_ENCODEIN_SIGNAL_LEVEL,
    AUDIO_COUNTER_ATTRIBUTE_BITS,
    AUDIO_COUNTER_AUDIO_PROFILE,
    AUDIO_COUNTER_N_SDK_R_VOS_T_A_LOSS,
    AUDIO_COUNTER_N_SDK_T_VOS_R_A_LOSS,
    AUDIO_COUNTER_SEND_FRACTION_LOST,
    AUDIO_COUNTER_SEND_RTT_MS,
    AUDIO_COUNTER_SEND_JITTER_MS,
};

const std::set<DATA_STREAM_COUNTER_TYPE> kDataStreamCounterCollection = {
    DATA_STREAM_COUNTER_BITRATE, DATA_STREAM_COUNTER_PACKETRATE, DATA_STREAM_COUNTER_DELAY,
    DATA_STREAM_COUNTER_LOST,    DATA_STREAM_COUNTER_ERROR_CODE, DATA_STREAM_COUNTER_MISSED,
    DATA_STREAM_COUNTER_CACHED};

ReportRule::ReportRule() : ReportRule(false, kDefaultReportCount, kDefaultReportInterval, false) {}

ReportRule::ReportRule(bool active, uint32_t count, uint32_t interval, bool default_rule)
    : active_(active),
      report_count_(count),
      report_interval_(interval),
      default_rule_(default_rule) {}

bool ReportRule::valid() const {
  if (!active_) return true;

  if (report_count_ > kMaxReportCount) return false;

  if (report_interval_ > kMaxReportInterval) return false;

  return true;
}

bool ReportRule::isSendTooQuick(int32_t count, int32_t interval_ms) const {
  if (interval_ms + kReportAcceptableDetla > report_interval_ * 1000) return false;

  if (count < report_count_) return false;

  return true;
}

uint32_t ReportRule::counterPerInterval() const {
  static const uint32_t kCounterCollectIntervalInSec = 2;
  static const uint32_t kDefaultCounterCacheCount = 1;

  if (active_ && report_interval_ > 0)
    return (report_interval_ + 1) / kCounterCollectIntervalInSec;
  else
    return kDefaultCounterCacheCount;
}

bool jsonToReportRule(const any_document_t& json_obj, ReportRule& output_rule) {
  // Parse failed error would be handled by caller
  if (!json_obj.isValid()) return false;

  int event_id = 0;
  int report_count = kDefaultReportCount;
  int report_interval = kDefaultReportInterval;
  if (!json_obj.tryGetIntValue("id", event_id)) return false;
  if (!json_obj.tryGetIntValue("report_count", report_count)) return false;
  if (!json_obj.tryGetIntValue("report_interval", report_interval)) return false;

  if (event_id < kAllEventCounterID) return false;
  if (report_count < kMinReportCount || report_count > kMaxReportCount) return false;
  if (report_interval < kMinReportInterval || report_interval > kMaxReportInterval) return false;

  output_rule =
      ReportRule(true, static_cast<uint32_t>(report_count), static_cast<uint32_t>(report_interval));
  return true;
}

ReportRuleList jsonStrToReportRule(const std::string& json_str, bool is_event) {
  static const char kEventKeyAll[] = "data.report.event.all";
  static const char kEventKeyPrefix[] = "data.report.event";
  static const char kCounterKeyAll[] = "data.report.counter.all";
  static const char kCounterKeyPrefix[] = "data.report.counter";

  ReportRuleList rules;

  const char* global_rule_name = nullptr;
  const char* rule_prefix = nullptr;
  if (is_event) {
    global_rule_name = kEventKeyAll;
    rule_prefix = kEventKeyPrefix;
  } else {
    global_rule_name = kCounterKeyAll;
    rule_prefix = kCounterKeyPrefix;
  }
  any_document_t json_root(json_str);
  if (!json_root.isValid()) {
    commons::log(commons::LOG_WARN, "%s: invalid config json string (%s)", MODULE_NAME,
                 json_str.c_str());
    return rules;
  }

  ReportRule global_rule(false, kDefaultReportCount, kDefaultReportInterval);
  any_document_t child = json_root.getObject(global_rule_name);
  if (child.isValid()) {
    if (jsonToReportRule(child, global_rule) && global_rule.valid()) {
      commons::log(commons::LOG_DEBUG, "%s: apply global rule {avtive:%d, count:%d, interval:%d}",
                   MODULE_NAME, global_rule.active(), global_rule.count(), global_rule.interval());

      rules.emplace(kAllEventCounterID, global_rule);
    } else {
      commons::log(commons::LOG_WARN, "%s: global not valid:%s", MODULE_NAME, child.getName());
    }
  } else {
    commons::log(commons::LOG_DEBUG, "%s: no global rule found", MODULE_NAME);
  }

  // parse per-item-rule to overwrite global rule
  int32_t event_id = 0;
  ReportRule per_event_rule(false, kDefaultReportCount, kDefaultReportInterval);
  child = json_root.getChild();
  while (child.isValid()) {
    if (0 != strncmp(child.getName(), rule_prefix, strlen(rule_prefix))) {
      child = child.getNext();
      continue;
    }

    if (!child.tryGetIntValue("id", event_id)) {
      commons::log(commons::LOG_INFO, "%s: rule id not found for %s", MODULE_NAME, child.getName());
      child = child.getNext();
      continue;
    }

    if (kAllEventCounterID == event_id) {
      child = child.getNext();
      continue;
    }

    if (jsonToReportRule(child, per_event_rule) && per_event_rule.valid()) {
      commons::log(commons::LOG_DEBUG,
                   "%s: apply config rule {id:%d, report_interval:%d, report_count:%d}",
                   MODULE_NAME, event_id, per_event_rule.interval(), per_event_rule.count());

      rules.emplace(event_id, per_event_rule);
    } else {
      commons::log(commons::LOG_WARN, "%s: invalid rule found for %s", MODULE_NAME,
                   child.getName());
    }
    child = child.getNext();
  }
  return rules;
}

}  // namespace rtc
}  // namespace agora
