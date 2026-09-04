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

    void setMacro(float macroValue)
    {
        // macroValue is always a normalized value between 0.0 and 1.0
        switch (activeEffect)
        {
            case 1: 
                // Gain: Map 0.0-1.0 to -24dB to +24dB
                gain.updateGain(juce::jmap(macroValue, -1.0f, 1.0f, -24.0f, 24.0f)); 
                break;
            case 2: 
                // Distortion: Map 0.0-1.0 to 0dB to 24dB of drive
                float unipolarMacro = std::max(0.0f, macroValue);
                distortion.updateDrive(juce::jmap(unipolarMacro, 0.0f, 1.0f, 0.0f, 24.0f)); 
                break;
        }
    }
private:
    int activeEffect = 0;

    GainModule gain;
    DistortionModule distortion;
};