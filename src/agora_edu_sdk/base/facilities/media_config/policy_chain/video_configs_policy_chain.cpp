//
// Agora Media SDK
//
// Copyright (c) 2019 Agora IO. All rights reserved.
//
#include "facilities/media_config/policy_chain/video_configs_policy_chain.h"

#include "base/AgoraBase.h"
#include "engine_adapter/video/video_node_internal.h"
#include "system_wrappers/include/cpu_info.h"

namespace agora {
namespace utils {

VideoConfigsPolicyChain::VideoConfigsPolicyChain() {
  // Get system value and agora default value
  QueryDefaultValue(options_[CONFIG_PRIORITY_INTERNAL]);
  QuerySystemValue(options_[CONFIG_PRIORITY_DEVICE]);

  // Calculate final value
  final_ = Calculate();
  old_final_ = final_;
}

bool VideoConfigsPolicyChain::Apply(rtc::VideoConfigurationEx& old_val,
                                    rtc::VideoConfigurationEx& new_val) {
  old_val.SetAll(new_val);
  return true;
}

rtc::VideoConfigurationEx VideoConfigsPolicyChain::Diff(const rtc::VideoConfigurationEx& lhs,
                                                        const rtc::VideoConfigurationEx& rhs) {
  rtc::VideoConfigurationEx ret;
#define GET_DIFF(X) ret.X = optional_diff(lhs.X, rhs.X)

  GET_DIFF(codec_type);
  GET_DIFF(frame_width);
  GET_DIFF(frame_height);
  GET_DIFF(frame_rate);
  GET_DIFF(start_bitrate);
  GET_DIFF(target_bitrate);
  GET_DIFF(min_bitrate);
  GET_DIFF(max_bitrate);
  GET_DIFF(orientation_mode);
  GET_DIFF(number_of_temporal_layers);
  GET_DIFF(sps_data);
  GET_DIFF(pps_data);
  GET_DIFF(h264_profile);
  GET_DIFF(adaptive_op_mode);
  GET_DIFF(number_of_spatial_layers);
  GET_DIFF(flexible_mode);
  GET_DIFF(interlayer_pred);
  GET_DIFF(num_of_encoder_cores);
  GET_DIFF(degradation_preference);
  GET_DIFF(complexity);
  GET_DIFF(denoising_on);
  GET_DIFF(automatic_resize_on);
  GET_DIFF(frame_dropping_on);
  GET_DIFF(key_frame_interval);
  GET_DIFF(entropy_coding_mode_flag);
  GET_DIFF(loop_filter_disable_idc);
  GET_DIFF(background_detection_on);
  GET_DIFF(posted_frames_waiting_for_encode);
  GET_DIFF(enable_hw_encoder);
  GET_DIFF(enable_hw_decoder);
  GET_DIFF(av_dec_common_input_format);
  GET_DIFF(av_dec_common_output_format);
  GET_DIFF(av_dec_mmcss_class);
  GET_DIFF(av_enc_codec_type);
  GET_DIFF(av_enc_common_buffer_in_level);
  GET_DIFF(av_enc_common_buffer_out_level);
  GET_DIFF(av_enc_common_buffer_size);
  GET_DIFF(av_enc_common_format_constraint);
  GET_DIFF(av_enc_common_low_latency);
  GET_DIFF(av_enc_common_max_bit_rate);
  GET_DIFF(av_enc_common_mean_bit_rate);
  GET_DIFF(av_enc_common_mean_bit_rate_interval);
  GET_DIFF(av_enc_common_min_bit_rate);
  GET_DIFF(av_enc_common_quality);
  GET_DIFF(av_enc_common_quality_vs_speed);
  GET_DIFF(av_enc_common_rate_control_mode);
  GET_DIFF(av_enc_common_real_time);
  GET_DIFF(av_enc_common_stream_end_handling);
  GET_DIFF(av_enc_mux_output_stream_type);
  GET_DIFF(av_dec_video_acceleration_h264);
  GET_DIFF(av_dec_video_acceleration_mpeg2);
  GET_DIFF(av_dec_video_acceleration_vc1);
  GET_DIFF(av_dec_video_drop_pic_with_missing_ref);
  GET_DIFF(av_dec_video_fast_decode_mode);
  GET_DIFF(av_dec_video_input_scan_type);
  GET_DIFF(av_dec_video_pixel_aspect_ratio);
  GET_DIFF(av_dec_video_software_deinterlace_mode);
  GET_DIFF(av_dec_video_sw_power_level);
  GET_DIFF(av_dec_video_thumbnail_generation_mode);
  GET_DIFF(av_enc_input_video_system);
  GET_DIFF(av_enc_video_cbr_motion_tradeoff);
  GET_DIFF(av_enc_video_coded_video_access_unit_size);
  GET_DIFF(av_enc_video_default_upper_field_dominant);
  GET_DIFF(av_enc_video_display_dimension);
  GET_DIFF(av_enc_video_encode_dimension);
  GET_DIFF(av_enc_video_encode_offset_origin);
  GET_DIFF(av_enc_video_field_swap);
  GET_DIFF(av_enc_video_force_source_scan_type);
  GET_DIFF(av_enc_video_header_drop_frame);
  GET_DIFF(av_enc_video_header_frames);
  GET_DIFF(av_enc_video_header_hours);
  GET_DIFF(av_enc_video_header_minutes);
  GET_DIFF(av_enc_video_header_seconds);
  GET_DIFF(av_enc_video_input_chroma_resolution);
  GET_DIFF(av_enc_video_input_chroma_subsampling);
  GET_DIFF(av_enc_video_input_color_lighting);
  GET_DIFF(av_enc_video_input_color_nominal_range);
  GET_DIFF(av_enc_video_input_color_primaries);
  GET_DIFF(av_enc_video_input_color_transfer_function);
  GET_DIFF(av_enc_video_input_color_transfer_matrix);
  GET_DIFF(av_enc_video_inverse_telecine_enable);
  GET_DIFF(av_enc_video_inverse_telecine_threshold);
  GET_DIFF(av_enc_video_max_keyframe_distance);
  GET_DIFF(av_enc_video_no_of_fields_to_encode);
  GET_DIFF(av_enc_video_no_of_fields_to_skip);
  GET_DIFF(av_enc_video_output_chroma_resolution);
  GET_DIFF(av_enc_video_output_chroma_subsampling);
  GET_DIFF(av_enc_video_output_color_lighting);
  GET_DIFF(av_enc_video_output_color_nominal_range);
  GET_DIFF(av_enc_video_output_color_primaries);
  GET_DIFF(av_enc_video_output_color_transfer_function);
  GET_DIFF(av_enc_video_output_color_transfer_matrix);
  GET_DIFF(av_enc_video_output_frame_rate);
  GET_DIFF(av_enc_video_output_frame_rate_conversion);
  GET_DIFF(av_enc_video_output_scan_type);
  GET_DIFF(av_enc_video_pixel_aspect_ratio);
  GET_DIFF(av_enc_video_source_film_content);
  GET_DIFF(av_enc_video_source_is_bw);
  GET_DIFF(av_enc_mpv_add_seq_end_code);
  GET_DIFF(av_enc_mpv_default_b_picture_count);
  GET_DIFF(av_enc_mpv_frame_field_mode);
  GET_DIFF(av_enc_mpv_generate_header_pic_disp_ext);
  GET_DIFF(av_enc_mpv_generate_header_pic_ext);
  GET_DIFF(av_enc_mpv_generate_header_seq_disp_ext);
  GET_DIFF(av_enc_mpv_generate_header_seq_ext);
  GET_DIFF(av_enc_mpv_generate_header_seq_scale_ext);
  GET_DIFF(av_enc_mpvgop_open);
  GET_DIFF(av_enc_mpvgops_in_seq);
  GET_DIFF(av_enc_mpvgop_size);
  GET_DIFF(av_enc_mpv_intra_dc_precision);
  GET_DIFF(av_enc_mpv_intra_vlc_table);
  GET_DIFF(av_enc_mpv_level);
  GET_DIFF(av_enc_mpv_profile);
  GET_DIFF(av_enc_mpvq_scale_type);
  GET_DIFF(av_enc_mpv_quant_matrix_chroma_intra);
  GET_DIFF(av_enc_mpv_quant_matrix_chroma_non_intra);
  GET_DIFF(av_enc_mpv_quant_matrix_intra);
  GET_DIFF(av_enc_mpv_quant_matrix_non_intra);
  GET_DIFF(av_enc_mpv_scan_pattern);
  GET_DIFF(av_enc_mpv_scene_detection);
  GET_DIFF(av_enc_mpv_use_concealment_motion_vectors);

#undef GET_DIFF

  return ret;
}

// refer to FillDefault (SEncParamExt& param)
void VideoConfigsPolicyChain::QueryDefaultValue(rtc::VideoConfigurationEx& val) {
  static const int kGopSeconds = 600;
  val.codec_type = static_cast<int>(webrtc::kVideoCodecH264);
  val.denoising_on = false;
  val.automatic_resize_on = false;
  val.frame_dropping_on = false;
  val.frame_rate = rtc::VIDEO_ENCODER_FRAME_RATE_DEFAULT;
  val.min_bitrate = rtc::VIDEO_ENCODER_MIN_BITRATE_DEFAULT;
  val.max_bitrate = rtc::VIDEO_ENCODER_MAX_BITRATE_DEFAULT;
  val.target_bitrate = rtc::VIDEO_ENCODER_BITRATE_DEFAULT;
  val.start_bitrate = rtc::VIDEO_ENCODER_MAX_BITRATE_DEFAULT;
  val.num_of_encoder_cores = webrtc::CpuInfo::DetectNumberOfCores();
  val.frame_width = rtc::VIDEO_RESOLUTION_WIDTH_DEFAULT;
  val.frame_height = rtc::VIDEO_RESOLUTION_HEIGHT_DEFAULT;
  val.complexity = static_cast<int>(webrtc::VideoCodecComplexity::kComplexityNormal);
  val.degradation_preference = static_cast<int>(rtc::MAINTAIN_FRAMERATE);
  val.key_frame_interval = val.frame_rate.value() * kGopSeconds;
#if defined(WEBRTC_IOS)
  val.enable_hw_encoder = true;
  val.enable_hw_decoder = true;
#elif defined(WEBRTC_ANDROID)
  val.enable_hw_encoder = true;
  val.enable_hw_decoder = false;
#elif defined(WEBRTC_WIN)
  val.enable_hw_encoder = true;
  val.enable_hw_decoder = false;
#else
  val.enable_hw_encoder = false;
  val.enable_hw_decoder = false;
#endif

  val.posted_frames_waiting_for_encode = 2;
  val.entropy_coding_mode_flag = 0;
  val.loop_filter_disable_idc = 0;
  val.background_detection_on = true;
}

void VideoConfigsPolicyChain::QuerySystemValue(rtc::VideoConfigurationEx& val) {
  // TODO(Ender):
  // check device capabilities
}

}  // namespace utils
}  // namespace agora
