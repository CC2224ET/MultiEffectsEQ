#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class MultieffectsEQEditor : public juce::AudioProcessorEditor,
                             public juce::Slider::Listener
{
public:
    MultieffectsEQEditor (MultieffectsEQProcessor&);
    ~MultieffectsEQEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    // Slider::Listener — enforces crossover ordering constraint in the UI
    void sliderValueChanged (juce::Slider* slider) override;

private:
    MultieffectsEQProcessor& audioProcessor;

    juce::Slider lowMidCrossoverSlider;
    juce::Slider midHighCrossoverSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowMidCrossoverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midHighCrossoverAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultieffectsEQEditor)
};