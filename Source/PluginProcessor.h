/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Filter.h"
#include "Lfo.h"


//==============================================================================
/**
*/
class ICMPfilterAudioProcessor  : public juce::AudioProcessor, juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    ICMPfilterAudioProcessor();
    ~ICMPfilterAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParamLayout();
    juce::AudioProcessorValueTreeState treeState {*this, nullptr, "params", createParamLayout()};
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    double getHostBpm() const;

private:
    void updateParameters();

    LFO lfo;
    int samplesSinceLastUpdate = 100;

    Filter filter;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothCutoff;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Multiplicative> smoothModCutoff;
    juce::SmoothedValue<float> smoothQ;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ICMPfilterAudioProcessor)
};
