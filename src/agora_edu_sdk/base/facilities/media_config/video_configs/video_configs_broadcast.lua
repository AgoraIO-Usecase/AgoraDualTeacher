--[[
    * fields with type of absl::optional may not contain value, 
    * so you should check whether it's valid or not before using it
    * the builtin function of check valid is mc_valid(..)
    * and the builtin function of get real value is mc_value(..)

--]]

-- used to override policy: small video to disable hw encoder
vdm_not_override_lua_smallvideo_not_use_hwenc_policy = true

-- Note that profile_name is the id of this option profile
profile_name = "broadcast"

if mc_valid(frame_width) and mc_value(frame_width) >= 1920 and mc_valid(frame_height) and mc_value(frame_height) >= 1080 then
    if mc_valid(enable_hw_encoder) and mc_value(enable_hw_encoder) then
        -- If hw encoder is anebled, not that much of CPU
    else
        -- Otherwise HD needs really lots of CPUs
    end
end

if mc_valid(frame_width) and mc_value(frame_width) < 256 and mc_valid(frame_height) and mc_value(frame_height) < 256 and mc_valid(vdm_not_override_lua_smallvideo_not_use_hwenc_policy) and mc_value(vdm_not_override_lua_smallvideo_not_use_hwenc_policy) then
    -- do not use hw encoder on small resolution
    enable_hw_encoder = false
end