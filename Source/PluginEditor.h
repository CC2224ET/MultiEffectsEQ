#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

class MultieffectsEQEditor : public juce::AudioProcessorEditor,
                             public juce::Timer,
                             public juce::Slider::Listener
{
public:
    MultieffectsEQEditor (MultieffectsEQProcessor&);
    ~MultieffectsEQEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sliderValueChanged (juce::Slider* slider) override;
    void updateMacroSliderRange();

private:
    MultieffectsEQProcessor& audioProcessor;

    juce::Slider lowMidCrossoverSlider;
    juce::Slider midHighCrossoverSlider;
    juce::Slider macroSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lowMidCrossoverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midHighCrossoverAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> macroAttachment;

    juce::ComboBox midSlot1Choice;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> midSlot1ChoiceAttachment;

    int lastActiveEffect = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultieffectsEQEditor)
};
