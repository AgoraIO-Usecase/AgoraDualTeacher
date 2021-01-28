//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-07.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#include "output_sample_rate_calculator.h"

#include "modules/audio_mixer/default_output_rate_calculator.h"

namespace agora {
namespace rtc {

OutputRateObserver::~OutputRateObserver() = default;

OutputRateSuggestion::~OutputRateSuggestion() = default;

OutputSampleRateCalculator::~OutputSampleRateCalculator() = default;

OutputSampleRateCalculator::OutputSampleRateCalculator(
    const std::string& position, const agora_refptr<OutputRateSuggestion> outputRateSuggestion,
    const agora_refptr<OutputRateObserver> outputRateObserver,
    const agora_refptr<ResampleStatistic> resampleStatistics)
    : position_(position),
      output_rate_calculator_(new webrtc::DefaultOutputRateCalculator()),
      output_rate_suggestion_(outputRateSuggestion),
      output_rate_observer_(outputRateObserver),
      resample_statistics_(resampleStatistics),
      processed_frames_(0) {}

int OutputSampleRateCalculator::CalculateOutputRate(
    const std::vector<int>& preferred_sample_rates) {
  int outSampleRateHz = output_rate_calculator_->CalculateOutputRate(preferred_sample_rates);
  if (output_rate_observer_ && !preferred_sample_rates.empty()) {
    output_rate_observer_->OutputRateCalculateCompleted(outSampleRateHz);
  }
  for (auto sampleRate : preferred_sample_rates) {
    if (sampleRate != outSampleRateHz) {
      resample_statistics_->LogStatistic(position_, sampleRate, outSampleRateHz);
    }
  }
  ++processed_frames_;
  return outSampleRateHz;
}

}  // namespace rtc
}  // namespace agora
