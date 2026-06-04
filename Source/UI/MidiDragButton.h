#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"

/**
 * @brief Кнопка экспорта MIDI-файла с поддержкой Drag-and-Drop (перетаскивание в DAW).
 * При зажатии кнопки мыши и перемещении, генерируется MIDI файл во временной директории,
 * содержащий арпеджированный аккорд, и инициируется системная операция перетаскивания.
 */
class MidiDragButton : public juce::Component
{
public:
    MidiDragButton(LofiChiptuneSynthAudioProcessor& processor);
    ~MidiDragButton() override = default;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void mouseEnter (const juce::MouseEvent& event) override;
    void mouseExit (const juce::MouseEvent& event) override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;

private:
    /**
     * @brief Вспомогательный метод для генерации MIDI-файла на основе текущего состояния.
     */
    juce::File generateMidiFile();

    /**
     * @brief Вспомогательный метод расчета 3-х нот трезвучия (аккорда) для экспорта.
     */
    void calculateChordNotes(int baseNote, int key, int scale, int (&chordNotes)[3]) noexcept;

    LofiChiptuneSynthAudioProcessor& processor;
    bool isHovered = false;
    bool isPressed = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiDragButton)
};
