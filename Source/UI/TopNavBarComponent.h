#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

/**
 * @brief Верхняя навигационная панель.
 * Содержит логотип плагина и 5 кнопок переключения вкладок
 * (OSC, FX, CHORDS, ROM EXPORT, PRESETS).
 */
class TopNavBarComponent : public juce::Component
{
public:
    TopNavBarComponent(LofiChiptuneSynthAudioProcessor& processor);
    ~TopNavBarComponent() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    /**
     * @brief Смена активной вкладки.
     */
    void selectTab(WindowState state);

    LofiChiptuneSynthAudioProcessor& processor;

    // Кнопки навигации
    juce::TextButton oscTabButton{"OSC"};
    juce::TextButton fxTabButton{"FX"};
    juce::TextButton chordsTabButton{"CHORDS"};
    juce::TextButton romExportTabButton{"ROM EXPORT"};
    juce::TextButton presetTabButton{"PRESETS"};
    juce::TextButton nsButton{"i"};

    void showAboutWindow();
    
    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopNavBarComponent)
};
