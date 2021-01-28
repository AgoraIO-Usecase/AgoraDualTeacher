--[[
  Input and output are defined in audio_sessions_keyword.h
	* fields with type of absl::optional may not contain value, 
	* so you should check whether it's valid or not before using it
	* the builtin function of check valid is mc_valid(..)
	* and the builtin function of get real value is mc_value(..)

--]]


-- Note that profile_name is the id of this option profile
profile_name = "high_definition"

-- playbackAndRecord
configs.playbackAndRecord = true

local is_low_cpu_device = false
if mc_valid(params.is_low_cpu_device) then
  is_low_cpu_device = mc_value(params.is_low_cpu_device)
end

local is_simulator = false
if mc_valid(params.is_simulator) then
  is_simulator = mc_value(params.is_simulator)
end

-- sampleRate
if is_simulator then
  configs.sampleRate = 44100
else
  if is_low_cpu_device then
    configs.sampleRate = 16000
  else
    configs.sampleRate = 48000
  end
end

-- inputNumberOfChannels
configs.inputNumberOfChannels = 1

-- outputNumberOfChannels
configs.outputNumberOfChannels = 2

-- ioBufferDuration
configs.ioBufferDuration = 0.02

-- allowBluetooth
configs.allowBluetooth = false
configs.allowBluetoothA2DP = true

-- allowMixWithOthers
configs.allowMixWithOthers = true

-- chatMode
configs.chatMode = false

-- about device id
local devid = 0
if mc_valid(params.device_magic_id) then
  devid = mc_value(params.device_magic_id)
end

-- special setting for kIPhonelow
if devid == 1000 then
  -- use_hardware_ = false;
end

-- special setting for kIPhone4
if devid == 1001 then
  -- use_hardware_ = false;
end

-- special setting for kIPhone4S
if devid == 1002 then
  -- use_hardware_ = false;
end

-- special setting for kIPhone5
if devid == 1003 then
  -- use_hardware_ = false;
end

-- special setting for kIPhone5S
if devid == 1004 then
  -- use_hardware_ = false;
end

-- special setting for kIPad1
if devid == 1100 then
  -- use_hardware_ = false;
end

-- special setting for kIPad2
if devid == 1101 then
  -- use_hardware_ = false;
  -- mpChatEngineObserver->OnChatEngineEvent(agora::media::IChatEngineObserver::AUDIO_ADM_REQUIRE_RESTART);
end

-- special setting for kIPadMini1
if devid == 1102 then
  -- use_hardware_ = false;
end

-- special setting for kIPad3
if devid == 1103 then
  -- use_hardware_ = false;
end

-- special setting for kIPad4
if devid == 1104 then
  -- use_hardware_ = false;
end

-- special setting for kIPod1
if devid == 1201 then
  -- use_hardware_ = false;
end

-- special setting for kIPod2
if devid == 1202 then
  -- use_hardware_ = false;
end

-- special setting for kIPod3
if devid == 1203 then
  -- use_hardware_ = false;
end

-- special setting for kIPod4
if devid == 1204 then
  -- use_hardware_ = false;
end

-- special setting for kIPod5
if devid == 1205 then
  -- use_hardware_ = false;
end

-- about low end cpu
