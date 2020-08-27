#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


class GadoGadoFXAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer
                                             
{
public:
    GadoGadoFXAudioProcessorEditor (GadoGadoFXAudioProcessor&);
    ~GadoGadoFXAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    enum RadioButtonIds
    {
        PemilihMode = 1001
    };


private:
    //======================================= all about parameters =========================================
    GadoGadoFXAudioProcessor& audioProcessor;

    juce::OwnedArray<juce::Label> labels;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::ToggleButton> toggles;
    juce::OwnedArray<juce::ComboBox> comboBoxes;

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    juce::OwnedArray<SliderAttachment> sliderAttachments;
    juce::OwnedArray<ButtonAttachment> toggleButtonAttachments;
    juce::OwnedArray<ComboBoxAttachment> comboBoxAttachments;

  //======================================= GUI Components =======================================================
    juce::Slider G_GainSlider; 
    juce::ToggleButton G_ToselGainControl;

    juce::Slider D_BalanceSlider;
    juce::Slider D_DelayTimeSlider; 
    juce::Slider D_FeedbackSlider; 
    juce::Slider D_MixSlider; 
    juce::ToggleButton D_ToselDelayControl;

    juce::Slider EQ_FrequencySlider;
    juce::Slider EQ_QFactorSlider;
    juce::Slider EQ_GainSlider;
    juce::ComboBox EQ_FilterTypeComboBox;
    juce::ToggleButton EQ_ToselEQ;

    juce::Slider PS_Shift;
    juce::ComboBox PS_FFTSize;
    juce::ComboBox PS_HopSize;
    juce::ComboBox PS_WindowType;
    juce::Label PS_BandwidthLabel;
    juce::ToggleButton PS_ToselPS;
    
    //====================================Functions========================================================
    void timerCallback() override;
    void updateUIcomponents();

    void MakeToggleButton(juce::ToggleButton& togglebutton, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter);
    void MakeSlider(juce::Slider& slider, const juce::Slider::SliderStyle& sliderstyle, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter);
    void MakeComboBox(juce::ComboBox &combobox, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter,int no_combobox, int default_value);
    
    void initScreen();
    void main_GainControl(const juce::Array<juce::AudioProcessorParameter*> parameters_saya);
    void main_Delay(const juce::Array<juce::AudioProcessorParameter*> parameters_saya);
    void main_PitchShiftMode(const juce::Array<juce::AudioProcessorParameter*> parameters_saya);
    void main_ParameterEQ(const juce::Array<juce::AudioProcessorParameter*> parameters_saya);

    //=============================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GadoGadoFXAudioProcessorEditor)
};
