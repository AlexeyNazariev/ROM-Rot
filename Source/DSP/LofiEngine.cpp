#include "LofiEngine.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

LofiEngine::LofiEngine()
{
    reset();
}

void LofiEngine::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;
    reverb.setSampleRate(sampleRate);
    
    // Вычисляем размер буфера для задержки Wow/Flutter (примерно 90 мс)
    // 4096 сэмплов при 44.1кГц — это около 93 мс, чего с избытком хватает для детонации ленты.
    bufferSize = 4096;
    
    delayBuffer[0].assign(bufferSize, 0.0f);
    delayBuffer[1].assign(bufferSize, 0.0f);
    
    reset();
}

void LofiEngine::reset()
{
    std::fill(delayBuffer[0].begin(), delayBuffer[0].end(), 0.0f);
    std::fill(delayBuffer[1].begin(), delayBuffer[1].end(), 0.0f);
    
    writeIndex = 0;
    wowPhase = 0.0f;
    flutterPhase = 0.0f;
    
    downsampleCounter[0] = 0.0f;
    downsampleCounter[1] = 0.0f;
    
    lastSampleValue[0] = 0.0f;
    lastSampleValue[1] = 0.0f;
    
    meltTarget = 0.0f;
    meltCurrent = 0.0f;
    meltRemainingSamples = 0;
    
    lastCrackleImpulse[0] = 0.0f;
    lastCrackleImpulse[1] = 0.0f;
    
    lastCrackleSample[0] = 0.0f;
    lastCrackleSample[1] = 0.0f;
    
    filterState[0] = 0.0f;
    filterState[1] = 0.0f;
    
    reverb.reset();
}

void LofiEngine::updateVibeParameters(float vibe, float manualSaturationDrive, float manualBitDepth, float manualSampleRateRatio) noexcept
{
    currentVibeValue = vibe;
    
    // Сглаживание и интерполяция параметров в зависимости от "эпохи" (Vibe slider)
    if (vibe <= 0.3f)
    {
        // Зона 1: Pure Chiptune (0.0) -> Lo-Fi Cassette (0.3)
        float t = vibe / 0.3f;
        
        // Линейно переходим от 8-битного звучания без детонации к чистому звуку с легкой кассетной детонацией
        bitDepth = juce::jmap(t, 8.0f, 16.0f);
        sampleRateRatio = 1.0f; // Без даунсэмплинга
        
        wowDepth = juce::jmap(t, 0.0f, 0.12f);
        wowSpeed = 1.0f;
        flutterDepth = juce::jmap(t, 0.0f, 0.08f);
        flutterSpeed = 8.0f;
        
        saturationMix = juce::jmap(t, 0.0f, 0.25f);
        saturationDrive = juce::jmap(t, 1.0f, 1.5f);
        
        noiseLevel = juce::jmap(t, 0.0f, 0.06f);
        crackleLevel = 0.0f; // Нет треска на кассете
        
        // Легкий спад высоких частот (1.0 = выключен, 0.85 = частоты срезаются)
        highShelfGain = juce::jmap(t, 1.0f, 0.85f);
    }
    else if (vibe <= 0.7f)
    {
        // Зона 2: Lo-Fi Cassette (0.3) -> VHS Retro (0.7)
        float t = (vibe - 0.3f) / 0.4f;
        
        // Небольшой биткрашинг до 12 бит, легкий даунсэмплинг
        bitDepth = juce::jmap(t, 16.0f, 12.0f);
        sampleRateRatio = juce::jmap(t, 1.0f, 0.8f);
        
        // Глубокий Wow & Flutter, появление эффекта проседания ("Melt")
        wowDepth = juce::jmap(t, 0.12f, 0.35f);
        wowSpeed = juce::jmap(t, 1.0f, 1.4f);
        flutterDepth = juce::jmap(t, 0.08f, 0.25f);
        flutterSpeed = juce::jmap(t, 8.0f, 10.0f);
        
        // Умеренное насыщение
        saturationMix = juce::jmap(t, 0.25f, 0.55f);
        saturationDrive = juce::jmap(t, 1.5f, 3.5f);
        
        // Появление шума ленты и винилового треска
        noiseLevel = juce::jmap(t, 0.06f, 0.25f);
        crackleLevel = juce::jmap(t, 0.0f, 0.35f);
        
        // Более ощутимый завал высоких частот
        highShelfGain = juce::jmap(t, 0.85f, 0.6f);
    }
    else
    {
        // Зона 3: VHS Retro (0.7) -> Dying Hardware (1.0)
        float t = (vibe - 0.7f) / 0.3f;
        
        // Экстремальное падение битности и частоты дискретизации
        bitDepth = juce::jmap(t, 12.0f, 4.0f);
        sampleRateRatio = juce::jmap(t, 0.8f, 0.18f);
        
        // Сильнейшая детонация и нестабильность
        wowDepth = juce::jmap(t, 0.35f, 0.75f);
        wowSpeed = juce::jmap(t, 1.4f, 2.2f);
        flutterDepth = juce::jmap(t, 0.25f, 0.5f);
        flutterSpeed = juce::jmap(t, 10.0f, 14.0f);
        
        // Тяжелый дисторшн
        saturationMix = juce::jmap(t, 0.55f, 0.85f);
        saturationDrive = juce::jmap(t, 3.5f, 7.0f);
        
        // Обильные шумы, треск, сильный срез ВЧ
        noiseLevel = juce::jmap(t, 0.25f, 0.55f);
        crackleLevel = juce::jmap(t, 0.35f, 0.7f);
        highShelfGain = juce::jmap(t, 0.6f, 0.3f);
    }

    // Комбинируем автоматический биткрашер с ручными параметрами
    // Используем минимум, так как меньшее значение означает большее искажение
    bitDepth = std::min(bitDepth, manualBitDepth);
    sampleRateRatio = std::min(sampleRateRatio, manualSampleRateRatio);

    // Смешиваем ручную сатурацию (saturation_drive) с кассетной/цифровой
    saturationDrive = saturationDrive + manualSaturationDrive * 4.0f;
    saturationMix = juce::jmax(saturationMix, manualSaturationDrive * 0.4f);

    // Настраиваем Horror Reverb в зависимости от Vibe
    juce::Reverb::Parameters reverbParams;
    reverbParams.roomSize = vibe * 0.92f;
    reverbParams.wetLevel = vibe * 0.45f;
    reverbParams.dryLevel = 1.0f - (vibe * 0.2f);
    reverbParams.width = 1.0f;
    reverbParams.damping = 0.4f;
    reverbParams.freezeMode = false;
    reverb.setParameters(reverbParams);
}

void LofiEngine::process(juce::AudioBuffer<float>& buffer) noexcept
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();
    
    // Обновляем фазовый шаг для LFO
    const float wowPhaseIncrement = static_cast<float>((2.0 * M_PI * wowSpeed) / currentSampleRate);
    const float flutterPhaseIncrement = static_cast<float>((2.0 * M_PI * flutterSpeed) / currentSampleRate);
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // 1. Обрабатываем логику "Melt" (случайные глубокие проседания скорости ленты)
        // Вероятность проседания пропорциональна глубине Vibe (активно после 0.4)
        if (currentVibeValue > 0.4f && meltRemainingSamples <= 0)
        {
            float meltProbability = (currentVibeValue - 0.4f) * 0.0001f; // вероятность на сэмпл
            if (randomGenerator.nextFloat() < meltProbability)
            {
                // Задаем целевую глубину проседания (в сэмплах задержки) и длительность
                meltTarget = randomGenerator.nextFloat() * 35.0f * currentVibeValue;
                meltRemainingSamples = 3000 + randomGenerator.nextInt(7001); // 70-220 мс
            }
        }
        
        // Сглаживание проседания ленты
        if (meltRemainingSamples > 0)
        {
            meltCurrent += (meltTarget - meltCurrent) * 0.002f; // медленный подъем
            --meltRemainingSamples;
            if (meltRemainingSamples == 0)
                meltTarget = 0.0f;
        }
        else
        {
            meltCurrent += (0.0f - meltCurrent) * 0.005f; // плавный возврат в норму
        }
        
        // Инкрементируем фазы LFO
        wowPhase = std::fmod(wowPhase + wowPhaseIncrement, static_cast<float>(2.0 * M_PI));
        flutterPhase = std::fmod(flutterPhase + flutterPhaseIncrement, static_cast<float>(2.0 * M_PI));
        
        // Шум убран из эффекта ROT, так как в OSC уже есть генератор шума
        float rawHiss = 0.0f;
        
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            float inSample = channelData[sample];
            
            // --- Тракт обработки сэмпла ---
            
            // Эффект 1: Wow & Flutter (Детонация высоты тона)
            float processed = processWowFlutter(inSample, channel);
            
            // Эффект 2: High-Shelf Filter (Имитация спада ВЧ кассетной головки)
            processed = processHighShelf(processed, channel);
            
            // Эффект 3: Saturation (Аналоговое насыщение ленты)
            processed = processSaturation(processed);
            
            // Эффект 4: Bitcrusher (Понижение разрешения и битности)
            processed = processBitcrusher(processed, channel);
            
            // Подмешивание виниловых щелчков
            float crackleImpulse = 0.0f;
            float crackleProb = crackleLevel * 0.00008f;
            if (randomGenerator.nextFloat() < crackleProb)
            {
                // Генерируем всплеск
                crackleImpulse = (randomGenerator.nextFloat() * 2.0f - 1.0f) * 0.25f * crackleLevel;
            }
            
            // Полосовой/высокочастотный фильтр для щелчка (чтобы он звучал мягко и "пыльно", а не как цифровой клип)
            float crackleSample = crackleImpulse - lastCrackleImpulse[channel] + 0.98f * lastCrackleSample[channel];
            lastCrackleImpulse[channel] = crackleImpulse;
            lastCrackleSample[channel] = crackleSample;
            
            // Смешиваем основной сигнал с шипением и треском
            channelData[sample] = processed + rawHiss + crackleSample;
        }
        
        // Сдвигаем индекс кольцевого буфера
        writeIndex = (writeIndex + 1) % bufferSize;
    }

    // Применяем Horror Reverb в конце
    if (numChannels >= 2)
    {
        reverb.processStereo(buffer.getWritePointer(0), buffer.getWritePointer(1), numSamples);
    }
    else if (numChannels == 1)
    {
        reverb.processMono(buffer.getWritePointer(0), numSamples);
    }
}

float LofiEngine::processWowFlutter(float input, int channel) noexcept
{
    // Записываем текущий сэмпл в буфер
    delayBuffer[channel][writeIndex] = input;
    
    // Рассчитываем модуляцию задержки
    float lfoWow = std::sin(wowPhase) * wowDepth * 18.0f;
    float lfoFlutter = std::sin(flutterPhase) * flutterDepth * 4.5f;
    
    // Добавляем органический случайный микро-дрейф к Flutter
    lfoFlutter += (randomGenerator.nextFloat() - 0.5f) * flutterDepth * 2.0f;
    
    // Общая величина задержки (30 сэмплов — опорная задержка для исключения отрицательных индексов)
    float delaySamples = 30.0f + lfoWow + lfoFlutter + meltCurrent;
    
    // Определяем позицию чтения
    float readIndex = static_cast<float>(writeIndex) - delaySamples;
    while (readIndex < 0.0f)
        readIndex += static_cast<float>(bufferSize);
        
    // Линейная интерполяция между соседними сэмплами в буфере для плавного плывущего звука без треска
    int idx0 = static_cast<int>(readIndex);
    int idx1 = (idx0 + 1) % bufferSize;
    float frac = readIndex - static_cast<float>(idx0);
    
    return delayBuffer[channel][idx0] * (1.0f - frac) + delayBuffer[channel][idx1] * frac;
}

float LofiEngine::processHighShelf(float input, int channel) noexcept
{
    // Простейший 1-полюсный сглаживающий фильтр
    // highShelfGain = 1.0 (полный обход), 0.3 (сильный срез)
    float alpha = highShelfGain;
    float output = filterState[channel] + alpha * (input - filterState[channel]);
    filterState[channel] = output;
    
    return output;
}

float LofiEngine::processSaturation(float input) noexcept
{
    // Формула: f(x) = tanh(x * drive)
    // Подмешиваем сатурированный сигнал к оригинальному
    float saturated = std::tanh(input * saturationDrive);
    
    // Компенсируем усиление, чтобы громкость не взлетала до небес при высоком drive
    float outputCompensated = saturated / std::tanh(saturationDrive);
    
    return input * (1.0f - saturationMix) + outputCompensated * saturationMix;
}

float LofiEngine::processBitcrusher(float input, int channel) noexcept
{
    // 1. Даунсэмплинг (Sample & Hold)
    // holdInterval = количество сэмплов удержания. При ratio = 1.0 это 1 (выполняется для каждого сэмпла)
    float holdInterval = 1.0f / std::max(0.01f, sampleRateRatio);
    
    downsampleCounter[channel] += 1.0f;
    if (downsampleCounter[channel] >= holdInterval)
    {
        // Сбрасываем счетчик с дробным переносом
        downsampleCounter[channel] = std::fmod(downsampleCounter[channel], holdInterval);
        lastSampleValue[channel] = input;
    }
    
    float sampleToQuantize = lastSampleValue[channel];
    
    // 2. Квантование амплитуды (Bit reduction)
    // Количество дискретных уровней для заданной битности (например, 2^8 - 1 = 255 для 8 бит)
    float levels = std::pow(2.0f, bitDepth) - 1.0f;
    
    // Квантуем значение через приведение к ближайшему уровню
    float crushed = std::round(sampleToQuantize * levels) / levels;
    
    return crushed;
}
