#pragma once
#include <juce_dsp/juce_dsp.h>

class FXModule
{
public:

    virtual ~FXModule() = default;

    virtual void prepare (const juce::dsp::ProcessSpec& spec) = 0;

    virtual void process (const juce::dsp::ProcessContextReplacing<float>& context) = 0;

    virtual void reset() = 0;
};