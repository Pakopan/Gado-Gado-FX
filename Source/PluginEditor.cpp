#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GadoGadoFXAudioProcessorEditor::GadoGadoFXAudioProcessorEditor (GadoGadoFXAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
   initScreen();
   main_GainControl();
   main_Delay();
   On_Off_Button();
}



void GadoGadoFXAudioProcessorEditor::initScreen() {
    
    setSize(515, 250);
}

//-------------------------------------start dari main effect---------------------------------------------------------
void GadoGadoFXAudioProcessorEditor::main_Delay() {
    const juce::Array<juce::AudioProcessorParameter*> parameters = audioProcessor.getParameters();
    MakeSlider(DelayTimeSlider, juce::Slider::SliderStyle::Rotary, parameters, 0);
    MakeSlider(FeedbackSlider, juce::Slider::SliderStyle::Rotary, parameters, 1);
    MakeSlider(MixSlider, juce::Slider::SliderStyle::Rotary, parameters, 2);
    
}

void GadoGadoFXAudioProcessorEditor::main_GainControl() {
    const juce::Array<juce::AudioProcessorParameter*> parameters = audioProcessor.getParameters();
    MakeSlider(gainSlider, juce::Slider::SliderStyle::Rotary, parameters, 3);
}

//----------------------------------------end dari main effect------------------------------------------------------

void GadoGadoFXAudioProcessorEditor::On_Off_Button() {
    const juce::Array<juce::AudioProcessorParameter*> parameters = audioProcessor.getParameters();
    MakeToggleButton(toselGainControl, parameters, 4);
    MakeToggleButton(toselDelay, parameters, 5);

    toselGainControl.setRadioGroupId(PemilihMode);
    toselDelay.setRadioGroupId(PemilihMode);
}


//--------------------------------------Start fungsi untuk bikin komponen --------------------------------------
void GadoGadoFXAudioProcessorEditor::MakeSlider(juce::Slider& slider, const juce::Slider::SliderStyle& sliderstyle
    , const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter)
{   
    
    const juce::AudioProcessorParameterWithID* parameter = dynamic_cast<juce::AudioProcessorParameterWithID*> (parameters[no_parameter]);
    SliderAttachment* aSliderAttachment;
    sliderAttachments.add(aSliderAttachment = new SliderAttachment(audioProcessor.parameters.valueTreeState, parameter->paramID, slider));
    slider.setSliderStyle(sliderstyle);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 25);
    addAndMakeVisible(slider);

    juce::Label* aLabel;
    labels.add(aLabel = new juce::Label(parameter->name, parameter->name));
    aLabel->attachToComponent(&slider, false);
    //aLabel->setBounds(koordinat[0], koordinat[1], koordinat[2], koordinat[3]);
    addAndMakeVisible(aLabel);
   
}


void GadoGadoFXAudioProcessorEditor::MakeToggleButton(juce::ToggleButton& togglebutton, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter) 
{
    const juce::AudioProcessorParameterWithID* parameter = dynamic_cast<juce::AudioProcessorParameterWithID*> (parameters[no_parameter]);
    togglebutton.setToggleState(parameter->getDefaultValue(), juce::dontSendNotification);
    ButtonAttachment* aToggleButtonAttachment;
    toggleButtonAttachments.add(aToggleButtonAttachment = new ButtonAttachment(audioProcessor.parameters.valueTreeState, parameter->paramID, togglebutton));
    
    juce::Label* aLabel;
    labels.add(aLabel = new juce::Label(parameter->name, parameter->name));
    aLabel->attachToComponent(&togglebutton, true);
    aLabel->setJustificationType(juce::Justification::centred);
    //togglebutton.changeWidthToFitText();
    addAndMakeVisible(togglebutton);
    addAndMakeVisible(aLabel);
}

//----------------------------------------end fungsi untuk bikin komponen-----------------------------------------------------




//==============================================================================
void GadoGadoFXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setFont(16.0f);
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    //g.drawText("Gain Control", 0, 20, 150, 20, juce::Justification::centred, true);
    g.drawText("dB", 0, 165, 150, 20, juce::Justification::centred, true); 
}

void GadoGadoFXAudioProcessorEditor::resized()
{

    gainSlider.setBounds(20, 50, 115, 115);
    toselGainControl.setBounds(50, 0, 20, 20);

   DelayTimeSlider.setBounds(155, 50, 80, 80);
    FeedbackSlider.setBounds(275, 50, 80, 80);
    MixSlider.setBounds(395, 50, 80, 80);
    toselDelay.setBounds(315, 0, 20, 20);    

}

GadoGadoFXAudioProcessorEditor::~GadoGadoFXAudioProcessorEditor()
{
}