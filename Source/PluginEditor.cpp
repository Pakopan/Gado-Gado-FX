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
   main_ParameterEQ();
   main_PitchShift();
   startTimer(50);
}



void GadoGadoFXAudioProcessorEditor::initScreen() {
    
    setSize(900, 450);
    
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
    MakeComboBox(FilterTypeComboBox, parameters, 9, 0);
    addAndMakeVisible(bandwidthLabel);
}

void GadoGadoFXAudioProcessorEditor::main_PitchShift()
{
    const juce::Array<juce::AudioProcessorParameter*> parameters = audioProcessor.getParameters();
    MakeSlider(PS_Shift, juce::Slider::SliderStyle::Rotary, parameters, 11);
    MakeComboBox(PS_FFTSize, parameters, 12,1);
    MakeComboBox(PS_HopSize, parameters, 13,2);
    MakeComboBox(PS_WindowType, parameters, 14,3);
}

//----------------------------------------end dari main effect------------------------------------------------------

void GadoGadoFXAudioProcessorEditor::On_Off_Button() {
    const juce::Array<juce::AudioProcessorParameter*> parameters = audioProcessor.getParameters();
    MakeToggleButton(toselGainControl, parameters, 4);
    MakeToggleButton(toselDelay, parameters, 5);
    MakeToggleButton(toselEQ, parameters, 10);
    MakeToggleButton(toselPS, parameters, 15);
    toselPS.setRadioGroupId(PemilihMode);
    toselEQ.setRadioGroupId(PemilihMode);
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
    slider.setTextValueSuffix(parameter->label);
    slider.setSliderStyle(sliderstyle);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 125, 20);
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


void GadoGadoFXAudioProcessorEditor::MakeComboBox(juce::ComboBox& combobox, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter, int no_combobox)
{

    const juce::AudioProcessorParameterWithID* parameter = dynamic_cast<juce::AudioProcessorParameterWithID*> (parameters[no_parameter]);
    ComboBoxAttachment* aComboBoxAttachment;
    comboBoxAttachments.add(aComboBoxAttachment = new ComboBoxAttachment(audioProcessor.parameters.valueTreeState, parameter->paramID, combobox));  
    combobox.setEditableText(false);
    combobox.setJustificationType(juce::Justification::left);
    combobox.addItemList(audioProcessor.parameters.comboBoxItemLists[no_combobox], 1);
    combobox.setSelectedId(1, true);
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
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  //  g.drawText("dB", 0, 165, 150, 20, juce::Justification::centred, true); 
}

void GadoGadoFXAudioProcessorEditor::resized()
{

    gainSlider.setBounds(20, 50, 115, 115);
    toselGainControl.setBounds(50, 0, 25, 20);

   DelayTimeSlider.setBounds(155, 50, 80, 80);
    FeedbackSlider.setBounds(275, 50, 80, 80);
    MixSlider.setBounds(395, 50, 80, 80);
    toselDelay.setBounds(315, 0, 25, 20);    

    FrequencySlider.setBounds(20, 250, 80,80);
    QFactorSlider.setBounds(155, 250, 80, 80);
    EQ_GainSlider.setBounds(275, 250, 80, 80);
    FilterTypeComboBox.setBounds(100, 400, 250, 30);

    bandwidthLabel.setBounds(250, 370, 250, 20);
    toselEQ.setBounds(200, 370, 25, 20);

    PS_Shift.setBounds(600, 50, 110, 80);
    PS_FFTSize.setBounds(600,150,250,30);
    PS_HopSize.setBounds(600, 250 , 250, 30);
    PS_WindowType.setBounds(600, 300, 250, 30);
    toselPS.setBounds(600, 400, 80,20);
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
        QFactorSlider.setEnabled(false);
        bandwidthLabel.setVisible(false);
    }
    else {
        QFactorSlider.setEnabled(true);
        bandwidthLabel.setVisible(true);
    }

    if (filterTypeDoesNotHaveGain)
        EQ_GainSlider.setEnabled(false);
    else
        EQ_GainSlider.setEnabled(true);
}

GadoGadoFXAudioProcessorEditor::~GadoGadoFXAudioProcessorEditor()
{
}