#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

/**
 * @brief Класс SynthVoice реализует один синтезаторный голос.
 * Особенности:
 * - 8-битные осцилляторы (Square с PWM, stepped Triangle, Saw, Noise).
 * - Огибающая ADSR.
 * - Встроенный сверхбыстрый арпеджиатор (Chord Assistant), работающий в реальном времени.
 */
class SynthVoice
{
public:
    SynthVoice();
    ~SynthVoice() = default;

    /**
     * @brief Настройка частоты дискретизации.
     */
    void prepare(double sampleRate) noexcept;

    /**
     * @brief Полный сброс фаз и огибающей.
     */
    void reset() noexcept;

    /**
     * @brief Запуск проигрывания ноты.
     * @param midiNoteNumber Базовая MIDI нота
     * @param velocity Сила нажатия
     * @param keySelect Выбранная тональность (0-11)
     * @param scaleSelect Выбранная гамма (0-2)
     * @param chordAssistantActive Включен ли автоаккорд
     */
    void noteOn(int midiNoteNumber, float velocity, int keySelect, int scaleSelect, bool chordAssistantActive) noexcept;

    /**
     * @brief Прекращение проигрывания ноты (переход во фазу Release).
     */
    void noteOff() noexcept;

    /**
     * @brief Возвращает true, если голос сейчас звучит (активна огибающая).
     */
    bool isActive() const noexcept;

    /**
     * @brief Возвращает MIDI ноту, которая запустила этот голос.
     */
    int getMidiNote() const noexcept { return currentBaseNote; }

    /**
     * @brief Установка текущих параметров синтеза.
     * @param waveType Форма волны (0: Sine, 1: Triangle, 2: Saw, 3: Pulse/PWM)
     * @param pulseWidth Ширина импульса для Pulse волны [0.05, 0.95]
     * @param pwmRate Частота модуляции ширины импульса LFO (в Гц)
     * @param pwmDepth Глубина модуляции ширины импульса [0.0, 1.0]
     * @param attack Время атаки (в секундах)
     * @param decay Время спада (в секундах)
     * @param sustain Уровень сустейна [0.0, 1.0]
     * @param release Время релиза (в секундах)
     * @param detune Расстройка осциллятора (в полутонах)
     * @param subGain Уровень саб-осциллятора [0.0, 1.0]
     * @param noiseGain Уровень генератора шума [0.0, 1.0]
     * @param patternType Тип паттерна аккорда (0-3)
     */
    void setParameters(int waveType, float pulseWidth, float pwmRate, float pwmDepth,
                       float attack, float decay, float sustain, float release, float detune,
                       float subGain, float noiseGain, int patternType) noexcept;

    /**
     * @brief Генерация следующего сэмпла для голоса.
     */
    float processNextSample() noexcept;

    /**
     * @brief Накопление сэмплов голоса в аудио буфере.
     */
    void processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept;

private:
    /**
     * @brief Расчет частоты по MIDI ноте с учетом расстройки.
     */
    float getFreqFromMidi(float midiNote) const noexcept;

    /**
     * @brief Вспомогательный метод расчета 3-х нот трезвучия (аккорда) на основе выбранной тональности и гаммы.
     */
    void calculateChordNotes(int baseNote, int key, int scale, int (&chordNotes)[3]) noexcept;

    // --- Параметры синтеза ---
    int currentWaveType = 3;
    float basePulseWidth = 0.5f;
    float pwmLfoRate = 2.0f;
    float pwmLfoDepth = 0.0f;
    float oscillatorDetune = 0.0f; // расстройка в полутонах
    float currentSubGain = 0.0f;
    float currentNoiseGain = 0.0f;
    int currentPatternType = 3; // SustainPad по умолчанию
    
    // --- Состояния голоса ---
    double currentSampleRate = 44100.0;
    bool active = false;
    int currentBaseNote = -1;
    float noteVelocity = 0.0f;
    
    // Фазы трех осцилляторов (для одновременного воспроизведения аккорда)
    float phases[3] = { 0.0f, 0.0f, 0.0f };
    float subPhases[3] = { 0.0f, 0.0f, 0.0f };
    
    // Фазы LFO для PWM
    float pwmPhases[3] = { 0.0f, 0.0f, 0.0f };
    
    // Фаза LFO для RhythmicChop (4 Гц)
    float chopLfoPhase = 0.0f;
    
    // Огибающая громкости
    juce::ADSR envelope;
    juce::ADSR::Parameters envelopeParams;

    // --- Состояния Chord Assistant ---
    bool isChordAssistantActive = false;
    int activeChordNotes[3] = { 0, 0, 0 };
    
    // Параметры арпеджиатора
    float arpSpeedHz = 35.0f; // скорость арпеджио (циклы в секунду)
    float arpSampleCounter = 0.0f;
    int currentArpIndex = 0;
    
    // Генератор шума для ретро-шумового осциллятора
    juce::Random noiseGenerator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthVoice)
};
