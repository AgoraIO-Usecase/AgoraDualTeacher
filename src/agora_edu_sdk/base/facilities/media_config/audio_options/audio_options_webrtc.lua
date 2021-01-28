--[[
	* fields with type of absl::optional may not contain value, 
	* so you should check whether it's valid or not before using it
	* the builtin function of check valid is mc_valid(..)
	* and the builtin function of get real value is mc_value(..)

--]]


-- Note that profile_name is the id of this option profile
profile_name = "webrtc"

#if defined(WEBRTC_IOS)

if mc_valid(ios_force_software_aec_HACK) and mc_value(ios_force_software_aec_HACK) then
  -- EC may be forced on for a device known to have non-functioning platform
  -- AEC.
  echo_cancellation = true
  extended_filter_aec = true
else
  -- On iOS, VPIO provides built-in EC.
  echo_cancellation = false
  extended_filter_aec = false
end

#elif defined(WEBRTC_ANDROID)

extended_filter_aec = false

#endif

-- Delay Agnostic AEC automatically turns on EC if not set except on iOS
-- where the feature is not supported.
#if !defined(WEBRTC_IOS)

if mc_valid(delay_agnostic_aec) and mc_value(delay_agnostic_aec) then
  echo_cancellation = true
  extended_filter_aec = true
end

#endif

-- Set and adjust noise suppressor options.
#if defined(WEBRTC_IOS)

-- On iOS, VPIO provides built-in NS.
typing_detection = false
experimental_ns = false

#elif defined(WEBRTC_ANDROID)

typing_detection = false
experimental_ns = false

#endif

-- Set and adjust gain control options.

#if defined(WEBRTC_IOS)

-- On iOS, VPIO provides built-in AGC.
auto_gain_control = false
experimental_agc = false

#elif defined(WEBRTC_ANDROID)

auto_gain_control = false
experimental_agc = false

#elif defined(WEBRTC_MAC)

auto_gain_control = true
tx_agc_target_dbov = 3
tx_agc_digital_compression_gain = 3
tx_agc_limiter = true

#elif defined(WEBRTC_WIN)

auto_gain_control = true
tx_agc_target_dbov = 5
tx_agc_digital_compression_gain = 10
tx_agc_limiter = true

#elif defined(WEBRTC_LINUX)

auto_gain_control = false

#endif
