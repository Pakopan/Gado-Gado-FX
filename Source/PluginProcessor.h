/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

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
private:
    void DelayMode(juce::AudioBuffer<float>& buffer);
    void GainControlMode(juce::AudioBuffer<float>& buffer);
    void DefaultMode (juce::AudioBuffer<float>& buffer);
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GadoGadoFXAudioProcessor)
};

