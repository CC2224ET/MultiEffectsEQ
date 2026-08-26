#pragma once
#include "FXModule.h"

class GainModule : public FXModule
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        gain.prepare(spec);

        gain.setRampDurationSeconds(0.05);
    }

    void process (const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        gain.process(context);
    }

    void reset() override
    {
        gain.reset();
    }

    void updateGain (float gainInDecibels)
    {
        gain.setGainDecibels(gainInDecibels);
    }

private:
    juce::dsp::Gain<float> gain;
};    

