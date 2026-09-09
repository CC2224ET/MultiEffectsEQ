#pragma once
#include "FXModule.h"

class AutoPanModule : public FXModule
{
public:
    AutoPanModule()
    {
        lfo.initialise ([] (float x) { return std::sin (x); });
    }

    void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        lfo.prepare(spec);
        lfo.setFrequency(1.0f);
    }

    void process (const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        auto& outputBlock = context.getOutputBlock();
        size_t numChannels = outputBlock.getNumChannels();
        size_t numSamples = outputBlock.getNumSamples();

        if (numChannels < 2) return;

        for (size_t i = 0; i < numSamples; ++i)
        {
            float lfoVal = lfo.processSample(0.0f);

            lfoVal *= depth;

            float panPosition = (lfoVal * 0.5f) + 0.5f;
            
            float leftGain  = std::cos(panPosition * juce::MathConstants<float>::halfPi);
            float rightGain = std::sin(panPosition * juce::MathConstants<float>::halfPi);

            outputBlock.getChannelPointer(0)[i] *= leftGain;
            outputBlock.getChannelPointer(1)[i] *= rightGain;

        }

    }

    void reset() override
    {
        lfo.reset();
    }

    void updateMacro (float macroValue)
    {
        depth = macroValue; 
        

        float rate = juce::jmap(macroValue, 0.0f, 1.0f, 0.5f, 8.0f);
        lfo.setFrequency(rate);
    }


private:
    juce::dsp::Oscillator<float> lfo;
    float depth = 0.0f;
};
