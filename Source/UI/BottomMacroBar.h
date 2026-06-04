#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"
#include "OscilloscopeComponent.h"
#include "CustomLookAndFeel.h"

/**
 * @brief Нижняя панель управления макро-параметрами.
 * Содержит глобальный слайдер "Vibe Macro" (Era Morpher),
 * слайдер "Master Volume" и визуальный осциллограф по центру.
 */
class BottomMacroBar : public juce::Component
{
public:
    BottomMacroBar(LofiChiptuneSynthAudioProcessor& processor);
    ~BottomMacroBar() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    LofiChiptuneSynthAudioProcessor& processor;
    CustomKnobLookAndFeel customKnobLookAndFeel;

    // Слайдеры управления
    juce::Slider vibeSlider;
    juce::Slider volumeSlider;

    // Текстовые подписи
    juce::Label vibeLabel;
    juce::Label volumeLabel;

    // Осциллограф реального времени
    OscilloscopeComponent oscilloscope;

    // Вложения параметров APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> vibeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomMacroBar)
};
