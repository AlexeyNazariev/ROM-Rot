#include "PluginProcessor.h"
#include "PluginEditor.h"

LofiChiptuneSynthAudioProcessorEditor::LofiChiptuneSynthAudioProcessorEditor (LofiChiptuneSynthAudioProcessor& p)
    : AudioProcessorEditor (&p),
      audioProcessor (p),
      topNavBar (p),
      bottomMacroBar (p),
      oscWindow (p),
      fxWindow (p),
      chordsWindow (p),
      romExportWindow (p),
      presetWindow (p)
{
    // Добавляем дочерние панели в UI
    addAndMakeVisible (topNavBar);
    addAndMakeVisible (bottomMacroBar);

    // Добавляем окна вкладок (по умолчанию невидимые, кроме активной)
    addChildComponent (oscWindow);
    addChildComponent (fxWindow);
    addChildComponent (chordsWindow);
    addChildComponent (romExportWindow);
    addChildComponent (presetWindow);

#if !JucePlugin_IsSynth
    audioProcessor.getWindowStateManager().setWindowState(WindowState::FX);
#endif

    // Устанавливаем размеры главного окна плагина (5-window Neo-Synth)
    setSize (800, 440);
}

void LofiChiptuneSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Задний фон по умолчанию (на случай задержки загрузки окон)
    g.fillAll (juce::Colour (0xFF0D0B14));
}

void LofiChiptuneSynthAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // Верхняя панель навигации (50 пикселей)
    topNavBar.setBounds (area.removeFromTop (50));

    // Нижняя макро-панель (75 пикселей)
    bottomMacroBar.setBounds (area.removeFromBottom (75));

    // Оставшаяся средняя секция предназначена для переключаемых вкладок
    auto middleArea = area;

    // Читаем текущее потокобезопасное состояние окон
    auto activeWindow = audioProcessor.getWindowStateManager().getWindowState();

    // Управляем видимостью окон вкладок
    oscWindow.setVisible (activeWindow == WindowState::OSC);
    fxWindow.setVisible (activeWindow == WindowState::FX);
    chordsWindow.setVisible (activeWindow == WindowState::CHORDS);
    romExportWindow.setVisible (activeWindow == WindowState::ROM_EXPORT);
    presetWindow.setVisible (activeWindow == WindowState::PRESETS);

    // Позиционируем их в среднюю область
    oscWindow.setBounds (middleArea);
    fxWindow.setBounds (middleArea);
    chordsWindow.setBounds (middleArea);
    romExportWindow.setBounds (middleArea);
    presetWindow.setBounds (middleArea);
}
