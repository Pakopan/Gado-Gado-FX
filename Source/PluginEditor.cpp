#include "PluginProcessor.h"
#include "PluginEditor.h"
//======================================= Built in functions ===========================================
GadoGadoFXAudioProcessorEditor::GadoGadoFXAudioProcessorEditor (GadoGadoFXAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
   const juce::Array<juce::AudioProcessorParameter*> parameters = audioProcessor.getParameters();

   initScreen();
   main_GainControl(parameters);
   main_Delay(parameters);
   main_ParameterEQ(parameters);
   main_PitchShiftMode(parameters);

   startTimer(50);
}

void GadoGadoFXAudioProcessorEditor::paint(juce::Graphics& g)
{

    g.setFont(16.0f);
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(juce::Colours::slategrey);
    
    juce::Line<float> line_1(juce::Point<float>(0, 215),
                             juce::Point<float>(500, 215));
    juce::Line<float> line_2(juce::Point<float>(500, 0),
                             juce::Point<float>(500, 450));
    juce::Line<float> line_3(juce::Point<float>(180, 0),
                             juce::Point<float>(180, 215));

    g.drawLine(line_1, 3.0f);
    g.drawLine(line_2, 3.0f);
    g.drawLine(line_3, 3.0f);

    juce::Image logo_imfi= juce::ImageCache::getFromMemory(BinaryData::logo_imfi_png, BinaryData::logo_imfi_pngSize);
    juce::Image logo_gado = juce::ImageCache::getFromMemory(BinaryData::gado_png, BinaryData::gado_pngSize);
    g.drawImageAt(logo_imfi, 560, 400);
    g.drawImageAt(logo_gado, 560, 350);
   
}

void GadoGadoFXAudioProcessorEditor::resized()
{

    G_GainSlider.setBounds(30, 75, 125, 125);
    G_ToselGainControl.setBounds(145, 10, 25, 20);

    D_BalanceSlider.setBounds(200, 60, 120, 50);
    D_DelayTimeSlider.setBounds(360, 60, 120, 50);
    D_FeedbackSlider.setBounds(200, 150, 120, 50);
    D_MixSlider.setBounds(360, 150, 120, 50);
    D_ToselDelayControl.setBounds(400, 10, 25, 20);

    EQ_FrequencySlider.setBounds(30, 280, 97  , 97);
    EQ_QFactorSlider.setBounds(205, 280, 97, 97);
    EQ_GainSlider.setBounds(380, 280, 97, 97);
    EQ_FilterTypeComboBox.setBounds(90, 400, 250, 30);

    PS_BandwidthLabel.setBounds(345, 405, 250, 20);
    EQ_ToselEQ.setBounds(300, 225, 25, 20);

    PS_Shift.setBounds(600, 80, 250, 60);
    PS_FFTSize.setBounds(600, 180, 250, 30);
    PS_HopSize.setBounds(600, 225, 250, 30);
    PS_WindowType.setBounds(600, 270, 250, 30);
    PS_ToselPS.setBounds(763, 10, 80, 20);
}

GadoGadoFXAudioProcessorEditor::~GadoGadoFXAudioProcessorEditor()
{
}

//==================================== End of Built in functions =========================================


//======================================================== start dari main effect ================================================================
void GadoGadoFXAudioProcessorEditor::initScreen() {

    setSize(880, 450);

}

void GadoGadoFXAudioProcessorEditor::main_Delay(const juce::Array<juce::AudioProcessorParameter*> parameters_saya) {
    MakeSlider(D_BalanceSlider, juce::Slider::SliderStyle::LinearHorizontal, parameters_saya, 0);
    MakeSlider(D_DelayTimeSlider, juce::Slider::SliderStyle::LinearHorizontal, parameters_saya, 1);
    MakeSlider(D_FeedbackSlider, juce::Slider::SliderStyle::LinearHorizontal, parameters_saya, 2);
    MakeSlider(D_MixSlider, juce::Slider::SliderStyle::LinearHorizontal, parameters_saya, 3);
    MakeToggleButton(D_ToselDelayControl, parameters_saya, 4);
    D_ToselDelayControl.setRadioGroupId(PemilihMode);
}

void GadoGadoFXAudioProcessorEditor::main_GainControl(const juce::Array<juce::AudioProcessorParameter*> parameters_saya) {
    MakeSlider(G_GainSlider, juce::Slider::SliderStyle::Rotary, parameters_saya, 5);
    MakeToggleButton(G_ToselGainControl, parameters_saya, 6);
    G_ToselGainControl.setRadioGroupId(PemilihMode);
}

void GadoGadoFXAudioProcessorEditor::main_ParameterEQ(const juce::Array<juce::AudioProcessorParameter*> parameters_saya)
{
    MakeSlider(EQ_FrequencySlider, juce::Slider::SliderStyle::Rotary, parameters_saya, 7);
    MakeSlider(EQ_QFactorSlider, juce::Slider::SliderStyle::Rotary, parameters_saya, 8);
    MakeSlider(EQ_GainSlider, juce::Slider::SliderStyle::Rotary, parameters_saya, 9);
    MakeComboBox(EQ_FilterTypeComboBox, parameters_saya, 10, 0, audioProcessor.filterTypePeakingNotch+1);
    MakeToggleButton(EQ_ToselEQ, parameters_saya, 11);
    EQ_ToselEQ.setRadioGroupId(PemilihMode);
    addAndMakeVisible(PS_BandwidthLabel);
}

void GadoGadoFXAudioProcessorEditor::main_PitchShiftMode(const juce::Array<juce::AudioProcessorParameter*> parameters_saya)
{
    MakeSlider(PS_Shift, juce::Slider::SliderStyle::LinearHorizontal, parameters_saya, 12);                                             //Typenya diuubah
    MakeComboBox(PS_FFTSize, parameters_saya, 13, 1, audioProcessor.fftSize1024+1);
    MakeComboBox(PS_HopSize, parameters_saya, 14, 2, audioProcessor.hopSize4+1);
    MakeComboBox(PS_WindowType, parameters_saya, 15, 3, audioProcessor.windowTypeBartlett+1);
    MakeToggleButton(PS_ToselPS, parameters_saya, 16);
    PS_ToselPS.setRadioGroupId(PemilihMode);
}
//============================================ End of main effect ========================================================


//======================================= Start fungsi untuk bikin komponen ==============================================
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
    addAndMakeVisible(togglebutton);

    juce::Label* aLabel;
    labels.add(aLabel = new juce::Label(parameter->name, parameter->name));
    aLabel->attachToComponent(&togglebutton, true);
    aLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(aLabel);
}

void GadoGadoFXAudioProcessorEditor::MakeComboBox(juce::ComboBox& combobox, const juce::Array<juce::AudioProcessorParameter*> parameters, int no_parameter, int no_combobox, int default_value)
{
    const juce::AudioProcessorParameterWithID* parameter = dynamic_cast<juce::AudioProcessorParameterWithID*> (parameters[no_parameter]);
    ComboBoxAttachment* aComboBoxAttachment;
    comboBoxAttachments.add(aComboBoxAttachment = new ComboBoxAttachment(audioProcessor.parameters.valueTreeState, parameter->paramID, combobox));  
    combobox.setEditableText(false);
    combobox.setJustificationType(juce::Justification::left);
    combobox.addItemList(audioProcessor.parameters.comboBoxItemLists[no_combobox], 1);
    combobox.setSelectedId(default_value);
    addAndMakeVisible(combobox);

    juce::Label* aLabel;
    labels.add(aLabel = new juce::Label(parameter->name, parameter->name));
    aLabel->attachToComponent(&combobox, true);
    aLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(aLabel); 
}
//==================================== end fungsi untuk bikin komponen ================================================


//==================================== Start of self built functions ======================================================
void GadoGadoFXAudioProcessorEditor::timerCallback()
{
    updateUIcomponents();
}

void GadoGadoFXAudioProcessorEditor::updateUIcomponents()
{
   juce::String bandwidthText = juce::String::formatted("Bandwidth: %.1fHz",
                                    audioProcessor.paramFrequency.getTargetValue() /
                                    audioProcessor.paramQfactor.getTargetValue());
                                                                   

    PS_BandwidthLabel.setText(bandwidthText, juce::dontSendNotification);

    //=============================================================================================

    bool filterTypeDoesNotHaveQfactor =
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeLowPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeHighPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeLowShelf ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeHighShelf;
    bool filterTypeDoesNotHaveGain =
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeLowPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeHighPass ||
        audioProcessor.paramFilterType.getTargetValue() == audioProcessor.filterTypeBandPass;
   
    if (filterTypeDoesNotHaveQfactor) {
        EQ_QFactorSlider.setEnabled(false);
        PS_BandwidthLabel.setVisible(false);
    }
    else {
        EQ_QFactorSlider.setEnabled(true);
        PS_BandwidthLabel.setVisible(true);
    }

    if (filterTypeDoesNotHaveGain)
        EQ_GainSlider.setEnabled(false);
    else
        EQ_GainSlider.setEnabled(true);
}
//==================================== End of self built functions =========================================================
