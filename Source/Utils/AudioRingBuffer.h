#pragma once

#include <array>
#include <atomic>
#include <algorithm>

/**
 * @brief Потокобезопасный кольцевой буфер без блокировок (Lock-free Ring Buffer).
 * Используется для передачи сэмплов из аудиопотока в графический осциллограф.
 */
class AudioRingBuffer
{
public:
    AudioRingBuffer() : writePos(0)
    {
        buffer.fill(0.0f);
    }
    
    ~AudioRingBuffer() = default;

    /**
     * @brief Запись сэмпла в буфер. Вызывается из аудиопотока (Audio Thread).
     */
    void pushSample(float sample) noexcept
    {
        int pos = writePos.load(std::memory_order_relaxed);
        buffer[pos] = sample;
        writePos.store((pos + 1) % buffer.size(), std::memory_order_release);
    }

    /**
     * @brief Чтение последних N сэмплов. Вызывается из UI потока (GUI Thread).
     */
    void readSamples(float* dest, int numSamples) noexcept
    {
        int pos = writePos.load(std::memory_order_acquire);
        
        // Находим стартовую позицию чтения (запись отстает на numSamples назад)
        int readStart = pos - numSamples;
        if (readStart < 0)
            readStart += static_cast<int>(buffer.size());
            
        for (int i = 0; i < numSamples; ++i)
        {
            dest[i] = buffer[static_cast<size_t>((readStart + i) % buffer.size())];
        }
    }

private:
    std::array<float, 2048> buffer;
    std::atomic<int> writePos;
};
