#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "FXModule.h"
#include "GainModule.h"
#include <vector>
#include <memory>

class MultieffectsEQProcessor : public juce::AudioProcessor
{
public:
    MultieffectsEQProcessor();
    ~MultieffectsEQProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;

    void releaseResources() override;

    void reset() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //Editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //Metadata
    const juce::String getName() const override { return "MultieffectsEQ"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //Presets
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int /*index*/) override {}
    const juce::String getProgramName (int /*index*/) override { return {}; }
    void changeProgramName (int /*index*/, const juce::String& /*newName*/) override {}

    //Saving
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    //Splits Lows
    juce::dsp::LinkwitzRileyFilter<float> lowMidCrossoverLP; 
    juce::dsp::LinkwitzRileyFilter<float> lowMidCrossoverHP;
    //Splits Highs
    juce::dsp::LinkwitzRileyFilter<float> midHighCrossoverLP; 
    juce::dsp::LinkwitzRileyFilter<float> midHighCrossoverHP;
    //Audio buffer for specific bands
    juce::AudioBuffer<float> lowBuffer;
    juce::AudioBuffer<float> midBuffer;
    juce::AudioBuffer<float> highBuffer;
    // The Modular Effect Chains
    std::vector<std::unique_ptr<FXModule>> lowBandChain;
    std::vector<std::unique_ptr<FXModule>> midBandChain;
    std::vector<std::unique_ptr<FXModule>> highBandChain;

    // Splits/Compensates Low Band
    juce::dsp::LinkwitzRileyFilter<float> lowCompensatorLP;
    juce::dsp::LinkwitzRileyFilter<float> lowCompensatorHP;
    juce::AudioBuffer<float> lowCompBuffer;

    juce::SmoothedValue<float> smoothedLowMidFreq;
    juce::SmoothedValue<float> smoothedMidHighFreq;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultieffectsEQProcessor)
};
