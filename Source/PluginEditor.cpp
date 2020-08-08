#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GadoGadoFXAudioProcessorEditor::GadoGadoFXAudioProcessorEditor (GadoGadoFXAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
   initScreen();
   main_GainControl();
   main_Delay();
   main_ParameterEQ();
   On_Off_Button();
}



void GadoGadoFXAudioProcessorEditor::initScreen() {
    
    setSize(515, 600);
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

void GadoGadoFXAudioProcessorEditor::main_ParameterEQ()
{
    const juce::Array<juce::AudioProcessorParameter*> parameters = audioProcessor.getParameters();
    MakeSlider(FrequencySlider, juce::Slider::SliderStyle::Rotary, parameters, 6);
    MakeSlider(QFactorSlider, juce::Slider::SliderStyle::Rotary, parameters, 7);
    MakeSlider(EQ_GainSlider, juce::Slider::SliderStyle::Rotary, parameters, 8);
    MakeComboBox(FilterTypeComboBox, parameters, 10, 0);
    addAndMakeVisible(&bandwidthLabel);
    //startTimer(50);
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
    addAndMakeVisible(togglebutton);
    addAndMakeVisible(aLabel);
}


void GadoGadoFXAudioProcessorEditor::MakeComboBox(juce::ComboBox& combobox, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter, int no_item_list)
{
    const juce::AudioProcessorParameterWithID* parameter = dynamic_cast<juce::AudioProcessorParameterWithID*> (parameters[no_parameter]);
  
    combobox.setEditableText(false);
    combobox.setJustificationType(juce::Justification::left);
    //combobox.addItemList(audioProcessor.parameters.comboBoxItemLists[2], 1);
   // combobox.addItemList(audioProcessor.filterTypeItemsUI[1], 1);
    ComboBoxAttachment* aComboBoxAttachment;
    comboBoxAttachments.add(aComboBoxAttachment = new ComboBoxAttachment(audioProcessor.parameters.valueTreeState, parameter->paramID, combobox));

    juce::Label* aLabel;
    labels.add(aLabel = new juce::Label(parameter->name, parameter->name));
    aLabel->attachToComponent(&combobox, true);
    aLabel->setJustificationType(juce::Justification::centred);

    addAndMakeVisible(combobox);
    addAndMakeVisible(aLabel);
}


//----------------------------------------end fungsi untuk bikin komponen-----------------------------------------------------




//==============================================================================
void GadoGadoFXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.setFont(16.0f);
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
   // g.drawText("dB", 0, 165, 150, 20, juce::Justification::centred, true); 
    //juce::StringArray baru = audioProcessor.parameters.comboBoxItemLists[0];
    //juce::String anyar = baru[0];
   // g.drawText(anyar, 0, 165, 150, 20, juce::Justification::centred, true);
}

void GadoGadoFXAudioProcessorEditor::resized()
{

    gainSlider.setBounds(20, 50, 115, 115);
    toselGainControl.setBounds(50, 0, 20, 20);

   DelayTimeSlider.setBounds(155, 50, 80, 80);
    FeedbackSlider.setBounds(275, 50, 80, 80);
    MixSlider.setBounds(395, 50, 80, 80);
    toselDelay.setBounds(315, 0, 20, 20);    

    FrequencySlider.setBounds(20, 250, 80,80);
    QFactorSlider.setBounds(155, 250, 80, 80);
    EQ_GainSlider.setBounds(275, 250, 80, 80);
    FilterTypeComboBox.setBounds(100, 400, 250, 30);

    //bandwidthLabel.setBounds();

}


void GadoGadoFXAudioProcessorEditor::timerCallback()
{
    updateUIcomponents();
}


void GadoGadoFXAudioProcessorEditor::updateUIcomponents()
{
    juce::String bandwidthText = juce::String::formatted("Bandwidth: %.1fHz",
                                    audioProcessor.paramFrequency.getTargetValue() /
                                    audioProcessor.paramQfactor.getTargetValue());

    bandwidthLabel.setText(bandwidthText, juce::dontSendNotification);

    //======================================

    bool filterTypeDoesNotHaveQfactor =
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeLowPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeHighPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeLowShelf ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeHighShelf;
    bool filterTypeDoesNotHaveGain =
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeLowPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeHighPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeBandPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeBandStop;

    if (filterTypeDoesNotHaveQfactor) {
        findChildWithID(audioProcessor.paramQfactor.paramID)->setEnabled(false);
        bandwidthLabel.setVisible(false);
    }
    else {
        findChildWithID(audioProcessor.paramQfactor.paramID)->setEnabled(true);
        bandwidthLabel.setVisible(true);
    }

    if (filterTypeDoesNotHaveGain)
        findChildWithID(audioProcessor.paramGain.paramID)->setEnabled(false);
    else
        findChildWithID(audioProcessor.paramGain.paramID)->setEnabled(true);
}

GadoGadoFXAudioProcessorEditor::~GadoGadoFXAudioProcessorEditor()
{
}