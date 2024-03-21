/*
  ==============================================================================

    Lfo.cpp
    Created: 17 Mar 2024 12:09:26pm
    Author:  Elja Markkanen

  ==============================================================================
*/

#include "Lfo.h"

void LFO::prepare(const juce::dsp::ProcessSpec &spec) {
    mSampleRate = spec.sampleRate;
    mMaxBlockSize = spec.maximumBlockSize;
    mNumChannels = spec.numChannels;
    
    osc.prepare(spec);
    osc.initialise([](float x) {return std::sin(x);}, 128);
}

void LFO::reset() {
    osc.reset();
}

void LFO::setFrequency(float freq) {
    osc.setFrequency(freq);
}
void LFO::setLfoDepth(float lfoDepth) {
    this->mLfoDepth = lfoDepth;
}


float LFO::getLfoDepth() const {
    return mLfoDepth;
}

void LFO::selectWaveform(float waveform) {
    this->mWaveform = static_cast<Waveform>(static_cast<int>(waveform));
    
    switch (mWaveform) {
        case SINE:
            osc.initialise([](float x) {return std::sin(x);}, 128);
            break;
        case RAMP_UP:
            osc.initialise ([] (float x){return juce::jmap(x, float(-juce::MathConstants<double>::pi), float(juce::MathConstants<double>::pi), float(-1), float(1));}, 3);
            break;
        case RAMP_DOWN:
            osc.initialise ([] (float x){return juce::jmap(x, float(-juce::MathConstants<double>::pi), float(juce::MathConstants<double>::pi), float(1), float(-1));}, 2);
            break;
        default:
            break;
    }
}

float LFO::processSample(float inputSample) {
    return osc.processSample(inputSample);
}

