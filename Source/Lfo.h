/*
  ==============================================================================

    Lfo.h
    Created: 17 Mar 2024 12:09:26pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class LFO {
    
public:
    void prepare (const juce::dsp::ProcessSpec& spec);
    void reset();
    void selectWaveform(float waveform);
    void setFrequency(float freq);
    void setLfoDepth(float lfoDepth);
    float processSample(float inputSample);
    
    float getLfoDepth() const;
    float processLfoFree(float cutoffVal);
    float processLfoSync(float cutoffVal, double hostBpm);

private:
    float mFrequency = 0.5f;
    double mSampleRate = 44100;
    float mMaxBlockSize = 512;
    float mNumChannels = 2;
    
    juce::dsp::Oscillator<float> osc;
    
    float mLfoDepth;
    
    enum Waveform {
        SINE,
        RAMP_UP,
        RAMP_DOWN,
        SQUARE
    };
    
    Waveform mWaveform = SINE;
};
