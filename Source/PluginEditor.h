#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class MultieffectsEQEditor : public juce::AudioProcessorEditor,
                             public juce::Timer
{
public:
    MultieffectsEQEditor (MultieffectsEQProcessor&);
    ~MultieffectsEQEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    MultieffectsEQProcessor& audioProcessor;

    juce::Slider lowMidCrossoverSlider;
    juce::Slider midHighCrossoverSlider;
    juce::Slider midGainSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowMidCrossoverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midHighCrossoverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midGainAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultieffectsEQEditor)
};
