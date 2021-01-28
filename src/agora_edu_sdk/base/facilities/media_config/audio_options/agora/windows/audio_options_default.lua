--[[
	* fields with type of absl::optional may not contain value, 
	* so you should check whether it's valid or not before using it
	* the builtin function of check valid is mc_valid(..)
	* and the builtin function of get real value is mc_value(..)

--]]

-- Note that profile_name is the id of this option profile
profile_name = "default"
options.audio_scenario = 0 --AUDIO_SCENARIO_DEFAULT

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
if mc_valid(dev_status.max_cpu_freq_in_mhz) and mc_value(dev_status.max_cpu_freq_in_mhz) < 3000 then
	options.apm_enable_agc = false --for low cpu device, we disable agc to lower cost
else
	options.apm_enable_agc = true
	options.apm_agc_mode = 0 --kAdaptiveAnalog
	options.apm_agc_target_level_dbfs = 5
	options.apm_agc_compression_gain_db = 10
end

options.apm_enable_md = false -- disable music detection in default
options.apm_enable_highpass_filter = true -- enable high pass filter in default

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
