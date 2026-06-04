#include "PluginProcessor.h"
#include "PluginEditor.h"

// Конструктор плагина
LofiChiptuneSynthAudioProcessor::LofiChiptuneSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    // Кешируем атомарные указатели параметров для быстрого lock-free чтения в processBlock()
    vibeMacroParameter      = apvts.getRawParameterValue("vibe_macro");
    masterVolumeParameter   = apvts.getRawParameterValue("master_volume");
    
    oscWaveParameter        = apvts.getRawParameterValue("osc_wave");
    pulseWidthParameter     = apvts.getRawParameterValue("pulse_width");
    pwmRateParameter        = apvts.getRawParameterValue("pwm_rate");
    pwmDepthParameter       = apvts.getRawParameterValue("pwm_depth");
    oscDetuneParameter      = apvts.getRawParameterValue("osc_detune");
    
    adsrAttackParameter     = apvts.getRawParameterValue("adsr_attack");
    adsrDecayParameter      = apvts.getRawParameterValue("adsr_decay");
    adsrSustainParameter    = apvts.getRawParameterValue("adsr_sustain");
    adsrReleaseParameter    = apvts.getRawParameterValue("adsr_release");
    
    keySelectParameter      = apvts.getRawParameterValue("key_select");
    scaleSelectParameter    = apvts.getRawParameterValue("scale_select");
    chordAssistantParameter = apvts.getRawParameterValue("chord_assistant");

    // Новые параметры v2
    subGainParameter         = apvts.getRawParameterValue("sub_gain");
    noiseGainParameter       = apvts.getRawParameterValue("noise_gain");
    saturationDriveParameter = apvts.getRawParameterValue("saturation_drive");
    patternTypeParameter     = apvts.getRawParameterValue("pattern_type");
    bitDepthParameter        = apvts.getRawParameterValue("bitdepth");
    sampleRateRatioParameter = apvts.getRawParameterValue("samplerate_ratio");

#if ! JucePlugin_IsSynth
    windowStateManager.setWindowState(WindowState::FX);
#endif
}

LofiChiptuneSynthAudioProcessor::~LofiChiptuneSynthAudioProcessor()
{
}

// Создание структуры параметров для APVTS
juce::AudioProcessorValueTreeState::ParameterLayout LofiChiptuneSynthAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Глобальные слайдеры нижней панели
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("vibe_macro", 1), "Vibe Macro", 0.0f, 1.0f, 0.0f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("master_volume", 1), "Master Volume", 0.0f, 1.0f, 0.7f));

    // Параметры осциллятора (OSC)
    // Choice: 0 = Sine, 1 = Triangle, 2 = Saw, 3 = Pulse
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("osc_wave", 1), "Osc Waveform", 
        juce::StringArray{"Sine", "Triangle", "Sawtooth", "Pulse (PWM)"}, 3));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("pulse_width", 1), "Pulse Width", 0.05f, 0.95f, 0.5f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("pwm_rate", 1), "PWM LFO Rate", 0.1f, 20.0f, 1.5f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("pwm_depth", 1), "PWM LFO Depth", 0.0f, 1.0f, 0.0f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("osc_detune", 1), "Osc Detune", -2.0f, 2.0f, 0.0f));

    // Новые параметры OSC v2
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("sub_gain", 1), "Sub Gain", 0.0f, 1.0f, 0.0f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("noise_gain", 1), "Noise Gain", 0.0f, 1.0f, 0.0f));

    // Параметры огибающей ADSR
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("adsr_attack", 1), "ADSR Attack", 0.001f, 3.0f, 0.01f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("adsr_decay", 1), "ADSR Decay", 0.01f, 3.0f, 0.1f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("adsr_sustain", 1), "ADSR Sustain", 0.0f, 1.0f, 0.8f));
        
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("adsr_release", 1), "ADSR Release", 0.01f, 5.0f, 0.3f));

    // Параметры FX v2
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("saturation_drive", 1), "Saturation Drive", 0.0f, 2.0f, 0.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("bitdepth", 1), "Bit Depth", 2.0f, 16.0f, 16.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID("samplerate_ratio", 1), "Sample Rate Ratio", 0.01f, 1.0f, 1.0f));

    // Параметры автоаккорда (Top Bar)
    // Key: 0=C, 1=C#, 2=D, 3=D#, 4=E, 5=F, 6=F#, 7=G, 8=G#, 9=A, 10=A#, 11=B
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("key_select", 1), "Key Select", 
        juce::StringArray{"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}, 0));
        
    // Scale: 0=Major, 1=Minor, 2=Phrygian
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("scale_select", 1), "Scale Select", 
        juce::StringArray{"Major", "Minor", "Phrygian"}, 0));
        
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID("chord_assistant", 1), "Chord Assistant Active", false));

    // Тип паттерна v2: 0=ClassicArp, 1=LoFiStrum, 2=RhythmicChop, 3=SustainPad
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID("pattern_type", 1), "Pattern Type",
        juce::StringArray{"Classic Arp", "Lo-Fi Strum", "Rhythmic Chop", "Sustain Pad"}, 3));

    return { params.begin(), params.end() };
}

void LofiChiptuneSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    // Подготавливаем голоса синтезатора
    for (int i = 0; i < numVoices; ++i)
    {
        voices[i].prepare(sampleRate);
    }
    
    // Подготавливаем DSP Lo-Fi эффектов
    lofiEngine.prepare(sampleRate);
}

void LofiChiptuneSynthAudioProcessor::releaseResources()
{
}

bool LofiChiptuneSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Поддерживаем только стерео выход
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
        
    return true;
}

void LofiChiptuneSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
    const int numSamples = buffer.getNumSamples();

    // 1. Читаем актуальные параметры из APVTS с помощью lock-free атомарных загрузок
    float vibeMacro = vibeMacroParameter->load(std::memory_order_relaxed);
    float masterVolume = masterVolumeParameter->load(std::memory_order_relaxed);
    
    int oscWave = static_cast<int>(oscWaveParameter->load(std::memory_order_relaxed));
    float pulseWidth = pulseWidthParameter->load(std::memory_order_relaxed);
    float pwmRate = pwmRateParameter->load(std::memory_order_relaxed);
    float pwmDepth = pwmDepthParameter->load(std::memory_order_relaxed);
    float oscDetune = oscDetuneParameter->load(std::memory_order_relaxed);
    
    float adsrAttack = adsrAttackParameter->load(std::memory_order_relaxed);
    float adsrDecay = adsrDecayParameter->load(std::memory_order_relaxed);
    float adsrSustain = adsrSustainParameter->load(std::memory_order_relaxed);
    float adsrRelease = adsrReleaseParameter->load(std::memory_order_relaxed);
    
    int keySelect = static_cast<int>(keySelectParameter->load(std::memory_order_relaxed));
    int scaleSelect = static_cast<int>(scaleSelectParameter->load(std::memory_order_relaxed));
    bool chordAssistant = chordAssistantParameter->load(std::memory_order_relaxed) > 0.5f;

    // Новые параметры v2
    float subGain = subGainParameter->load(std::memory_order_relaxed);
    float noiseGain = noiseGainParameter->load(std::memory_order_relaxed);
    float saturationDrive = saturationDriveParameter->load(std::memory_order_relaxed);
    int patternType = static_cast<int>(patternTypeParameter->load(std::memory_order_relaxed));
    float bitDepth = bitDepthParameter->load(std::memory_order_relaxed);
    float sampleRateRatio = sampleRateRatioParameter->load(std::memory_order_relaxed);

    // 2. Обновляем параметры у всех голосов синтезатора
    for (int i = 0; i < numVoices; ++i)
    {
        voices[i].setParameters(oscWave, pulseWidth, pwmRate, pwmDepth,
                                adsrAttack, adsrDecay, adsrSustain, adsrRelease, oscDetune,
                                subGain, noiseGain, patternType);
    }

    // 3. Обновляем параметры у Lo-Fi движка эффектов
    lofiEngine.updateVibeParameters(vibeMacro, saturationDrive, bitDepth, sampleRateRatio);

#if JucePlugin_IsSynth
    // Поскольку мы являемся синтезатором-генератором, очищаем входной аудио-буфер
    buffer.clear();
#endif

    // 4. Обрабатываем MIDI сообщения
    for (const auto metadata : midiMessages)
    {
        const auto msg = metadata.getMessage();
        
        if (msg.isNoteOn())
        {
            const int note = msg.getNoteNumber();
            const float velocity = msg.getFloatVelocity();
            
            int targetVoice = -1;
            
            // Пытаемся найти уже звучащий голос с такой же нотой для перезапуска (legato)
            for (int i = 0; i < numVoices; ++i)
            {
                if (voices[i].isActive() && voices[i].getMidiNote() == note)
                {
                    targetVoice = i;
                    break;
                }
            }
            
            // Если не нашли, ищем любой неактивный голос
            if (targetVoice == -1)
            {
                for (int i = 0; i < numVoices; ++i)
                {
                    int checkIndex = (lastAssignedVoiceIndex + 1 + i) % numVoices;
                    if (!voices[checkIndex].isActive())
                    {
                        targetVoice = checkIndex;
                        lastAssignedVoiceIndex = checkIndex;
                        break;
                    }
                }
            }
            
            // Если все голоса заняты, принудительно переназначаем следующий по очереди голос (Voice Stealing)
            if (targetVoice == -1)
            {
                targetVoice = (lastAssignedVoiceIndex + 1) % numVoices;
                lastAssignedVoiceIndex = targetVoice;
            }
            
            voices[targetVoice].noteOn(note, velocity, keySelect, scaleSelect, chordAssistant);
        }
        else if (msg.isNoteOff())
        {
            const int note = msg.getNoteNumber();
            for (int i = 0; i < numVoices; ++i)
            {
                if (voices[i].isActive() && voices[i].getMidiNote() == note)
                {
                    voices[i].noteOff();
                }
            }
        }
        else if (msg.isAllNotesOff())
        {
            for (int i = 0; i < numVoices; ++i)
            {
                voices[i].reset();
            }
        }
    }

#if JucePlugin_IsSynth
    // 5. Рендерим активные голоса синтезатора в буфер
    for (int i = 0; i < numVoices; ++i)
    {
        if (voices[i].isActive())
        {
            voices[i].processBlock(buffer, 0, numSamples);
        }
    }
#endif

    // 6. Пропускаем суммарный сигнал через цепочку Lo-Fi эффектов
    lofiEngine.process(buffer);

    // 7. Применяем мастер-громкость к стерео-выходу
    buffer.applyGain(masterVolume);

    // 8. Пушим выходные сэмплы в кольцевой буфер для осциллографа
    const float* leftChannel = buffer.getReadPointer(0);
    const float* rightChannel = (buffer.getNumChannels() > 1) ? buffer.getReadPointer(1) : nullptr;
    
    for (int sample = 0; sample < numSamples; ++sample)
    {
        float monoSample = leftChannel[sample];
        if (rightChannel != nullptr)
        {
            monoSample = (monoSample + rightChannel[sample]) * 0.5f;
        }
        ringBuffer.pushSample(monoSample);
    }
}

// Заглушки для пресетов
void LofiChiptuneSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Сохраняем состояние APVTS в XML для хоста DAW
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void LofiChiptuneSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Восстанавливаем состояние APVTS из XML
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
    {
        if (xmlState->hasTagName (apvts.state.getType()))
        {
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
        }
    }
}

// Метод для вызова нот с UI клавиатуры (через прямую инжекцию)
void LofiChiptuneSynthAudioProcessor::triggerNoteOn(int midiNote, float velocity) noexcept
{
    // В данном случае, так как вызов идет из UI потока, мы можем безопасно найти
    // свободный голос и запустить его. noteOn потокобезопасен благодаря внутреннему устройству
    // голоса и отсутствию аллокаций.
    int targetVoice = -1;
    for (int i = 0; i < numVoices; ++i)
    {
        if (!voices[i].isActive())
        {
            targetVoice = i;
            break;
        }
    }
    
    if (targetVoice == -1)
        targetVoice = 0; // Стиллинг первого голоса
        
    float adsrAttack = adsrAttackParameter->load(std::memory_order_relaxed);
    float adsrDecay = adsrDecayParameter->load(std::memory_order_relaxed);
    float adsrSustain = adsrSustainParameter->load(std::memory_order_relaxed);
    float adsrRelease = adsrReleaseParameter->load(std::memory_order_relaxed);
    int oscWave = static_cast<int>(oscWaveParameter->load(std::memory_order_relaxed));
    float pulseWidth = pulseWidthParameter->load(std::memory_order_relaxed);
    float pwmRate = pwmRateParameter->load(std::memory_order_relaxed);
    float pwmDepth = pwmDepthParameter->load(std::memory_order_relaxed);
    float oscDetune = oscDetuneParameter->load(std::memory_order_relaxed);
    
    int keySelect = static_cast<int>(keySelectParameter->load(std::memory_order_relaxed));
    int scaleSelect = static_cast<int>(scaleSelectParameter->load(std::memory_order_relaxed));
    bool chordAssistant = chordAssistantParameter->load(std::memory_order_relaxed) > 0.5f;

    float subGain = subGainParameter->load(std::memory_order_relaxed);
    float noiseGain = noiseGainParameter->load(std::memory_order_relaxed);
    int patternType = static_cast<int>(patternTypeParameter->load(std::memory_order_relaxed));

    voices[targetVoice].setParameters(oscWave, pulseWidth, pwmRate, pwmDepth,
                                      adsrAttack, adsrDecay, adsrSustain, adsrRelease, oscDetune,
                                      subGain, noiseGain, patternType);
    voices[targetVoice].noteOn(midiNote, velocity, keySelect, scaleSelect, chordAssistant);
}

void LofiChiptuneSynthAudioProcessor::triggerNoteOff(int midiNote) noexcept
{
    for (int i = 0; i < numVoices; ++i)
    {
        if (voices[i].isActive() && voices[i].getMidiNote() == midiNote)
        {
            voices[i].noteOff();
        }
    }
}

// Создание редактора (UI)
juce::AudioProcessorEditor* LofiChiptuneSynthAudioProcessor::createEditor()
{
    return new LofiChiptuneSynthAudioProcessorEditor (*this);
}

// Создание инстанса плагина для JUCE
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LofiChiptuneSynthAudioProcessor();
}
