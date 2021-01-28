//  Agora RTC/MEDIA SDK
//
//  Created by Pengfei Han in 2020-07.
//  Copyright (c) 2020 Agora.io. All rights reserved.
//

#pragma once

#include <memory>
#include <string>

#include "AgoraRefPtr.h"
#include "facilities/tools/resample_statistic.h"
#include "modules/audio_mixer/output_rate_calculator.h"

namespace agora {
namespace rtc {

class OutputRateObserver : virtual public agora::RefCountInterface {
 public:
  virtual ~OutputRateObserver();
  virtual int OutputRateCalculateCompleted(int outputSampleRate) = 0;
};

class OutputRateSuggestion : virtual public agora::RefCountInterface {
 public:
  virtual ~OutputRateSuggestion();
  virtual int GetPreferredOutputRate() = 0;
};

class OutputSampleRateCalculator : public webrtc::OutputRateCalculator {
 public:
  OutputSampleRateCalculator(const std::string& position,
                             const agora_refptr<OutputRateSuggestion> outputRateSuggestion,
                             const agora_refptr<OutputRateObserver> outputRateObserver,
                             const agora_refptr<ResampleStatistic> resampleStatistics);
  virtual ~OutputSampleRateCalculator();

  int CalculateOutputRate(const std::vector<int>& preferred_sample_rates) override;

 private:
  const std::string position_;
  const std::unique_ptr<OutputRateCalculator> output_rate_calculator_;
  const agora_refptr<OutputRateSuggestion> output_rate_suggestion_;
  const agora_refptr<OutputRateObserver> output_rate_observer_;
  const agora_refptr<ResampleStatistic> resample_statistics_;
  int32_t processed_frames_;
};

}  // namespace rtc
}  // namespace agora
