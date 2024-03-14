/*
  ==============================================================================

    Filter.h
    Created: 14 Mar 2024 10:08:12am
    Author:  Elja Markkanen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>



class Filter {

    enum FilterType {
        LPF,
        HPF,
        BPF
    };
    
public:
    void setCutoff (float cutoff);
    void setQ (float q);
    void setType (float type);
    void setSampleRate (double sampleRate);

    void reset ();
    float processSample (int channel, float inputSample);
    
private:
    void updateCoefficents();

    const float mPI = juce::MathConstants<float>::pi;
    float mFc = 20000.f;
    double mFs = 44100;
    float mQ = 0.7;
    float alpha = 0.f, omega = 0.f;
    float a0 = 1.f, a1 = 0.f, a2 = 0.f, b0 = 0.f, b1 = 0.f, b2 = 0.f;
    
    std::array<float, 2> xn_1;
    std::array<float, 2> xn_2;
    
    std::array<float, 2> yn_1;
    std::array<float, 2> yn_2;

    FilterType mFilterType = LPF;
};
