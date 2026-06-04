#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @brief Кастомный стиль элементов управления (крутилок/слайдеров) в стиле ROM-Rot.
 * Отрисовывает четкий фоновый круг темно-серого цвета (#20252b) для хорошей
 * видимости на черном фоне, а также неоновый заполняющий дуговой сегмент и стрелку.
 * Поддерживает линейные слайдеры ADSR и тумблеры (чекбоксы) с галочками.
 */
class CustomKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomKnobLookAndFeel()
    {
        setColour (juce::TextEditor::textColourId, juce::Colour (0xFF0DF5E3));
        setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xFF060509));
        setColour (juce::TextEditor::highlightColourId, juce::Colour (0xFFFF2E93).withAlpha (0.6f));
        setColour (juce::TextEditor::highlightedTextColourId, juce::Colours::white);
        setColour (juce::TextEditor::outlineColourId, juce::Colour (0xFFFF2E93).withAlpha (0.25f));
        setColour (juce::TextEditor::focusedOutlineColourId, juce::Colour (0xFFFF2E93).withAlpha (0.5f));
    }
    ~CustomKnobLookAndFeel() override = default;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        // 1. Цвета крутилки
        juce::Colour activeColor = slider.findColour (juce::Slider::thumbColourId);
        if (activeColor.getAlpha() == 0)
            activeColor = slider.findColour (juce::Slider::rotarySliderFillColourId).withAlpha (1.0f);
        if (activeColor.getAlpha() == 0)
            activeColor = juce::Colour (0xFF0DF5E3); // Дефолтный неоновый циан

        juce::Colour trackBgColor = juce::Colour (0xff20252b); // Темно-серый контурный круг

        // Вычисляем размеры и центр круга
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (6.0f);
        auto radius = std::min (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        auto toX = bounds.getCentreX();
        auto toY = bounds.getCentreY();
        auto rx = toX - radius;
        auto ry = toY - radius;

        // Рисуем темно-черную заливку внутри крутилки для премиального объема
        g.setColour (juce::Colour (0xFF0D0B14));
        g.fillEllipse (rx + 2.0f, ry + 2.0f, (radius - 2.0f) * 2.0f, (radius - 2.0f) * 2.0f);

        // 2. Рисуем фоновую круговую дорожку (границы крутилки)
        g.setColour (trackBgColor);
        g.drawEllipse (rx, ry, radius * 2.0f, radius * 2.0f, 3.5f);

        // 3. Рисуем активную неоновую дугу заполнения
        if (sliderPosProportional > 0.0f)
        {
            juce::Path activeArc;
            activeArc.addCentredArc (toX, toY, radius, radius, 0.0f,
                                    rotaryStartAngle,
                                    rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle),
                                    true);
            g.setColour (activeColor);
            g.strokePath (activeArc, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // 4. Рисуем указатель-стрелку
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto pointerLength = radius * 0.75f;
        auto pointerThickness = 3.0f;

        juce::Path p;
        p.startNewSubPath (toX, toY);
        p.lineTo (toX + pointerLength * std::sin (angle),
                 toY - pointerLength * std::cos (angle));
        
        g.setColour (activeColor);
        g.strokePath (p, juce::PathStrokeType (pointerThickness, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        if (style == juce::Slider::LinearVertical)
        {
            juce::Colour activeColor = slider.findColour (juce::Slider::thumbColourId);
            if (activeColor.getAlpha() == 0)
                activeColor = juce::Colour (0xFFFF2E93); // По умолчанию неоново-розовый

            // Подложка: чуть более светлая и заметная версия цвета слайдера
            juce::Colour bgColor = activeColor.withAlpha (0.22f); 

            float trackWidth = 8.0f;
            float rx = x + width * 0.5f - trackWidth * 0.5f;
            float ry = (float)y;
            float rw = trackWidth;
            float rh = (float)height;

            // 1. Рисуем подложку (весь трек)
            g.setColour (bgColor);
            g.fillRoundedRectangle (rx, ry, rw, rh, trackWidth * 0.5f);
            
            // Тонкий контур подложки
            g.setColour (activeColor.withAlpha (0.35f));
            g.drawRoundedRectangle (rx, ry, rw, rh, trackWidth * 0.5f, 1.0f);

            // 2. Рисуем активную заполненную часть снизу вверх
            float fillY = sliderPos;
            float fillH = maxSliderPos - sliderPos;
            if (fillH > 0.0f)
            {
                g.setColour (activeColor);
                g.fillRoundedRectangle (rx, fillY, rw, fillH, trackWidth * 0.5f);
            }

            // 3. Рисуем бегунок (thumb)
            float thumbHeight = 6.0f;
            float thumbWidth = width * 0.75f;
            float tx = x + width * 0.5f - thumbWidth * 0.5f;
            float ty = sliderPos - thumbHeight * 0.5f;
            
            ty = juce::jlimit ((float)y, (float)(y + height - thumbHeight), ty);

            g.setColour (juce::Colours::white);
            g.fillRoundedRectangle (tx, ty, thumbWidth, thumbHeight, thumbHeight * 0.5f);
            
            g.setColour (activeColor);
            g.drawRoundedRectangle (tx, ty, thumbWidth, thumbHeight, thumbHeight * 0.5f, 1.5f);
        }
        else if (style == juce::Slider::LinearHorizontal)
        {
            juce::Colour activeColor = slider.findColour (juce::Slider::thumbColourId);
            if (activeColor.getAlpha() == 0)
                activeColor = juce::Colour (0xFF0DF5E3); // По умолчанию неоново-циан

            // Подложка: чуть более светлая и заметная версия цвета слайдера
            juce::Colour bgColor = activeColor.withAlpha (0.22f); 

            float trackHeight = 6.0f;
            float rx = (float)x;
            float ry = y + height * 0.5f - trackHeight * 0.5f;
            float rw = (float)width;
            float rh = trackHeight;

            // 1. Рисуем подложку (весь трек)
            g.setColour (bgColor);
            g.fillRoundedRectangle (rx, ry, rw, rh, trackHeight * 0.5f);
            
            // Тонкий контур подложки
            g.setColour (activeColor.withAlpha (0.35f));
            g.drawRoundedRectangle (rx, ry, rw, rh, trackHeight * 0.5f, 1.0f);

            // 2. Рисуем active заполнение слева направо
            float fillW = sliderPos - (float)x;
            if (fillW > 0.0f)
            {
                g.setColour (activeColor);
                g.fillRoundedRectangle (rx, ry, fillW, rh, trackHeight * 0.5f);
            }

            // 3. Рисуем бегунок (thumb)
            float thumbWidth = 6.0f;
            float thumbHeight = height * 0.75f;
            float tx = sliderPos - thumbWidth * 0.5f;
            float ty = y + height * 0.5f - thumbHeight * 0.5f;
            
            tx = juce::jlimit ((float)x, (float)(x + width - thumbWidth), tx);

            g.setColour (juce::Colours::white);
            g.fillRoundedRectangle (tx, ty, thumbWidth, thumbHeight, thumbWidth * 0.5f);
            
            g.setColour (activeColor);
            g.drawRoundedRectangle (tx, ty, thumbWidth, thumbHeight, thumbWidth * 0.5f, 1.5f);
        }
        else
        {
            juce::LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        }
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
        // Не используем shouldDrawButtonAsDown — иначе JUCE будет "темнить"
        // кнопку во время нажатия, что пользователь воспринимает как баг
        juce::ignoreUnused (shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);

        const bool isOn = button.getToggleState();

        auto buttonArea = button.getLocalBounds();
        const int boxSize = 18;
        const int boxX    = buttonArea.getX() + 2;
        const int boxY    = buttonArea.getCentreY() - boxSize / 2;

        juce::Colour neonPink = juce::Colour (0xFFFF2E93);

        if (isOn)
        {
            // ── АКТИВНОЕ состояние: яркий неоновый фон ──────────────────
            // Заливка чекбокса розовым — неизбежно заметно
            g.setColour (neonPink.withAlpha (0.85f));
            g.fillRoundedRectangle ((float)boxX, (float)boxY, (float)boxSize, (float)boxSize, 4.0f);

            // Лёгкое свечение вокруг
            g.setColour (neonPink.withAlpha (0.25f));
            g.fillRoundedRectangle ((float)boxX - 3.0f, (float)boxY - 3.0f,
                                    (float)boxSize + 6.0f, (float)boxSize + 6.0f, 6.0f);

            // Обводка
            g.setColour (neonPink);
            g.drawRoundedRectangle ((float)boxX, (float)boxY, (float)boxSize, (float)boxSize, 4.0f, 2.0f);

            // Белая жирная галочка
            g.setColour (juce::Colours::white);
            juce::Path tickPath;
            tickPath.startNewSubPath  ((float)boxX + 3.5f, (float)boxY + boxSize * 0.52f);
            tickPath.lineTo ((float)boxX + boxSize * 0.42f, (float)boxY + boxSize * 0.76f);
            tickPath.lineTo ((float)boxX + boxSize - 3.0f,  (float)boxY + 3.5f);
            g.strokePath (tickPath, juce::PathStrokeType (3.0f,
                juce::PathStrokeType::mitered, juce::PathStrokeType::rounded));
        }
        else
        {
            // ── НЕАКТИВНОЕ состояние: тёмный фон, серая рамка ───────────
            g.setColour (juce::Colour (0xFF1A1527));
            g.fillRoundedRectangle ((float)boxX, (float)boxY, (float)boxSize, (float)boxSize, 4.0f);

            g.setColour (juce::Colours::grey.withAlpha (0.5f));
            g.drawRoundedRectangle ((float)boxX, (float)boxY, (float)boxSize, (float)boxSize, 4.0f, 1.5f);
        }

        // Текст кнопки
        g.setFont (juce::Font ("Outfit", 11.5f, juce::Font::bold));
        g.setColour (isOn ? juce::Colours::white : juce::Colours::white.withAlpha (0.75f));
        const int textX     = boxX + boxSize + 8;
        const int textWidth = buttonArea.getWidth() - textX - 2;
        g.drawFittedText (button.getButtonText(), textX, 0, textWidth, buttonArea.getHeight(),
                          juce::Justification::centredLeft, 1);
    }
};
