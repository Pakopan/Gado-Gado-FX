#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameter.h"
//==============================================================================
GadoGadoFXAudioProcessor::GadoGadoFXAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif
    parameters(*this)
    , paramBalance(parameters, "Balance input", "", 0.0f, 1.0f, 0.25f)          //0
    , paramDelayTime(parameters, "Delay time", "s", 0.0f, 5.0f, 0.1f)           //1
    , paramFeedback(parameters, "Feedback", "", 0.0f, 0.9f, 0.7f)               //2
    , paramMix(parameters, "Mix", "", 0.0f, 1.0f, 1.0f)                         //3
    , paramToggleDelay(parameters, "DELAY CONTROL ON")                          //4

    , paramGainControl(parameters, "Gain", "dB", -40.0f, 20.0f, 0.0f)           //5
    , paramToggleGainControl(parameters, "GAIN CONTROL ON")                     //6

    , paramFrequency(parameters, "Frequency", "Hz", 10.0f, 20000.0f, 1500.0f,                                           //7
        [this](float value) { paramFrequency.setCurrentAndTargetValue(value); updateFilters(); return value; })
    , paramQfactor(parameters, "Q Factor", "", 0.1f, 20.0f, sqrt(2.0f),                                                 //8
        [this](float value) { paramQfactor.setCurrentAndTargetValue(value); updateFilters(); return value; })
    , paramGain(parameters, "Gain P_EQ", "dB", -12.0f, 12.0f, 12.0f,                                                    //9
        [this](float value) { paramGain.setCurrentAndTargetValue(value); updateFilters(); return value; })
    , paramFilterType(parameters, "Filter type", filterTypeItemsUI, NULL ,                                              //10
        [this](float value) { paramFilterType.setCurrentAndTargetValue(value); updateFilters(); return value; })
    , paramToggleEQ(parameters, "PARAMETER EQ ON")                                                                     //11
    
    , paramShift(parameters, "Shift", " Semitone(s)", -12.0f, 12.0f, 0.0f,                                              //12
        [this](float value) { return powf(2.0f, value / 12.0f); })
    , paramFftSize(parameters, "FFT size", fftSizeItemsUI, NULL,                                                        //13
        [this](float value) {
            const juce::ScopedLock sl(lock);
            value = (float)(1 << ((int)value + 5));
            paramFftSize.setCurrentAndTargetValue(value);
            updateFftSize();
            updateHopSize();
            updateAnalysisWindow();
            updateWindowScaleFactor();
            return value;
        })
    , paramHopSize(parameters, "Hop size", hopSizeItemsUI, NULL ,                                                      //14
        [this](float value) {
            const juce::ScopedLock sl(lock);
            value = (float)(1 << ((int)value + 1));
            paramHopSize.setCurrentAndTargetValue(value);
            updateFftSize();
            updateHopSize();
            updateAnalysisWindow();
            updateWindowScaleFactor();
            return value;
        })
            , paramWindowType(parameters, "Window type", windowTypeItemsUI, NULL,                                       //15
                [this](float value) {
                    const juce::ScopedLock sl(lock);
                    paramWindowType.setCurrentAndTargetValue(value);
                    updateFftSize();
                    updateHopSize();
                    updateAnalysisWindow();
                    updateWindowScaleFactor();
                    return value;
                })
            , paramTogglePS(parameters, "PITCH SHIFT ON")                                                               //16
{
    parameters.valueTreeState.state = juce::ValueTree(juce::Identifier(getName().removeCharacters("- ")));
}

//============================================= Built in functions  ==========================================================
GadoGadoFXAudioProcessor::~GadoGadoFXAudioProcessor()
{
}

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

                                //======================
void GadoGadoFXAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    {
        const double smoothTime = 1e-3;

        paramGainControl.reset(sampleRate, smoothTime);
        paramToggleGainControl.reset(sampleRate, smoothTime);

        paramBalance.reset(sampleRate, smoothTime);
        paramDelayTime.reset(sampleRate, smoothTime);
        paramFeedback.reset(sampleRate, smoothTime);
        paramMix.reset(sampleRate, smoothTime);
        paramToggleDelay.reset(sampleRate, smoothTime);
        
        
        paramFrequency.reset(sampleRate, smoothTime);
        paramQfactor.reset(sampleRate, smoothTime);
        paramGain.reset(sampleRate, smoothTime);
        paramFilterType.reset(sampleRate, smoothTime);
        paramToggleEQ.reset(sampleRate, smoothTime);

        paramShift.reset(sampleRate, smoothTime);
        paramFftSize.reset(sampleRate, smoothTime);
        paramHopSize.reset(sampleRate, smoothTime);
        paramWindowType.reset(sampleRate, smoothTime);
        paramTogglePS.reset(sampleRate, smoothTime);

        //==================================== init var for pitch shift
        needToResetPhases = true;

        //====================================== Delay part

        float maxDelayTime = paramDelayTime.maxValue;
        delayBufferSamples = (int)(maxDelayTime * (float)sampleRate) + 1;
        if (delayBufferSamples < 1)
            delayBufferSamples = 1;

        delayBufferChannels = getTotalNumInputChannels();
        delayBuffer.setSize(delayBufferChannels, delayBufferSamples);
        delayBuffer.clear();

        delayWritePosition = 0;
        //============================================ Parameter EQ part

        filters.clear();
        for (int i = 0; i < getTotalNumInputChannels(); ++i) {
            Filter* filter;
            filters.add(filter = new Filter());
        }
        updateFilters();
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

void GadoGadoFXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
   GainControlMode(buffer);
   DelayMode(buffer);
   ParameterEQMode(buffer);
   PitchShiftMode(buffer);
    
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

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GadoGadoFXAudioProcessor();
}

//=============================== self built function===========================================

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

void GadoGadoFXAudioProcessor::GainControlMode(juce::AudioBuffer<float>& buffer)
{
    if (paramToggleGainControl.getNextValue()) {
        juce::ScopedNoDenormals noDenormals;
        auto totalNumInputChannels = getTotalNumInputChannels();
        auto totalNumOutputChannels = getTotalNumOutputChannels();

        float gainValue = pow(10.0f, float(paramGainControl.getNextValue()) / 20.0f);

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

void GadoGadoFXAudioProcessor::DelayMode(juce::AudioBuffer<float>& buffer) {
    if (paramToggleDelay.getNextValue())
    {
        juce::ScopedNoDenormals noDenormals;

        const int numInputChannels = getTotalNumInputChannels();
        const int numOutputChannels = getTotalNumOutputChannels();
        const int numSamples = buffer.getNumSamples();

        //======================================

        float currentBalance = paramBalance.getNextValue();
        float currentDelayTime = paramDelayTime.getTargetValue() * (float)getSampleRate();
        float currentFeedback = paramFeedback.getNextValue();
        float currentMix = paramMix.getNextValue();

        int localWritePosition = delayWritePosition;

        float* channelDataL = buffer.getWritePointer(0);
        float* channelDataR = buffer.getWritePointer(1);
        float* delayDataL = delayBuffer.getWritePointer(0);
        float* delayDataR = delayBuffer.getWritePointer(1);

        for (int sample = 0; sample < numSamples; ++sample) {
            const float inL = (1.0f - currentBalance) * channelDataL[sample];
            const float inR = currentBalance * channelDataR[sample];
            float outL = 0.0f;
            float outR = 0.0f;

            float readPosition =
                fmodf((float)localWritePosition - currentDelayTime + (float)delayBufferSamples, delayBufferSamples);
            int localReadPosition = floorf(readPosition);

            if (localReadPosition != localWritePosition) {
                float fraction = readPosition - (float)localReadPosition;
                float delayed1L = delayDataL[(localReadPosition + 0)];
                float delayed1R = delayDataR[(localReadPosition + 0)];
                float delayed2L = delayDataL[(localReadPosition + 1) % delayBufferSamples];
                float delayed2R = delayDataR[(localReadPosition + 1) % delayBufferSamples];
                outL = delayed1L + fraction * (delayed2L - delayed1L);
                outR = delayed1R + fraction * (delayed2R - delayed1R);

                channelDataL[sample] = inL + currentMix * (outL - inL);
                channelDataR[sample] = inR + currentMix * (outR - inR);
                delayDataL[localWritePosition] = inL + outR * currentFeedback;
                delayDataR[localWritePosition] = inR + outL * currentFeedback;
            }

            if (++localWritePosition >= delayBufferSamples)
                localWritePosition -= delayBufferSamples;
        }

        delayWritePosition = localWritePosition;

        //======================================

        for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
            buffer.clear(channel, 0, numSamples);
    }
    else DefaultMode(buffer);
}

void GadoGadoFXAudioProcessor::ParameterEQMode(juce::AudioBuffer<float>& buffer)
{
    if (paramToggleEQ.getNextValue())
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
    else DefaultMode(buffer);
}

void  GadoGadoFXAudioProcessor::PitchShiftMode(juce::AudioBuffer<float>& buffer)
{
    if (paramTogglePS.getNextValue())
    {
        const juce::ScopedLock sl(lock);

        juce::ScopedNoDenormals noDenormals;

        const int numInputChannels = getTotalNumInputChannels();
        const int numOutputChannels = getTotalNumOutputChannels();
        const int numSamples = buffer.getNumSamples();

        //================================================================   

        int currentInputBufferWritePosition;
        int currentOutputBufferWritePosition;
        int currentOutputBufferReadPosition;
        int currentSamplesSinceLastFFT;

        float shift = paramShift.getNextValue();
        float ratio = roundf(shift * (float)hopSize) / (float)hopSize; //rasio hop size antara frekuensi output(yang diinginkan) dengan frekuensi input/asli
        int resampledLength = floorf((float)fftSize / ratio); //panjang sampel setelah dishift frekuensinya
        juce::HeapBlock<float> resampledOutput(resampledLength, true); //buffer untuk menampung resample
        juce::HeapBlock<float> synthesisWindow(resampledLength, true); //buffer untuk menampung fungsi window sintesis
        updateWindow(synthesisWindow, resampledLength); // digunakan untuk interpolasi linier paling akhir

        for (int channel = 0; channel < numInputChannels; ++channel) {
            float* channelData = buffer.getWritePointer(channel);

            currentInputBufferWritePosition = inputBufferWritePosition;      
            currentOutputBufferWritePosition = outputBufferWritePosition;
            currentOutputBufferReadPosition = outputBufferReadPosition;
            currentSamplesSinceLastFFT = samplesSinceLastFFT;

            for (int sample = 0; sample < numSamples; ++sample) { //lakukan sampling dulu tiap sampel per channel

                //===========================================

                const float in = channelData[sample];
                channelData[sample] = outputBuffer.getSample(channel, currentOutputBufferReadPosition);

                //===========================================

                outputBuffer.setSample(channel, currentOutputBufferReadPosition, 0.0f); // kosongkan output buffer
                if (++currentOutputBufferReadPosition >= outputBufferLength)
                    currentOutputBufferReadPosition = 0;

                //======================================

                inputBuffer.setSample(channel, currentInputBufferWritePosition, in); // isi output buffer
                if (++currentInputBufferWritePosition >= inputBufferLength)
                    currentInputBufferWritePosition = 0;

                //======================================

                if (++currentSamplesSinceLastFFT >= hopSize) { //jika sampel untuk fft per hop size sudah cukup
                    // dengan kata lain hasil pitch shifting dikeluarkan per hop size
                    currentSamplesSinceLastFFT = 0;

                    //================== Tiap hop size di
                    // digunaknan nilai N=M, dimana N meerupakan jumlah sampel windowing, dan M merupakan bagian fft
                    // nilai bisa M>=N ========================================
                    int inputBufferIndex = currentInputBufferWritePosition;
                    for (int index = 0; index < fftSize; ++index) { 
                        fftTimeDomain[index].real(sqrtf(fftWindow[index]) * inputBuffer.getSample(channel, inputBufferIndex)); // windowing -> masukkan ke variabel kompleks time domain
                        fftTimeDomain[index].imag(0.0f); // time domain -> j = 0

                        if (++inputBufferIndex >= inputBufferLength)
                            inputBufferIndex = 0; //

                    // inisialisasi data audio time domain yang sudah diwindow.
                    }

                    //======================================

                    fft->perform(fftTimeDomain, fftFrequencyDomain, false); // lakukan fft ke hasil windowing input time domain

                    if (paramShift.isSmoothing()) needToResetPhases = true;
                    if (shift == paramShift.getTargetValue() && needToResetPhases) 
                    {
                        inputPhase.clear();
                        outputPhase.clear();
                        needToResetPhases = false;
                    }

                    for (int index = 0; index < fftSize; ++index) {
                        float magnitude = abs(fftFrequencyDomain[index]); // cari magnitude tiap komponen hasil fft
                        float phase = arg(fftFrequencyDomain[index]); // cari fase tiap komponen hasil fft

                        float phaseDeviation = phase - inputPhase.getSample(channel, index) - omega[index] * (float)hopSize;
                        float deltaPhi = omega[index] * hopSize + princArg(phaseDeviation);
                        float newPhase = princArg(outputPhase.getSample(channel, index) + deltaPhi * ratio);

                        inputPhase.setSample(channel, index, phase);
                        outputPhase.setSample(channel, index, newPhase); //set fase yg sudah digeser ke buffer outputPhase
                        fftFrequencyDomain[index] = std::polar(magnitude, newPhase);
                    }

                    fft->perform(fftFrequencyDomain, fftTimeDomain, true); // inverse fft untuk kembali ke time domain
                       

                    //===========================================================================================================
                    // interpolasi linear untuk membuat konstan waktu audio
                    for (int index = 0; index < resampledLength; ++index) {
                        float x = (float)index * (float)fftSize / (float)resampledLength;
                        int ix = (int)floorf(x);
                        float dx = x - (float)ix;

                        float sample1 = fftTimeDomain[ix].real();
                        float sample2 = fftTimeDomain[(ix + 1) % fftSize].real();
                        resampledOutput[index] = sample1 + dx * (sample2 - sample1);
                        resampledOutput[index] *= sqrtf(synthesisWindow[index]);
                    }

                    int outputBufferIndex = currentOutputBufferWritePosition;
                    for (int index = 0; index < resampledLength; ++index) {
                        float out = outputBuffer.getSample(channel, outputBufferIndex);
                        out += resampledOutput[index] * windowScaleFactor;
                        outputBuffer.setSample(channel, outputBufferIndex, out);

                        if (++outputBufferIndex >= outputBufferLength) outputBufferIndex = 0;
                    }
                    currentOutputBufferWritePosition += hopSize;
                    if (currentOutputBufferWritePosition >= outputBufferLength)
                        currentOutputBufferWritePosition = 0;
                }
            }
        }

        inputBufferWritePosition = currentInputBufferWritePosition;
        outputBufferWritePosition = currentOutputBufferWritePosition;
        outputBufferReadPosition = currentOutputBufferReadPosition;
        samplesSinceLastFFT = currentSamplesSinceLastFFT;

        //======================================

        for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
            buffer.clear(channel, 0, numSamples);
    }
    else DefaultMode(buffer);
}


//======================================== additional self built functions ===================
void  GadoGadoFXAudioProcessor::updateFftSize()
{
    fftSize = (int)paramFftSize.getTargetValue(); //baca nilai di combo box fftsize
    fft = std::make_unique<juce::dsp::FFT>(log2(fftSize)); // -> fftsize=2^x
    
    inputBufferLength = fftSize;
    inputBufferWritePosition = 0;
    inputBuffer.clear();
    inputBuffer.setSize(getTotalNumInputChannels(), inputBufferLength);

    float maxRatio = powf(2.0f, paramShift.minValue / 12.0f); // rasio panjang sinyal asli : sinyal yg distretch
    outputBufferLength = (int)floorf((float)fftSize / maxRatio); // menentukan panjang max buffer yg diperlukan untuk menampung calon output
    outputBufferWritePosition = 0; 
    outputBufferReadPosition = 0;
    outputBuffer.clear();
    outputBuffer.setSize(getTotalNumInputChannels(), outputBufferLength);

    fftWindow.realloc(fftSize);
    fftWindow.clear(fftSize);

    fftTimeDomain.realloc(fftSize);
    fftTimeDomain.clear(fftSize);

    fftFrequencyDomain.realloc(fftSize);
    fftFrequencyDomain.clear(fftSize);

    samplesSinceLastFFT = 0;

    //======================================

    omega.realloc(fftSize);
    for (int index = 0; index < fftSize; ++index)
        omega[index] = 2.0f * M_PI * index / (float)fftSize;

    inputPhase.clear();
    inputPhase.setSize(getTotalNumInputChannels(), outputBufferLength);

    outputPhase.clear();
    outputPhase.setSize(getTotalNumInputChannels(), outputBufferLength);
}

void  GadoGadoFXAudioProcessor::updateHopSize()
{
    overlap = (int)paramHopSize.getTargetValue(); //faktor hop size = baca nilai di combo box hop size
    if (overlap != 0) 
    {
        hopSize = fftSize / overlap; //menentukan jmlh komponen sinyal yg bersinggungan dengan sample yg tlh diwindow
        outputBufferWritePosition = hopSize % outputBufferLength; // still confused about how this statement works
    }
}

void  GadoGadoFXAudioProcessor::updateAnalysisWindow()
{
    updateWindow(fftWindow, fftSize);
    //============================ Nothing special ==================================//
}

void  GadoGadoFXAudioProcessor::updateWindow(const juce::HeapBlock<float>& window, const int windowLength)
{
    switch ((int)paramWindowType.getTargetValue()) {
    case windowTypeBartlett: {
        for (int sample = 0; sample < windowLength; ++sample)
            window[sample] = 1.0f - fabs(2.0f * (float)sample / (float)(windowLength - 1) - 1.0f); //lihat buku joshua untuk rumus
        break;
    }
    case windowTypeHann: {
        for (int sample = 0; sample < windowLength; ++sample)
            window[sample] = 0.5f - 0.5f * cosf(2.0f * M_PI * (float)sample / (float)(windowLength - 1)); //lihat buku joshua untuk rumus
        break;
    }
    case windowTypeHamming: {
        for (int sample = 0; sample < windowLength; ++sample)
            window[sample] = 0.54f - 0.46f * cosf(2.0f * M_PI * (float)sample / (float)(windowLength - 1)); //lihat buku joshua rumus
        break;
    }
                          //========================= tambahkan tipe yang lain ==================================//
    }
    //============================================================= nothing special ==========================================================//
}

void  GadoGadoFXAudioProcessor::updateWindowScaleFactor()
{
    float windowSum = 0.0f;
    for (int sample = 0; sample < fftSize; ++sample)
        windowSum += fftWindow[sample]; // jumlah dari semua sampel fft yang diwindowing (jadi satu window) -> tidak dipecah menjadi bbrp window

    windowScaleFactor = 0.0f;
    if (overlap != 0 && windowSum != 0.0f)
        windowScaleFactor = 1.0f / (float)overlap / windowSum * (float)fftSize; // still dunno
}

float GadoGadoFXAudioProcessor::princArg(const float phase)
{
    if (phase >= 0.0f) return fmod(phase + M_PI, 2.0f * M_PI) - M_PI;
    else return fmod(phase + M_PI, -2.0f * M_PI) + M_PI;
    // no clues for those 2 conditions but the others just normal equations from Zolzer's DAFX book
}

void GadoGadoFXAudioProcessor::updateFilters()
{
    double discreteFrequency = 2.0 * M_PI * (double)paramFrequency.getTargetValue() / getSampleRate();
    double qFactor = (double)paramQfactor.getTargetValue();
    double gain = pow(10.0, (double)paramGain.getTargetValue() * 0.05);
    int type = (int)paramFilterType.getTargetValue();
    double fs = getSampleRate();
    for (int i = 0; i < filters.size(); ++i)
        filters[i]->updateCoefficients(discreteFrequency, qFactor, gain, type, fs);

}