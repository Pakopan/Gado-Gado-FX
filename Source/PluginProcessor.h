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
       "Low-pass",
       "High-pass",
       "Low-shelf",
       "High-shelf",
       "Band-pass",
       "Band-stop",
       "Peaking/Notch",
    };

    juce::StringArray fftSizeItemsUI = {
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
        fftSize32 = 0,
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
    };

    juce::StringArray windowTypeItemsUI = {
        "Bartlett",
        "Hann",
        "Hamming",
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
            double tan_half_bw = tan(bandwidth / 2.0);
            double tan_half_wc = tan(discreteFrequency / 2.0);
            double sqrt_gain = sqrt(gain);
           
            switch (filterType) {
            case filterTypeLowPass: {
                coefficients = juce::IIRCoefficients(/* b0 */ tan_half_wc,
                    /* b1 */ tan_half_wc,
                    /* b2 */ 0.0,
                    /* a0 */ tan_half_wc + 1.0,
                    /* a1 */ tan_half_wc - 1.0,
                    /* a2 */ 0.0);
                break;
            }
            case filterTypeHighPass: {
                coefficients = juce::IIRCoefficients(/* b0 */ 1.0,
                    /* b1 */ -1.0,
                    /* b2 */ 0.0,
                    /* a0 */ tan_half_wc + 1.0,
                    /* a1 */ tan_half_wc - 1.0,
                    /* a2 */ 0.0);
                break;
            }
            case filterTypeLowShelf: {
                coefficients = juce::IIRCoefficients(/* b0 */ gain * tan_half_wc + sqrt_gain,
                    /* b1 */ gain * tan_half_wc - sqrt_gain,
                    /* b2 */ 0.0,
                    /* a0 */ tan_half_wc + sqrt_gain,
                    /* a1 */ tan_half_wc - sqrt_gain,
                    /* a2 */ 0.0);
                break;
            }
            case filterTypeHighShelf: {
                coefficients = juce::IIRCoefficients(/* b0 */ sqrt_gain * tan_half_wc + gain,
                    /* b1 */ sqrt_gain * tan_half_wc - gain,
                    /* b2 */ 0.0,
                    /* a0 */ sqrt_gain * tan_half_wc + 1.0,
                    /* a1 */ sqrt_gain * tan_half_wc - 1.0,
                    /* a2 */ 0.0);
                break;
            }
            case filterTypeBandPass: {
                coefficients = juce::IIRCoefficients(/* b0 */ tan_half_bw,
                    /* b1 */ 0.0,
                    /* b2 */ -tan_half_bw,
                    /* a0 */ 1.0 + tan_half_bw,
                    /* a1 */ two_cos_wc,
                    /* a2 */ 1.0 - tan_half_bw);
                break;
            }
            case filterTypeBandStop: {
                coefficients = juce::IIRCoefficients(/* b0 */ 1.0,
                    /* b1 */ two_cos_wc,
                    /* b2 */ 1.0,
                    /* a0 */ 1.0 + tan_half_bw,
                    /* a1 */ two_cos_wc,
                    /* a2 */ 1.0 - tan_half_bw);
                break;
            }
            case filterTypePeakingNotch: {
                coefficients = juce::IIRCoefficients(/* b0 */ sqrt_gain + gain * tan_half_bw,
                    /* b1 */ sqrt_gain * two_cos_wc,
                    /* b2 */ sqrt_gain - gain * tan_half_bw,
                    /* a0 */ sqrt_gain + tan_half_bw,
                    /* a1 */ sqrt_gain * two_cos_wc,
                    /* a2 */ sqrt_gain - tan_half_bw);
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

    PluginParameterLinSlider paramDelayTime;
    PluginParameterLinSlider paramFeedback;
    PluginParameterLinSlider paramMix;
    PluginParameterLinSlider paramGainControl;

    PluginParameterToggle paramToggleGainControl;
    PluginParameterToggle paramToggleDelay;
    //efek delay

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


