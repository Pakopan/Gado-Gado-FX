#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


class GadoGadoFXAudioProcessorEditor : public juce::AudioProcessorEditor                                          
                                             
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
    
    void On_Off_Button();
    void main_GainControl();
    void initScreen();
    void main_Delay();
    void MakeToggleButton(juce::ToggleButton& togglebutton, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter);
    void MakeSlider(juce::Slider& slider, const juce::Slider::SliderStyle& sliderstyle, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter, const juce::Array <int> koordinat);
    //void GadoGadoFXAudioProcessorEditor::New_make_slider(const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter );

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    GadoGadoFXAudioProcessor& audioProcessor; 
    juce::OwnedArray<SliderAttachment> sliderAttachments;
    juce::OwnedArray<ButtonAttachment> toggleButtonAttachments;
    juce::OwnedArray<juce::Label> labels;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::ToggleButton> toggles;
    juce::Slider gainSlider; const juce::Array <int> koor_gain_slider = { 20, 35, 80, 20 };
    juce::ToggleButton toselGainControl;

    juce::Array<juce::Label> label_ku[5];



    juce::Slider DelayTimeSlider; const juce::Array <int> koor_delay_time_slider={ 120, 35, 80, 20 };
    juce::Slider FeedbackSlider; const juce::Array <int> koor_feedback_slider = { 220, 35, 80, 20 };
    juce::Slider MixSlider; const juce::Array <int> koor_mix_slider = { 320, 35, 150, 20 };
    juce::ToggleButton toselDelay;


    //=============================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GadoGadoFXAudioProcessorEditor)
};
