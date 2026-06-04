#include "FxWindow.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

FxWindow::FxWindow(LofiChiptuneSynthAudioProcessor& p)
    : processor(p)
{
    juce::Colour neonCyan = juce::Colour::fromString("#0DF5E3");

    // Битность
    bitDepthSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    bitDepthSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    bitDepthSlider.setLookAndFeel(&customKnobLookAndFeel);
    addAndMakeVisible(bitDepthSlider);
    bitDepthSlider.setColour(juce::Slider::thumbColourId, neonCyan);
    bitDepthSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromString("#1A1527"));
    bitDepthSlider.setColour(juce::Slider::rotarySliderFillColourId, neonCyan.withAlpha(0.6f));

    bitDepthLabel.setText("BIT DEPTH", juce::dontSendNotification);
    bitDepthLabel.setFont(juce::Font("Outfit", 11.0f, juce::Font::bold));
    bitDepthLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    bitDepthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bitDepthLabel);

    // Даунсэмплинг
    sampleRateSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    sampleRateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    sampleRateSlider.setLookAndFeel(&customKnobLookAndFeel);
    addAndMakeVisible(sampleRateSlider);
    sampleRateSlider.setColour(juce::Slider::thumbColourId, neonCyan);
    sampleRateSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromString("#1A1527"));
    sampleRateSlider.setColour(juce::Slider::rotarySliderFillColourId, neonCyan.withAlpha(0.6f));

    sampleRateLabel.setText("SAMPLE RATE", juce::dontSendNotification);
    sampleRateLabel.setFont(juce::Font("Outfit", 11.0f, juce::Font::bold));
    sampleRateLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    sampleRateLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sampleRateLabel);

    // Сатурация (Saturation Drive)
    saturationSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    saturationSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    saturationSlider.setLookAndFeel(&customKnobLookAndFeel);
    addAndMakeVisible(saturationSlider);
    saturationSlider.setColour(juce::Slider::thumbColourId, neonCyan);
    saturationSlider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour::fromString("#1A1527"));
    saturationSlider.setColour(juce::Slider::rotarySliderFillColourId, neonCyan.withAlpha(0.6f));

    saturationLabel.setText("SAT DRIVE", juce::dontSendNotification);
    saturationLabel.setFont(juce::Font("Outfit", 11.0f, juce::Font::bold));
    saturationLabel.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    saturationLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(saturationLabel);

    // Связываем с APVTS процессора
    auto& apvts = processor.getAPVTS();
    bitDepthAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "bitdepth", bitDepthSlider);
    sampleRateAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "samplerate_ratio", sampleRateSlider);
    saturationAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "saturation_drive", saturationSlider);

    // Запускаем таймер анимации (30 кадров в секунду)
    startTimerHz(30);
}

FxWindow::~FxWindow()
{
    stopTimer();
    bitDepthSlider.setLookAndFeel(nullptr);
    sampleRateSlider.setLookAndFeel(nullptr);
    saturationSlider.setLookAndFeel(nullptr);
}

void FxWindow::timerCallback()
{
    float vibe = processor.getAPVTS().getRawParameterValue("vibe_macro")->load(std::memory_order_relaxed);
    
    // Вращаем бобины кассеты, только если Vibe > 0.05 (включен Lo-Fi режим)
    if (vibe > 0.05f)
    {
        // Базовая скорость зависит от Vibe (моделирует скорость прохождения ленты)
        float speedFactor = vibe * 0.12f;
        
        // Моделируем детонацию (Wow & Flutter) в анимации!
        float timeMs = static_cast<float>(juce::Time::getMillisecondCounterHiRes());
        float flutterWobble = std::sin(timeMs * 0.008f) * 0.04f * vibe;
        float wowWobble = std::sin(timeMs * 0.001f) * 0.015f * vibe;
        
        float finalSpeed = speedFactor + flutterWobble + wowWobble;
        
        leftReelAngle += finalSpeed;
        rightReelAngle += finalSpeed;
        
        // Ограничиваем угол в пределах 2*PI
        leftReelAngle = std::fmod(leftReelAngle, static_cast<float>(2.0 * M_PI));
        rightReelAngle = std::fmod(rightReelAngle, static_cast<float>(2.0 * M_PI));
        
        repaint();
    }
}

void FxWindow::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Задний фон с градиентом
    juce::Colour bgStart = juce::Colour(0xFF0F0D1B);
    juce::Colour bgEnd   = juce::Colour(0xFF080710);
    juce::ColourGradient grad(bgStart, 0, 0, bgEnd, 0, bounds.getHeight(), false);
    g.setGradientFill(grad);
    g.fillAll();

    // Делим на две панели
    auto leftPanel = bounds.removeFromLeft(bounds.getWidth() * 0.40f).reduced(12.0f);
    auto rightPanel = bounds.reduced(12.0f);

    // Отрисовка подложек панелей
    g.setColour(juce::Colour(0xFF1D1830).withAlpha(0.3f));
    g.fillRoundedRectangle(leftPanel, 10.0f);
    g.fillRoundedRectangle(rightPanel, 10.0f);

    // Рамки панелей с неоновым свечением
    g.setColour(juce::Colour(0xFF0DF5E3).withAlpha(0.2f));
    g.drawRoundedRectangle(leftPanel, 10.0f, 1.5f);
    
    g.setColour(juce::Colour(0xFFFF2E93).withAlpha(0.2f));
    g.drawRoundedRectangle(rightPanel, 10.0f, 1.5f);

    // Заголовки разделов
    g.setColour(juce::Colour(0xFF0DF5E3));
    g.setFont(juce::Font("Outfit", 14.0f, juce::Font::bold));
    g.drawText("MANUAL BITCRUSHER", leftPanel.removeFromTop(25.0f).reduced(10.0f, 0.0f), juce::Justification::left, true);

    g.setColour(juce::Colour(0xFFFF2E93));
    g.drawText("TAPE EMULATION STATUS", rightPanel.removeFromTop(25.0f).reduced(10.0f, 0.0f), juce::Justification::left, true);

    // --- Отрисовка векторной анимации аудиокассеты ---
    auto tapeArea = rightPanel.reduced(15.0f);
    
    // 1. Корпус кассеты
    juce::Colour cassetteColor = juce::Colour(0xFF120E1F);
    g.setColour(cassetteColor);
    g.fillRoundedRectangle(tapeArea, 12.0f);
    g.setColour(juce::Colour(0xFFFF2E93).withAlpha(0.4f));
    g.drawRoundedRectangle(tapeArea, 12.0f, 2.0f);
    
    // Внутреннее окно для бобин
    auto windowArea = tapeArea.reduced(tapeArea.getWidth() * 0.15f, tapeArea.getHeight() * 0.25f);
    g.setColour(juce::Colour(0xFF050409));
    g.fillRoundedRectangle(windowArea, 6.0f);
    g.setColour(juce::Colours::black);
    g.drawRoundedRectangle(windowArea, 6.0f, 1.0f);

    // 2. Рисуем две бобины
    float reelRadius = windowArea.getHeight() * 0.35f;
    float leftReelX = windowArea.getX() + windowArea.getWidth() * 0.28f;
    float rightReelX = windowArea.getX() + windowArea.getWidth() * 0.72f;
    float reelY = windowArea.getCentreY();

    auto drawReel = [&](float cx, float cy, float angle, float tapeAmount) {
        // Лента, намотанная на бобину (темный рулон)
        if (tapeAmount > 0.0f)
        {
            g.setColour(juce::Colour(0xFF2A2218)); // Цвет магнитной ленты
            g.fillEllipse(cx - (reelRadius + tapeAmount * 12.0f), cy - (reelRadius + tapeAmount * 12.0f),
                          (reelRadius + tapeAmount * 12.0f) * 2.0f, (reelRadius + tapeAmount * 12.0f) * 2.0f);
        }

        // Сама бобина (пластиковое колесо)
        g.setColour(juce::Colour(0xFF1A162B));
        g.fillEllipse(cx - reelRadius, cy - reelRadius, reelRadius * 2.0f, reelRadius * 2.0f);
        
        g.setColour(juce::Colour(0xFF0DF5E3).withAlpha(0.8f));
        g.drawEllipse(cx - reelRadius, cy - reelRadius, reelRadius * 2.0f, reelRadius * 2.0f, 1.5f);

        // Зубья бобины (вращающиеся спицы)
        g.setColour(juce::Colour(0xFF0DF5E3));
        int numSpokes = 6;
        for (int i = 0; i < numSpokes; ++i)
        {
            float spAngle = angle + static_cast<float>(i * (2.0 * M_PI) / numSpokes);
            float sx = cx + std::cos(spAngle) * (reelRadius - 2.0f);
            float sy = cy + std::sin(spAngle) * (reelRadius - 2.0f);
            g.drawLine(cx, cy, sx, sy, 2.0f);
        }
        
        // Центральное отверстие бобины
        g.setColour(juce::Colours::black);
        g.fillEllipse(cx - reelRadius * 0.3f, cy - reelRadius * 0.3f, reelRadius * 0.6f, reelRadius * 0.6f);
    };

    // Читаем Vibe для определения количества ленты (визуально)
    float vibe = processor.getAPVTS().getRawParameterValue("vibe_macro")->load(std::memory_order_relaxed);
    
    // Эмулируем перематывание ленты: левая бобина отдает, правая принимает
    // Мы можем связать это просто со временем для визуального движения
    float timeSecs = static_cast<float>(juce::Time::getMillisecondCounterHiRes() * 0.0001f);
    float tapeRatio = 0.5f + 0.3f * std::sin(timeSecs); // плавает от 0.2 до 0.8
    
    drawReel(leftReelX, reelY, leftReelAngle, tapeRatio);
    drawReel(rightReelX, reelY, -rightReelAngle, 1.0f - tapeRatio); // крутится в ту же сторону, ленты меньше

    // Дополнительный текст состояния под кассетой
    g.setColour(juce::Colour(0xFFFF2E93).withAlpha(0.8f));
    g.setFont(juce::Font("Outfit", 12.0f, juce::Font::bold));
    
    juce::String statusText = "PURE CHIPTUNE (CLEAN BYPASS)";
    if (vibe > 0.05f && vibe <= 0.3f) statusText = "LO-FI CASSETTE ACTIVE";
    else if (vibe > 0.3f && vibe <= 0.7f) statusText = "VHS RETRO - TAPE MELLING";
    else if (vibe > 0.7f) statusText = "DYING HARDWARE - BITCRUSH HELL";
    
    g.drawText(statusText, tapeArea.removeFromBottom(20.0f), juce::Justification::centred, true);
}

void FxWindow::resized()
{
    auto area = getLocalBounds();
    
    // Левая секция (Биткрашер)
    auto leftArea = area.removeFromLeft(area.getWidth() * 0.40f).reduced(20);
    leftArea.removeFromTop(20); // отступ для заголовка
    
    int knobH = leftArea.getHeight() / 3;
    
    auto knob1 = leftArea.removeFromTop(knobH).reduced(12);
    bitDepthLabel.setBounds(knob1.removeFromBottom(16));
    bitDepthSlider.setBounds(knob1);

    auto knob2 = leftArea.removeFromTop(knobH).reduced(12);
    sampleRateLabel.setBounds(knob2.removeFromBottom(16));
    sampleRateSlider.setBounds(knob2);

    auto knob3 = leftArea.reduced(12);
    saturationLabel.setBounds(knob3.removeFromBottom(16));
    saturationSlider.setBounds(knob3);
}
