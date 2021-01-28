--[[
	* fields with type of absl::optional may not contain value, 
	* so you should check whether it's valid or not before using it
	* the builtin function of check valid is mc_valid(..)
	* and the builtin function of get real value is mc_value(..)

--]]


-- Note that profile_name is the id of this option profile
profile_name = "high_definition"

options.audio_scenario = 6 --AUDIO_SCENARIO_HIGH_DEFINITION
-- never use hw aec in game streaming mode
options.adm_use_hw_aec = false
-- recording device
options.adm_enable_record_but_not_publish = false
-- do not use opensl in game_streaming mode as it may cause aec problem
options.adm_enable_opensl = false
options.adm_audio_layer = 5  --kAndroidJavaAudio

local audio_routing = 0
if mc_valid(options.audio_routing) then
  audio_routing = mc_value(options.audio_routing)
end

if audio_routing == 1 then --Earpiece
  options.adm_use_hw_aec = true
else
  options.adm_use_hw_aec = false
end

-- preselectAudioDevices
local is_simulator = false
if mc_valid(dev_status.is_simulator) then
  is_simulator = mc_value(dev_status.is_simulator)
end

local device_magic_id = 0
if mc_valid(dev_status.device_magic_id) then
  device_magic_id = mc_value(dev_status.device_magic_id)
end

local useOpenSLES = false
if mc_valid(options.adm_enable_opensl) then
  useOpenSLES = mc_value(options.adm_enable_opensl)
end

options.adm_input_sample_rate = 48000
options.adm_output_sample_rate = 48000
options.adm_stereo_out = true
if is_simulator == true then
  options.adm_stereo_out = false
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

-- agc parameters start
options.apm_enable_agc = false --disable agc in game_streaming
-- agc parameters end
