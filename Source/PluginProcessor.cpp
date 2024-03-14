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
}

ICMPfilterAudioProcessor::~ICMPfilterAudioProcessor()
{
    treeState.removeParameterListener("cutoff", this);
    treeState.removeParameterListener("quality", this);
    treeState.removeParameterListener("fType", this);
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

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
            auto* in = buffer.getReadPointer (channel);
            auto* out = buffer.getWritePointer (channel);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            out[sample] = filter.processSample(channel, in[sample]);
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

juce::AudioProcessorValueTreeState::ParameterLayout ICMPfilterAudioProcessor::createParamLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    using pID = juce::ParameterID;
    using range = juce::NormalisableRange<float>;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(pID{"cutoff", 1}, "Cutoff", range{10, 20000, 1, 0.3}, 20000));
    layout.add(std::make_unique<juce::AudioParameterFloat>(pID{"quality", 1}, "Q", range{0.7, 10, 0.1}, 0.7));
    layout.add(std::make_unique<juce::AudioParameterChoice>(pID{"fType", 1}, "Type", juce::StringArray{"LP", "HP","BP"}, 0));
    
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
        filter.setType(newValue);
    }
}
