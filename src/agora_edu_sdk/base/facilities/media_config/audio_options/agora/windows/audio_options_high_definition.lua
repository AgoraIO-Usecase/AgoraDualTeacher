--[[
	* fields with type of absl::optional may not contain value, 
	* so you should check whether it's valid or not before using it
	* the builtin function of check valid is mc_valid(..)
	* and the builtin function of get real value is mc_value(..)

--]]

-- Note that profile_name is the id of this option profile
profile_name = "high_definition"
options.audio_scenario =6 --AUDIO_SCENARIO_HIGH_DEFINITION

options.apm_enable_aec = true
options.apm_enable_ns = true
options.apm_enable_agc = false --disable agc in game_streaming
options.apm_enable_md = true -- enable music detection in game_streaming
options.apm_enable_highpass_filter = false -- disable high pass filter in game_streaming


if mc_valid(options.apm_override_lua_enable_aec) and mc_value(options.apm_override_lua_enable_aec) then
	options.adm_use_hw_aec = true
	options.apm_enable_aec = false
end

if mc_valid(options.apm_override_lua_enable_ns) and mc_value(options.apm_override_lua_enable_ns) then
	options.apm_enable_ns = false
end

if mc_valid(options.apm_override_lua_enable_agc) and mc_value(options.apm_override_lua_enable_agc) then
	options.apm_enable_agc = false
end

if mc_valid(options.apm_override_lua_enable_md) and mc_value(options.apm_override_lua_enable_md) then
	options.apm_enable_md = false
end