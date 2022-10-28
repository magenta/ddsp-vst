/*
Copyright 2022 The DDSP-VST Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#pragma once

#include <string_view>

namespace ddsp
{

// If true, model inference is carried out on the audio thread instead of the timer thread.
constexpr bool kInferenceOnAudioThread = false;
constexpr bool kEnableReverb = true;

// Model related constants.
// Input and output tensor names as string refs.
inline constexpr std::string_view kInputTensorName_F0 = "call_f0_scaled:0";
inline constexpr std::string_view kInputTensorName_Loudness = "call_pw_scaled:0";
inline constexpr std::string_view kInputTensorName_State = "call_state:0";

inline constexpr std::string_view kOutputTensorName_Amplitude = "StatefulPartitionedCall:0";
inline constexpr std::string_view kOutputTensorName_Harmonics = "StatefulPartitionedCall:1";
inline constexpr std::string_view kOutputTensorName_NoiseAmps = "StatefulPartitionedCall:2";
inline constexpr std::string_view kOutputTensorName_State = "StatefulPartitionedCall:3";

constexpr int kNumPredictControlsInputTensors = 3;
constexpr int kNumPredictControlsOutputTensors = 4;
constexpr int kNumFeatureExtractionThreads = 4;
constexpr int kNumPredictControlsThreads = 1;
constexpr int kNoiseAmpsSize = 65;
constexpr int kHarmonicsSize = 60;
constexpr int kAmplitudeSize = 1;
constexpr int kLoudnessSize = 1;
constexpr int kF0Size = 1;
constexpr int kNumEmbeddedPredictControlsModels = 11;
constexpr int kGruModelStateSize = 512;

// The models were trained at 16 kHz sample rate.
constexpr float kModelSampleRate_Hz = 16000.0f;
constexpr float kModelInferenceTimerCallbackInterval_ms = 20.0f;
constexpr float kTotalInferenceLatency_ms = 64.0f;
constexpr int kModelFrameSize = 1024;
constexpr int kModelHopSize = 320;

// URLs.
inline constexpr std::string_view kModelTrainingColabUrl = "https://g.co/magenta/train-ddsp-vst";
inline constexpr std::string_view kInfoUrl = "https://g.co/magenta/ddsp-vst-help";

constexpr float kPitchRangeMin_Hz = 8.18f; // midi 0
constexpr float kPitchRangeMax_Hz = 12543.84f; // midi 127

} // namespace ddsp
