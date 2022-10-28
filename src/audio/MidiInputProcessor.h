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

#include "JuceHeader.h"

#include "audio/tflite/ModelBase.h"
#include "audio/tflite/ModelTypes.h"
#include "util/InputUtils.h"

namespace ddsp
{

class MidiInputProcessor
{
public:
    void prepareToPlay (double sampleRate, int blockSize);
    void processMidiMessages (juce::MidiBuffer& midiMessages);
    // Threadsafe call to fetch audio-related features for further processing.
    ddsp::AudioFeatures getCurrentPredictControlsInput();

    // ADSR setters/getters. Modifies the amplitude envelope of each set midi note.
    void setAttack (float attackTimeSeconds);
    void setDecay (float decayTimeSeconds);
    void setSustain (float sustainLevel);
    void setRelease (float releaseTimeSeconds);

    // TODO: Add MIDI feature snapping module.
    // TODO: Add Vibrato/LFO for pitch/loudness.

private:
    // User-provided hop-size, intended to be overwritten after calling prepareToPlay().
    int userHopSize = 0;

    std::atomic<int> currentPitchBend = { static_cast<int> (ddsp::kPitchBendBase) };
    std::atomic<float> currentMidiNote = { ddsp::kMidiNoteA4 };
    std::atomic<float> currentMidiVelocity = { 0.0f };

    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
};

} // namespace ddsp