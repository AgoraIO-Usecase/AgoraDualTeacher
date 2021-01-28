--[[
  Input and output are defined in audio_sessions_keyword.h
	* fields with type of absl::optional may not contain value, 
	* so you should check whether it's valid or not before using it
	* the builtin function of check valid is mc_valid(..)
	* and the builtin function of get real value is mc_value(..)

--]]


-- Note that profile_name is the id of this option profile
profile_name = "chatroom_entertainment"

options.audio_scenario = 1 --AUDIO_SCENARIO_CHATROOM_ENTERTAINMENT

local has_published_stream = false
if mc_valid(options.has_published_stream) then
  has_published_stream = mc_value(options.has_published_stream)
end

local has_subscribed_stream = false 
if mc_valid(options.has_subscribed_stream) then
  has_subscribed_stream = mc_value(options.has_subscribed_stream)
end

if has_subscribed_stream and not has_published_stream then -- audience
  options.adm_use_hw_aec = true
elseif not has_subscribed_stream and has_published_stream then -- solo broadcaster
  options.adm_use_hw_aec = true
elseif has_subscribed_stream and has_published_stream then -- interactive broadcaster
  options.adm_use_hw_aec = true
else
  options.adm_use_hw_aec = false
end

local audio_routing = 0
if mc_valid(options.audio_routing) then
  audio_routing = mc_value(options.audio_routing)
end

if audio_routing == 0 then --ROUTE_HEADSET
  options.adm_use_hw_aec = false
elseif audio_routing == 2 then --ROUTE_HEADSETNOMIC
  options.adm_use_hw_aec = false
elseif audio_routing == 5 then --ROUTE_HEADSETBLUETOOTH
  options.adm_use_hw_aec = false
else
  -- keep no change
end

-- recording device
options.adm_enable_record_but_not_publish = true
