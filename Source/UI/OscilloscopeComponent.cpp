#include "OscilloscopeComponent.h"
#include <cmath>

OscilloscopeComponent::OscilloscopeComponent(AudioRingBuffer& rb)
    : audioRingBuffer(rb)
{
    std::fill(sampleBuffer, sampleBuffer + numDisplaySamples, 0.0f);
    startTimerHz(30); // 30 кадров в секунду для плавной анимации
}

OscilloscopeComponent::~OscilloscopeComponent()
{
    stopTimer();
}

void OscilloscopeComponent::timerCallback()
{
    // Читаем самые свежие сэмплы из кольцевого буфера
    audioRingBuffer.readSamples(sampleBuffer, numDisplaySamples);
    repaint();
}

void OscilloscopeComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Черно-угольный фон
    g.setColour(juce::Colour::fromString("#060509"));
    g.fillRoundedRectangle(bounds, 6.0f);
    
    // Рисуем сетку виртуального осциллографа (CRT grid)
    g.setColour(juce::Colour::fromString("#FF2E93").withAlpha(0.08f));
    
    int numGridLinesX = 10;
    int numGridLinesY = 6;
    
    float stepX = bounds.getWidth() / static_cast<float>(numGridLinesX);
    float stepY = bounds.getHeight() / static_cast<float>(numGridLinesY);
    
    // Вертикальные линии сетки
    for (int i = 1; i < numGridLinesX; ++i)
    {
        g.drawVerticalLine(static_cast<int>(i * stepX), 0.0f, bounds.getHeight());
    }
    
    // Горизонтальные линии сетки
    for (int i = 1; i < numGridLinesY; ++i)
    {
        g.drawHorizontalLine(static_cast<int>(i * stepY), 0.0f, bounds.getWidth());
    }

    // Тонкая рамка вокруг экрана
    g.setColour(juce::Colour::fromString("#FF2E93").withAlpha(0.25f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.2f);

    // Строим путь Path для отрисовки волны
    juce::Path wavePath;
    float centerY = bounds.getHeight() * 0.5f;
    float scaleX = bounds.getWidth() / static_cast<float>(numDisplaySamples - 1);
    float scaleY = bounds.getHeight() * 0.42f; // Масштаб амплитуды (не даем вылезать за рамки)

    wavePath.startNewSubPath(0.0f, centerY - sampleBuffer[0] * scaleY);

    for (int i = 1; i < numDisplaySamples; ++i)
    {
        float x = static_cast<float>(i) * scaleX;
        // Ограничиваем сэмпл для стабильности
        float clampedSample = juce::jlimit(-1.0f, 1.0f, sampleBuffer[i]);
        float y = centerY - clampedSample * scaleY;
        wavePath.lineTo(x, y);
    }

    // Рисуем светящуюся линию волны (неоновый розовый)
    // Шаг 1: Толстая полупрозрачная линия для эффекта свечения (Glow)
    g.setColour(juce::Colour::fromString("#FF2E93").withAlpha(0.25f));
    g.strokePath(wavePath, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Шаг 2: Тонкая сплошная линия по центру для четкости луча
    g.setColour(juce::Colour::fromString("#FF2E93"));
    g.strokePath(wavePath, juce::PathStrokeType(1.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

void OscilloscopeComponent::resized()
{
}
