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

#include "audio/MidiInputProcessor.h"

namespace ddsp
{

void MidiInputProcessor::prepareToPlay (double sampleRate, int blockSize)
{
    jassert (blockSize > 0);
    userHopSize = blockSize;

    // ADSR.
    adsr.setSampleRate (sampleRate);
}

void MidiInputProcessor::processMidiMessages (juce::MidiBuffer& midiMessages)
{
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        const int midiNoteNumber = static_cast<float> (message.getNoteNumber());

        if (message.isNoteOn())
        {
            adsr.noteOn();

            // TODO: Add a lock-free message queue to pass midi messages.
            currentMidiNote.store (midiNoteNumber, std::memory_order_release);
            currentMidiVelocity.store (message.getFloatVelocity(), std::memory_order_release);
        }
        if (message.isNoteOff())
        {
            // Only turn off if the current note is turned off.
            if (midiNoteNumber == currentMidiNote.load (std::memory_order_acquire))
            {
                adsr.noteOff();
            }
        }
        if (message.isPitchWheel())
        {
            currentPitchBend.store (message.getPitchWheelValue(), std::memory_order_release);
        }
    }
}

AudioFeatures MidiInputProcessor::getCurrentPredictControlsInput()
{
    AudioFeatures predictControlsInput;
    const float f0_hz = getFreqFromNoteAndBend (currentMidiNote.load (std::memory_order_acquire),
                                                currentPitchBend.load (std::memory_order_acquire));
    const float f0_norm = juce::mapFromLog10 (f0_hz, kPitchRangeMin_Hz, kPitchRangeMax_Hz);
    predictControlsInput.f0_norm = f0_norm;
    predictControlsInput.f0_hz = f0_hz;

    for (int i = 0; i < userHopSize; ++i)
    {
        predictControlsInput.loudness_norm =
            adsr.getNextSample() * currentMidiVelocity.load (std::memory_order_acquire);
    }

    return predictControlsInput;
}

void MidiInputProcessor::setAttack (float attackTimeSeconds)
{
    adsrParams.attack = attackTimeSeconds;
    adsr.setParameters (adsrParams);
}
void MidiInputProcessor::setDecay (float decayTimeSeconds)
{
    adsrParams.decay = decayTimeSeconds;
    adsr.setParameters (adsrParams);
}
void MidiInputProcessor::setSustain (float sustainLevel)
{
    adsrParams.sustain = sustainLevel;
    adsr.setParameters (adsrParams);
}
void MidiInputProcessor::setRelease (float releaseTimeSeconds)
{
    adsrParams.release = releaseTimeSeconds;
    adsr.setParameters (adsrParams);
}

} // namespace ddsp