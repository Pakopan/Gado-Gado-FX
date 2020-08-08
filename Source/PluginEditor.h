#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"


class GadoGadoFXAudioProcessorEditor : public juce::AudioProcessorEditor//, private juce::Timer
                                             
{
public:
    GadoGadoFXAudioProcessorEditor (GadoGadoFXAudioProcessor&);
    ~GadoGadoFXAudioProcessorEditor() override;

    //==============================================================================
 //   void drawLabel(juce::Graphics& g, juce::Label& label) override;
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
    void main_ParameterEQ();
    void MakeToggleButton(juce::ToggleButton& togglebutton, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter);
    void MakeSlider(juce::Slider& slider, const juce::Slider::SliderStyle& sliderstyle, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter);
    void MakeComboBox(juce::ComboBox& combobox, juce::Array<juce::StringArray> comboBoxItemLists, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter);
    

    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef juce::AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;
    GadoGadoFXAudioProcessor& audioProcessor; 

    juce::OwnedArray<SliderAttachment> sliderAttachments;
    juce::OwnedArray<ButtonAttachment> toggleButtonAttachments;
    juce::OwnedArray<ComboBoxAttachment> comboBoxAttachments;

    juce::OwnedArray<juce::Label> labels;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::ToggleButton> toggles;
    juce::OwnedArray<juce::ComboBox> comboBoxes;
    
    juce::Slider gainSlider; 
    juce::ToggleButton toselGainControl;
    juce::Label label_gain;

    juce::Slider DelayTimeSlider; 
    juce::Slider FeedbackSlider; 
    juce::Slider MixSlider; 
    juce::ToggleButton toselDelay;

   // void timerCallback() override;
   // void updateUIcomponents();
    juce::Label bandwidthLabel;

    juce::Slider FrequencySlider;
    juce::Slider QFactorSlider;
    juce::Slider EQ_GainSlider;
    juce::ComboBox FilterTypeComboBox;

    //=============================================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GadoGadoFXAudioProcessorEditor)
};
