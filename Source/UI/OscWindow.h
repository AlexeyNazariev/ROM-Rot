#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"
#include "CustomLookAndFeel.h"

/**
 * @brief Вкладка управления осциллятором (OSC) и огибающей (ADSR).
 */
class OscWindow : public juce::Component
{
public:
    OscWindow(LofiChiptuneSynthAudioProcessor& processor);
    ~OscWindow() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    /**
     * @brief Вспомогательный метод инициализации роторных (круговых) слайдеров.
     */
    void setupRotarySlider(juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour colour);

    /**
     * @brief Вспомогательный метод инициализации вертикальных слайдеров огибающей.
     */
    void setupVerticalSlider(juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour colour);

    LofiChiptuneSynthAudioProcessor& processor;
    CustomKnobLookAndFeel customKnobLookAndFeel;

    // Выбор формы волны
    juce::Label waveLabel{"", "OSCILLATOR WAVE"};
    juce::ComboBox waveSelector;

    // Слайдеры параметров осциллятора
    juce::Slider detuneSlider;
    juce::Label detuneLabel;

    juce::Slider pwSlider;
    juce::Label pwLabel;

    juce::Slider pwmRateSlider;
    juce::Label pwmRateLabel;

    juce::Slider pwmDepthSlider;
    juce::Label pwmDepthLabel;

    juce::Slider subGainSlider;
    juce::Label subGainLabel;

    juce::Slider noiseGainSlider;
    juce::Label noiseGainLabel;

    // Слайдеры ADSR (Вертикальные)
    juce::Slider attackSlider;
    juce::Label attackLabel;
    juce::Slider decaySlider;
    juce::Label decayLabel;
    juce::Slider sustainSlider;
    juce::Label sustainLabel;
    juce::Slider releaseSlider;
    juce::Label releaseLabel;

    // Вложения параметров APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> waveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> detuneAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pwAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pwmRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pwmDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> subGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> noiseGainAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> decayAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sustainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscWindow)
};
