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

namespace ddsp
{

class AudioRingBuffer : public juce::AbstractFifo
{
public:
    AudioRingBuffer (int bufferSize) : AbstractFifo (bufferSize) { buffer.setSize (1, bufferSize); }
    // Copies the incoming buffer into the ring buffer.
    void push (const juce::AudioBuffer<float>& bufferToAdd)
    {
        int start1, size1, start2, size2;
        prepareToWrite (bufferToAdd.getNumSamples(), start1, size1, start2, size2);
        if (size1 > 0)
        {
            buffer.copyFrom (0, start1, bufferToAdd.getReadPointer (0), size1);
        }
        if (size2 > 0)
        {
            buffer.copyFrom (0, start2, bufferToAdd.getReadPointer (0, size1), size2);
        }
        finishedWrite (size1 + size2);
    }
    // Advances the read pointer by numSamples. Does not clear the samples or return them.
    void pop (int numSamples)
    {
        int start1, size1, start2, size2;
        prepareToRead (numSamples, start1, size1, start2, size2);
        finishedRead (size1 + size2);
    }
    // Copies the front of the buffer into bufferToFill.
    void copy (juce::AudioBuffer<float>& bufferToFill)
    {
        int start1, size1, start2, size2;
        prepareToRead (bufferToFill.getNumSamples(), start1, size1, start2, size2);
        if (size1 > 0)
        {
            bufferToFill.copyFrom (0, 0, buffer.getReadPointer (0, start1), size1);
        }
        if (size2 > 0)
        {
            bufferToFill.copyFrom (0, size1, buffer.getReadPointer (0, start2), size2);
        }
    }

    // Zero-pads the buffer.
    void clear()
    {
        reset();
        buffer.clear();
    }

private:
    juce::AudioBuffer<float> buffer;
};

} // namespace ddsp
