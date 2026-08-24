#include "PluginProcessor.h"
#include "PluginEditor.h"


MultieffectsEQProcessor::MultieffectsEQProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
       
       
       apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

MultieffectsEQProcessor::~MultieffectsEQProcessor() {}


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

    return { params.begin(), params.end() };
}

void MultieffectsEQProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
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
}

void MultieffectsEQProcessor::releaseResources()
{
}

bool MultieffectsEQProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}


void MultieffectsEQProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float currentLowMidFreq = apvts.getRawParameterValue("low_mid_crossover")->load();
    float currentMidHighFreq = apvts.getRawParameterValue("mid_high_crossover")->load();

    lowMidCrossoverLP.setCutoffFrequency(currentLowMidFreq);
    lowMidCrossoverHP.setCutoffFrequency(currentLowMidFreq);
    midHighCrossoverLP.setCutoffFrequency(currentMidHighFreq);
    midHighCrossoverHP.setCutoffFrequency(currentMidHighFreq);

    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        auto numSamples = buffer.getNumSamples();
        lowBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        midBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
        highBuffer.copyFrom(ch, 0, buffer, ch, 0, numSamples);
    }

    juce::dsp::AudioBlock<float> lowBlock(lowBuffer);
    juce::dsp::AudioBlock<float> midBlock(midBuffer);
    juce::dsp::AudioBlock<float> highBlock(highBuffer);

    juce::dsp::ProcessContextReplacing<float> lowContext(lowBlock);
    juce::dsp::ProcessContextReplacing<float> midContext(midBlock);
    juce::dsp::ProcessContextReplacing<float> highContext(highBlock);

    lowMidCrossoverLP.process(lowContext);

    lowMidCrossoverHP.process(midContext);
    midHighCrossoverLP.process(midContext);
    
    lowMidCrossoverHP.process(highContext); 
    midHighCrossoverHP.process(highContext);

    buffer.clear();
    for (int ch = 0; ch < totalNumInputChannels; ++ch)
    {
        buffer.addFrom(ch, 0, lowBuffer, ch, 0, buffer.getNumSamples());
        buffer.addFrom(ch, 0, midBuffer, ch, 0, buffer.getNumSamples());
        buffer.addFrom(ch, 0, highBuffer, ch, 0, buffer.getNumSamples());
    }

  
}

juce::AudioProcessorEditor* MultieffectsEQProcessor::createEditor()
{
    return new MultieffectsEQEditor (*this);
}



void MultieffectsEQProcessor::getStateInformation (juce::MemoryBlock& destData)
{

    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

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