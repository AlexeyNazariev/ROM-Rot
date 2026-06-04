#include "OscWindow.h"

OscWindow::OscWindow(LofiChiptuneSynthAudioProcessor& p)
    : processor(p)
{
    // Тональности цвета: #0DF5E3 (циан), #FF2E93 (розовый)
    juce::Colour neonCyan = juce::Colour(0xFF0DF5E3);
    juce::Colour neonPink = juce::Colour(0xFFFF2E93);

    // Выбор формы волны
    waveSelector.addItemList(juce::StringArray{"Sine", "Triangle", "Sawtooth", "Pulse (PWM)"}, 1);
    addAndMakeVisible(waveSelector);
    addAndMakeVisible(waveLabel);
    waveLabel.setFont(juce::Font("Outfit", 13.0f, juce::Font::bold));
    waveLabel.setColour(juce::Label::textColourId, neonCyan);
    waveSelector.setMouseCursor(juce::MouseCursor::PointingHandCursor);

    // Слайдеры генератора (крутилки с подписями под ними)
    setupRotarySlider(detuneSlider,   detuneLabel,   "DETUNE",        neonCyan);
    setupRotarySlider(pwSlider,       pwLabel,       "PULSE WIDTH",   neonCyan);
    setupRotarySlider(pwmRateSlider,  pwmRateLabel,  "PWM LFO RATE",  neonCyan);
    setupRotarySlider(pwmDepthSlider, pwmDepthLabel, "PWM LFO DEPTH", neonCyan);
    setupRotarySlider(subGainSlider,  subGainLabel,  "SUB OSC",       neonCyan);
    setupRotarySlider(noiseGainSlider,noiseGainLabel,"NOISE OSC",     neonCyan);

    // Слайдеры ADSR огибающей (вертикальные).
    setupVerticalSlider(attackSlider,  attackLabel,  "ATTACK",  neonPink);
    setupVerticalSlider(decaySlider,   decayLabel,   "DECAY",   neonPink);
    setupVerticalSlider(sustainSlider, sustainLabel, "SUSTAIN", neonPink);
    setupVerticalSlider(releaseSlider, releaseLabel, "RELEASE", neonPink);

    // Связываем с APVTS процессора
    auto& apvts = processor.getAPVTS();
    
    waveAttachment      = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, "osc_wave",         waveSelector);
    detuneAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "osc_detune",       detuneSlider);
    pwAttachment        = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "pulse_width",      pwSlider);
    pwmRateAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "pwm_rate",         pwmRateSlider);
    pwmDepthAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "pwm_depth",        pwmDepthSlider);
    subGainAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "sub_gain",         subGainSlider);
    noiseGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "noise_gain",       noiseGainSlider);

    attackAttachment    = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "adsr_attack",      attackSlider);
    decayAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "adsr_decay",       decaySlider);
    sustainAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "adsr_sustain",     sustainSlider);
    releaseAttachment   = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>  (apvts, "adsr_release",     releaseSlider);
}

OscWindow::~OscWindow()
{
    detuneSlider.setLookAndFeel(nullptr);
    pwSlider.setLookAndFeel(nullptr);
    pwmRateSlider.setLookAndFeel(nullptr);
    pwmDepthSlider.setLookAndFeel(nullptr);
    subGainSlider.setLookAndFeel(nullptr);
    noiseGainSlider.setLookAndFeel(nullptr);
    attackSlider.setLookAndFeel(nullptr);
    decaySlider.setLookAndFeel(nullptr);
    sustainSlider.setLookAndFeel(nullptr);
    releaseSlider.setLookAndFeel(nullptr);
}

void OscWindow::setupRotarySlider(juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour colour)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(&customKnobLookAndFeel);
    addAndMakeVisible(slider);
    
    slider.setColour(juce::Slider::thumbColourId,             colour);
    slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xFF1A1527));
    slider.setColour(juce::Slider::rotarySliderFillColourId,  colour.withAlpha(0.6f));
    
    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font("Outfit", 11.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.8f));
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

// Версия для вертикальных слайдеров ADSR — метка добавляется как child-компонент.
void OscWindow::setupVerticalSlider(juce::Slider& slider, juce::Label& label, const juce::String& text, juce::Colour colour)
{
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    slider.setLookAndFeel(&customKnobLookAndFeel);
    addAndMakeVisible(slider);

    slider.setColour(juce::Slider::thumbColourId,      colour);
    slider.setColour(juce::Slider::trackColourId,      colour.withAlpha(0.6f));
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xFF1A1527));

    label.setText(text, juce::dontSendNotification);
    label.setFont(juce::Font("Outfit", 11.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, colour);
    label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(label);
}

void OscWindow::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Градиент заднего фона
    juce::Colour bgStart = juce::Colour(0xFF0F0D1B);
    juce::Colour bgEnd   = juce::Colour(0xFF080710);
    juce::ColourGradient grad(bgStart, 0, 0, bgEnd, 0, bounds.getHeight(), false);
    g.setGradientFill(grad);
    g.fillAll();

    // Рисуем рамки двух панелей в стиле Glassmorphism
    auto leftPanel  = bounds.removeFromLeft(bounds.getWidth() * 0.55f).reduced(12.0f);
    auto rightPanel = bounds.reduced(12.0f);

    // Фон панелей
    g.setColour(juce::Colour(0xFF1D1830).withAlpha(0.3f));
    g.fillRoundedRectangle(leftPanel, 10.0f);
    g.fillRoundedRectangle(rightPanel, 10.0f);

    // Рамки панелей с неоновым оттенком
    g.setColour(juce::Colour(0xFF0DF5E3).withAlpha(0.2f));
    g.drawRoundedRectangle(leftPanel, 10.0f, 1.5f);
    g.setColour(juce::Colour(0xFFFF2E93).withAlpha(0.2f));
    g.drawRoundedRectangle(rightPanel, 10.0f, 1.5f);

    // Заголовки панелей
    g.setColour(juce::Colour(0xFF0DF5E3));
    g.setFont(juce::Font("Outfit", 14.0f, juce::Font::bold));
    g.drawText("GENERATOR (OSC & PWM)", leftPanel.removeFromTop(25.0f).reduced(10.0f, 0.0f), juce::Justification::left, true);

    g.setColour(juce::Colour(0xFFFF2E93));
    g.drawText("AMPLITUDE ADSR ENVELOPE", rightPanel.removeFromTop(25.0f).reduced(10.0f, 0.0f), juce::Justification::left, true);

}

void OscWindow::resized()
{
    auto area = getLocalBounds();
    
    // Левая секция (генераторы)
    auto leftArea = area.removeFromLeft(static_cast<int>(area.getWidth() * 0.55f)).reduced(20);
    leftArea.removeFromTop(18); // Отступ сверху для предотвращения наложения на заголовок
    
    // Wave Selector вверху слева
    auto waveArea = leftArea.removeFromTop(45);
    waveLabel.setBounds(waveArea.removeFromLeft(130).reduced(0, 10));
    waveSelector.setBounds(waveArea.reduced(5, 8));

    // Сетка 2×3 для регуляторов генератора
    int sliderW = leftArea.getWidth()  / 3;
    int sliderH = leftArea.getHeight() / 2;
    
    auto row1  = leftArea.removeFromTop(sliderH);
    auto cell1 = row1.removeFromLeft(sliderW).reduced(6);
    detuneLabel.setBounds(cell1.removeFromBottom(15));
    detuneSlider.setBounds(cell1);
    
    auto cell2 = row1.removeFromLeft(sliderW).reduced(6);
    subGainLabel.setBounds(cell2.removeFromBottom(15));
    subGainSlider.setBounds(cell2);
    
    auto cell3 = row1.reduced(6);
    noiseGainLabel.setBounds(cell3.removeFromBottom(15));
    noiseGainSlider.setBounds(cell3);

    auto row2  = leftArea;
    auto cell4 = row2.removeFromLeft(sliderW).reduced(6);
    pwLabel.setBounds(cell4.removeFromBottom(15));
    pwSlider.setBounds(cell4);
    
    auto cell5 = row2.removeFromLeft(sliderW).reduced(6);
    pwmRateLabel.setBounds(cell5.removeFromBottom(15));
    pwmRateSlider.setBounds(cell5);
    
    auto cell6 = row2.reduced(6);
    pwmDepthLabel.setBounds(cell6.removeFromBottom(15));
    pwmDepthSlider.setBounds(cell6);

    // Правая секция (ADSR) — подписи внизу
    auto rightArea = area.reduced(25);
    rightArea.removeFromTop(30); // Отступ под заголовок (увеличен для предотвращения наложения)

    const int adsrW = rightArea.getWidth() / 4;

    // Каждая ячейка: слайдер занимает всю высоту кроме 18px снизу под подпись
    auto placeAdsr = [&](juce::Slider& s, juce::Label& l, juce::Rectangle<int> cell)
    {
        auto labelCell = cell.removeFromBottom(18); // место под подпись снизу
        l.setBounds(labelCell);
        s.setBounds(cell.reduced(5, 5));
    };

    placeAdsr(attackSlider,  attackLabel,  rightArea.removeFromLeft(adsrW));
    placeAdsr(decaySlider,   decayLabel,   rightArea.removeFromLeft(adsrW));
    placeAdsr(sustainSlider, sustainLabel, rightArea.removeFromLeft(adsrW));
    placeAdsr(releaseSlider, releaseLabel, rightArea);
}
