#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"
#include "CustomLookAndFeel.h"

/**
 * @brief Вкладка управления эффектами деградации (FX).
 * Содержит элементы управления Bitcrusher и анимированные индикаторы
 * параметров Tape Emulation (Wow, Saturation, Hiss), привязанные к Vibe Macro.
 * 
 * Включает в себя таймер для анимации вращения кассетных бобин.
 */
class FxWindow : public juce::Component, public juce::Timer
{
public:
    FxWindow(LofiChiptuneSynthAudioProcessor& processor);
    ~FxWindow() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    // Метод таймера для анимации
    void timerCallback() override;

private:
    LofiChiptuneSynthAudioProcessor& processor;

    // Разделы интерфейса
    juce::Label bitcrusherTitle{"", "BITCRUSHER"};
    juce::Label tapeTitle{"", "TAPE EMULATION"};

    // Элементы управления Bitcrusher (привязаны к APVTS)
    juce::Slider bitDepthSlider;
    juce::Label bitDepthLabel;

    juce::Slider sampleRateSlider;
    juce::Label sampleRateLabel;

    juce::Slider saturationSlider;
    juce::Label saturationLabel;

    // Вложения параметров APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bitDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sampleRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> saturationAttachment;

    // Переменные анимации кассетных бобин
    float leftReelAngle = 0.0f;
    float rightReelAngle = 0.0f;

    CustomKnobLookAndFeel customKnobLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FxWindow)
};
