/*
  ==============================================================================

    Code by Juan Gil <https://juangil.com/>.
    Copyright (C) 2017-2019 Juan Gil.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <https://www.gnu.org/licenses/>.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
using Parameter = juce::AudioProcessorValueTreeState::Parameter;

//==============================================================================

class PluginParametersManager
{
public:
    PluginParametersManager(juce::AudioProcessor& p) : valueTreeState(p, nullptr)
    {
    }

    juce::AudioProcessorValueTreeState valueTreeState;
    juce::StringArray parameterTypes;
    juce::Array<juce::StringArray> comboBoxItemLists;
};

//==============================================================================

class PluginParameter
    : public juce::LinearSmoothedValue<float>
    , public juce::AudioProcessorValueTreeState::Listener
{
protected:
    PluginParameter(PluginParametersManager& parametersManager,
        const std::function<float(float)> callback = nullptr)
        : parametersManager(parametersManager)
        , callback(callback)
    {
    }

public:
    void updateValue(float value)
    {
        if (callback != nullptr)
            setCurrentAndTargetValue(callback(value));
        else
            setCurrentAndTargetValue(value);
    }

    void parameterChanged(const juce::String& parameterID, float newValue) override
    {
        updateValue(newValue);
    }

    PluginParametersManager& parametersManager;
    std::function<float(float)> callback;
    juce::String paramID;
};

//==============================================================================

class PluginParameterSlider : public PluginParameter
{
protected:
    PluginParameterSlider(PluginParametersManager& parametersManager,
        const juce::String& paramName,
        const juce::String& labelText,
        const float minValue,
        const float maxValue,
        const float defaultValue,
        const std::function<float(float)> callback,
        const bool logarithmic)
        : PluginParameter(parametersManager, callback)
        , paramName(paramName)
        , labelText(labelText)
        , minValue(minValue)
        , maxValue(maxValue)
        , defaultValue(defaultValue)
    {
        paramID = paramName.removeCharacters(" ").toLowerCase();
        parametersManager.parameterTypes.add("Slider");

        juce::NormalisableRange<float> range(minValue, maxValue);
        if (logarithmic)
            range.setSkewForCentre(sqrt(minValue * maxValue));

        parametersManager.valueTreeState.createAndAddParameter(std::make_unique<Parameter>
            (paramID, paramName, labelText, range, defaultValue,
                [](float value) { return juce::String(value, 2); },
                [](const juce::String& text) { return text.getFloatValue(); })
        );

        parametersManager.valueTreeState.addParameterListener(paramID, this);
        updateValue(defaultValue);
    }

public:
    const juce::String& paramName;
    const juce::String& labelText;
    const float minValue;
    const float maxValue;
    const float defaultValue;
};

//======================================

class PluginParameterLinSlider : public PluginParameterSlider
{
public:
    PluginParameterLinSlider(PluginParametersManager& parametersManager,
                                const juce::String& paramName,
                                const juce::String& labelText,
                                const float minValue,
                                const float maxValue,
                                const float defaultValue,
                                const 
                                const std::function<float(float)> callback = nullptr)
        : PluginParameterSlider(parametersManager,
                                paramName,
                                labelText,
                                minValue,
                                maxValue,
                                defaultValue,
                                callback,
                                false)
    {
    }
};

//======================================

class PluginParameterLogSlider : public PluginParameterSlider
{
public:
    PluginParameterLogSlider(PluginParametersManager& parametersManager,
        const juce::String& paramName,
        const juce::String& labelText,
        const float minValue,
        const float maxValue,
        const float defaultValue,
        const std::function<float(float)> callback = nullptr)
        : PluginParameterSlider(parametersManager,
            paramName,
            labelText,
            minValue,
            maxValue,
            defaultValue,
            callback,
            true)
    {
    }
};

//==============================================================================

class PluginParameterToggle : public PluginParameter
{
public:
    PluginParameterToggle(PluginParametersManager& parametersManager,
        const juce::String& paramName,
        const bool defaultState = false,
        const std::function<float(float)> callback = nullptr)
        : PluginParameter(parametersManager, callback)
        , paramName(paramName)
        , defaultState(defaultState)
    {
        paramID = paramName.removeCharacters(" ").toLowerCase();
        parametersManager.parameterTypes.add("ToggleButton");

        const juce::StringArray toggleStates = { "False", "True" };
        juce::NormalisableRange<float> range(0.0f, 1.0f, 1.0f);

        parametersManager.valueTreeState.createAndAddParameter(std::make_unique<Parameter>
            (paramID, paramName, "", range, (float)defaultState,
                [toggleStates](float value) { return toggleStates[(int)value]; },
                [toggleStates](const juce::String& text) { return toggleStates.indexOf(text); })
        );

        parametersManager.valueTreeState.addParameterListener(paramID, this);
        updateValue((float)defaultState);
    }

    const juce::String& paramName;
    const bool defaultState;
};

//==============================================================================

class PluginParameterComboBox : public PluginParameter
{
public:
    PluginParameterComboBox(PluginParametersManager& parametersManager,
                            const juce::String& paramName,
                            const juce::StringArray items,
                            const int defaultChoice = 0,
                            const std::function<float(const float)> callback = nullptr)
        : PluginParameter(parametersManager, callback)
        , paramName(paramName)
        , items(items)
        , defaultChoice(defaultChoice)
    {
        paramID = paramName.removeCharacters(" ").toLowerCase();
        parametersManager.parameterTypes.add("ComboBox");

        parametersManager.comboBoxItemLists.add(items);
       juce::NormalisableRange<float> range(0.0f, (float)items.size() - 1.0f, 1.0f);

        parametersManager.valueTreeState.createAndAddParameter(std::make_unique<Parameter>
            (paramID, paramName, "", range, (float)defaultChoice,
                [items](float value) { return items[(int)value]; },
                [items](const juce::String& text) { return items.indexOf(text); })
        );

        parametersManager.valueTreeState.addParameterListener(paramID, this);
        updateValue((float)defaultChoice);
    }

    const juce::String& paramName;
    const juce::StringArray items;
    const int defaultChoice;
};

//==============================================================================
