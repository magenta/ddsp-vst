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

#include "Constants.h"
#include "JuceHeader.h"

namespace ddsp
{

// Midi-related magic numbers.
static constexpr float kSemitonesPerOctave = 12.0f;
static constexpr float kMidiNoteA4 = 69.0f;
static constexpr float kFreqA4_Hz = 440.f;
// Midi pitch bend messages instruct the synthesizer to shift by a small interval.
// They range from [0,16383], and 8192 represents no pitch bending.
static constexpr float kPitchBendRange = 16384.0f;
static constexpr float kPitchBendBase = kPitchBendRange / 2;
// Most synthesizers are calibrated by default to bend +-2 semitones for this given range.
static constexpr float kPitchRangePerSemitone = kPitchBendRange / 4;

// Remapping methods.
// Maps pitch_Hz to [0,1] based on predefined pitch ranges.
static inline float normalizedPitch (float pitch_Hz)
{
    // Convert to MIDI scale.
    pitch_Hz = juce::jlimit<float> (kPitchRangeMin_Hz, kPitchRangeMax_Hz, pitch_Hz);
    float midi = kSemitonesPerOctave * (std::log2 (pitch_Hz) - std::log2 (kFreqA4_Hz)) + kMidiNoteA4;
    // Normalize.
    return juce::jmap (midi, 0.0f, 127.0f, 0.0f, 1.0f);
}

// Linear maps loudness to [0,1].
// Using 80dB for parity with DDSP-based normalization (github.com/magenta/ddsp/blob/main/ddsp/core.py#L27).
static inline float normalizedLoudness (float dB) { return (dB / 80.0f) + 1.0f; }

// Helper methods.
static inline float offsetPitch (float pitch, int semitoneOffset)
{
    return pitch * std::pow (2, semitoneOffset / kSemitonesPerOctave);
}

static inline std::pair<int, int> pitchLoudnessToMidi (float pitch_Hz, float normalizedLoudness)
{
    pitch_Hz = juce::jlimit<float> (kPitchRangeMin_Hz, kPitchRangeMax_Hz, pitch_Hz);
    int midiPitch =
        static_cast<int> (kSemitonesPerOctave * (std::log2 (pitch_Hz) - std::log2 (kFreqA4_Hz)) + kMidiNoteA4);
    // ensure loudness is between 0 and 1
    normalizedLoudness = juce::jlimit<float> (0.0f, 1.0f, normalizedLoudness);
    int midiVelocity = juce::roundToInt (juce::jmap (normalizedLoudness, 0.0f, 127.0f));

    return std::make_pair (midiPitch, midiVelocity);
}

// Given a midiNote and a pitchBend value, return the pitch in hz.
// Calculations taken from https://dsp.stackexchange.com/questions/1645/converting-a-pitch-bend-midi-value-to-a-normal-pitch-value.
static inline float getFreqFromNoteAndBend (int midiNote, int pitchBend)
{
    const float noteInOctave = (midiNote - kMidiNoteA4) / kSemitonesPerOctave;
    const float pitchBendInOctave = (pitchBend - kPitchBendBase) / kPitchRangePerSemitone / kSemitonesPerOctave;
    // Convert semitones into hz range. Each "octave" represents a doubling in frequency.
    const float f0_Hz = std::pow (2.0f, noteInOctave + pitchBendInOctave) * kFreqA4_Hz;

    return f0_Hz;
}

} // namespace ddsp
