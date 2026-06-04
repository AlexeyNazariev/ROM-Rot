# ROM-Rot v2.5 — Hybrid Chiptune Synth & Tape Lo-Fi Processor

[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)
[![Language: C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Framework: JUCE 7](https://img.shields.io/badge/Framework-JUCE%207.0.12-orange.svg)](https://juce.com/)

**ROM-Rot v2.5** is a cross-platform (VST3/AU) hybrid plug-in synthesizer and audio degradation effects processor. It seamlessly combines 8-bit wavetable synthesis (Chiptune) with analog hardware emulation (Tape Lo-Fi) and advanced digital downsampling algorithms.

The plug-in architecture is divided into 5 independent control windows (tabs) featuring a continuous stereo audio stream, a global **ROT** macro controller, and an integrated real-time oscilloscope. A standout feature is the **Retro ROM Exporter** terminal, which translates the current sound parameters into functional source code for retro hardware and virtual consoles (PICO-8, NES, SEGA, GameBoy).

---

## 🚀 Key Features

* **Dual Target Architecture (Synth & FX):** Operates either as a fully polyphonic instrument with MIDI input or as a standalone insert effect for processing external audio.
* **Hybrid DSP Engine:** Authentic discrete approximation of retro hardware waveforms combined with physical modeling of magnetic tape degradation.
* **Intelligent Chord Assistant:** Automatic scale quantization (Scale Lock) and retro arpeggio generation mimicking classic hardware limitations.
* **Low-Level Code Export:** Instantly translate your sound patches into compilation-ready C, Lua, or 6502/Z80 assembly code.
* **Lock-Free Architecture:** Thread-safe real-time audio processing completely isolated from the UI thread without using mutexes.

---

## 🛠 Technical Architecture & DSP Specifications

### 1. Thread Safety & Dual Target Routing
Developed in **C++20** utilizing the **JUCE 7.0.12** framework[cite: 1]. The build system is managed via CMake, configuring two distinct plugin targets that share a core codebase[cite: 1]:
* `LofiChiptuneSynth` (with `IS_SYNTH TRUE`) — A polyphonic synthesizer instrument[cite: 1].
* `LofiChiptuneSynthFX` (with `IS_SYNTH FALSE`) — A stereo audio effects processor[cite: 1].

To ensure strict real-time safety, communication between the UI thread and the Audio Thread is completely **lock-free**[cite: 1]:
* Automation parameters are bound to `juce::AudioProcessorValueTreeState` (APVTS)[cite: 1].
* Inside `processBlock()`, parameter pointers are cached as `std::atomic<float>*` and read using relaxed memory ordering (`std::memory_order_relaxed`)[cite: 1].
* UI tab switching is handled by a custom `WindowStateManager` using atomic variables with `release` / `acquire` ordering synchronization[cite: 1].

### 2. Synthesis Engine (OSC Tab)
Handled via `SynthVoice` and `LofiChiptuneSynthAudioProcessor` classes[cite: 1].
* **Polyphony:** Supports up to 8 independent voices allocated cyclically[cite: 1]. Features a custom *voice stealing* algorithm that smoothly fades out the oldest active voice to completely eliminate audio clicks[cite: 1].
* **Oscillator Waveforms:**
    * `Sine`: A mathematically pure sine wave with zero harmonic distortion, ideal for warm low-end control and sub-basses.
    * `Triangle`: An authentic 16-step discrete approximation of a triangle wave[cite: 1]. Instead of a smoothed linear ramp, the algorithm quantizes the waveform amplitude into 16 fixed levels[cite: 1]. This perfectly emulates the hardware constraints and the distinct "woody", clicking character of the Ricoh 2A03 sound chip (NES/Dendy)[cite: 1].
    * `Sawtooth`: A linear sawtooth wave containing a full spectrum of even and odd harmonics for aggressive cyber-chiptune, Witch House, and Synthwave leads.
    * `Pulse (PWM)`: A rectangular wave with dynamic Pulse Width Modulation, extended by manual `PULSE WIDTH` offset (50% down to 12.5%) and a dedicated internal PWM LFO (`pwm_rate` and `pwm_depth`)[cite: 1].
* **Synthesis Utility Controls:**
    * `DETUNE`: Micro-pitch detuning between oscillator phases for wide analog unison and stereo widening[cite: 1].
    * `SUB OSC`: Mixes a pure sine wave exactly one octave below the fundamental frequency for massive low-frequency foundations[cite: 1].
    * `NOISE OSC`: Controls the gain of a pseudo-random white noise generator, essential for adding Lo-Fi texture, dust, or chiptune percussion bursts[cite: 1].
* **Amplitude ADSR Envelope:** Vertical faders controlling a non-linear `juce::ADSR` envelope to shape volume dynamics from instant retro clicks to smooth pads[cite: 1].

### 3. Audio Degradation Module (FX Tab / "ROT" DSP Engine)
Implemented in the `LofiEngine` class, processing audio through sequential non-linear and time-based degradation algorithms[cite: 1]:

* **Wow & Flutter (Tape Speed Instability):** Utilizes a stereo circular delay buffer (`delayBuffer[2]`) of 4096 samples (~90ms delay)[cite: 1]. `Wow` models low-frequency speed variations via an LFO (~1.0–2.2 Hz)[cite: 1]. `Flutter` simulates high-frequency tape jitter (~8.0–14.0 Hz) modulated by random micro-noise to prevent artificial periodicity[cite: 1]. 
    To eliminate digital artifacts during continuous delay time modulation, linear fractional interpolation is applied[cite: 1]:
    $$output = sample[floor] \cdot (1 - frac) + sample[floor + 1] \cdot frac$$
* **"Melt" Effect:** A random algorithm generating sudden, deep playback speed drops (delay sharply spikes up to 35 samples), replicating physical tape damage[cite: 1].
* **High-Shelf Filter (HF Roll-off):** A 1st-order one-pole smoothing low-pass filter emulating magnetic tape head high-frequency loss[cite: 1]:
    $$output[n] = state + \alpha \cdot (input[n] - state)$$
    The $\alpha$ coefficient dynamically scales from 1.0 (clean bypass) down to 0.3 (deep HF cut) based on the global macro control[cite: 1].
* **Tape Saturation:** Non-linear harmonic distortion driven by a hyperbolic tangent function with automatic gain compensation[cite: 1]:
    $$saturated = \tanh(input \cdot saturationDrive)$$
* **Bitcrusher (Digital Destruction):** 
    1. *Sample & Hold (Downsampling):* Subsamples the stream, holding values based on the `1.0 / sampleRateRatio`[cite: 1].
    2. *Amplitude Quantization (Bit Reduction):* Rounds the sample to the nearest allowed discrete level based on the selected bit depth (down to 4-bit)[cite: 1]:
    $$crushed = \frac{\text{round}(input \cdot levels)}{levels}$$
* **Vinyl Crackle:** A random generator creating impulses with a defined probability (`crackleLevel * 0.00008f`), shaped through a 1st-order IIR bandpass filter to sound like soft, organic dust crackles[cite: 1].
* **Horror Reverb:** Built on `juce::Reverb`[cite: 1]. As the global macro slider increases, it dynamically scales the room size (`roomSize` up to 0.92) and wet level (`wetLevel` up to 0.45) for an immersive eerie space[cite: 1].

---

## 🎹 Pattern Automation & Chord Routing

Handled within `SynthVoice` when parsing incoming MIDI data streams[cite: 1].
* **Scale Lock:** A specialized array filtering semitones (`scaleSteps`)[cite: 1]. If a played note falls outside the chosen musical scale (Major, Minor, Pentatonic, Dorian, Phrygian, Lydian, Mixolydian), it instantly snaps down to the nearest allowed step within the selected root key (`keySelect`)[cite: 1].
* **Chord Assistant:** Automatically translates single MIDI notes into complex triads or 7th chords[cite: 1].
* **Retro Arpeggio Mode:** Breaks down chords into a rapid cyclical arpeggiation cycle (35 Hz)[cite: 1]. This perfectly recreates the authentic style of NES/GameBoy sound chips that lacked hardware polyphony[cite: 1].
* **DAW MIDI Export:** Fully supports native OS Drag-and-Drop. Allows musicians to drag the generated arpeggio or chord pattern directly out of the plugin window as a `.mid` file into any modern DAW timeline or Piano Roll (FL Studio, Ableton Live, Reaper, Cubase, etc.).

---

## 👾 Low-Level Code Terminal: Retro ROM Exporter

The `RetroRomExporter.h` and `RomExportWindow.cpp` modules analyze the current audio engine properties and translate them into production-ready assembly or high-level source code for retro architectures[cite: 1]:

1.  **PICO-8 SFX (Lua):** Outputs Lua arrays containing pitch, volume, effects, and waveform data mapped to PICO-8's sound subsystem memory via `poke()` function calls[cite: 1].
2.  **NES 2A03 (6502 ASM):** Generates 6502 assembly byte tables initializing the NES APU, writing volume, duty cycle, and period values straight to hardware registers `$4000` / `$4002`[cite: 1].
3.  **SEGA Genesis (YM2612 Regs):** Compiles C-arrays or assembly code mapping FM operator registers for the YM2612 chip (including feedback, algorithm routing, frequency multipliers, and hardware ADSR parameters)[cite: 1].
4.  **GameBoy Color (GBDK C-Code):** Outputs clean C source code for the GBDK compiler, writing directly to the GameBoy hardware sound registers (`NR10` to `NR14`) to control the primary pulse channel[cite: 1].

---

## 🎨 UI/UX and Visualization

The user interface features a sleek, unified **Dark Neon Synthwave** aesthetic.
* **Typography:** Text components are rendered using the modern geometric typeface `Outfit`, embedded natively into the plugin's binary resources[cite: 1].
* **CustomLookAndFeel:** A complete override of JUCE's default rendering engine[cite: 1]. Sliders are drawn as custom glowing neon gradient rings with a distinct center indicator.
* **Tape Animation:** The FX tab runs an internal 30 Hz timer thread animating two rotating cassette spools[cite: 1]. The rotational speed is dynamically modulated by the active audio thread's Wow & Flutter parameters to visualize tape drag in real time[cite: 1].
* **Stereo Oscilloscope:** The `OscilloscopeComponent` processes data out of a 2048-sample lock-free ring buffer[cite: 1]. It draws a neon-pink representation of the waveform in real time, visually demonstrating bitcrusher quantization steps without causing audio processing overhead[cite: 1].

---

## 📦 Build Instructions

### Dependencies
* C++20 compatible compiler (MSVC 2022 / GCC 11+ / Clang 13+)
* CMake 3.22 or higher
* JUCE 7.0.12 (managed automatically via CMake FetchContent or as a git submodule)

### Building via Terminal:
```bash
git clone https://github.com/AlexeyNazariev/ROM-Rot.git
cd ROM-Rot
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Compiles artifacts (`.vst3`, `.component`) into the `build/ROM-Rot_artefacts/` directory.

---

## 📄 License

This project is licensed under the terms of the **MIT License**. You are free to use, modify, and distribute this codebase for both non-commercial and commercial projects. See the [LICENSE](LICENSE) file for details.

Developed by: **Alexey Nazariev**
