--[[
  Input and output are defined in audio_sessions_keyword.h
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

options.apm_enable_md = true -- enable music detection in game_streaming
options.apm_enable_highpass_filter = false -- disable high pass filter in game_streaming
options.playback_volume = 80 --scale playback volume to 80%

-- iOS some device need force restart ADM when occur some issue
local prev_audio_routing = 99
if mc_valid(prev_options.audio_routing) then
  prev_audio_routing = mc_value(prev_options.audio_routing)
end

local curr_audio_routing = 99
if mc_valid(options.audio_routing) then
  curr_audio_routing = mc_value(options.audio_routing)
end

local prev_headset = false
if prev_audio_routing == 0 then
  prev_headset = true
elseif prev_audio_routing == 2 then
  prev_headset = true
else
end

local curr_headset = false
if curr_audio_routing == 0 then
  curr_headset = true
elseif curr_audio_routing == 2 then
  curr_headset = true
else
end

local device_magic_id = 0
if mc_valid(dev_status.device_magic_id) then
  device_magic_id = mc_value(dev_status.device_magic_id)
end

local ipad2_or_ipad3 = false
if device_magic_id == 1101 then
  ipad2_or_ipad3 = true
elseif device_magic_id == 1103 then
  ipad2_or_ipad3 = true
else
end

if prev_headset and not curr_headset and ipad2_or_ipad3 then
  options.adm_force_restart = true
end
