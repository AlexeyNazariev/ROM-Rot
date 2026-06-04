#pragma once

#include <atomic>

/**
 * @brief Перечисление состояний отображаемого окна плагина.
 */
enum class WindowState
{
    OSC,        // Панель осцилляторов и огибающей ADSR
    FX,         // Панель Lo-Fi эффектов деградации
    CHORDS,     // Панель Chord Assistant и паттернов
    ROM_EXPORT, // Панель экспорта кода ROM (PICO-8 / NES ASM)
    PRESETS     // Панель браузера пресетов
};

/**
 * @brief Потокобезопасный контроллер переключения окон интерфейса.
 * Обеспечивает безопасное чтение и запись состояния из аудиопотока и UI потока без блокировок.
 */
class WindowStateManager
{
public:
    WindowStateManager() : activeState(WindowState::OSC) {}
    ~WindowStateManager() = default;

    /**
     * @brief Устанавливает новое состояние окна.
     * Используется упорядочивание памяти release для гарантированной передачи изменений.
     */
    void setWindowState(WindowState newState) noexcept
    {
        activeState.store(newState, std::memory_order_release);
    }

    /**
     * @brief Возвращает текущее активное окно.
     * Используется упорядочивание памяти acquire для получения самого свежего состояния.
     */
    WindowState getWindowState() const noexcept
    {
        return activeState.load(std::memory_order_acquire);
    }

private:
    std::atomic<WindowState> activeState;
};
