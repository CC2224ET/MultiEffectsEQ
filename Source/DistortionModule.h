#pragma once
#include "FXModule.h"

class DistortionModule : public FXModule
{
public:
   DistortionModule()
    {
        waveshaper.functionToUse = [] (float x)
        {
            return std::tanh (x);
        };
    }

    void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        waveshaper.prepare(spec);

        inputGain.prepare(spec);
        outputGain.prepare(spec);

        inputGain.setRampDurationSeconds(0.02);
        outputGain.setRampDurationSeconds(0.02);
    }

    void process (const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        inputGain.process(context);
        waveshaper.process(context);
        outputGain.process(context); 
    }

    void reset() override
    {
        waveshaper.reset();
        inputGain.reset();
        outputGain.reset();
    }

    void updateDrive (float driveDecibels)
    {
        driveDecibels = std::max(0.0f, driveDecibels);
        inputGain.setGainDecibels(driveDecibels);
        outputGain.setGainDecibels(-driveDecibels);
    }

private:
    juce::dsp::Gain<float> inputGain;
    juce::dsp::WaveShaper<float> waveshaper;
    juce::dsp::Gain<float> outputGain;
};


