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

    lowMidCrossoverSlider.addListener(this);
    midHighCrossoverSlider.addListener(this);
    //attach the values to the slider
    lowMidCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "low_mid_crossover", lowMidCrossoverSlider);
    midHighCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mid_high_crossover", midHighCrossoverSlider);
    midGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mid_band_gain", midGainSlider);

    setSize (600, 400);
    startTimerHz(30);
}
void MultieffectsEQEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);

    auto visualizerArea = getLocalBounds().removeFromTop(getHeight() * 0.6f);
    int width = visualizerArea.getWidth();
    int height = visualizerArea.getHeight();

    float lowMidFreq = audioProcessor.apvts.getRawParameterValue("low_mid_crossover")->load();
    float midHighFreq = audioProcessor.apvts.getRawParameterValue("mid_high_crossover")->load();
    float midGainDB = audioProcessor.apvts.getRawParameterValue("mid_band_gain")->load();

    float midCenterFreq = std::sqrt(lowMidFreq * midHighFreq);

    double sampleRate = audioProcessor.getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 44100.0;
    auto midCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(
        sampleRate, midCenterFreq, 1.0f, juce::Decibels::decibelsToGain(midGainDB));
    
    juce::Path responseCurve;

    for (int x = 0; x < width; ++x)
    {
        float freq = 20.0f * std::pow(20000.0f / 20.0f, x / (float)width);

        float magnitude = midCoefficients->getMagnitudeForFrequency(freq, sampleRate);
        float decibels = juce::Decibels::gainToDecibels(magnitude);

        float yMap = juce::jmap(decibels, 24.0f, -24.0f, 0.0f, (float)height);

        if (x == 0)
            responseCurve.startNewSubPath((float)x, yMap);
        else
            responseCurve.lineTo((float)x, yMap);
    }

    g.setColour (juce::Colours::darkgrey);
    g.drawHorizontalLine(height / 2, 2.0f, (float)width);

    g.setColour (juce::Colours::cyan);
    g.strokePath (responseCurve, juce::PathStrokeType(2.0f));
}



void MultieffectsEQEditor::resized()
{
    auto sliderArea = getLocalBounds().removeFromBottom(getHeight() * 0.4f);

    int sliderWidth = sliderArea.getWidth() / 3;

    lowMidCrossoverSlider.setBounds(sliderArea.removeFromLeft(sliderWidth));
    midGainSlider.setBounds(sliderArea.removeFromLeft(sliderWidth));
    midHighCrossoverSlider.setBounds(sliderArea);
}

void MultieffectsEQEditor::sliderValueChanged (juce::Slider* slider)
{
    
    if (slider == &lowMidCrossoverSlider)
    {
        if (lowMidCrossoverSlider.getValue() >= midHighCrossoverSlider.getValue())
        {
            midHighCrossoverSlider.setValue(lowMidCrossoverSlider.getValue());
        }
    }
    
    else if (slider == &midHighCrossoverSlider)
    {
        if (midHighCrossoverSlider.getValue() <= lowMidCrossoverSlider.getValue())
        {
            lowMidCrossoverSlider.setValue(midHighCrossoverSlider.getValue());
        }
    }
}

MultieffectsEQEditor::~MultieffectsEQEditor()
{
}

void MultieffectsEQEditor::timerCallback()
{
    repaint(); 
}
