/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ICMPfilterAudioProcessor::ICMPfilterAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    treeState.addParameterListener("cutoff", this);
    treeState.addParameterListener("quality", this);
    treeState.addParameterListener("fType", this);
    treeState.addParameterListener("lfoOn", this);
    treeState.addParameterListener("lfoDepth", this);
    treeState.addParameterListener("lfoRate", this);



}

ICMPfilterAudioProcessor::~ICMPfilterAudioProcessor()
{
    treeState.removeParameterListener("cutoff", this);
    treeState.removeParameterListener("quality", this);
    treeState.removeParameterListener("fType", this);
    treeState.removeParameterListener("lfoOn", this);
    treeState.removeParameterListener("lfoDepth", this);
    treeState.removeParameterListener("lfoRate", this);


}

//==============================================================================
const juce::String ICMPfilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ICMPfilterAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ICMPfilterAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ICMPfilterAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ICMPfilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ICMPfilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ICMPfilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ICMPfilterAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ICMPfilterAudioProcessor::getProgramName (int index)
{
    return {};
}

void ICMPfilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ICMPfilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    filter.setSampleRate(sampleRate);
    filter.setCutoff(treeState.getRawParameterValue("cutoff")->load());
    filter.setQ(treeState.getRawParameterValue("quality")->load());
    filter.reset();
    
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumInputChannels();
    spec.maximumBlockSize = samplesPerBlock;
    
    lfo.prepare(spec);
    lfo.initialise([](float x) {return std::sin(x);}, 128);
    lfo.setFrequency(0.5);
}

void ICMPfilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ICMPfilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ICMPfilterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    auto cutoffVal = treeState.getRawParameterValue("cutoff")->load();
    bool isLfoOn = treeState.getRawParameterValue("lfoOn")->load();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
        
        auto* in = buffer.getReadPointer (channel);
        auto* out = buffer.getWritePointer (channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
                
            if (isLfoOn && samplesSinceLastUpdate == 0) {
                auto lfoOut = lfo.processSample(0.f);
                auto lfoCutoffHz = juce::jmap(lfoOut, -1.f, 1.f, 1.f, mLfoDepth);
                lfoCutoffHz += cutoffVal;
                auto limitedCutoff = juce::jmin(20000.f, lfoCutoffHz);
                filter.setCutoff(limitedCutoff);
        
                samplesSinceLastUpdate = 10;
            }
            
            out[sample] = filter.processSample(channel, in[sample]);
            
            if (samplesSinceLastUpdate > 0) {
                samplesSinceLastUpdate--;
            }
        }
    }
}

//==============================================================================
bool ICMPfilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ICMPfilterAudioProcessor::createEditor()
{
//    return new ICMPfilterAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void ICMPfilterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ICMPfilterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ICMPfilterAudioProcessor();
}

void ICMPfilterAudioProcessor::setLfoDepth(float lfoDepth) {
    this->mLfoDepth = lfoDepth;
}

juce::AudioProcessorValueTreeState::ParameterLayout ICMPfilterAudioProcessor::createParamLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    using pID = juce::ParameterID;
    using range = juce::NormalisableRange<float>;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(pID{"cutoff", 1}, "Cutoff", range{20, 20000, 1, 0.3}, 20000));
    layout.add(std::make_unique<juce::AudioParameterFloat>(pID{"quality", 1}, "Q", range{0.3, 3, 0.1}, 0.3));
    layout.add(std::make_unique<juce::AudioParameterChoice>(pID{"fType", 1}, "Type", juce::StringArray{"LP","HP","BP","AP"}, 0));
    layout.add(std::make_unique<juce::AudioParameterBool>(pID{"lfoOn", 1}, "LFO On", false));
    layout.add(std::make_unique<juce::AudioParameterFloat>(pID{"lfoDepth", 1}, "LFO Depth", range{1.f, 10000.f, 1}, 500.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(pID{"lfoRate", 1}, "LFO Rate", range{0.1f, 100.f, 0.1}, 1.f));



    
    return layout;
}

void ICMPfilterAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if (parameterID == "cutoff") {
        filter.setCutoff(newValue);
    }
    
    if (parameterID == "quality") {
        filter.setQ(newValue);
    }
    
    if (parameterID == "fType") {
        filter.reset();
        filter.setType(newValue);
    }
    
    if (parameterID == "lfoDepth") {
        setLfoDepth(newValue);
    }
    
    if (parameterID == "lfoRate") {
        lfo.setFrequency(newValue);
    }

}


