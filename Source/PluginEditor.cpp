#include "PluginProcessor.h"
#include "PluginEditor.h"

MultieffectsEQEditor::MultieffectsEQEditor (MultieffectsEQProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Low/Mid Crossover
    lowMidCrossoverSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    lowMidCrossoverSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(lowMidCrossoverSlider);

    lowMidCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "low_mid_crossover", lowMidCrossoverSlider);

    // Mid/High Crossover
    midHighCrossoverSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    midHighCrossoverSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    addAndMakeVisible(midHighCrossoverSlider);

    midHighCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "mid_high_crossover", midHighCrossoverSlider);

    // Register listeners after attachments — so the initial value set by the
    // attachment doesn't trigger sliderValueChanged during construction
    lowMidCrossoverSlider.addListener(this);
    midHighCrossoverSlider.addListener(this);

    setSize (600, 400);
}

MultieffectsEQEditor::~MultieffectsEQEditor()
{
    lowMidCrossoverSlider.removeListener(this);
    midHighCrossoverSlider.removeListener(this);
}

void MultieffectsEQEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);
    
    g.setColour (juce::Colours::white);
    g.setFont (20.0f);
    g.drawFittedText ("MultieffectsEQ", getLocalBounds().reduced(10), juce::Justification::centredTop, 1);

    // Labels for the sliders
    g.setFont (15.0f);
    g.drawFittedText ("Low/Mid Crossover", lowMidCrossoverSlider.getBounds().translated(0, -25).withHeight(20), juce::Justification::centred, 1);
    g.drawFittedText ("Mid/High Crossover", midHighCrossoverSlider.getBounds().translated(0, -25).withHeight(20), juce::Justification::centred, 1);
}

void MultieffectsEQEditor::resized()
{
    lowMidCrossoverSlider.setBounds(150, 150, 120, 120);
    midHighCrossoverSlider.setBounds(330, 150, 120, 120);
}

void MultieffectsEQEditor::sliderValueChanged (juce::Slider* slider)
{
    if (slider == &lowMidCrossoverSlider)
    {
        // If the low/mid crossover has been pushed above mid/high, clamp mid/high up to match
        if (lowMidCrossoverSlider.getValue() > midHighCrossoverSlider.getValue())
            midHighCrossoverSlider.setValue(lowMidCrossoverSlider.getValue(), juce::sendNotificationSync);
    }
    else if (slider == &midHighCrossoverSlider)
    {
        // If the mid/high crossover has been pulled below low/mid, clamp low/mid down to match
        if (midHighCrossoverSlider.getValue() < lowMidCrossoverSlider.getValue())
            lowMidCrossoverSlider.setValue(midHighCrossoverSlider.getValue(), juce::sendNotificationSync);
    }
}