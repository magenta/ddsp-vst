#include <memory>

#include "PluginProcessor.h"

#include <gtest/gtest.h>

constexpr char ASSETS_DIR[] = "assets";

juce::File locateAsset (const std::string& name)
{
    juce::File executableDir = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    for (juce::File enclosingDir = executableDir; ! enclosingDir.isRoot();
         enclosingDir = enclosingDir.getParentDirectory())
    {
        juce::File maybeAsset = enclosingDir.getChildFile (ASSETS_DIR).getChildFile (name);
        if (maybeAsset.existsAsFile())
        {
            return maybeAsset;
        }
    }
    return juce::File {};
}

std::unique_ptr<juce::FileOutputStream> openForOverwriting (const juce::File& file)
{
    auto stream = std::make_unique<juce::FileOutputStream> (file);
    if (stream->openedOk())
    {
        stream->setPosition (0);
        stream->truncate();
    }
    return stream;
}

TEST (EndToEndTest, Render)
{
    constexpr char inputFilename[] = "ddsp_input_48k.wav";
    constexpr char outputFilename[] = "ddsp_input_48k_test_output.wav";
    constexpr double sampleRate = 48000.0;
    constexpr int numChannels = 1;
    constexpr int bitsPerSample = 16;
    constexpr int frameSize = 512;

    // For message manager, timers, etc.
    juce::ScopedJuceInitialiser_GUI juce_framework;

    juce::File inputFile = locateAsset (inputFilename);
    ASSERT_NE (inputFile, juce::File {}) << "Could not locate the input file: " << inputFile.getFullPathName();

    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    juce::AudioFormatReader* reader = formatManager.createReaderFor (inputFile);
    ASSERT_NE (reader, nullptr) << "Could not open the input file for reading: " << inputFile.getFullPathName();

    auto readerSource = std::make_unique<juce::AudioFormatReaderSource> (reader, true);
    juce::AudioTransportSource transportSource;
    transportSource.setSource (readerSource.get(), 0, nullptr, reader->sampleRate);

    juce::File outputFile = inputFile.getSiblingFile (outputFilename);
    std::unique_ptr<juce::FileOutputStream> outputStream = openForOverwriting (outputFile);

    std::unique_ptr<juce::AudioFormatWriter> writer;
    juce::WavAudioFormat format;
    writer.reset (format.createWriterFor (outputStream.release(), sampleRate, numChannels, bitsPerSample, {}, 0));
    ASSERT_NE (writer, nullptr) << "Could not open the output file for writing: " << outputFile.getFullPathName();

    DDSPAudioProcessor processor (/*singleThreaded=*/true);

    transportSource.prepareToPlay (frameSize, sampleRate);
    processor.prepareToPlay (sampleRate, frameSize);

    juce::AudioBuffer<float> buffer (numChannels, frameSize);
    juce::MidiBuffer midiBuffer;
    juce::AudioSourceChannelInfo info (buffer);
    transportSource.start();

    while (! transportSource.hasStreamFinished())
    {
        transportSource.getNextAudioBlock (info);
        processor.processBlock (buffer, midiBuffer);
        writer->writeFromAudioSampleBuffer (buffer, 0, frameSize);
    }

    transportSource.releaseResources();
}
