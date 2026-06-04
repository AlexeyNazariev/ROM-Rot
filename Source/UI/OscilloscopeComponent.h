#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Utils/AudioRingBuffer.h"

/**
 * @brief Визуализатор звуковой волны (осциллограф).
 * Отображает форму аудиосигнала на выходе плагина в реальном времени.
 */
class OscilloscopeComponent : public juce::Component, public juce::Timer
{
public:
    OscilloscopeComponent(AudioRingBuffer& ringBuffer);
    ~OscilloscopeComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

private:
    AudioRingBuffer& audioRingBuffer;
    
    // Временный буфер для хранения сэмплов перед отрисовкой
    static constexpr int numDisplaySamples = 512;
    float sampleBuffer[numDisplaySamples];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscilloscopeComponent)
};
