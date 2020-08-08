/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameter.h"
//==============================================================================
GadoGadoFXAudioProcessor::GadoGadoFXAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this)
    , paramDelayTime(parameters, "Delay time", "s", 0.0f, 1.0f, 0.1f)
    , paramFeedback(parameters, "Feedback", "", 0.0f, 0.9f, 0.7f)
    , paramMix(parameters, "Mix", "", 0.0f, 1.0f, 1.0f)
    , paramGainControl(parameters, "Gain", "", -40.0f, 20.0f, 0.0f)
    , paramToggleGainControl(parameters, "ON")
    , paramToggleDelay(parameters, "OW")
    , paramFrequency(parameters, "Frequency", "Hz", 10.0f, 20000.0f, 1500.0f,
        [this](float value) { paramFrequency.setCurrentAndTargetValue(value); updateFilters(); return value; })
    , paramQfactor(parameters, "Q Factor", "", 0.1f, 20.0f, sqrt(2.0f),
        [this](float value) { paramQfactor.setCurrentAndTargetValue(value); updateFilters(); return value; })
    , paramGain(parameters, "Gain P_EQ", "dB", -12.0f, 12.0f, 12.0f,
        [this](float value) { paramGain.setCurrentAndTargetValue(value); updateFilters(); return value; })
    , paramFilterType(parameters, "Filter type", filterTypeItemsUI, filterTypePeakingNotch,
        [this](float value) { paramFilterType.setCurrentAndTargetValue(value); updateFilters(); return value; })
{
    parameters.valueTreeState.state = juce::ValueTree(juce::Identifier(getName().removeCharacters("- ")));
}


GadoGadoFXAudioProcessor::~GadoGadoFXAudioProcessor()
{
}

//==============================================================================
const juce::String GadoGadoFXAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GadoGadoFXAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GadoGadoFXAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GadoGadoFXAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GadoGadoFXAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GadoGadoFXAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int GadoGadoFXAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GadoGadoFXAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String GadoGadoFXAudioProcessor::getProgramName (int index)
{
    return {};
}

void GadoGadoFXAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void GadoGadoFXAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    {
        const double smoothTime = 1e-3;
        paramDelayTime.reset(sampleRate, smoothTime);
        paramFeedback.reset(sampleRate, smoothTime);
        paramMix.reset(sampleRate, smoothTime);
        paramGainControl.reset(sampleRate, smoothTime);
        paramToggleDelay.reset(sampleRate, smoothTime);
        paramToggleGainControl.reset(sampleRate, smoothTime);
        //======================================

        float maxDelayTime = paramDelayTime.maxValue;
        delayBufferSamples = (int)(maxDelayTime * (float)sampleRate) + 1;
        if (delayBufferSamples < 1)
            delayBufferSamples = 1;

        delayBufferChannels = getTotalNumInputChannels();
        delayBuffer.setSize(delayBufferChannels, delayBufferSamples);
        delayBuffer.clear();

        delayWritePosition = 0;
        //============================================
/*
        filters.clear();
        for (int i = 0; i < getTotalNumInputChannels(); ++i) {
            Filter* filter;
            filters.add(filter = new Filter());
        }
        updateFilters();
        */
    }

}

void GadoGadoFXAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool GadoGadoFXAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void GadoGadoFXAudioProcessor::DelayMode(juce::AudioBuffer<float>& buffer) {
    if ((bool)paramToggleDelay.getNextValue()==1) 
    {
        juce::ScopedNoDenormals noDenormals;

        const int numInputChannels = getTotalNumInputChannels();
        const int numOutputChannels = getTotalNumOutputChannels();
        const int numSamples = buffer.getNumSamples();

        //======================================

        float currentDelayTime = paramDelayTime.getTargetValue() * (float)getSampleRate();
        float currentFeedback = paramFeedback.getNextValue();
        float currentMix = paramMix.getNextValue();

        int localWritePosition;

        for (int channel = 0; channel < numInputChannels; ++channel) {
            float* channelData = buffer.getWritePointer(channel);
            float* delayData = delayBuffer.getWritePointer(channel);
            localWritePosition = delayWritePosition;

            for (int sample = 0; sample < numSamples; ++sample) {
                const float in = channelData[sample];
                float out = 0.0f;

                float readPosition =
                    fmodf((float)localWritePosition - currentDelayTime + (float)delayBufferSamples, delayBufferSamples);
                int localReadPosition = floorf(readPosition);

                if (localReadPosition != localWritePosition) {
                    float fraction = readPosition - (float)localReadPosition;
                    float delayed1 = delayData[(localReadPosition + 0)];
                    float delayed2 = delayData[(localReadPosition + 1) % delayBufferSamples];
                    out = delayed1 + fraction * (delayed2 - delayed1);

                    channelData[sample] = in + currentMix * (out - in);
                    delayData[localWritePosition] = in + out * currentFeedback;
                }

                if (++localWritePosition >= delayBufferSamples)
                    localWritePosition -= delayBufferSamples;
            }
        }

        delayWritePosition = localWritePosition;

        //======================================

        for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
            buffer.clear(channel, 0, numSamples);
    }
    else DefaultMode(buffer);
}

void GadoGadoFXAudioProcessor::GainControlMode(juce::AudioBuffer<float>& buffer)
{  
    if ((bool)paramToggleGainControl.getNextValue()==1) {
        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        float gainValue = pow(10.0f, float(paramGainControl.getNextValue())/20.0f);

        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
            buffer.clear(i, 0, buffer.getNumSamples());

        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* channelData = buffer.getWritePointer(channel);

            for (int sampel = 0; sampel < buffer.getNumSamples(); sampel++) {
                channelData[sampel] = buffer.getSample(channel, sampel) * gainValue;
            }
        }
    }
    else DefaultMode(buffer);
}

void GadoGadoFXAudioProcessor::DefaultMode(juce::AudioBuffer<float>& buffer)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
    }
}


void GadoGadoFXAudioProcessor::ParameterEQMode(juce::AudioBuffer<float>& buffer)
{  
    juce::ScopedNoDenormals noDenormals;

    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();

    //======================================

    for (int channel = 0; channel < numInputChannels; ++channel) {
        float* channelData = buffer.getWritePointer(channel);
        filters[channel]->processSamples(channelData, numSamples);
    }

    for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
        buffer.clear(channel, 0, numSamples);
}

void GadoGadoFXAudioProcessor::updateFilters()
{
    double discreteFrequency = 2.0 * M_PI * (double)paramFrequency.getTargetValue() / getSampleRate();
    double qFactor = (double)paramQfactor.getTargetValue();
    double gain = pow(10.0, (double)paramGain.getTargetValue() * 0.05);
    int type = (int)paramFilterType.getTargetValue();

    for (int i = 0; i < filters.size(); ++i)
        filters[i]->updateCoefficients(discreteFrequency, qFactor, gain, type);
        
}

void GadoGadoFXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
   GainControlMode(buffer);
   DelayMode(buffer);
  // ParameterEQMode(buffer);
    
}

//==============================================================================
bool GadoGadoFXAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* GadoGadoFXAudioProcessor::createEditor()
{
    return new GadoGadoFXAudioProcessorEditor (*this);
}

//==============================================================================
void GadoGadoFXAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.valueTreeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void GadoGadoFXAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.valueTreeState.state.getType()))
            parameters.valueTreeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GadoGadoFXAudioProcessor();
}
