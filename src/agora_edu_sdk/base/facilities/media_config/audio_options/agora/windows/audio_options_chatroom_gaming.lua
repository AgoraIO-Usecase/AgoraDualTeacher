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

options.apm_enable_ns = true
options.apm_enable_agc = true

if mc_valid(options.has_subscribed_stream) and mc_value(options.has_subscribed_stream) and
   mc_valid(options.has_published_stream) and mc_value(options.has_published_stream) then
options.apm_enable_pitch_smoother = true
else
options.apm_enable_pitch_smoother = false
end
