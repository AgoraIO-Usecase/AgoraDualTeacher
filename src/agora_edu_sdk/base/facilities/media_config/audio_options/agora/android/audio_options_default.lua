--[[
  Input and output are defined in audio_sessions_keyword.h
	* fields with type of absl::optional may not contain value, 
	* so you should check whether it's valid or not before using it
	* the builtin function of check valid is mc_valid(..)
	* and the builtin function of get real value is mc_value(..)

--]]


-- Note that profile_name is the id of this option profile
profile_name = "default"

options.audio_scenario = 0 --AUDIO_SCENARIO_DEFAULT

local has_published_stream = false
if mc_valid(options.has_published_stream) then
  has_published_stream = mc_value(options.has_published_stream)
end

local has_subscribed_stream = false 
if mc_valid(options.has_subscribed_stream) then
  has_subscribed_stream = mc_value(options.has_subscribed_stream)
end

if has_subscribed_stream and not has_published_stream then -- audience
  options.adm_use_hw_aec = false
elseif not has_subscribed_stream and has_published_stream then -- solo broadcaster
  options.adm_use_hw_aec = false
elseif has_subscribed_stream and has_published_stream then -- interactive broadcaster
  options.adm_use_hw_aec = true
else
  options.adm_use_hw_aec = false
end

local audio_routing = 0
if mc_valid(options.audio_routing) then
  audio_routing = mc_value(options.audio_routing)
end

if audio_routing == 1 then --Earpiece
  options.adm_use_hw_aec = true
elseif audio_routing == 0 then --ROUTE_HEADSET
  options.adm_use_hw_aec = false
elseif audio_routing == 2 then --ROUTE_HEADSETNOMIC
  options.adm_use_hw_aec = false
elseif audio_routing == 5 then --ROUTE_HEADSETBLUETOOTH
  options.adm_use_hw_aec = false
else
  --keep no change
end

if not has_subscribed_stream and not has_published_stream then -- out of channel
  options.adm_use_hw_aec = false
end

-- recording device
options.adm_enable_record_but_not_publish = false

local device_magic_id = 0
if mc_valid(dev_status.device_magic_id) then
  device_magic_id = mc_value(dev_status.device_magic_id)
end

local useOpenSLES = false
if mc_valid(options.adm_enable_opensl) then
  useOpenSLES = mc_value(options.adm_enable_opensl)
end

if options.adm_use_hw_aec == true then
  options.adm_audio_source = 7 --VOICE_COMMUNICATION
else
  options.adm_audio_source = 1 --MIC
  if device_magic_id == 315 then --kHuaweiHonor9
    options.adm_audio_source = 6 --VOICE_RECOGNITION
  end
end

-- device specfic settings
if device_magic_id == 25 then --kMi5
  options.derived_headset_black_list_device = true
elseif device_magic_id == 27 then --kMi6
  options.derived_headset_black_list_device = true
elseif device_magic_id == 314 then --kHuaweiHonorV10
  options.derived_headset_black_list_device = true
else
  options.derived_headset_black_list_device = false
end

-- setProfiledAecDelayOffset
if device_magic_id == 1 then --kNexus5
  if useOpenSLES then
    options.apm_delay_offset_ms = 124
  else
    options.apm_delay_offset_ms = 80
  end
elseif device_magic_id == 202 then --kNexus5x
  options.apm_delay_offset_ms = 132
elseif device_magic_id == 201 then --kNexus4
  if useOpenSLES then
    options.apm_delay_offset_ms = 156
  else
    options.apm_delay_offset_ms = 120
  end
elseif device_magic_id == 25 then --kMi5
  options.apm_delay_offset_ms = 80
elseif device_magic_id == 5 then --kMiNote
  if useOpenSLES then
    options.apm_delay_offset_ms = 128
  else
    options.apm_delay_offset_ms = 212
  end
elseif device_magic_id == 22 then --kMiNotePro
  options.apm_delay_offset_ms = 180
elseif device_magic_id == 2 then --kMi4
  if useOpenSLES then
    options.apm_delay_offset_ms = 128
  else
    options.apm_delay_offset_ms = 188
  end
elseif device_magic_id == 4 then --kMi3td
  options.apm_delay_offset_ms = 192
elseif device_magic_id == 3 then --kMi3wc
  if useOpenSLES then
    options.apm_delay_offset_ms = 164
  else
    options.apm_delay_offset_ms = 212
  end
elseif device_magic_id == 6 then --kMi2s
  if useOpenSLES then
    options.apm_delay_offset_ms = 280
  else
    options.apm_delay_offset_ms = 210
  end
elseif device_magic_id == 7 then --kMi2
  if useOpenSLES then
    options.apm_delay_offset_ms = 280
  else
    options.apm_delay_offset_ms = 210
  end
elseif device_magic_id == 9 then --kRedMi
  if useOpenSLES then
    options.apm_delay_offset_ms = 390
  else
    options.apm_delay_offset_ms = 190
  end
elseif device_magic_id == 19 then --kMi2c
  if useOpenSLES then
    options.apm_delay_offset_ms = 280
  else
    options.apm_delay_offset_ms = 178
  end
elseif device_magic_id == 8 then --kMi1
  if useOpenSLES then
    options.apm_delay_offset_ms = 342
  else
    options.apm_delay_offset_ms72
  end
elseif device_magic_id == 11 then --kRedMiLte
  if useOpenSLES then
    options.apm_delay_offset_ms = 400
  else
    options.apm_delay_offset_ms = 260
  end
elseif device_magic_id == 21 then --kRedMi1STD
  if useOpenSLES then
    options.apm_delay_offset_ms = 400
  else
    options.apm_delay_offset_ms = 252
  end
elseif device_magic_id == 10 then --kRedMi1SC
  if useOpenSLES then
    options.apm_delay_offset_ms = 220
  else
    options.apm_delay_offset_ms = 324
  end
elseif device_magic_id == 14 then --kRedMiNote
  if useOpenSLES then
    options.apm_delay_offset_ms = 362
  else
    options.apm_delay_offset_ms = 172
  end
elseif device_magic_id == 20 then --kRedMiNote2
  options.apm_delay_offset_ms = 216
elseif device_magic_id == 15 then --kRedMiNoteLte
  if useOpenSLES then
    options.apm_delay_offset_ms = 316
  else
    options.apm_delay_offset_ms = 298
  end
elseif device_magic_id == 17 then --kRedMiNote1s
  if useOpenSLES then
    options.apm_delay_offset_ms = 182
  else
    options.apm_delay_offset_ms = 232
  end
elseif device_magic_id == 12 then --kRedMi2
  if useOpenSLES then
    options.apm_delay_offset_ms = 100
  else
    options.apm_delay_offset_ms = 228
  end
elseif device_magic_id == 24 then --kRedMi3
  options.apm_delay_offset_ms = 244
elseif device_magic_id == 13 then --kRedMi2A
  if useOpenSLES then
    options.apm_delay_offset_ms = 345
  else
    options.apm_delay_offset_ms = 148
  end
elseif device_magic_id == 34 then --Samsung Galaxy S5
  if useOpenSLES then
    options.apm_delay_offset_ms = 204
  else
    options.apm_delay_offset_ms = 178
  end
elseif device_magic_id == 30 then --kSMNote2
  options.apm_delay_offset_ms = 64
elseif device_magic_id == 31 then --kSMNote3
  options.apm_delay_offset_ms = 180
elseif device_magic_id == 809 then --kSMNote3Intl
  options.apm_delay_offset_ms = 150
elseif device_magic_id == 39 then --kSMGalaxyGrand2
  if useOpenSLES then
    options.apm_delay_offset_ms = 244
  else
    options.apm_delay_offset_ms = 108
  end
elseif device_magic_id == 93 then --kMeizuMX4
  options.apm_delay_offset_ms = 132
elseif device_magic_id == 96 then --kMeizuM1Note
  options.apm_delay_offset_ms = 200
elseif device_magic_id == 98 then --kMeizuProMX5
  options.apm_delay_offset_ms52
elseif device_magic_id == 46 then --kHuaweiHonor3X
  if useOpenSLES then
    options.apm_delay_offset_ms = 228
  else
    options.apm_delay_offset_ms = 160
  end
elseif device_magic_id == 61 then --kZTEMemo5S
  if useOpenSLES then
    options.apm_delay_offset_ms = 336
  else
    options.apm_delay_offset_ms = 208
  end
elseif device_magic_id == 62 then --kNubiaNX403
  if useOpenSLES then
    options.apm_delay_offset_ms = 224
  else
    options.apm_delay_offset_ms = 300
  end
elseif device_magic_id == 65 then --kNubiaNX507
  options.apm_delay_offset_ms = 240
elseif device_magic_id == 71 then --kHtcD816
  if useOpenSLES then
    options.apm_delay_offset_ms = 356
  else
    options.apm_delay_offset_ms = 220
  end
elseif device_magic_id == 63 then --kZTEGrandSII
  if useOpenSLES then
    options.apm_delay_offset_ms = 120
  else
    options.apm_delay_offset_ms = 220
  end
elseif device_magic_id == 101 then --kSonyXperiaZ2
  if useOpenSLES then
    options.apm_delay_offset_ms = 100
  else
    options.apm_delay_offset_ms = 230
  end
elseif device_magic_id == 112 then --kMotoG
  if useOpenSLES then
    options.apm_delay_offset_ms = 176
  else
    options.apm_delay_offset_ms = 220
  end
elseif device_magic_id == 48 then --kHuaweiG700
  if useOpenSLES then
    options.apm_delay_offset_ms = 300
  else
    options.apm_delay_offset_ms = 200
  end
elseif device_magic_id == 57 then --kHuaweiP6
  if useOpenSLES then
    options.apm_delay_offset_ms = 180
  else
    options.apm_delay_offset_ms = 82
  end
elseif device_magic_id == 64 then --kZTEGrandMemoII
  if useOpenSLES then
    options.apm_delay_offset_ms = 220
  else
    options.apm_delay_offset_ms = 150
  end
elseif device_magic_id == 49 then --kHuaweiHonor2
  if useOpenSLES then
    options.apm_delay_offset_ms = 176
  else
    options.apm_delay_offset_ms = 100
  end
elseif device_magic_id == 309 then --kHuaweiHonor7i
  options.apm_delay_offset_ms = 260
elseif device_magic_id == 50 then --kHuaweiC199
  if useOpenSLES then
    options.apm_delay_offset_ms = 136
  else
    options.apm_delay_offset_ms = 224
  end
elseif device_magic_id == 121 then --kVivoX5S
  if useOpenSLES then
    options.apm_delay_offset_ms = 300
  else
    options.apm_delay_offset_ms = 348
  end
elseif device_magic_id == 122 then --kVivoY13T
  if useOpenSLES then
    options.apm_delay_offset_ms = 304
  else
    options.apm_delay_offset_ms = 212
  end
elseif device_magic_id == 123 then --kVivoX3T
  if useOpenSLES then
    options.apm_delay_offset_ms = 200
  else
    options.apm_delay_offset_ms = 240
  end
elseif device_magic_id == 124 then --kVivoX3L
  if useOpenSLES then
    options.apm_delay_offset_ms = 248
  else
    options.apm_delay_offset_ms = 252
  end
elseif device_magic_id == 127 then --kVivoX5pro
    options.apm_delay_offset_ms = 180
elseif device_magic_id == 131 then --kSmartisanT1
  if useOpenSLES then
    options.apm_delay_offset_ms = 120
  else
    options.apm_delay_offset_ms = 228
  end
elseif device_magic_id == 132 then --kSmartisanJG
  options.apm_delay_offset_ms = 226
elseif device_magic_id == 135 then --kOnePlusA1
  options.apm_delay_offset_ms = 210
elseif device_magic_id == 136 then --kOnePlusA2
  options.apm_delay_offset_ms = 232
elseif device_magic_id == 139 then --kOnePlusRest
  options.apm_delay_offset_ms = 196
elseif device_magic_id == 92 then --kCP86Series
  options.apm_delay_offset_ms = 238
elseif device_magic_id == 901 then --kZlr
  options.apm_delay_offset_ms = 212
elseif device_magic_id == 900 then --kAlps
  options.apm_delay_offset_ms = 366
elseif device_magic_id == 75 then --kHtcSenxl
  options.apm_delay_offset_ms = 176
elseif device_magic_id == 170 then --kLenovoA380t
  options.apm_delay_offset_ms = 260
elseif device_magic_id == 175 then --kLenovoA850
  options.apm_delay_offset_ms = 264
elseif device_magic_id == 151 then --kOppoFind7
  options.apm_delay_offset_ms = 200
elseif device_magic_id == 180 then --kLetvRest
  options.apm_delay_offset_ms = 146
elseif device_magic_id == 906 then --kLavaIrisx8
  options.apm_delay_offset_ms = 240
elseif device_magic_id == 911 then --kXoloQ1000s
  options.apm_delay_offset_ms = 224
elseif device_magic_id == 916 then --kmmA106
  options.apm_delay_offset_ms = 240
elseif device_magic_id == 917 then --kmmA110
  options.apm_delay_offset_ms = 188
elseif device_magic_id == 113 then --kMotoG3
  options.apm_delay_offset_ms = 228
elseif device_magic_id == 114 then --kMotoE2
  options.apm_delay_offset_ms = 248
elseif device_magic_id == 145 then --kMeituM4
  options.apm_delay_offset_ms = 324
elseif device_magic_id == 990 then --kCVTE1
  options.apm_delay_offset_ms = 182
elseif device_magic_id == 3001 then --kParallelD
  options.apm_delay_offset_ms = 140
elseif device_magic_id == 3002 then --kMiPad2Win
  options.apm_delay_offset_ms = 84
elseif device_magic_id == 3003 then --kSurfaceWin
  options.apm_delay_offset_ms = 48
elseif device_magic_id == 3006 then --kCrazyTeacher
  options.apm_delay_offset_ms = 48
elseif device_magic_id == 313 then --kHuaweiCX6
  options.apm_delay_offset_ms = 212
else  --default value
  options.apm_delay_offset_ms = 100
end

-- aec parameters start
options.apm_enable_aec = true
options.apm_aec_suppression_level = 1 --EchoCancellation::kModerateSuppression
options.apm_aec_delay_type = 1 --EchoCancellation::kDelayCorrelation
options.apm_aec_nlp_aggressiveness = 0 --EchoCancellation::kNotSpecified
-- aec parameters end

-- ans parameters start
options.apm_enable_ns = true
--options.apm_ns_level = -1U --NoiseSuppression::kNotSpecified
-- ans patameters end

options.apm_enable_md = false -- disable music detection in default
options.apm_enable_highpass_filter = true -- enable high pass filter in default

-- agc parameters start
options.apm_enable_agc = false
if audio_routing == 0 then  --ROUTE_HEADSET
  options.apm_agc_mode = 2 --kFixedDigital
elseif audio_routing == 1 then --ROUTE_EARPIECE
  options.apm_agc_mode = 2 --kFixedDigital
elseif audio_routing == 2 then --ROUTE_HEADSETNOMIC
  options.apm_agc_mode = 2 --kFixedDigital
elseif audio_routing == 3 then --ROUTE_SPEAKERPHONE
  options.apm_agc_mode = 2 --kFixedDigital
elseif audio_routing == 4 then --ROUTE_LOUDSPEAKER
  options.apm_agc_mode = 1 --kAdaptiveDigital
else
  options.apm_agc_mode = 2 --kFixedDigital
end

-- setProfiledAgcConfig, 0-kAdaptiveAnalog, 1-kAdaptiveDigital, 2-kFixedDigital
if audio_routing == 1 then --ROUTE_EARPIECE
  if device_magic_id == 11 then -- kRedMiLte
    options.apm_enable_agc = false
  elseif device_magic_id == 34 then -- KSMS5
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 10 then -- kRedMi1SC
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 81 then -- kCP87Series
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 4 then -- kMi3td
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 7 then -- kMi2
    options.apm_agc_target_level_dbfs = 10
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 6 then -- kMi2s
    options.apm_agc_target_level_dbfs = 10
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 19 then -- kMi2c
    options.apm_agc_target_level_dbfs = 10
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 62 then -- kNubiaNX403
    options.apm_agc_target_level_dbfs = 1
    options.apm_agc_compression_gain_db = 18
    options.apm_agc_mode = 1 --kAgcAdaptiveDigital
  elseif device_magic_id == 48 then -- kHuaweiG700
    options.apm_agc_target_level_dbfs = 1
    options.apm_agc_compression_gain_db = 18
    options.apm_agc_mode = 1 --kAgcAdaptiveDigital
  elseif device_magic_id == 1 then -- kNexus5
    options.apm_agc_target_level_dbfs = 1
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 15 then -- kRedMiNoteLte
    options.apm_agc_target_level_dbfs = 1
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 201 then --kNexus4
    if options.adm_use_hw_aec == true then --mic source is very small when using built-in AEC
      options.apm_agc_target_level_dbfs = 1
      options.apm_agc_compression_gain_db = 18
      options.apm_agc_mode = 1 --kAgcAdaptiveDigital
    else
      options.apm_agc_target_level_dbfs = 10
      options.apm_agc_compression_gain_db = 9 --mic source is kinda saturated, reduce to half
    end
  elseif device_magic_id == 12 then -- kRedMi2
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 9
  else
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 0
  end
elseif audio_routing == 0 then --ROUTE_HEADSET
  if device_magic_id == 2 then --kMi4
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 3 then --kMi3wc
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 4 then --kMi3td
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 7 then --kMi2
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 6 then --kMi2s
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 19 then --kMi2c
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 201 then --kNexus4
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 48 then --kHuaweiG700
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 81 then --kCP87Series
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 13 then --kRedMi2A
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 11 then --kRedMiLte
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 6
  elseif device_magic_id == 12 then --kRedMi2
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 0
  elseif device_magic_id == 14 then --kRedMiNote
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 0
  else
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 8
  end
elseif audio_routing == 2 then --ROUTE_HEADSETNOMIC
  if device_magic_id == 2 then --kMi4
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 3 then --kMi3wc
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 4 then --kMi3td
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 7 then --kMi2
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 6 then --kMi2s
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 19 then --kMi2c
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 201 then --kNexus4
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 48 then --kHuaweiG700
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 81 then --kCP87Series
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 13 then --kRedMi2A
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 11 then --kRedMiLte
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 6
  elseif device_magic_id == 12 then --kRedMi2
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 0
  elseif device_magic_id == 14 then --kRedMiNote
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 0
  else
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 8
  end
elseif audio_routing == 3 then --ROUTE_SPEAKERPHONE
  if device_magic_id == 2 then --kMi4
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 10
  elseif device_magic_id == 806 then --kSMNote2ROM
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
    options.apm_agc_mode = 1 --kAgcAdaptiveDigital
  elseif device_magic_id == 3 then --kMi3wc
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 12
  elseif device_magic_id == 4 then --kMi3td
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 12
  elseif device_magic_id == 9 then --kRedMi
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 34 then --kSMS5
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 203 then --kNexus6p
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 14 then --kRedMiNote
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 12 then --kRedMi2
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 4
  elseif device_magic_id == 7 then --kMi2
    options.apm_agc_target_level_dbfs = 10
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 6 then --kMi2s
    options.apm_agc_target_level_dbfs = 10
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 19 then --kMi2c
    options.apm_agc_target_level_dbfs = 10
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 135 then --kOnePlusA1
    options.apm_agc_target_level_dbfs = 10
    options.apm_agc_compression_gain_db = 9
  elseif device_magic_id == 62 then --kNubiaNX403
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 81 then --kCP87Series
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 5 then --kMiNote
    options.apm_agc_target_level_dbfs = 2
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 1 then --kNexus5
    options.apm_agc_target_level_dbfs = 1
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 172 then --kLenovoA6000
    options.apm_agc_target_level_dbfs = 1
    options.apm_agc_compression_gain_db = 18
  elseif device_magic_id == 201 then --kNexus4
    options.apm_agc_target_level_dbfs = 10
    options.apm_agc_compression_gain_db = 9
  else
    options.apm_agc_target_level_dbfs = 3
    options.apm_agc_compression_gain_db = 9
  end
else
  options.apm_agc_target_level_dbfs = 3
  options.apm_agc_compression_gain_db = 9
end
-- agc parameters end
