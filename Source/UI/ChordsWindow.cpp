#include "ChordsWindow.h"

ChordsWindow::ChordsWindow(LofiChiptuneSynthAudioProcessor& p)
    : processor(p), midiDragButton(p)
{
    juce::Colour neonCyan = juce::Colour(0xFF0DF5E3);
    juce::Colour neonPink = juce::Colour(0xFFFF2E93);

    // Инициализация ComboBox тональности
    keySelector.addItemList(juce::StringArray{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 1);
    addAndMakeVisible(keySelector);
    keySelector.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    
    keyLabel.setText("BASE KEY", juce::dontSendNotification);
    keyLabel.setFont(juce::Font("Outfit", 12.0f, juce::Font::bold));
    keyLabel.setColour(juce::Label::textColourId, neonCyan);
    addAndMakeVisible(keyLabel);

    // Инициализация ComboBox гаммы
    scaleSelector.addItemList(juce::StringArray{"Major", "Minor", "Phrygian"}, 1);
    addAndMakeVisible(scaleSelector);
    scaleSelector.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    scaleLabel.setText("SCALE / MODE", juce::dontSendNotification);
    scaleLabel.setFont(juce::Font("Outfit", 12.0f, juce::Font::bold));
    scaleLabel.setColour(juce::Label::textColourId, neonCyan);
    addAndMakeVisible(scaleLabel);

    // Инициализация ComboBox паттерна
    patternSelector.addItemList(juce::StringArray{"Classic Arp", "Lo-Fi Strum", "Rhythmic Chop", "Sustain Pad"}, 1);
    addAndMakeVisible(patternSelector);
    patternSelector.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    patternLabel.setText("ARPEGGIATOR / CHORD PATTERN", juce::dontSendNotification);
    patternLabel.setFont(juce::Font("Outfit", 12.0f, juce::Font::bold));
    patternLabel.setColour(juce::Label::textColourId, neonCyan);
    addAndMakeVisible(patternLabel);

    // Инициализация Toggle включения ассистента
    addAndMakeVisible(assistantToggle);
    assistantToggle.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    assistantToggle.setLookAndFeel(&customKnobLookAndFeel);
    assistantToggle.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    assistantToggle.setColour(juce::ToggleButton::tickColourId, neonPink);
    assistantToggle.setColour(juce::ToggleButton::tickDisabledColourId, juce::Colours::grey);

    // Синхронизируем начальное состояние из параметра
    {
        auto* param = processor.getAPVTS().getParameter("chord_assistant");
        if (param != nullptr)
            assistantToggle.setToggleState(param->getValue() > 0.5f, juce::dontSendNotification);
    }

    assistantToggle.setButtonText("ACTIVATE CHORD ASSISTANT");

    // Ручной onClick: обновляем параметр и принудительно перерисовываем галочку
    assistantToggle.onClick = [this]()
    {
        // ToggleButton уже переключил своё состояние ДО вызова onClick
        const bool newState = assistantToggle.getToggleState();

        // Передаём значение в DAW/хост
        auto* param = processor.getAPVTS().getParameter("chord_assistant");
        if (param != nullptr)
            param->setValueNotifyingHost(newState ? 1.0f : 0.0f);

        // Немедленная перерисовка — без этого JUCE оставляет кнопку
        // в состоянии "нажата" (тёмный фон) до следующего paint-цикла
        assistantToggle.repaint();
    };

    // Кнопка экспорта
    addAndMakeVisible(midiDragButton);

    // Связывание с параметрами процессора
    auto& apvts = processor.getAPVTS();
    keyAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "key_select",    keySelector);
    scaleAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "scale_select",  scaleSelector);
    patternAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "pattern_type",  patternSelector);

    // Регистрируемся как слушатель параметра chord_assistant, чтобы
    // отслеживать изменения из хоста (automation, preset recall и пр.)
    apvts.addParameterListener("chord_assistant", this);
}

ChordsWindow::~ChordsWindow()
{
    // Снимаем слушатель параметра
    processor.getAPVTS().removeParameterListener("chord_assistant", this);
    assistantToggle.setLookAndFeel(nullptr);
}

// ---------------------------------------------------------------------------
// Вызывается из аудио-потока/хоста при изменении chord_assistant снаружи.
// Делегируем обновление GUI в основной поток и форсируем repaint.
// ---------------------------------------------------------------------------
void ChordsWindow::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "chord_assistant")
    {
        juce::MessageManager::callAsync([this, newValue]()
        {
            assistantToggle.setToggleState(newValue > 0.5f, juce::dontSendNotification);
            assistantToggle.repaint();
        });
    }
}

void ChordsWindow::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Фон
    juce::Colour bgStart = juce::Colour(0xFF0F0D1B);
    juce::Colour bgEnd   = juce::Colour(0xFF080710);
    juce::ColourGradient grad(bgStart, 0, 0, bgEnd, 0, bounds.getHeight(), false);
    g.setGradientFill(grad);
    g.fillAll();

    // 2 панели (Настройки и Экспорт)
    auto leftPanel = bounds.removeFromLeft(bounds.getWidth() * 0.55f).reduced(12.0f);
    auto rightPanel = bounds.reduced(12.0f);

    // Фон панелей
    g.setColour(juce::Colour(0xFF1D1830).withAlpha(0.3f));
    g.fillRoundedRectangle(leftPanel, 10.0f);
    g.fillRoundedRectangle(rightPanel, 10.0f);

    // Обводка
    g.setColour(juce::Colour(0xFF0DF5E3).withAlpha(0.2f));
    g.drawRoundedRectangle(leftPanel, 10.0f, 1.5f);
    g.setColour(juce::Colour(0xFFFF2E93).withAlpha(0.2f));
    g.drawRoundedRectangle(rightPanel, 10.0f, 1.5f);

    // Заголовки
    g.setColour(juce::Colour(0xFF0DF5E3));
    g.setFont(juce::Font("Outfit", 14.0f, juce::Font::bold));
    g.drawText("CHORD CONFIGURATION", leftPanel.removeFromTop(25.0f).reduced(10.0f, 0.0f), juce::Justification::left, true);

    g.setColour(juce::Colour(0xFFFF2E93));
    g.drawText("EXPORT PATTERN TO DAW", rightPanel.removeFromTop(25.0f).reduced(10.0f, 0.0f), juce::Justification::left, true);
    
    // Текстовая инструкция в правой панели
    auto descArea = rightPanel.reduced(15.0f).removeFromTop(80.0f);
    g.setColour(juce::Colours::white.withAlpha(0.7f));
    g.setFont(juce::Font("Outfit", 12.0f, juce::Font::plain));
    g.drawFittedText("Зажмите розовую кнопку ниже и перетащите её прямо на MIDI-дорожку вашей DAW (Reaper, Cubase, FL Studio, Ableton и др.). Будет создан MIDI-клип с выбранным аккордом и паттерном.",
                     descArea.toNearestInt(), juce::Justification::centred, 5);
}

void ChordsWindow::resized()
{
    auto area = getLocalBounds();

    // Левая секция настроек
    auto leftArea = area.removeFromLeft(area.getWidth() * 0.55f).reduced(25);
    leftArea.removeFromTop(15); // отступ для заголовка

    int controlH = 42;
    int spacing  = 8;

    // Выбор тональности
    auto row1 = leftArea.removeFromTop(controlH);
    keyLabel.setBounds(row1.removeFromLeft(120).reduced(0, 10));
    keySelector.setBounds(row1.reduced(5, 7));

    leftArea.removeFromTop(spacing);

    // Выбор гаммы
    auto row2 = leftArea.removeFromTop(controlH);
    scaleLabel.setBounds(row2.removeFromLeft(120).reduced(0, 10));
    scaleSelector.setBounds(row2.reduced(5, 7));

    leftArea.removeFromTop(spacing);

    // Выбор паттерна
    auto row3 = leftArea.removeFromTop(controlH);
    patternLabel.setBounds(row3.removeFromLeft(120).reduced(0, 10));
    patternSelector.setBounds(row3.reduced(5, 7));

    leftArea.removeFromTop(spacing + 5);

    // Активация автоаккорда
    assistantToggle.setBounds(leftArea.removeFromTop(30).reduced(5, 0));

    // Правая секция экспорта
    auto rightArea = area.reduced(25);
    rightArea.removeFromTop(100); // отступ под описание и заголовок

    // Позиционируем MidiDragButton по центру
    int btnW = 180;
    int btnH = 45;
    midiDragButton.setBounds(rightArea.getCentreX() - btnW / 2, rightArea.getCentreY() - btnH / 2, btnW, btnH);
}
