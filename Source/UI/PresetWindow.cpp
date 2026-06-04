#include "PresetWindow.h"

PresetWindow::PresetWindow(LofiChiptuneSynthAudioProcessor& p)
    : processor(p)
{
    juce::Colour neonCyan = juce::Colour(0xFF0DF5E3);
    juce::Colour neonPink = juce::Colour(0xFFFF2E93);

    // Заголовок раздела
    presetSectionTitle.setFont(juce::Font("Outfit", 16.0f, juce::Font::bold));
    presetSectionTitle.setColour(juce::Label::textColourId, neonCyan);
    presetSectionTitle.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(presetSectionTitle);

    // Настройка кнопок
    for (int i = 0; i < numPresets; ++i)
    {
        presetButtons[i].setButtonText(presetNames[i]);
        presetButtons[i].setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(presetButtons[i]);
        
        presetButtons[i].onClick = [this, i]() { loadPreset(i + 1); };

        // Стильные неоновые оттенки для кнопок пресетов
        presetButtons[i].setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF141124));
        presetButtons[i].setColour(juce::TextButton::buttonOnColourId, neonPink.withAlpha(0.7f));
        presetButtons[i].setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.85f));
        presetButtons[i].setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    }
}

void PresetWindow::loadPreset(int presetIndex)
{
    auto& apvts = processor.getAPVTS();

    auto setParam = [&](const juce::String& paramId, float value) {
        if (auto* param = apvts.getParameter(paramId))
        {
            param->setValueNotifyingHost(param->convertTo0to1(value));
        }
    };

    switch (presetIndex)
    {
        case 1: // 8 bit lead (Плотный лид с саб-басом)
            setParam("osc_wave", 3.0f);          // Pulse
            setParam("pulse_width", 0.45f);
            setParam("pwm_depth", 0.1f);
            setParam("pwm_rate", 1.5f);
            setParam("osc_detune", 0.0f);
            setParam("sub_gain", 0.45f);
            setParam("noise_gain", 0.04f);
            setParam("saturation_drive", 0.3f);
            setParam("adsr_attack", 0.005f);
            setParam("adsr_decay", 0.15f);
            setParam("adsr_sustain", 0.75f);
            setParam("adsr_release", 0.2f);
            setParam("vibe_macro", 0.15f);        // Легкий дрейф
            setParam("chord_assistant", 0.0f);   // Выкл
            break;

        case 2: // DUSTY VHS PAD (Размытая пила из 80-х в миноре)
            setParam("osc_wave", 2.0f);          // Saw
            setParam("osc_detune", 0.25f);
            setParam("sub_gain", 0.35f);
            setParam("noise_gain", 0.15f);
            setParam("saturation_drive", 0.8f);
            setParam("adsr_attack", 0.65f);       // Медленная атака
            setParam("adsr_decay", 1.2f);
            setParam("adsr_sustain", 0.8f);
            setParam("adsr_release", 1.5f);       // Длинный релиз
            setParam("vibe_macro", 0.62f);        // VHS Melt
            setParam("chord_assistant", 1.0f);   // Вкл
            setParam("pattern_type", 3.0f);      // Sustain Pad
            setParam("key_select", 0.0f);        // C
            setParam("scale_select", 1.0f);      // Minor
            break;

        case 3: // ARCADE COIN ARP (Классическое арпеджио из игровых автоматов)
            setParam("osc_wave", 3.0f);          // Pulse (Square)
            setParam("pulse_width", 0.5f);
            setParam("pwm_depth", 0.0f);
            setParam("osc_detune", 0.0f);
            setParam("sub_gain", 0.0f);
            setParam("noise_gain", 0.0f);
            setParam("saturation_drive", 0.0f);
            setParam("adsr_attack", 0.001f);
            setParam("adsr_decay", 0.08f);
            setParam("adsr_sustain", 0.0f);      // Только щелчок
            setParam("adsr_release", 0.08f);
            setParam("vibe_macro", 0.0f);         // Чистый байпас эффектов
            setParam("chord_assistant", 1.0f);   // Вкл
            setParam("pattern_type", 0.0f);      // Classic Arp (35 Гц)
            setParam("key_select", 0.0f);        // C
            setParam("scale_select", 0.0f);      // Major
            break;

        case 4: // TAPE DRIP KEYS (Плывущий кассетный треугольник)
            setParam("osc_wave", 1.0f);          // Triangle
            setParam("osc_detune", 0.15f);
            setParam("sub_gain", 0.2f);
            setParam("noise_gain", 0.08f);
            setParam("saturation_drive", 0.4f);
            setParam("adsr_attack", 0.01f);
            setParam("adsr_decay", 0.35f);
            setParam("adsr_sustain", 0.3f);
            setParam("adsr_release", 0.45f);
            setParam("vibe_macro", 0.48f);        // Кассетный флаттер
            setParam("chord_assistant", 0.0f);
            break;

        case 5: // SUB-ROT BASS (Тяжелый сатурированный бас на синусе)
            setParam("osc_wave", 0.0f);          // Sine
            setParam("osc_detune", 0.0f);
            setParam("sub_gain", 0.95f);         // Максимум саба
            setParam("noise_gain", 0.02f);
            setParam("saturation_drive", 1.1f);  // Сильная сатурация
            setParam("adsr_attack", 0.015f);
            setParam("adsr_decay", 0.25f);
            setParam("adsr_sustain", 0.7f);
            setParam("adsr_release", 0.35f);
            setParam("vibe_macro", 0.32f);
            setParam("chord_assistant", 0.0f);
            break;

        case 6: // GLITCH CRUSH (Экстремальное падение битов, скрежет)
            setParam("osc_wave", 3.0f);          // Pulse
            setParam("pulse_width", 0.15f);      // Узкий импульс
            setParam("pwm_depth", 0.75f);
            setParam("pwm_rate", 14.5f);         // Очень быстрый PWM LFO
            setParam("osc_detune", 0.5f);
            setParam("sub_gain", 0.25f);
            setParam("noise_gain", 0.35f);
            setParam("saturation_drive", 1.3f);
            setParam("adsr_attack", 0.001f);
            setParam("adsr_decay", 0.06f);
            setParam("adsr_sustain", 0.25f);
            setParam("adsr_release", 0.08f);
            setParam("vibe_macro", 0.98f);        // Dying Hardware
            setParam("chord_assistant", 0.0f);
            break;

        case 7: // HAUNTED MUSIC BOX (Медленный перебор шкатулки)
            setParam("osc_wave", 0.0f);          // Sine
            setParam("osc_detune", 0.1f);
            setParam("sub_gain", 0.0f);
            setParam("noise_gain", 0.06f);
            setParam("saturation_drive", 0.2f);
            setParam("adsr_attack", 0.004f);
            setParam("adsr_decay", 0.45f);
            setParam("adsr_sustain", 0.0f);
            setParam("adsr_release", 0.65f);
            setParam("vibe_macro", 0.52f);        // Заметное плывущее расстроение
            setParam("chord_assistant", 1.0f);   // Вкл
            setParam("pattern_type", 1.0f);      // Lo-Fi Strum (6 Гц)
            setParam("key_select", 9.0f);        // A
            setParam("scale_select", 1.0f);      // Minor
            break;

        case 8: // GHOST IN THE ROM (Мрачный фригийский хоррор-аккорд)
            setParam("osc_wave", 1.0f);          // Triangle
            setParam("osc_detune", 0.3f);
            setParam("sub_gain", 0.55f);
            setParam("noise_gain", 0.22f);
            setParam("saturation_drive", 0.9f);
            setParam("adsr_attack", 0.35f);
            setParam("adsr_decay", 1.0f);
            setParam("adsr_sustain", 0.75f);
            setParam("adsr_release", 1.2f);
            setParam("vibe_macro", 0.86f);        // Глубокое погружение в реверб
            setParam("chord_assistant", 1.0f);   // Вкл
            setParam("pattern_type", 2.0f);      // Rhythmic Chop (стробирование)
            setParam("key_select", 4.0f);        // E
            setParam("scale_select", 2.0f);      // Phrygian
            break;

        default:
            break;
    }
}

void PresetWindow::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Задний фон с градиентом
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

void PresetWindow::resized()
{
    auto area = getLocalBounds().reduced(20);
    
    presetSectionTitle.setBounds(area.removeFromTop(30));
    area.removeFromTop(10);
    
    // Сетка 2 ряда по 4 колонки
    int rowH = area.getHeight() / 2;
    int colW = area.getWidth() / 4;
    
    auto row1 = area.removeFromTop(rowH);
    for (int i = 0; i < 4; ++i)
    {
        presetButtons[i].setBounds(row1.removeFromLeft(colW).reduced(6, 6));
    }
    
    for (int i = 4; i < 8; ++i)
    {
        presetButtons[i].setBounds(area.removeFromLeft(colW).reduced(6, 6));
    }
}
