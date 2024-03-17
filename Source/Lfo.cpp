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
            osc.initialise ([] (float x){return juce::jmap(x, float(-juce::MathConstants<double>::pi), float(juce::MathConstants<double>::pi), float(-1), float(1));}, 2);
            break;
        case RAMP_DOWN:
            osc.initialise ([] (float x){return juce::jmap(x, float(-juce::MathConstants<double>::pi), float(juce::MathConstants<double>::pi), float(1), float(-1));}, 2);
            break;
        case SQUARE:
            osc.initialise([](float x) { return sin(x) >= 0 ? 1.0 : -1.0; }, 1);
            break;
        default:
            break;
    }
}

float LFO::processSample(float inputSample) {
    return osc.processSample(inputSample);
}

float LFO::processLfoFree(float cutoffVal) {
    auto lfoOut = osc.processSample(0.f);
    auto lfoCutoffHz = juce::jmap(lfoOut, -1.f, 1.f, 1.f, mLfoDepth); // korjaa lfo ei mene tarpeeksi alas
    lfoCutoffHz += cutoffVal;
    auto rangeLimitedCutoff = juce::jmin(20000.f, lfoCutoffHz);
    
    return rangeLimitedCutoff;
}
             
float LFO::processLfoSync(float cutoffVal, double hostBpm) {
    auto beatTimeInSeconds = 60.0f / hostBpm;
    auto lfoCutoffHzSynced = 1.0f / (beatTimeInSeconds * 4);
    osc.setFrequency(lfoCutoffHzSynced);
    
    auto lfoOut = osc.processSample(0.f);
    auto lfoCutoffHz = juce::jmap(lfoOut, -1.f, 1.f, 1.f, mLfoDepth);   
    lfoCutoffHz += cutoffVal;
    auto rangeLimitedCutoff = juce::jmin(20000.f, lfoCutoffHz);
    
    return rangeLimitedCutoff;
}
   
