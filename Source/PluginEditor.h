#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class MultieffectsEQEditor : public juce::AudioProcessorEditor
{
public:
    MultieffectsEQEditor (MultieffectsEQProcessor&);
    ~MultieffectsEQEditor() override;

    
    void paint (juce::Graphics&) override;
    
    void resized() override;

private:

    MultieffectsEQProcessor& audioProcessor;

    
    juce::Slider lowMidCrossoverSlider;
    
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowMidCrossoverAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultieffectsEQEditor)
};