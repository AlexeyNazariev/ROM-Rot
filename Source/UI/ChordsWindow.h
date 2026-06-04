#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../PluginProcessor.h"
#include "MidiDragButton.h"
#include "CustomLookAndFeel.h"

/**
 * @brief Панель Chord Assistant (CHORDS).
 * Позволяет настраивать тональность (Key), гамму (Scale),
 * тип паттерна воспроизведения (Pattern Type), активировать автоаккорды
 * и экспортировать паттерн в DAW через MidiDragButton.
 */
class ChordsWindow : public juce::Component, public juce::AudioProcessorValueTreeState::Listener
{
public:
    ChordsWindow(LofiChiptuneSynthAudioProcessor& processor);
    ~ChordsWindow() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void parameterChanged (const juce::String& parameterID, float newValue) override;

private:
    LofiChiptuneSynthAudioProcessor& processor;

    // Секции заголовков
    juce::Label settingsTitle{"", "CHORD CONFIGURATION"};
    juce::Label dragTitle{"", "EXPORT PATTERN TO DAW"};

    // Элементы управления
    juce::Label keyLabel{"", "BASE KEY"};
    juce::ComboBox keySelector;

    juce::Label scaleLabel{"", "SCALE / MODE"};
    juce::ComboBox scaleSelector;

    juce::Label patternLabel{"", "ARPEGGIATOR / CHORD PATTERN"};
    juce::ComboBox patternSelector;

    juce::ToggleButton assistantToggle{"ACTIVATE CHORD ASSISTANT"};

    // Кнопка экспорта MIDI
    MidiDragButton midiDragButton;
    CustomKnobLookAndFeel customKnobLookAndFeel;

    // Вложения параметров APVTS
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> keyAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> patternAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordsWindow)
};
