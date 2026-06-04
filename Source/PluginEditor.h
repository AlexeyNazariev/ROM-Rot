#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"
#include "UI/TopNavBarComponent.h"
#include "UI/BottomMacroBar.h"
#include "UI/OscWindow.h"
#include "UI/FxWindow.h"
#include "UI/ChordsWindow.h"
#include "UI/RomExportWindow.h"
#include "UI/PresetWindow.h"

/**
 * @brief Главное окно визуального редактора плагина (Editor).
 * Наследуется от juce::DragAndDropContainer для обеспечения работы MIDI Drag-and-Drop.
 */
class LofiChiptuneSynthAudioProcessorEditor : public juce::AudioProcessorEditor,
                                              public juce::DragAndDropContainer
{
public:
    LofiChiptuneSynthAudioProcessorEditor (LofiChiptuneSynthAudioProcessor&);
    ~LofiChiptuneSynthAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LofiChiptuneSynthAudioProcessor& audioProcessor;

    // Вспомогательные дочерние панели интерфейса
    TopNavBarComponent topNavBar;
    BottomMacroBar bottomMacroBar;

    // Окна вкладок (5-window layout)
    OscWindow oscWindow;
    FxWindow fxWindow;
    ChordsWindow chordsWindow;
    RomExportWindow romExportWindow;
    PresetWindow presetWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LofiChiptuneSynthAudioProcessorEditor)
};
