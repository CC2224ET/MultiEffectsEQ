#pragma once
#include "FXModule.h"
#include "GainModule.h"
#include "DistortionModule.h"

class FXSlot : public FXModule
{
public:
    void prepare (const juce::dsp::ProcessSpec& spec) override
    {
        gain.prepare(spec);
        distortion.prepare(spec);
    }

    void process (const juce::dsp::ProcessContextReplacing<float>& context) override
    {
        switch(activeEffect)
        {
            case 0: break;
            case 1: gain.process(context); break;
            case 2: distortion.process(context); break;
        }
    }

    void reset() override
    {
        gain.reset();
        distortion.reset();
    }

    void setEffectChoice (int choice)
    {
        activeEffect = choice;
    }

    GainModule& getGainModule() {return gain; }
    DistortionModule& getDistortionModule() { return distortion; }

private:
    int activeEffect = 0;

    GainModule gain;
    DistortionModule distortion;
};