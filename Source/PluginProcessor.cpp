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
    // A vector to hold our parameters before handing them to the APVTS.
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // We define a float parameter for the crossover point.
    // The string ID ("low_mid_crossover") is how we look up the value in the DSP and attach the UI.
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("low_mid_crossover", 1), // ID and version
        "Low/Mid Crossover",                       // Human-readable name
        juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.3f), // Min, Max, Step size, Skew (logarithmic curve for freq)
        250.0f));                                  // Default value

    return { params.begin(), params.end() };
}

void MultieffectsEQProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
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