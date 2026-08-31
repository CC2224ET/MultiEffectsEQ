#include "PluginProcessor.h"
#include "PluginEditor.h"

MultieffectsEQEditor::MultieffectsEQEditor (MultieffectsEQProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //Setup Slider
    for (auto* slider : { &lowMidCrossoverSlider, &midHighCrossoverSlider, &midGainSlider})
    {
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        addAndMakeVisible(slider);
    }
    //attach the values to the slider
    auto lowMidCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "low_mid_crossover", lowMidCrossoverSlider);
    auto midHighCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mid_high_crossover", midHighCrossoverSlider);
    auto midGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "low_mid_crossover", midGainSlider);

    setSize (600, 400);
    startTimerHz(30);
}

void MultieffectsEQEditor::resized()
{
    auto sliderArea = getLocalBounds().removeFromBottom(getHeight() * 0.4f);

    int sliderWidth = sliderArea.getWidth() / 3;

    lowMidCrossoverSlider.setBounds(sliderArea.removeFromLeft(sliderWidth));
    midGainSlider.setBounds(sliderArea.removeFromLeft(sliderWidth));
    midHighCrossoverSlider.setBounds(sliderArea);
}

MultieffectsEQEditor::~MultieffectsEQEditor()
{
}

void MultieffectsEQEditor::timerCallback()
{
    repaint(); 
}
