#include "PluginProcessor.h"
#include "PluginEditor.h"

MultieffectsEQEditor::MultieffectsEQEditor (MultieffectsEQProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // 1. Configure how the slider looks and behaves
    lowMidCrossoverSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lowMidCrossoverSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    
    // 2. Make it visible on the screen. (If you forget this, the slider won't appear).
    addAndMakeVisible(lowMidCrossoverSlider);

    // 3. Connect the visual slider to the DSP parameter using the exact string ID we defined earlier.
    lowMidCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "low_mid_crossover", lowMidCrossoverSlider);

    // Set the overall window dimensions.
    setSize (600, 400);
}

MultieffectsEQEditor::~MultieffectsEQEditor()
{
}

void MultieffectsEQEditor::paint (juce::Graphics& g)
{
    // Fills the background with a solid color.
    g.fillAll (juce::Colours::darkgrey);
    
    // Draws a simple title at the top center of the window.
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("MultieffectsEQ", getLocalBounds().reduced(10), juce::Justification::centredTop, 1);
}

void MultieffectsEQEditor::resized()
{
    // Sets the exact position and size of the slider within the window.
    // Arguments: X position, Y position, Width, Height
    lowMidCrossoverSlider.setBounds(50, 50, 100, 100);
}