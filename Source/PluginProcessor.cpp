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
    treeState.addParameterListener("lfoWave", this);
    treeState.addParameterListener("lfoDepth", this);
    treeState.addParameterListener("lfoRate", this);



}

ICMPfilterAudioProcessor::~ICMPfilterAudioProcessor()
{
    treeState.removeParameterListener("cutoff", this);
    treeState.removeParameterListener("quality", this);
    treeState.removeParameterListener("fType", this);
    treeState.removeParameterListener("lfoOn", this);
    treeState.removeParameterListener("lfoWave", this);
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
    filter.reset();
    smoothCutoff.reset(10);
    smoothModCutoff.reset(10);
    smoothedQ.reset(10);
    
    filter.setSampleRate(sampleRate);
//    filter.setCutoff(treeState.getRawParameterValue("cutoff")->load());
//    filter.setQ(treeState.getRawParameterValue("quality")->load());

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate /2;
    spec.numChannels = getTotalNumOutputChannels();
    spec.maximumBlockSize = samplesPerBlock;

    lfo.prepare(spec);
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
    auto cutoffVal = treeState.getRawParameterValue("cutoff")->load();
    auto qVal = treeState.getRawParameterValue("quality")->load();
    bool isLfoOn = treeState.getRawParameterValue("lfoOn")->load();
    
    smoothCutoff.setTargetValue(cutoffVal);
    float currentCutoff = smoothCutoff.getNextValue();
    filter.setCutoff(currentCutoff);
    
    smoothedQ.setTargetValue(qVal);
    float currentQ = smoothedQ.getNextValue();
    filter.setQ(currentQ);
    
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < getTotalNumInputChannels(); ++channel) {
        auto* in = buffer.getReadPointer (channel);
        auto* out = buffer.getWritePointer (channel);
                
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            if (isLfoOn && samplesSinceLastUpdate == 0) {
                auto lfoValue = lfo.processLfoFree(cutoffVal);
                smoothModCutoff.setTargetValue(lfoValue);
                auto currentCutoff = smoothModCutoff.getNextValue();
                filter.setCutoff(currentCutoff);
                samplesSinceLastUpdate = buffer.getNumSamples() / 2;
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

double ICMPfilterAudioProcessor::getHostBpm() const
{
    if (auto bpmFromHost = *getPlayHead()->getPosition()->getBpm()) {
        auto bpm = bpmFromHost;
        return bpm;
    }
    else {
        treeState.getParameter("lfoSync")->setValueNotifyingHost(0);
        return 0;
    }

}

void ICMPfilterAudioProcessor::updateParameters() {
    
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
//    layout.add(std::make_unique<juce::AudioParameterBool>(pID{"lfoSync", 1}, "LFO Sync", false));
    layout.add(std::make_unique<juce::AudioParameterChoice>(pID{"lfoWave", 1}, "LFO Waveform", juce::StringArray{"Sine","Ramp Up", "Ramp Down", "Square"}, 0));
    layout.add(std::make_unique<juce::AudioParameterFloat>(pID{"lfoDepth", 1}, "LFO Depth", range{1.f, 20000.f, 1}, 500.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>(pID{"lfoRate", 1}, "LFO Rate", range{0.1f, 250.f, 0.1}, 1.f));
    
    return layout;
}


void ICMPfilterAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
//    if (parameterID == "cutoff") {
//        smoothCutoff.setTargetValue(newValue);
//        float currentCutoff = smoothCutoff.getNextValue();
//        filter.setCutoff(currentCutoff);
//    }
//    if (parameterID == "quality") {
//        smoothedQ.setTargetValue(newValue);
//        float currentQ = smoothedQ.getNextValue();
//        filter.setQ(currentQ);
//    }
    if (parameterID == "fType") {
        filter.reset();
        filter.setType(newValue);
    }
    if (parameterID == "lfoOn") {
        filter.reset();
    }
    if (parameterID == "lfoWave") {
        filter.reset();
        lfo.selectWaveform(newValue);
    }
    if (parameterID == "lfoDepth") {
        lfo.setLfoDepth(newValue);
    }
    
    if (parameterID == "lfoRate") {
        lfo.setFrequency(newValue);
    }
}


