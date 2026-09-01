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

    auto visualizerArea = getLocalBounds().removeFromTop(juce::roundToInt(getHeight() * 0.6f));
    int width = visualizerArea.getWidth();
    int height = visualizerArea.getHeight();

    float lowMidFreq = audioProcessor.apvts.getRawParameterValue("low_mid_crossover")->load();
    float midHighFreq = audioProcessor.apvts.getRawParameterValue("mid_high_crossover")->load();
    float midGainDB = audioProcessor.apvts.getRawParameterValue("mid_band_gain")->load();
    float midGainLinear = juce::Decibels::decibelsToGain(midGainDB);

    double sampleRate = audioProcessor.getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 44100.0;

    // Draw background grid lines (horizontal dB lines)
    g.setColour (juce::Colours::darkgrey.withAlpha(0.5f));
    const float dBGridLines[] = { 12.0f, 0.0f, -12.0f };
    for (float db : dBGridLines)
    {
        float y = juce::jmap(db, 24.0f, -24.0f, 0.0f, (float)height);
        if (db == 0.0f)
        {
            g.setColour(juce::Colours::grey);
            g.drawHorizontalLine(juce::roundToInt(y), 0.0f, (float)width);
            g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
        }
        else
        {
            g.drawHorizontalLine(juce::roundToInt(y), 0.0f, (float)width);
        }
    }

    // Draw frequency grid lines (vertical lines for 100Hz, 1kHz, 10kHz)
    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    const float freqGridLines[] = { 100.0f, 1000.0f, 10000.0f };
    g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
    for (float fGrid : freqGridLines)
    {
        float normX = std::log(fGrid / minFreq) / std::log(maxFreq / minFreq);
        float x = normX * width;
        g.drawVerticalLine(juce::roundToInt(x), 0.0f, (float)height);
    }

    // Draw crossover frequency indicator lines
    g.setColour(juce::Colours::orange.withAlpha(0.6f));
    float lowMidNormX = std::log(lowMidFreq / minFreq) / std::log(maxFreq / minFreq);
    float lowMidX = lowMidNormX * width;
    g.drawVerticalLine(juce::roundToInt(lowMidX), 0.0f, (float)height);

    g.setColour(juce::Colours::violet.withAlpha(0.6f));
    float midHighNormX = std::log(midHighFreq / minFreq) / std::log(maxFreq / minFreq);
    float midHighX = midHighNormX * width;
    g.drawVerticalLine(juce::roundToInt(midHighX), 0.0f, (float)height);

    // Calculate Linkwitz-Riley 4th order filter response for the 3-band crossover network
    auto lowMidLP  = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lowMidFreq);
    auto lowMidHP  = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowMidFreq);
    auto midHighLP = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, midHighFreq);
    auto midHighHP = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, midHighFreq);

    juce::Path responseCurve;

    for (int x = 0; x < width; ++x)
    {
        float freq = minFreq * std::pow(maxFreq / minFreq, x / (float)width);

        // 2nd order Butterworth magnitude responses
        float magLowMidLP_2nd  = lowMidLP->getMagnitudeForFrequency(freq, sampleRate);
        float magLowMidHP_2nd  = lowMidHP->getMagnitudeForFrequency(freq, sampleRate);
        float magMidHighLP_2nd = midHighLP->getMagnitudeForFrequency(freq, sampleRate);
        float magMidHighHP_2nd = midHighHP->getMagnitudeForFrequency(freq, sampleRate);

        // LR4 magnitude response (squared Butterworth 2nd order)
        float magLR4_lowMidLP  = magLowMidLP_2nd * magLowMidLP_2nd;
        float magLR4_lowMidHP  = magLowMidHP_2nd * magLowMidHP_2nd;
        float magLR4_midHighLP = magMidHighLP_2nd * magMidHighLP_2nd;
        float magLR4_midHighHP = magMidHighHP_2nd * magMidHighHP_2nd;

        // Sum the in-phase band magnitudes (Low + Mid*Gain + High)
        float lowMag  = magLR4_lowMidLP;
        float midMag  = magLR4_lowMidHP * magLR4_midHighLP * midGainLinear;
        float highMag = magLR4_lowMidHP * magLR4_midHighHP;

        float totalMag = lowMag + midMag + highMag;
        float decibels = juce::Decibels::gainToDecibels(totalMag, -100.0f);

        float yMap = juce::jmap(decibels, 24.0f, -24.0f, 0.0f, (float)height);

        if (x == 0)
            responseCurve.startNewSubPath((float)x, yMap);
        else
            responseCurve.lineTo((float)x, yMap);
    }

    g.setColour (juce::Colours::cyan);
    g.strokePath (responseCurve, juce::PathStrokeType(2.0f));
}



void MultieffectsEQEditor::resized()
{
    auto sliderArea = getLocalBounds().removeFromBottom(juce::roundToInt(getHeight() * 0.4f));

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
