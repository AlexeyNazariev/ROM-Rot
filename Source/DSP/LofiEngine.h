#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

/**
 * @brief Класс LofiEngine реализует звуковой процессор для создания эффектов Lo-Fi деградации.
 * Содержит в себе: Bitcrusher (бит-крашер), Wow & Flutter (плывущий тон магнитофона),
 * Saturation (ленточное насыщение), Tape Hiss и Vinyl Crackle (шум ленты и щелчки винила).
 * 
 * Все операции оптимизированы для выполнения в реальном времени в аудиопотоке.
 * Никаких аллокаций памяти внутри processBlock().
 */
class LofiEngine
{
public:
    LofiEngine();
    ~LofiEngine() = default;

    /**
     * @brief Инициализация процессора. Вызывается перед воспроизведением в prepareToPlay.
     * Выполняет аллокацию памяти под буфер задержки.
     */
    void prepare(double sampleRate);

    /**
     * @brief Полный сброс внутренних состояний процессора.
     */
    void reset();

    /**
     * @brief Вычисление промежуточных параметров эффектов на основе глобального макро-слайдера Vibe, ручной сатурации и ручного биткрашера.
     * @param vibe Значение макро-слайдера от 0.0f до 1.0f
     * @param manualSaturationDrive Ручной коэффициент сатурации [0.0, 2.0]
     * @param manualBitDepth Ручная битность [2.0, 16.0]
     * @param manualSampleRateRatio Ручной коэффициент даунсэмплинга [0.01, 1.0]
     */
    void updateVibeParameters(float vibe, float manualSaturationDrive, float manualBitDepth, float manualSampleRateRatio) noexcept;

    /**
     * @brief Обработка блока аудиоданных в реальном времени.
     */
    void process(juce::AudioBuffer<float>& buffer) noexcept;

private:
    // --- Внутренние методы DSP ---
    
    /**
     * @brief Эмуляция биткрашера (квантование амплитуды и даунсэмплинг).
     */
    float processBitcrusher(float input, int channel) noexcept;

    /**
     * @brief Эмуляция плывущей ленты (Wow, Flutter, Melt).
     */
    float processWowFlutter(float input, int channel) noexcept;

    /**
     * @brief Ленточное насыщение (софт-клиппинг).
     */
    float processSaturation(float input) noexcept;

    /**
     * @brief Простой шельфовый фильтр высоких частот (High-Shelf) для имитации частотной характеристики пленки.
     */
    float processHighShelf(float input, int channel) noexcept;

    // --- Параметры DSP (рассчитываются через updateVibeParameters) ---
    float currentVibeValue = 0.0f;
    
    float bitDepth = 16.0f;          // Битность квантования [2.0f, 16.0f]
    float sampleRateRatio = 1.0f;     // Коэффициент даунсэмплинга [0.01f, 1.0f]
    
    float wowDepth = 0.0f;           // Глубина медленной модуляции Wow
    float wowSpeed = 1.0f;           // Частота Wow LFO (в Гц)
    float flutterDepth = 0.0f;       // Глубина быстрого дрожания Flutter
    float flutterSpeed = 8.0f;       // Частота Flutter LFO (в Гц)
    
    float saturationDrive = 1.0f;    // Коэффициент насыщения [1.0f, 10.0f]
    float saturationMix = 0.0f;      // Степень подмешивания насыщенного сигнала
    
    float noiseLevel = 0.0f;         // Громкость шипения кассеты
    float crackleLevel = 0.0f;       // Громкость виниловых щелчков
    float highShelfGain = 1.0f;      // Ослабление высоких частот для симуляции пленки (1.0 = без ослабления)

    // --- Переменные состояния DSP ---
    double currentSampleRate = 44100.0;
    
    // Состояния Bitcrusher
    float downsampleCounter[2] = { 0.0f, 0.0f };
    float lastSampleValue[2] = { 0.0f, 0.0f };
    
    // Состояния Wow & Flutter (Кольцевой буфер)
    std::vector<float> delayBuffer[2];
    int writeIndex = 0;
    int bufferSize = 4096;
    
    // Фазы LFO
    float wowPhase = 0.0f;
    float flutterPhase = 0.0f;
    
    // Логика эффекта проседания ленты "Melt"
    juce::Random randomGenerator;
    float meltTarget = 0.0f;
    float meltCurrent = 0.0f;
    int meltRemainingSamples = 0;

    // Состояния шума
    float lastCrackleImpulse[2] = { 0.0f, 0.0f };
    float lastCrackleSample[2] = { 0.0f, 0.0f };

    // Состояния High-Shelf фильтра (1-й порядок IIR)
    float filterState[2] = { 0.0f, 0.0f };
    
    // Модуль Horror Reverb
    juce::Reverb reverb;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LofiEngine)
};
