#include "SynthVoice.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

SynthVoice::SynthVoice()
{
    envelope.setSampleRate(currentSampleRate);
    reset();
}

void SynthVoice::prepare(double sampleRate) noexcept
{
    currentSampleRate = sampleRate;
    envelope.setSampleRate(currentSampleRate);
    reset();
}

void SynthVoice::reset() noexcept
{
    for (int i = 0; i < 3; ++i)
    {
        phases[i] = 0.0f;
        subPhases[i] = 0.0f;
        pwmPhases[i] = 0.0f;
    }
    chopLfoPhase = 0.0f;
    active = false;
    currentBaseNote = -1;
    noteVelocity = 0.0f;
    arpSampleCounter = 0.0f;
    currentArpIndex = 0;
    envelope.reset();
}

void SynthVoice::noteOn(int midiNoteNumber, float velocity, int keySelect, int scaleSelect, bool chordAssistantActive) noexcept
{
    currentBaseNote = midiNoteNumber;
    noteVelocity = velocity;
    isChordAssistantActive = chordAssistantActive;
    
    // Сброс фаз при перезапуске ноты (legato / retrigger)
    for (int i = 0; i < 3; ++i)
    {
        phases[i] = 0.0f;
        subPhases[i] = 0.0f;
        pwmPhases[i] = 0.0f;
    }
    chopLfoPhase = 0.0f;
    arpSampleCounter = 0.0f;
    currentArpIndex = 0;
    
    // Рассчитываем ноты аккорда, если активирован Chord Assistant
    calculateChordNotes(midiNoteNumber, keySelect, scaleSelect, activeChordNotes);
    
    // Запускаем огибающую
    envelope.noteOn();
    active = true;
}

void SynthVoice::noteOff() noexcept
{
    envelope.noteOff();
}

bool SynthVoice::isActive() const noexcept
{
    return active && envelope.isActive();
}

void SynthVoice::setParameters(int waveType, float pulseWidth, float pwmRate, float pwmDepth,
                               float attack, float decay, float sustain, float release, float detune,
                               float subGain, float noiseGain, int patternType) noexcept
{
    currentWaveType = waveType;
    basePulseWidth = pulseWidth;
    pwmLfoRate = pwmRate;
    pwmLfoDepth = pwmDepth;
    oscillatorDetune = detune;
    currentSubGain = subGain;
    currentNoiseGain = noiseGain;
    currentPatternType = patternType;
    
    envelopeParams.attack = attack;
    envelopeParams.decay = decay;
    envelopeParams.sustain = sustain;
    envelopeParams.release = release;
    envelope.setParameters(envelopeParams);
}

float SynthVoice::getFreqFromMidi(float midiNote) const noexcept
{
    // Стандартная формула перевода MIDI ноты в частоту (Гц)
    return 440.0f * std::pow(2.0f, (midiNote - 69.0f) / 12.0f);
}

void SynthVoice::calculateChordNotes(int baseNote, int key, int scale, int (&chordNotes)[3]) noexcept
{
    chordNotes[0] = baseNote;
    
    // Определяем полутона сыгранной ноты относительно тоники (key)
    int semitonesFromKey = (baseNote - key) % 12;
    if (semitonesFromKey < 0)
        semitonesFromKey += 12;
        
    // Описание диатонических интервалов гамм в полутонах
    // Мажор (Major): I(0), II(2), III(4), IV(5), V(7), VI(9), VII(11)
    int majorScale[7] = { 0, 2, 4, 5, 7, 9, 11 };
    
    // Натуральный минор (Minor): I(0), II(2), III(3), IV(5), V(7), VI(8), VII(10)
    int minorScale[7] = { 0, 2, 3, 5, 7, 8, 10 };
    
    // Фригийский лад (Phrygian): I(0), II(1), III(3), IV(5), V(7), VI(8), VII(10)
    int phrygianScale[7] = { 0, 1, 3, 5, 7, 8, 10 };
    
    const int* scalePattern = majorScale;
    if (scale == 1)
        scalePattern = minorScale;
    else if (scale == 2)
        scalePattern = phrygianScale;
        
    // Ищем, на какой ступени гаммы находится сыгранная нота
    int rootDegree = -1;
    for (int i = 0; i < 7; ++i)
    {
        if (semitonesFromKey == scalePattern[i])
        {
            rootDegree = i;
            break;
        }
    }
    
    // Если нота входит в выбранную гамму, строим чистое диатоническое трезвучие (I-III-V ступени)
    if (rootDegree != -1)
    {
        // 3-я и 5-я ступени аккорда
        int deg3 = (rootDegree + 2) % 7;
        int deg5 = (rootDegree + 4) % 7;
        
        // Определение сдвига октавы при переходе через границу гаммы
        int octave3 = (rootDegree + 2) / 7;
        int octave5 = (rootDegree + 4) / 7;
        
        int semitones3 = scalePattern[deg3] + octave3 * 12;
        int semitones5 = scalePattern[deg5] + octave5 * 12;
        
        // Добавляем к базовой ноте интервалы третьей и пятой ступеней
        chordNotes[1] = baseNote + (semitones3 - semitonesFromKey);
        chordNotes[2] = baseNote + (semitones5 - semitonesFromKey);
    }
    else
    {
        // Если нота хроматическая (вне гаммы), строим стандартные интервалы
        if (scale == 0)
        {
            // Мажорное трезвучие (+4, +7 полутонов)
            chordNotes[1] = baseNote + 4;
            chordNotes[2] = baseNote + 7;
        }
        else
        {
            // Минорное трезвучие (+3, +7 полутонов)
            chordNotes[1] = baseNote + 3;
            chordNotes[2] = baseNote + 7;
        }
    }
}

float SynthVoice::processNextSample() noexcept
{
    if (!active)
        return 0.0f;
        
    // 1. Логика арпеджиатора Chord Assistant для монофонических паттернов
    if (isChordAssistantActive)
    {
        if (currentPatternType == 0 || currentPatternType == 1)
        {
            float targetArpSpeed = (currentPatternType == 0) ? 35.0f : 6.0f;
            float arpPeriod = static_cast<float>(currentSampleRate / targetArpSpeed);
            
            arpSampleCounter += 1.0f;
            if (arpSampleCounter >= arpPeriod)
            {
                arpSampleCounter = std::fmod(arpSampleCounter, arpPeriod);
                currentArpIndex = (currentArpIndex + 1) % 3;
            }
        }
    }
    
    int numActiveNotes = 1;
    float targetMidiNotes[3] = { 0.0f, 0.0f, 0.0f };

    if (isChordAssistantActive)
    {
        if (currentPatternType == 0 || currentPatternType == 1)
        {
            numActiveNotes = 1;
            targetMidiNotes[0] = static_cast<float>(activeChordNotes[currentArpIndex]);
        }
        else // RhythmicChop (2) или SustainPad (3)
        {
            numActiveNotes = 3;
            targetMidiNotes[0] = static_cast<float>(activeChordNotes[0]);
            targetMidiNotes[1] = static_cast<float>(activeChordNotes[1]);
            targetMidiNotes[2] = static_cast<float>(activeChordNotes[2]);
        }
    }
    else
    {
        numActiveNotes = 1;
        targetMidiNotes[0] = static_cast<float>(currentBaseNote);
    }
    
    float combinedMainWave = 0.0f;
    float combinedSubWave = 0.0f;

    for (int k = 0; k < numActiveNotes; ++k)
    {
        float freq = getFreqFromMidi(targetMidiNotes[k] + oscillatorDetune);
        float phaseIncrement = freq / static_cast<float>(currentSampleRate);
        
        phases[k] = std::fmod(phases[k] + phaseIncrement, 1.0f);
        subPhases[k] = std::fmod(subPhases[k] + phaseIncrement * 0.5f, 1.0f);

        // Генерация основной формы волны для фазы k
        float waveValue = 0.0f;
        switch (currentWaveType)
        {
            case 0: // Sine
            {
                waveValue = std::sin(phases[k] * 2.0f * M_PI);
                break;
            }
            case 1: // Triangle (NES RP2A03 16-step)
            {
                float tri = phases[k] < 0.5f ? (4.0f * phases[k] - 1.0f) : (3.0f - 4.0f * phases[k]);
                waveValue = std::round(tri * 8.0f) / 8.0f;
                break;
            }
            case 2: // Saw
            {
                waveValue = 2.0f * phases[k] - 1.0f;
                break;
            }
            case 3: // Pulse / PWM
            {
                float pwmIncrement = pwmLfoRate / static_cast<float>(currentSampleRate);
                pwmPhases[k] = std::fmod(pwmPhases[k] + pwmIncrement, 1.0f);
                float lfoVal = std::sin(pwmPhases[k] * 2.0f * M_PI);
                float currentPW = basePulseWidth + lfoVal * pwmLfoDepth * 0.4f;
                currentPW = juce::jlimit(0.05f, 0.95f, currentPW);
                waveValue = phases[k] < currentPW ? 1.0f : -1.0f;
                break;
            }
            default:
                break;
        }

        combinedMainWave += waveValue;
        combinedSubWave += std::sin(subPhases[k] * 2.0f * M_PI);
    }

    combinedMainWave /= static_cast<float>(numActiveNotes);
    combinedSubWave /= static_cast<float>(numActiveNotes);

    // Генератор шума
    float noiseWave = noiseGenerator.nextFloat() * 2.0f - 1.0f;

    // Смешиваем основную волну, саб-осциллятор и генератор шума
    float finalSample = combinedMainWave + (combinedSubWave * currentSubGain) + (noiseWave * currentNoiseGain);

    // Логика стробирования для RhythmicChop (2) - 4 Гц LFO прямоугольной формы
    if (isChordAssistantActive && currentPatternType == 2)
    {
        float chopIncrement = 4.0f / static_cast<float>(currentSampleRate);
        chopLfoPhase = std::fmod(chopLfoPhase + chopIncrement, 1.0f);
        float gate = (chopLfoPhase < 0.5f) ? 1.0f : 0.0f;
        finalSample *= gate;
    }

    // Обработка огибающей ADSR
    float envVal = envelope.getNextSample();
    
    if (!envelope.isActive())
    {
        active = false;
        currentBaseNote = -1;
    }
    
    return finalSample * envVal * noteVelocity * 0.25f;
}

void SynthVoice::processBlock(juce::AudioBuffer<float>& buffer, int startSample, int numSamples) noexcept
{
    if (!active)
        return;
        
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float synthVal = processNextSample();
        
        // Добавляем к текущему контенту буфера (поскольку голоса полифоничны)
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
        {
            buffer.addSample(channel, startSample + sample, synthVal);
        }
    }
}
