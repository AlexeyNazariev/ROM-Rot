#include "BottomMacroBar.h"

BottomMacroBar::BottomMacroBar(LofiChiptuneSynthAudioProcessor& p)
    : processor(p),
      oscilloscope(p.getRingBuffer())
{
    // Настройка Vibe Macro Slider (Era Morpher)
    vibeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    vibeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    vibeSlider.setLookAndFeel(&customKnobLookAndFeel);
    addAndMakeVisible(vibeSlider);
    
    vibeSlider.setColour(juce::Slider::thumbColourId,      juce::Colour(0xFFFF2E93));
    vibeSlider.setColour(juce::Slider::trackColourId,      juce::Colour(0xFFFF2E93).withAlpha(0.6f));
    vibeSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF151122));
    vibeSlider.setMouseCursor(juce::MouseCursor::NormalCursor);

    // Настройка Master Volume Slider
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    volumeSlider.setLookAndFeel(&customKnobLookAndFeel);
    addAndMakeVisible(volumeSlider);
    
    volumeSlider.setColour(juce::Slider::thumbColourId,      juce::Colour(0xFF0DF5E3));
    volumeSlider.setColour(juce::Slider::trackColourId,      juce::Colour(0xFF0DF5E3).withAlpha(0.6f));
    volumeSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF151122));

    // Осциллограф
    addAndMakeVisible(oscilloscope);

    // Подпись ROT (Vibe Macro)
    vibeLabel.setText("ROT", juce::dontSendNotification);
    vibeLabel.setFont(juce::Font("Outfit", 12.0f, juce::Font::bold));
    vibeLabel.setColour(juce::Label::textColourId, juce::Colour(0xFFFF2E93));
    vibeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(vibeLabel);

    // Подпись VOLUME
    volumeLabel.setText("VOLUME", juce::dontSendNotification);
    volumeLabel.setFont(juce::Font("Outfit", 12.0f, juce::Font::bold));
    volumeLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF0DF5E3));
    volumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volumeLabel);

    // Вложения параметров APVTS процессора
    auto& apvts = processor.getAPVTS();
    vibeAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "vibe_macro",    vibeSlider);
    volumeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "master_volume", volumeSlider);
}

BottomMacroBar::~BottomMacroBar()
{
    vibeSlider.setLookAndFeel(nullptr);
    volumeSlider.setLookAndFeel(nullptr);
}

// Константы геометрии, чтобы paint() и resized() использовали одно и то же
static constexpr int kLeftPanelW  = 240;
static constexpr int kRightPanelW = 220;
static constexpr int kPanelMarginH = 8;   // вертикальный отступ панели
static constexpr int kPanelMarginX = 10;  // горизонтальный отступ панели
static constexpr int kLabelH       = 18;  // высота подписи
static constexpr int kContentPad   = 8;   // отступ внутри панели

void BottomMacroBar::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Тёмно-стеклянный фон с градиентом
    juce::Colour bgStart = juce::Colour(0xFF151122);
    juce::Colour bgEnd   = juce::Colour(0xFF0D0B14);
    juce::ColourGradient grad(bgStart, 0, 0, bgEnd, 0, bounds.getHeight(), false);
    g.setGradientFill(grad);
    g.fillAll();

    // Тонкая верхняя неоновая линия-разделитель
    g.setColour(juce::Colour(0xFFFF2E93).withAlpha(0.5f));
    g.drawLine(0.0f, 0.0f, bounds.getWidth(), 0.0f, 1.5f);

    // Вычисляем области подложек — те же координаты, что в resized()
    auto area = getLocalBounds();

    auto leftPanel  = area.removeFromLeft (kLeftPanelW) .reduced(kPanelMarginX, kPanelMarginH).toFloat();
    auto rightPanel = area.removeFromRight(kRightPanelW).reduced(kPanelMarginX, kPanelMarginH).toFloat();

    // Подложки панелей (Glassmorphism)
    g.setColour(juce::Colour(0xFF1D1830).withAlpha(0.25f));
    g.fillRoundedRectangle(leftPanel,  8.0f);
    g.fillRoundedRectangle(rightPanel, 8.0f);

    // Рамки панелей с неоновым оттенком
    g.setColour(juce::Colour(0xFFFF2E93).withAlpha(0.25f));  // Розовый для VIBE FX
    g.drawRoundedRectangle(leftPanel,  8.0f, 1.2f);

    g.setColour(juce::Colour(0xFF0DF5E3).withAlpha(0.25f));  // Циан для VOLUME
    g.drawRoundedRectangle(rightPanel, 8.0f, 1.2f);
}

void BottomMacroBar::resized()
{
    auto area = getLocalBounds();

    // Левая панель — Vibe slider (подпись ROT снизу)
    auto leftPanel   = area.removeFromLeft(kLeftPanelW) .reduced(kPanelMarginX, kPanelMarginH);
    auto leftContent = leftPanel.reduced(kContentPad, 2);
    auto leftLabelArea = leftContent.removeFromBottom(kLabelH);
    vibeLabel.setBounds(leftLabelArea);
    vibeSlider.setBounds(leftContent.reduced(0, 2));

    // Правая панель — Volume slider (подпись VOLUME снизу)
    auto rightPanel   = area.removeFromRight(kRightPanelW).reduced(kPanelMarginX, kPanelMarginH);
    auto rightContent = rightPanel.reduced(kContentPad, 2);
    auto rightLabelArea = rightContent.removeFromBottom(kLabelH);
    volumeLabel.setBounds(rightLabelArea);
    volumeSlider.setBounds(rightContent.reduced(0, 2));

    // Осциллограф занимает оставшееся место по центру
    oscilloscope.setBounds(area.reduced(10, kPanelMarginH - 2));
}
