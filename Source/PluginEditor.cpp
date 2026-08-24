#include "PluginProcessor.h"
#include "PluginEditor.h"

MultieffectsEQEditor::MultieffectsEQEditor (MultieffectsEQProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    lowMidCrossoverSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lowMidCrossoverSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    
    addAndMakeVisible(lowMidCrossoverSlider);

    
    lowMidCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "low_mid_crossover", lowMidCrossoverSlider);

    
    setSize (600, 400);
}

MultieffectsEQEditor::~MultieffectsEQEditor()
{
}

void MultieffectsEQEditor::paint (juce::Graphics& g)
{
    
    g.fillAll (juce::Colours::darkgrey);
    
    
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("MultieffectsEQ", getLocalBounds().reduced(10), juce::Justification::centredTop, 1);
}

void MultieffectsEQEditor::resized()
{
    
    lowMidCrossoverSlider.setBounds(50, 50, 100, 100);
}