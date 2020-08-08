#pragma once
#define _USE_MATH_DEFINES
#include <cmath>
#include <JuceHeader.h>
#include "PluginParameter.h"
//==============================================================================
/**
*/
class GadoGadoFXAudioProcessor  : public juce::AudioProcessor
{
public:
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
    PluginParameterComboBox testing;

  
 const juce::StringArray filterTypeItemsUI = {
        "Low-pass",
        "High-pass",
        "Low-shelf",
        "High-shelf",
        "Band-pass",
        "Band-stop",
        "Peaking/Notch"
    };

    enum filterTypeIndex {
        filterTypeLowPass = 0,
        filterTypeHighPass,
        filterTypeLowShelf,
        filterTypeHighShelf,
        filterTypeBandPass,
        filterTypeBandStop,
        filterTypePeakingNotch,
    };

    //======================================

    class Filter : public juce::IIRFilter
    {
    public:
        void updateCoefficients(const double discreteFrequency,
            const double qFactor,
            const double gain,
            const int filterType) noexcept
        {
            jassert(discreteFrequency > 0);
            jassert(qFactor > 0);

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

private:
    void DelayMode(juce::AudioBuffer<float>& buffer);
    void GainControlMode(juce::AudioBuffer<float>& buffer);
    void ParameterEQMode(juce::AudioBuffer<float>& buffer);
    void DefaultMode (juce::AudioBuffer<float>& buffer);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GadoGadoFXAudioProcessor)
};


