--[[
  Input and output are defined in audio_sessions_keyword.h
	* fields with type of absl::optional may not contain value, 
	* so you should check whether it's valid or not before using it
	* the builtin function of check valid is mc_valid(..)
	* and the builtin function of get real value is mc_value(..)

--]]


-- Note that profile_name is the id of this option profile
profile_name = "chatroom_gaming"

options.audio_scenario = 5 --AUDIO_SCENARIO_CHATROOM_GAMING

local audio_routing = 99
if mc_valid(options.audio_routing) then
  audio_routing = mc_value(options.audio_routing)
end
local has_published_stream = false
if mc_valid(options.has_published_stream) then
  has_published_stream = mc_value(options.has_published_stream)
end
local has_subscribed_stream = false
if mc_valid(options.has_subscribed_stream) then
  has_subscribed_stream = mc_value(options.has_subscribed_stream)
end

if audio_routing == 0 then --ROUTE_HEADSET
  options.adm_use_hw_aec = false
elseif audio_routing == 2 then --ROUTE_HEADSETNOMIC
  options.adm_use_hw_aec = false
elseif audio_routing == 5 then --ROUTE_HEADSETBLUETOOTH
else
  if has_subscribed_stream and not has_published_stream then -- audience
    options.adm_use_hw_aec = true
  elseif not has_subscribed_stream and has_published_stream then -- solo broadcaster
    options.adm_use_hw_aec = true
  elseif has_subscribed_stream and has_published_stream then -- interactive broadcaster
    options.adm_use_hw_aec = true
  else
  end
end

-- recording device
options.adm_enable_record_but_not_publish = true

-- aec parameters start
-- aec parameters end

-- ans parameters start
if has_subscribed_stream and not has_published_stream then -- audience
  options.apm_enable_ns = false
elseif not has_subscribed_stream and has_published_stream then -- solo broadcaster
  if audio_routing == 3 then --ROUTE_SPEAKERPHONE
    options.apm_enable_ns = true
  else
    options.apm_enable_ns = false
  end
elseif has_subscribed_stream and has_published_stream then -- interactive broadcaster
  options.apm_enable_ns = false
else
end
--options.apm_ns_level = -1U --NoiseSuppression::kNotSpecified
-- ans patameters end

-- agc parameters start
options.apm_enable_agc = false --disable agc in default scenario
-- agc parameters end

if mc_valid(options.has_subscribed_stream) and mc_value(options.has_subscribed_stream) and
   mc_valid(options.has_published_stream) and mc_value(options.has_published_stream) then
options.apm_enable_pitch_smoother = true
else
options.apm_enable_pitch_smoother = false
end

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
