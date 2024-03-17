/*
  ==============================================================================

    Filter.cpp
    Created: 14 Mar 2024 10:08:12am
    Author:  Elja Markkanen

  ==============================================================================
*/

#include "Filter.h"


void Filter::setCutoff(float cutoff) {
    this->mFc = cutoff;
    updateCoefficents();
}

void Filter::setQ(float q) {
    this->mQ = q;
    updateCoefficents();
}

void Filter::setType(float type) {
    this->mFilterType = static_cast<FilterType>(static_cast<int>(type));
    updateCoefficents();
}

void Filter::setSampleRate(double sampleRate) {
    this->mFs = sampleRate;
}

void Filter::reset() {
    std::fill(xn_1.begin(), xn_1.end(), 0.f);
    std::fill(xn_2.begin(), xn_2.end(), 0.f);
    std::fill(yn_1.begin(), yn_1.end(), 0.f);
    std::fill(yn_2.begin(), yn_2.end(), 0.f);
}


void Filter::updateCoefficents() {
    omega = (2 * mPI) * (mFc / mFs);
    alpha = sin(omega) / (2 * mQ);
    
    switch (mFilterType) {
        case LPF:
            a0 = 1 + alpha;
            a1 = -2 * cos(omega);
            a2 = 1 - alpha;
            b0 = (1 - cos(omega)) / 2;
            b1 = 1 - cos(omega);
            b2 = (1 - cos(omega)) / 2;
            break;
        case HPF:
            a0 = 1 + alpha;
            a1 = -2 * cos(omega);
            a2 = 1 - alpha;
            b0 = (1 + cos(omega)) / 2;
            b1 = -(1 + cos(omega));
            b2 = (1 + cos(omega)) / 2;
            break;
        case BPF:
            a0 = 1 + alpha;
            a1 = -2 * cos(omega);
            a2 = 1 - alpha;
            b0 = alpha;
            b1 = 0.f;
            b2 = -alpha;
            break;
        case APF:
            a0 = 1 + alpha;
            a1 = -2 * cos(omega);
            a2 = 1 - alpha;
            b0 = 1 - alpha;
            b1 = -2 * cos(omega);
            b2 = 1 + alpha;
            break;
        default:
            break;
    }

}

float Filter::processSample(int channel, float inputSample) {
    float x0 = inputSample;
    float y0 = (b0/a0)*x0 + (b1/a0)*xn_1[channel] + (b2/a0)*xn_2[channel] - (a1/a0)*yn_1[channel] - (a2/a0)*yn_2[channel];
    auto outputSample = y0;

    xn_2[channel] = xn_1[channel];
    xn_1[channel] = x0;
    yn_2[channel] = yn_1[channel];
    yn_1[channel] = y0;
    
    return outputSample;
}
