#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include "WindowStateManager.h"
#include "DSP/SynthVoice.h"
#include "DSP/LofiEngine.h"
#include "Utils/AudioRingBuffer.h"

/**
 * @brief Главный аудиопроцессор (хост DSP и параметров плагина).
 * Наследуется от juce::AudioProcessor и управляет состоянием синтезатора и эффектов.
 */
class LofiChiptuneSynthAudioProcessor : public juce::AudioProcessor
{
public:
    LofiChiptuneSynthAudioProcessor();
    ~LofiChiptuneSynthAudioProcessor() override;

    // --- Методы инициализации и подготовки DSP ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    // --- Редактор плагина (GUI) ---
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // --- Метаданные плагина ---
    const juce::String getName() const override { return "Lo-Fi Chiptune Synth"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // --- Пресеты и состояние APVTS ---
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int index) override { juce::ignoreUnused(index); }
    const juce::String getProgramName (int index) override { juce::ignoreUnused(index); return {}; }
    void changeProgramName (int index, const juce::String& newName) override { juce::ignoreUnused(index, newName); }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- Доступ к модулям для GUI ---
    WindowStateManager& getWindowStateManager() noexcept { return windowStateManager; }
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts; }
    AudioRingBuffer& getRingBuffer() noexcept { return ringBuffer; }

    /**
     * @brief Запускает звучание определенной ноты (используется для воспроизведения с виртуальной клавиатуры в UI).
     */
    void triggerNoteOn (int midiNote, float velocity) noexcept;
    void triggerNoteOff (int midiNote) noexcept;

private:
    /**
     * @brief Создает и возвращает структуру параметров APVTS.
     */
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    // --- Потокобезопасные модули управления ---
    WindowStateManager windowStateManager;
    juce::AudioProcessorValueTreeState apvts;

    // --- Движок синтезатора ---
    static constexpr int numVoices = 8;
    SynthVoice voices[numVoices];
    
    // --- Логика голосового распределителя (Voice Allocator) ---
    int lastAssignedVoiceIndex = 0;

    // --- Движок Lo-Fi эффектов ---
    LofiEngine lofiEngine;

    // --- Кольцевой буфер для осциллографа ---
    AudioRingBuffer ringBuffer;

    // Кешированные атомарные указатели параметров для быстрого чтения без блокировок в аудиопотоке
    std::atomic<float>* vibeMacroParameter = nullptr;
    std::atomic<float>* masterVolumeParameter = nullptr;
    
    std::atomic<float>* oscWaveParameter = nullptr;
    std::atomic<float>* pulseWidthParameter = nullptr;
    std::atomic<float>* pwmRateParameter = nullptr;
    std::atomic<float>* pwmDepthParameter = nullptr;
    std::atomic<float>* oscDetuneParameter = nullptr;
    
    std::atomic<float>* adsrAttackParameter = nullptr;
    std::atomic<float>* adsrDecayParameter = nullptr;
    std::atomic<float>* adsrSustainParameter = nullptr;
    std::atomic<float>* adsrReleaseParameter = nullptr;
    
    std::atomic<float>* keySelectParameter = nullptr;
    std::atomic<float>* scaleSelectParameter = nullptr;
    std::atomic<float>* chordAssistantParameter = nullptr;

    // Новые параметры v2
    std::atomic<float>* subGainParameter = nullptr;
    std::atomic<float>* noiseGainParameter = nullptr;
    std::atomic<float>* saturationDriveParameter = nullptr;
    std::atomic<float>* patternTypeParameter = nullptr;
    std::atomic<float>* bitDepthParameter = nullptr;
    std::atomic<float>* sampleRateRatioParameter = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LofiChiptuneSynthAudioProcessor)
};
