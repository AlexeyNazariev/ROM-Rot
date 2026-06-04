#include "TopNavBarComponent.h"

TopNavBarComponent::TopNavBarComponent(LofiChiptuneSynthAudioProcessor& p)
    : processor(p)
{
    // Настройка кнопок навигации
#if JucePlugin_IsSynth
    auto setupTabButton = [this](juce::TextButton& btn, WindowState state) {
        btn.setMouseCursor(juce::MouseCursor::PointingHandCursor);
        addAndMakeVisible(btn);
        btn.onClick = [this, state]() { selectTab(state); };
    };

    setupTabButton(oscTabButton, WindowState::OSC);
    setupTabButton(fxTabButton, WindowState::FX);
    setupTabButton(chordsTabButton, WindowState::CHORDS);
    setupTabButton(romExportTabButton, WindowState::ROM_EXPORT);
    setupTabButton(presetTabButton, WindowState::PRESETS);
#else
    titleLabel.setText("ROM-Rot (FX)", juce::dontSendNotification);
    titleLabel.setFont(juce::Font("Outfit", 16.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xFF0DF5E3));
    addAndMakeVisible(titleLabel);
#endif

    // Настройка кнопки NS (Nazariev-S / Лицензия)
    nsButton.setMouseCursor(juce::MouseCursor::PointingHandCursor);
    addAndMakeVisible(nsButton);
    nsButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFFFF2E93).withAlpha(0.2f));
    nsButton.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFFFF2E93).withAlpha(0.45f));
    nsButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    nsButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    nsButton.onClick = [this]() { showAboutWindow(); };
}

void TopNavBarComponent::selectTab(WindowState state)
{
    processor.getWindowStateManager().setWindowState(state);
    repaint();
    
    // Оповещаем PluginEditor о необходимости перерисовки окон
    if (auto* parent = getParentComponent())
    {
        parent->resized();
        parent->repaint();
    }
}

void TopNavBarComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Градиентный темно-кассетный фон
    juce::Colour bgStart = juce::Colour(0xFF0D0B14);
    juce::Colour bgEnd   = juce::Colour(0xFF151122);
    juce::ColourGradient grad(bgStart, 0, 0, bgEnd, 0, bounds.getHeight(), false);
    g.setGradientFill(grad);
    g.fillAll();

    // Неоновая розовая разделительная полоса внизу
    g.setColour(juce::Colour(0xFFFF2E93).withAlpha(0.6f));
    g.drawLine(0.0f, bounds.getHeight() - 1.5f, bounds.getWidth(), bounds.getHeight() - 1.5f, 1.5f);

    // Подсветка активного таба
#if JucePlugin_IsSynth
    auto activeState = processor.getWindowStateManager().getWindowState();
#endif
    
    auto highlightTab = [&](juce::TextButton& btn, bool isActive) {
        if (isActive)
        {
            // Неоновый фон для активной вкладки
            btn.setColour(juce::TextButton::buttonColourId, juce::Colour(0xFF0DF5E3).withAlpha(0.18f));
            btn.setColour(juce::TextButton::buttonOnColourId, juce::Colour(0xFF0DF5E3).withAlpha(0.22f));
            // Принудительно ставим чистый белый цвет для текста
            btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
            btn.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            
            // Светящаяся полоска снизу таба
            g.setColour(juce::Colour(0xFF0DF5E3));
            g.fillRect(btn.getX(), btn.getBottom() - 3, btn.getWidth(), 3);
        }
        else
        {
            // Прозрачный фон для неактивной вкладки
            btn.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
            btn.setColour(juce::TextButton::buttonOnColourId, juce::Colours::transparentBlack);
            // Полупрозрачный текст для неактивной вкладки
            btn.setColour(juce::TextButton::textColourOffId, juce::Colours::white.withAlpha(0.6f));
            btn.setColour(juce::TextButton::textColourOnId, juce::Colours::white.withAlpha(0.6f));
        }
    };

#if JucePlugin_IsSynth
    highlightTab(oscTabButton, activeState == WindowState::OSC);
    highlightTab(fxTabButton, activeState == WindowState::FX);
    highlightTab(chordsTabButton, activeState == WindowState::CHORDS);
    highlightTab(romExportTabButton, activeState == WindowState::ROM_EXPORT);
    highlightTab(presetTabButton, activeState == WindowState::PRESETS);
#endif
}

void TopNavBarComponent::resized()
{
    auto area = getLocalBounds();
    const int margin = 8;
    
    // Берем уменьшенную область для кнопок
    auto tabArea = area.reduced(10, margin);
    
    // Кнопка NS справа (40 пикселей)
    int nsBtnWidth = 40;
    nsButton.setBounds(tabArea.removeFromRight(nsBtnWidth).reduced(2, 0));
    
    // Оставшаяся область делится поровну между вкладками
#if JucePlugin_IsSynth
    int numButtons = 5;
    int btnWidth = tabArea.getWidth() / numButtons;
    
    oscTabButton.setBounds(tabArea.removeFromLeft(btnWidth).reduced(3, 0));
    fxTabButton.setBounds(tabArea.removeFromLeft(btnWidth).reduced(3, 0));
    chordsTabButton.setBounds(tabArea.removeFromLeft(btnWidth).reduced(3, 0));
    romExportTabButton.setBounds(tabArea.removeFromLeft(btnWidth).reduced(3, 0));
    presetTabButton.setBounds(tabArea.reduced(3, 0));
#else
    titleLabel.setBounds(tabArea.removeFromLeft(200));
#endif
}

void TopNavBarComponent::showAboutWindow()
{
    juce::AlertWindow::showOkCancelBox (
        juce::AlertWindow::InfoIcon,
        "About ROM-Rot",
        "ROM-Rot Synthesizer v2.5\nCreated by Nazariev-S\n\nLicense: MIT License\nCopyright (c) 2026 Alexey Nazariev\n\nPermission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the \"Software\"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:\n\nThe above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.\n\nTHE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.",
        "Visit GitHub",
        "OK",
        this,
        juce::ModalCallbackFunction::create ([] (int result) {
            if (result != 0)
            {
                juce::URL("https://github.com/AlexeyNazariev").launchInDefaultBrowser();
            }
        })
    );
}
