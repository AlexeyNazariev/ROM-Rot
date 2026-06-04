#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

/**
 * @brief Вкладка браузера пресетов (PRESETS).
 * Содержит 8 кнопок для быстрого выбора пресетов с разнообразными
 * тембрами и настройками эффектов.
 */
class PresetWindow : public juce::Component
{
public:
    PresetWindow(LofiChiptuneSynthAudioProcessor& processor);
    ~PresetWindow() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    /**
     * @brief Загрузка пресета по его индексу.
     */
    void loadPreset(int presetIndex);

    LofiChiptuneSynthAudioProcessor& processor;

    juce::Label presetSectionTitle{"", "PRESET LIBRARY"};
    
    // 8 кнопок пресетов
    static constexpr int numPresets = 8;
    juce::TextButton presetButtons[numPresets];
    juce::String presetNames[numPresets] = {
        "8 bit lead",
        "DUSTY VHS PAD",
        "ARCADE COIN ARP",
        "TAPE DRIP KEYS",
        "SUB-ROT BASS",
        "GLITCH CRUSH",
        "HAUNTED MUSIC BOX",
        "GHOST IN THE ROM"
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetWindow)
};
