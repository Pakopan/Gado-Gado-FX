#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <JuceHeader.h>
#include "PluginParameter.h"


class GadoGadoFXAudioProcessor  : public juce::AudioProcessor
{
public:
    enum filterTypeIndex {
        filterTypeLowPass = 0,
        filterTypeHighPass,
        filterTypeLowShelf,
        filterTypeHighShelf,
        filterTypeBandPass,
        filterTypeBandStop,
        filterTypePeakingNotch,
    };
    juce::StringArray filterTypeItemsUI = {
       "Low-pass ORDE 2",
       "High-pass ORDE 2",
       "Low-shelf ORDE 2",
       "High-shelf ORDE 2",
       "Band-pass ORDE 1",
       "Band-stop ORDE 1",
       "Peaking/Notch",
    };

    juce::StringArray fftSizeItemsUI = {
    "16",
    "32",
    "64",
    "128",
    "256",
    "512",
    "1024",
    "2048",
    "4096",
    "8192"
    };

    enum fftSizeIndex {
        fftSize16 = 0,
        fftSize32,
        fftSize64,
        fftSize128,
        fftSize256,
        fftSize512,
        fftSize1024,
        fftSize2048,
        fftSize4096,
        fftSize8192,
    };

    //======================================
    enum hopSizeIndex {
        hopSize2 = 0,
        hopSize4,
        hopSize8,
    };

    juce::StringArray hopSizeItemsUI = {
        "1/2 Window",
        "1/4 Window",
        "1/8 Window",
    };

    //======================================
    enum windowTypeIndex {
        windowTypeBartlett = 0,
        windowTypeHann,
        windowTypeHamming,
        windowTypeRectangular,
        windowTypeBarthann,
    };

    juce::StringArray windowTypeItemsUI = {
        "Bartlett",
        "Hann",
        "Hamming",
        "Rectangular",
        "Barthann",
    };


    //==============================================================================
    GadoGadoFXAudioProcessor();
    ~GadoGadoFXAudioProcessor() override;

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
    //float penguatan;

    juce::AudioSampleBuffer delayBuffer;
    int delayBufferSamples;
    int delayBufferChannels;
    int delayWritePosition;
    //======================================

    class Filter : public juce::IIRFilter
    {
    public:
        void updateCoefficients(const double discreteFrequency,
            const double qFactor,
            const double gain,
            const int filterType,
            const double fs) noexcept
        {
            jassert(discreteFrequency > 0);
            jassert(qFactor > 0);
            jassert(fs > 0);
            double bandwidth = juce::jmin(discreteFrequency / qFactor, M_PI * 0.99);
            double two_cos_wc = -2.0 * cos(discreteFrequency);
            double cos_x = 2.0 * cos(3.0 * M_PI / 4.0);
            double tan_half_bw = tan(bandwidth / 2.0);
            double tan_half_wc = tan(discreteFrequency / 2.0);
            double tan_half_kuadrat = pow(tan_half_wc, 2);
            double sqrt_gain = sqrt(gain);

            double b0c = tan_half_kuadrat;
            double b1c = 2 * tan_half_kuadrat;
            double b2c = tan_half_kuadrat;
            double a0c = 1 + tan_half_kuadrat - (cos_x * tan_half_wc);
            double a1c = 2 * tan_half_kuadrat - 2;
            double a2c = 1 + tan_half_kuadrat + (cos_x * tan_half_wc);

            double b0h = 1 - tan_half_wc * cos_x;
            double b1h = -2.0;
            double b2h = 1 + tan_half_wc * cos_x;
            double a0h = 1 + tan_half_kuadrat - (cos_x * tan_half_wc);
            double a1h = 2 * tan_half_kuadrat - 2;
            double a2h = 1 + tan_half_kuadrat + (cos_x * tan_half_wc);

            double b0ls = 1 - tan_half_wc * cos_x + gain * tan_half_kuadrat;
            double b1ls = 2 * gain * tan_half_kuadrat - 2.0;
            double b2ls = 1 + tan_half_wc * cos_x + gain * tan_half_kuadrat;
            double a0ls = 1 + tan_half_kuadrat - (cos_x * tan_half_wc);
            double a1ls = 2 * tan_half_kuadrat - 2;
            double a2ls = 1 + tan_half_kuadrat + (cos_x * tan_half_wc);

            double b0hs = gain - gain * tan_half_wc * cos_x + tan_half_kuadrat;
            double b1hs = 2 * tan_half_kuadrat - 2.0 * gain;
            double b2hs = gain + gain * tan_half_wc * cos_x + tan_half_kuadrat;
            double a0hs = 1 + tan_half_kuadrat - (cos_x * tan_half_wc);
            double a1hs = 2 * tan_half_kuadrat - 2;
            double a2hs = 1 + tan_half_kuadrat + (cos_x * tan_half_wc);

            double b0bp = tan_half_bw;
            double b1bp = 0.0;
            double b2bp = -tan_half_bw;
            double a0bp = 1.0 + tan_half_bw;
            double a1bp = two_cos_wc;
            double a2bp = 1.0 - tan_half_bw;

            double b0bs = 1.0;
            double b1bs = two_cos_wc;
            double b2bs = 1.0;
            double a0bs = 1.0 + tan_half_bw;
            double a1bs = two_cos_wc;
            double a2bs = 1.0 - tan_half_bw;

            double b0n = sqrt_gain + gain * tan_half_bw;
            double b1n = sqrt_gain * two_cos_wc;
            double b2n = sqrt_gain - gain * tan_half_bw;
            double a0n = sqrt_gain + tan_half_bw;
            double a1n = sqrt_gain * two_cos_wc;
            double a2n = sqrt_gain - tan_half_bw;
           
            switch (filterType) {
            case filterTypeLowPass: {
                coefficients = juce::IIRCoefficients(b0c, b1c, b2c, a0c, a1c, a2c);
                break;
            }
            case filterTypeHighPass: {
                coefficients = juce::IIRCoefficients(b0h, b1h, b2h, a0h, a1h, a2h);
                break;
            }
            case filterTypeLowShelf: {
                coefficients = juce::IIRCoefficients(b0ls, b1ls, b2ls, a0ls, a1ls, a2ls);
                break;
            }
            case filterTypeHighShelf: {
                coefficients = juce::IIRCoefficients(b0hs, b1hs, b2hs, a0hs, a1hs, a2hs);
                break;
            }
            case filterTypeBandPass: {
                coefficients = juce::IIRCoefficients(b0bp, b1bp, b2bp, a0bp, a1bp, a2bp);
                break;
            }
            case filterTypeBandStop: {
                coefficients = juce::IIRCoefficients(b0bs, b1bs, b2bs, a0bs, a1bs, a2bs);
                break;
            }
            case filterTypePeakingNotch: {
                coefficients = juce::IIRCoefficients(b0n, b1n, b2n, a0n, a1n, a2n);
                break;
            }
            }

            setCoefficients(coefficients);
        }
    };
    juce::OwnedArray<Filter> filters;
    void updateFilters();
    //=======================================

    void updateFftSize();
    void updateHopSize();
    void updateAnalysisWindow();
    void updateWindow(const juce::HeapBlock<float>& window, const int windowLength);
    void updateWindowScaleFactor();

    float princArg(const float phase);

    //======================================

    juce::CriticalSection lock;

    int fftSize;
    std::unique_ptr<juce::dsp::FFT> fft;

    int inputBufferLength;
    int inputBufferWritePosition;
    juce::AudioSampleBuffer inputBuffer;

    int outputBufferLength;
    int outputBufferWritePosition;
    int outputBufferReadPosition;
    juce::AudioSampleBuffer outputBuffer;

    juce::HeapBlock<float> fftWindow;
    juce::HeapBlock<juce::dsp::Complex<float>> fftTimeDomain;
    juce::HeapBlock<juce::dsp::Complex<float>> fftFrequencyDomain;

    int samplesSinceLastFFT;

    int overlap;
    int hopSize;
    float windowScaleFactor;

    //======================================

    juce::HeapBlock<float> omega;
    juce::AudioSampleBuffer inputPhase;
    juce::AudioSampleBuffer outputPhase;
    bool needToResetPhases;

 

    //========================================
    PluginParametersManager parameters;

    PluginParameterLinSlider paramBalance;
    PluginParameterLinSlider paramDelayTime;
    PluginParameterLinSlider paramFeedback;
    PluginParameterLinSlider paramMix;
    PluginParameterToggle paramToggleDelay;
    //efek delay

    PluginParameterLinSlider paramGainControl;
    PluginParameterToggle paramToggleGainControl;
    //efek Gain control

    //=======================================================
    PluginParameterLogSlider paramFrequency;
    PluginParameterLinSlider paramQfactor;
    PluginParameterLinSlider paramGain;
    PluginParameterComboBox paramFilterType;
    PluginParameterToggle paramToggleEQ;
    //======================================================

    PluginParameterLinSlider paramShift;
    PluginParameterComboBox paramFftSize;
    PluginParameterComboBox paramHopSize;
    PluginParameterComboBox paramWindowType;
    PluginParameterToggle paramTogglePS;
private:
    

    void DelayMode(juce::AudioBuffer<float>& buffer);
    void GainControlMode(juce::AudioBuffer<float>& buffer);
    void ParameterEQMode(juce::AudioBuffer<float>& buffer);
    void PitchShiftMode(juce::AudioBuffer<float>& buffer);
    void DefaultMode (juce::AudioBuffer<float>& buffer);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GadoGadoFXAudioProcessor)
};


