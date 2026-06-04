#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"
#include "CustomLookAndFeel.h"

/**
 * @brief Панель экспорта кода для ретро-платформ (ROM_EXPORT).
 * Позволяет экспортировать текущие настройки параметров синтезатора в код для PICO-8 Lua
 * или ассемблер NES 2A03 (6502 Assembly).
 */
class RomExportWindow : public juce::Component, public juce::Button::Listener
{
public:
    RomExportWindow(LofiChiptuneSynthAudioProcessor& processor);
    ~RomExportWindow() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void buttonClicked (juce::Button* button) override;

private:
    /**
     * @brief Запускает генерацию кода на основе выбранного формата и текущих параметров синтезатора.
     */
    void updateGeneratedCode();

    LofiChiptuneSynthAudioProcessor& processor;

    // Секции и элементы управления
    juce::Label headerLabel{"", "RETRO SYSTEM CODE EXPORTER"};
    juce::ComboBox formatSelector;
    juce::TextButton copyButton{"COPY CODE"};
    juce::TextButton refreshButton{"REFRESH CODE"};

    // Текстовое окно вывода кода
    juce::TextEditor codeEditor;
    CustomKnobLookAndFeel customKnobLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RomExportWindow)
};
