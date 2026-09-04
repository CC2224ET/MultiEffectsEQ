#include "PluginProcessor.h"
#include "PluginEditor.h"

MultieffectsEQEditor::MultieffectsEQEditor (MultieffectsEQProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // 1. Setup Sliders (Replaced specific effect sliders with the unified macroSlider)
    for (auto* slider : { &lowMidCrossoverSlider, &midHighCrossoverSlider, &macroSlider})
    {
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
        slider->addListener(this);
        addAndMakeVisible(slider);
    }

    // 2. Setup the Dropdown Menu
    midSlot1Choice.addItemList({"Bypass", "Gain", "Distortion"}, 1);
    midSlot1Choice.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(midSlot1Choice);

    // 3. Bind UI to APVTS
    lowMidCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "low_mid_crossover", lowMidCrossoverSlider);
    midHighCrossoverAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mid_high_crossover", midHighCrossoverSlider);
    midSlot1ChoiceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "mid_slot_1_fx", midSlot1Choice);
    macroAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mid_macro", macroSlider);

    midSlot1Choice.onChange = [this] { updateMacroSliderRange(); };
    updateMacroSliderRange();

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
    float macroValue = audioProcessor.apvts.getRawParameterValue("mid_macro")->load();
    int activeEffect = (int)audioProcessor.apvts.getRawParameterValue("mid_slot_1_fx")->load();

    double sampleRate = audioProcessor.getSampleRate();
    if (sampleRate <= 0.0) sampleRate = 44100.0;

    g.setColour (juce::Colours::darkgrey.withAlpha(0.5f));
    const float dBGridLines[] = { 12.0f, 0.0f, -12.0f };
    for (float db : dBGridLines)
    {
        float y = juce::jmap(db, 24.0f, -24.0f, 0.0f, (float)height);
        if (db == 0.0f) g.setColour(juce::Colours::grey);
        g.drawHorizontalLine(juce::roundToInt(y), 0.0f, (float)width);
        if (db == 0.0f) g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));
    }

    const float minFreq = 20.0f;
    const float maxFreq = 20000.0f;
    const float freqGridLines[] = { 100.0f, 1000.0f, 10000.0f };
    g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
    for (float fGrid : freqGridLines)
    {
        float normX = std::log(fGrid / minFreq) / std::log(maxFreq / minFreq);
        g.drawVerticalLine(juce::roundToInt(normX * width), 0.0f, (float)height);
    }

    g.setColour(juce::Colours::orange.withAlpha(0.6f));
    float lowMidNormX = std::log(lowMidFreq / minFreq) / std::log(maxFreq / minFreq);
    g.drawVerticalLine(juce::roundToInt(lowMidNormX * width), 0.0f, (float)height);

    g.setColour(juce::Colours::violet.withAlpha(0.6f));
    float midHighNormX = std::log(midHighFreq / minFreq) / std::log(maxFreq / minFreq);
    g.drawVerticalLine(juce::roundToInt(midHighNormX * width), 0.0f, (float)height);

    float visualBumpDB = 0.0f;
    juce::Colour curveColor = juce::Colours::grey; // Default for Bypass

    if (activeEffect == 1) // Gain
    {
        visualBumpDB = juce::jmap(macroValue, -1.0f, 1.0f, -24.0f, 24.0f);
        curveColor = juce::Colours::cyan;
    }
    else if (activeEffect == 2) // Distortion
    {
        float unipolarMacro = std::max(0.0f, macroValue);
        visualBumpDB = juce::jmap(unipolarMacro, 0.0f, 1.0f, 0.0f, 24.0f);
        curveColor = juce::Colours::pink;
    }

    float midVisualLinear = juce::Decibels::decibelsToGain(visualBumpDB);

    // --- 3. Calculate and Draw Curve ---
    auto lowMidLP  = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, lowMidFreq);
    auto lowMidHP  = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, lowMidFreq);
    auto midHighLP = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, midHighFreq);
    auto midHighHP = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, midHighFreq);

    juce::Path responseCurve;

    for (int x = 0; x < width; ++x)
    {
        float freq = minFreq * std::pow(maxFreq / minFreq, x / (float)width);

        float m1 = std::pow(lowMidLP->getMagnitudeForFrequency(freq, sampleRate), 2.0f);
        float m2 = std::pow(lowMidHP->getMagnitudeForFrequency(freq, sampleRate), 2.0f);
        float m3 = std::pow(midHighLP->getMagnitudeForFrequency(freq, sampleRate), 2.0f);
        float m4 = std::pow(midHighHP->getMagnitudeForFrequency(freq, sampleRate), 2.0f);

        float lowMag  = m1;
        float midMag  = m2 * m3 * midVisualLinear; // Apply the visual bump here
        float highMag = m2 * m4;

        float totalMag = lowMag + midMag + highMag;
        float decibels = juce::Decibels::gainToDecibels(totalMag, -100.0f);
        float yMap = juce::jmap(decibels, 24.0f, -24.0f, 0.0f, (float)height);

        if (x == 0) responseCurve.startNewSubPath((float)x, yMap);
        else        responseCurve.lineTo((float)x, yMap);
    }

    g.setColour(curveColor);
    g.strokePath(responseCurve, juce::PathStrokeType(2.0f));
}

void MultieffectsEQEditor::resized()
{
    auto bottomArea = getLocalBounds().removeFromBottom(getHeight() * 0.4f);
    
    // Divide into exactly 3 columns
    int columnWidth = bottomArea.getWidth() / 3;

    // 1. Left Crossover
    lowMidCrossoverSlider.setBounds(bottomArea.removeFromLeft(columnWidth));
    
    // 2. Right Crossover (pull from the right side!)
    midHighCrossoverSlider.setBounds(bottomArea.removeFromRight(columnWidth));
    
    // 3. Center Area (Dropdown + Macro Knob)
    auto centerArea = bottomArea; 
    midSlot1Choice.setBounds(centerArea.removeFromTop(40).withSizeKeepingCentre(120, 24));
    macroSlider.setBounds(centerArea);
}

void MultieffectsEQEditor::sliderValueChanged (juce::Slider* slider)
{
    const double minGap = 10.0;

    if (slider == &lowMidCrossoverSlider)
    {
        if (lowMidCrossoverSlider.getValue() >= midHighCrossoverSlider.getValue() - minGap)
        {
            lowMidCrossoverSlider.setValue(midHighCrossoverSlider.getValue() - minGap, juce::sendNotificationSync);
        }
    }
    else if (slider == &midHighCrossoverSlider)
    {
        if (midHighCrossoverSlider.getValue() <= lowMidCrossoverSlider.getValue() + minGap)
        {
            midHighCrossoverSlider.setValue(lowMidCrossoverSlider.getValue() + minGap, juce::sendNotificationSync);
        }
    }
    else if (slider == &macroSlider)
    {
        int activeEffect = (int)audioProcessor.apvts.getRawParameterValue("mid_slot_1_fx")->load();
        if (activeEffect == 2 && macroSlider.getValue() < 0.0)
        {
            macroSlider.setValue(0.0, juce::sendNotificationSync);
        }
    }
}

MultieffectsEQEditor::~MultieffectsEQEditor()
{
}

void MultieffectsEQEditor::updateMacroSliderRange()
{
    int activeEffect = (int)audioProcessor.apvts.getRawParameterValue("mid_slot_1_fx")->load();
    if (activeEffect != lastActiveEffect)
    {
        lastActiveEffect = activeEffect;
        if (activeEffect == 2) // Distortion
        {
            macroSlider.setRange(0.0, 1.0, 0.01);
            if (macroSlider.getValue() < 0.0)
                macroSlider.setValue(0.0, juce::sendNotificationSync);
        }
        else // Gain or Bypass
        {
            macroSlider.setRange(-1.0, 1.0, 0.01);
        }
    }
}

void MultieffectsEQEditor::timerCallback()
{
    updateMacroSliderRange();
    repaint(); 
}
