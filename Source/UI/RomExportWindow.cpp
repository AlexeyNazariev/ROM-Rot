#include "RomExportWindow.h"
#include "../Utils/RetroRomExporter.h"

RomExportWindow::RomExportWindow(LofiChiptuneSynthAudioProcessor& p)
    : processor(p)
{
    juce::Colour neonCyan = juce::Colour(0xFF0DF5E3);
    juce::Colour neonPink = juce::Colour(0xFFFF2E93);

    // Заголовок
    headerLabel.setFont(juce::Font("Outfit", 14.0f, juce::Font::bold));
    headerLabel.setColour(juce::Label::textColourId, neonCyan);
    addAndMakeVisible(headerLabel);

    // Выбор формата
    formatSelector.addItem("PICO-8 SFX (Lua)", 1);
    formatSelector.addItem("NES 2A03 (6502 ASM)", 2);
    formatSelector.addItem("SEGA Genesis (YM2612 Regs)", 3);
    formatSelector.addItem("GameBoy Color (GBDK C-Code)", 4);
    formatSelector.setSelectedId(1);
    addAndMakeVisible(formatSelector);
    formatSelector.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    formatSelector.onChange = [this]() { updateGeneratedCode(); };

    // Кнопка обновления
    refreshButton.addListener(this);
    addAndMakeVisible(refreshButton);
    refreshButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    refreshButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF1A1527));
    refreshButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    // Кнопка копирования
    copyButton.addListener(this);
    addAndMakeVisible(copyButton);
    copyButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    copyButton.setColour(juce::TextButton::buttonColourId, neonPink.withAlpha(0.7f));
    copyButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    // ---------------------------------------------------------------------------
    // Текстовое поле терминала (NES/PICO-8 Terminal)
    // ---------------------------------------------------------------------------
    codeEditor.setMultiLine(true, false);
    codeEditor.setReadOnly(true);
    codeEditor.setCaretVisible(false);
    codeEditor.setFont(juce::Font("Consolas", 13.0f, juce::Font::plain));

    // Прямая установка цветов на компоненте (приоритет выше LookAndFeel)
    codeEditor.setColour(juce::TextEditor::textColourId,           neonCyan);
    codeEditor.setColour(juce::TextEditor::backgroundColourId,     juce::Colour(0xff060509));
    codeEditor.setColour(juce::TextEditor::outlineColourId,        neonPink.withAlpha(0.25f));
    codeEditor.setColour(juce::TextEditor::focusedOutlineColourId, neonPink.withAlpha(0.5f));
    codeEditor.setColour(juce::TextEditor::highlightColourId,      neonPink.withAlpha(0.5f));
    codeEditor.setColour(juce::TextEditor::highlightedTextColourId, juce::Colours::white);

    addAndMakeVisible(codeEditor);

    // Первая генерация кода
    updateGeneratedCode();
}

RomExportWindow::~RomExportWindow()
{
}

void RomExportWindow::buttonClicked (juce::Button* button)
{
    if (button == &refreshButton)
    {
        updateGeneratedCode();
    }
    else if (button == &copyButton)
    {
        juce::SystemClipboard::copyTextToClipboard(codeEditor.getText());
        copyButton.setButtonText("COPIED!");
        
        // Возвращаем исходный текст кнопки через 1.5 секунды
        juce::Timer::callAfterDelay(1500, [this]()
        {
            copyButton.setButtonText("COPY CODE");
        });
    }
}

void RomExportWindow::updateGeneratedCode()
{
    auto& apvts = processor.getAPVTS();

    int   waveType   = static_cast<int>(apvts.getRawParameterValue("osc_wave")->load(std::memory_order_relaxed));
    float pulseWidth = apvts.getRawParameterValue("pulse_width")->load(std::memory_order_relaxed);
    float attack     = apvts.getRawParameterValue("adsr_attack")->load(std::memory_order_relaxed);
    float decay      = apvts.getRawParameterValue("adsr_decay")->load(std::memory_order_relaxed);
    float sustain    = apvts.getRawParameterValue("adsr_sustain")->load(std::memory_order_relaxed);
    float release    = apvts.getRawParameterValue("adsr_release")->load(std::memory_order_relaxed);
    float vibe       = apvts.getRawParameterValue("vibe_macro")->load(std::memory_order_relaxed);

    juce::String generatedCode;
    int formatId = formatSelector.getSelectedId();

    if (formatId == 1)
        generatedCode = RetroRomExporter::exportToPico8(waveType, pulseWidth, attack, decay, sustain, release, vibe);
    else if (formatId == 2)
        generatedCode = RetroRomExporter::exportToNesAssembly(waveType, pulseWidth, attack, decay, sustain, release);
    else if (formatId == 3)
        generatedCode = RetroRomExporter::exportToSegaYM2612(waveType, pulseWidth, attack, decay, sustain, release);
    else if (formatId == 4)
        generatedCode = RetroRomExporter::exportToGameBoyColor(waveType, pulseWidth, attack, decay, sustain, release);

    // Устанавливаем текст, затем ПРИНУДИТЕЛЬНО применяем цвет ко всем символам.
    // Это гарантирует нужный цвет даже если setText() создаёт секции
    // с дефолтным цветом LookAndFeel.
    codeEditor.setText(generatedCode, false);
    codeEditor.applyColourToAllText(juce::Colour(0xFF0DF5E3), true);
}

void RomExportWindow::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Фон с градиентом
    juce::Colour bgStart = juce::Colour(0xFF0F0D1B);
    juce::Colour bgEnd   = juce::Colour(0xFF080710);
    juce::ColourGradient grad(bgStart, 0, 0, bgEnd, 0, bounds.getHeight(), false);
    g.setGradientFill(grad);
    g.fillAll();

    // Общая рамка в стиле Glassmorphism
    auto panelArea = bounds.reduced(12.0f);
    g.setColour(juce::Colour(0xFF1D1830).withAlpha(0.3f));
    g.fillRoundedRectangle(panelArea, 10.0f);
    
    g.setColour(juce::Colour(0xFF0DF5E3).withAlpha(0.2f));
    g.drawRoundedRectangle(panelArea, 10.0f, 1.5f);
}

void RomExportWindow::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    // Верхняя панель управления
    auto topRow = area.removeFromTop(40);
    headerLabel.setBounds(topRow.removeFromLeft(220));
    
    // Выбор формата по центру
    formatSelector.setBounds(topRow.removeFromLeft(220).reduced(5, 5));
    
    // Кнопки справа
    copyButton.setBounds(topRow.removeFromRight(110).reduced(2, 5));
    refreshButton.setBounds(topRow.removeFromRight(130).reduced(2, 5));

    // Оставшаяся область под текстовый редактор терминала
    area.removeFromTop(10); // небольшой отступ
    codeEditor.setBounds(area);
}
