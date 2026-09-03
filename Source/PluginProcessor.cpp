#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "FXSlot.h"

//Boilerplate setup
MultieffectsEQProcessor::MultieffectsEQProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    midBandChain.push_back(std::make_unique<FXSlot>());
}

MultieffectsEQProcessor::~MultieffectsEQProcessor() {}

//Create the crossover frequency parameters
juce::AudioProcessorValueTreeState::ParameterLayout MultieffectsEQProcessor::createParameterLayout()
{
   
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("low_mid_crossover", 1), 
        "Low/Mid Crossover",                      
        juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.3f), 
        250.0f));                                  
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mid_high_crossover", 1),
        "Mid/High Crossover", 
        juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.3f), 
        2000.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("mid_band_gain", 1), "Mid Band Gain",
        juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f, 1.0f), 
        0.0f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
    "mid_slot_1_fx", "Mid Slot 1 Effect", 
    juce::StringArray{"Bypass", "Gain", "Distortion"}, 
    0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
    "mid_drive", "Mid Drive", 
    juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f, 1.0f), 
    0.0f));

    return { params.begin(), params.end() };
}

void MultieffectsEQProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //Holds the processing details for the DSP
    juce::dsp::ProcessSpec spec;

    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    lowMidCrossoverLP.prepare(spec);
    lowMidCrossoverLP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);

    lowMidCrossoverHP.prepare(spec);
    lowMidCrossoverHP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    midHighCrossoverLP.prepare(spec);
    midHighCrossoverLP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);

    midHighCrossoverHP.prepare(spec);
    midHighCrossoverHP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    lowBuffer.setSize(spec.numChannels, samplesPerBlock);
    midBuffer.setSize(spec.numChannels, samplesPerBlock);
    highBuffer.setSize(spec.numChannels, samplesPerBlock);
    lowCompBuffer.setSize(spec.numChannels, samplesPerBlock);
    //Compensate for the phase change
    lowCompensatorLP.prepare(spec);
    lowCompensatorLP.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);

    lowCompensatorHP.prepare(spec);
    lowCompensatorHP.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    // Initialise smoothers — 20ms ramp to avoid zipper noise on crossover changes
    float initLowMid  = apvts.getRawParameterValue("low_mid_crossover")->load();
    float initMidHigh = apvts.getRawParameterValue("mid_high_crossover")->load();

    smoothedLowMidFreq.reset(sampleRate, 0.02);
    smoothedLowMidFreq.setCurrentAndTargetValue(initLowMid);

    smoothedMidHighFreq.reset(sampleRate, 0.02);
    smoothedMidHighFreq.setCurrentAndTargetValue(initMidHigh);

    for (auto& effect : lowBandChain)  { effect->prepare(spec); }
    for (auto& effect : midBandChain)  { effect->prepare(spec); }
    for (auto& effect : highBandChain) { effect->prepare(spec); }
}

void MultieffectsEQProcessor::releaseResources()
{
}

//Clears the internal states to prevent audio tails
void MultieffectsEQProcessor::reset()
{
    lowMidCrossoverLP.reset();
    lowMidCrossoverHP.reset();
    midHighCrossoverLP.reset();
    midHighCrossoverHP.reset();
    lowCompensatorLP.reset();
    lowCompensatorHP.reset();
    //same
    for (auto& effect : lowBandChain)  { effect->reset(); }
    for (auto& effect : midBandChain)  { effect->reset(); }
    for (auto& effect : highBandChain) { effect->reset(); }
}
//checking if the DAW layout is supported by the plugin
bool MultieffectsEQProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != layouts.getMainOutputChannelSet())
        return false;

    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void MultieffectsEQProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    //Prevent floating point issue
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    //clear output channels that don't have an input channel
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    auto numSamples = buffer.getNumSamples();
    
    //check the incoming block size isn't larger than what we have allocated
    jassert (numSamples <= lowBuffer.getNumSamples());
    if (numSamples > lowBuffer.getNumSamples())
        return;
    
    //Get the parameter value
    // Set smoothing targets from APVTS
    float targetLowMidFreq  = apvts.getRawParameterValue("low_mid_crossover")->load();
    float targetMidHighFreq = apvts.getRawParameterValue("mid_high_crossover")->load();
    float currentMidGain = apvts.getRawParameterValue("mid_band_gain")->load();

    smoothedLowMidFreq.setTargetValue(targetLowMidFreq);
    smoothedMidHighFreq.setTargetValue(targetMidHighFreq);

    float currentLowMidFreq  = smoothedLowMidFreq.getCurrentValue();
    float currentMidHighFreq = smoothedMidHighFreq.getCurrentValue();

    smoothedLowMidFreq.skip(numSamples);
    smoothedMidHighFreq.skip(numSamples);

    //Apply the smoothed value
    lowMidCrossoverLP.setCutoffFrequency(currentLowMidFreq);
    lowMidCrossoverHP.setCutoffFrequency(currentLowMidFreq);
    midHighCrossoverLP.setCutoffFrequency(currentMidHighFreq);
    midHighCrossoverHP.setCutoffFrequency(currentMidHighFreq);
    lowCompensatorLP.setCutoffFrequency(currentMidHighFreq);
    lowCompensatorHP.setCutoffFrequency(currentMidHighFreq);

    //Copy the incoming audio to the processing buffer
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        lowBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        midBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }

    juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
    juce::dsp::AudioBlock<float> midBlock(midBuffer);
    juce::dsp::AudioBlock<float> highBlock(highBuffer);

    //Subblock
    auto activeLowBlock = lowBlock.getSubBlock(0, (size_t) numSamples);
    auto activeMidBlock = midBlock.getSubBlock(0, (size_t) numSamples);
    auto activeHighBlock = highBlock.getSubBlock(0, (size_t) numSamples);

    juce::dsp::ProcessContextReplacing<float> lowContext(activeLowBlock);
    juce::dsp::ProcessContextReplacing<float> midContext(activeMidBlock);
    juce::dsp::ProcessContextReplacing<float> highContext(activeHighBlock);

    lowMidCrossoverLP.process(lowContext);
    lowMidCrossoverHP.process(midContext);

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        highBuffer.copyFrom(ch, 0, midBuffer, ch, 0, numSamples);
    }

    midHighCrossoverLP.process(midContext);
    midHighCrossoverHP.process(highContext);

    //compensate for the phase difference 
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        lowCompBuffer.copyFrom(ch, 0, lowBuffer, ch, 0, numSamples);
    }

    juce::dsp::AudioBlock<float> lowCompBlock(lowCompBuffer);
    auto activeLowCompBlock = lowCompBlock.getSubBlock(0, (size_t) numSamples);
    juce::dsp::ProcessContextReplacing<float> lowCompContext(activeLowCompBlock);

    lowCompensatorLP.process(lowContext);
    lowCompensatorHP.process(lowCompContext);
    
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        lowBuffer.addFrom(ch, 0, lowCompBuffer, ch, 0, numSamples);
    }

    float currentMidDrive = apvts.getRawParameterValue("mid_drive")->load();
    int midSlotChoice = (int)apvts.getRawParameterValue("mid_slot_1_fx")->load();

    if (!midBandChain.empty())
    {
    
        if (auto* slot = dynamic_cast<FXSlot*>(midBandChain[0].get()))
        {
            slot->setEffectChoice(midSlotChoice);

            slot->getGainModule().updateGain(currentMidGain);
            slot->getDistortionModule().updateDrive(currentMidDrive);
        }
    }

// Process the audio through all loaded effects serially
    for (auto& effect : lowBandChain)  { effect->process(lowContext); }
    for (auto& effect : midBandChain)  { effect->process(midContext); }
    for (auto& effect : highBandChain) { effect->process(highContext); }

    buffer.clear();
    //Add all the bands back together
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        buffer.addFrom(ch, 0, lowBuffer, ch, 0, numSamples);
        buffer.addFrom(ch, 0, midBuffer, ch, 0, numSamples);
        buffer.addFrom(ch, 0, highBuffer, ch, 0, numSamples);
    }


  
}

juce::AudioProcessorEditor* MultieffectsEQProcessor::createEditor()
{
    return new MultieffectsEQEditor (*this);
    //return new juce::GenericAudioProcessorEditor (*this);
}


//Called by the host when saving a daw or preset
void MultieffectsEQProcessor::getStateInformation (juce::MemoryBlock& destData)
{

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}
//Called by the host when loading a daw or preset
void MultieffectsEQProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MultieffectsEQProcessor();
}