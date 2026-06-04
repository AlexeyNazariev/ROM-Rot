#include "MidiDragButton.h"

MidiDragButton::MidiDragButton(LofiChiptuneSynthAudioProcessor& p)
    : processor(p)
{
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

void MidiDragButton::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Рисуем стильный неоновый градиент для кнопки
    juce::Colour colorStart = juce::Colour::fromString("#FF2E93"); // Яркий розовый
    juce::Colour colorEnd   = juce::Colour::fromString("#9B00E8"); // Глубокий фиолетовый
    
    if (isHovered)
    {
        colorStart = colorStart.brighter(0.15f);
        colorEnd = colorEnd.brighter(0.15f);
    }
    
    if (isPressed)
    {
        colorStart = colorStart.darker(0.15f);
        colorEnd = colorEnd.darker(0.15f);
    }
    
    // Фон с закругленными углами
    juce::Graphics::ScopedSaveState saveState(g);
    
    // Тень/свечение кнопки
    if (isHovered)
    {
        g.setColour(colorStart.withAlpha(0.35f));
        g.fillRoundedRectangle(bounds.reduced(1.0f), 12.0f);
    }
    
    juce::ColourGradient grad(colorStart, bounds.getX(), bounds.getY(),
                              colorEnd, bounds.getRight(), bounds.getBottom(), false);
    g.setGradientFill(grad);
    g.fillRoundedRectangle(bounds.reduced(3.0f), 10.0f);
    
    // Обводка
    g.setColour(juce::Colour::fromString("#0DF5E3").withAlpha(isHovered ? 0.8f : 0.4f)); // Неоновый циан
    g.drawRoundedRectangle(bounds.reduced(3.0f), 10.0f, 1.5f);
    
    // Рисуем иконку (MIDI разъем или стрелочку)
    g.setColour(juce::Colours::white);
    
    // Текст
    g.setFont(juce::Font("Outfit", 15.0f, juce::Font::bold));
    
    // Рисуем текст по центру
    g.drawText("DRAG MIDI", getLocalBounds(), juce::Justification::centred, true);
}

void MidiDragButton::resized()
{
}

void MidiDragButton::mouseEnter (const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    isHovered = true;
    repaint();
}

void MidiDragButton::mouseExit (const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    isHovered = false;
    repaint();
}

void MidiDragButton::mouseDown (const juce::MouseEvent& event)
{
    juce::ignoreUnused(event);
    isPressed = true;
    repaint();
}

void MidiDragButton::mouseDrag (const juce::MouseEvent& event)
{
    // Если пользователь потянул кнопку достаточно далеко, начинаем перетаскивание внешнего файла
    auto dragOffset = event.getOffsetFromDragStart();
    if (dragOffset.x * dragOffset.x + dragOffset.y * dragOffset.y > 64)
    {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        
        // Генерируем временный MIDI файл
        juce::File midiFile = generateMidiFile();
        
        if (midiFile.existsAsFile())
        {
            juce::StringArray files;
            files.add(midiFile.getFullPathName());
            
            // Запускаем перетаскивание за пределы плагина в DAW
            if (auto* dragContainer = juce::DragAndDropContainer::findParentDragContainerFor(this))
            {
                dragContainer->performExternalDragDropOfFiles(files, false, this, [this]() {
                    // Возвращаем исходные состояния по окончании
                    isPressed = false;
                    setMouseCursor(juce::MouseCursor::PointingHandCursor);
                    repaint();
                });
            }
        }
    }
}

juce::File MidiDragButton::generateMidiFile()
{
    auto& apvts = processor.getAPVTS();
    
    int key = static_cast<int>(apvts.getRawParameterValue("key_select")->load(std::memory_order_relaxed));
    int scale = static_cast<int>(apvts.getRawParameterValue("scale_select")->load(std::memory_order_relaxed));
    int patternType = static_cast<int>(apvts.getRawParameterValue("pattern_type")->load(std::memory_order_relaxed));
    
    // Базовая нота для экспорта - До 4-й октавы (MIDI 60) + выбранная тональность
    int baseNote = 60 + key;
    
    int chordNotes[3];
    calculateChordNotes(baseNote, key, scale, chordNotes);
    
    juce::MidiMessageSequence sequence;
    
    int ticksPerQuarterNote = 960; // стандартное разрешение
    int sixteenthNoteTicks = ticksPerQuarterNote / 4; // 240 тиков
    
    int currentTick = 0;
    
    if (patternType == 0) // ClassicArp (0)
    {
        int pattern[4] = { 0, 1, 2, 1 };
        const int totalNotes = 32; // 32 шестнадцатых нот в 2 тактах
        
        for (int i = 0; i < totalNotes; ++i)
        {
            int noteIndex = pattern[i % 4];
            int midiNote = chordNotes[noteIndex];
            
            auto noteOn = juce::MidiMessage::noteOn(1, midiNote, static_cast<juce::uint8>(100));
            noteOn.setTimeStamp(currentTick);
            sequence.addEvent(noteOn);
            
            auto noteOff = juce::MidiMessage::noteOff(1, midiNote);
            noteOff.setTimeStamp(currentTick + static_cast<int>(sixteenthNoteTicks * 0.85f));
            sequence.addEvent(noteOff);
            
            currentTick += sixteenthNoteTicks;
        }
    }
    else if (patternType == 1) // LoFiStrum (1)
    {
        // Медленный перебор восьмыми нотами: 0, 1, 2, тишина, 0, 1, 2, тишина...
        int eighthNoteTicks = ticksPerQuarterNote / 2;
        int pattern[4] = { 0, 1, 2, -1 };
        const int totalNotes = 16; // 16 восьмых нот в 2 тактах
        
        for (int i = 0; i < totalNotes; ++i)
        {
            int noteIndex = pattern[i % 4];
            if (noteIndex != -1)
            {
                int midiNote = chordNotes[noteIndex];
                
                auto noteOn = juce::MidiMessage::noteOn(1, midiNote, static_cast<juce::uint8>(100));
                noteOn.setTimeStamp(currentTick);
                sequence.addEvent(noteOn);
                
                auto noteOff = juce::MidiMessage::noteOff(1, midiNote);
                noteOff.setTimeStamp(currentTick + static_cast<int>(eighthNoteTicks * 0.85f));
                sequence.addEvent(noteOff);
            }
            currentTick += eighthNoteTicks;
        }
    }
    else if (patternType == 2) // RhythmicChop (2)
    {
        // Пульсация аккорда: звучит 8-ю ноту, молчит 8-ю ноту
        int eighthNoteTicks = ticksPerQuarterNote / 2;
        const int totalChops = 8; // 8 пульсаций в 2 тактах (всего 16 восьмых)
        
        for (int i = 0; i < totalChops; ++i)
        {
            for (int k = 0; k < 3; ++k)
            {
                auto noteOn = juce::MidiMessage::noteOn(1, chordNotes[k], static_cast<juce::uint8>(100));
                noteOn.setTimeStamp(currentTick);
                sequence.addEvent(noteOn);
                
                auto noteOff = juce::MidiMessage::noteOff(1, chordNotes[k]);
                noteOff.setTimeStamp(currentTick + eighthNoteTicks);
                sequence.addEvent(noteOff);
            }
            currentTick += eighthNoteTicks * 2;
        }
    }
    else // SustainPad (3)
    {
        // 2 такта сплошного звучания
        int totalTicks = ticksPerQuarterNote * 8;
        
        for (int k = 0; k < 3; ++k)
        {
            auto noteOn = juce::MidiMessage::noteOn(1, chordNotes[k], static_cast<juce::uint8>(90));
            noteOn.setTimeStamp(0);
            sequence.addEvent(noteOn);
            
            auto noteOff = juce::MidiMessage::noteOff(1, chordNotes[k]);
            noteOff.setTimeStamp(totalTicks - 10);
            sequence.addEvent(noteOff);
        }
    }
    
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(ticksPerQuarterNote);
    midiFile.addTrack(sequence);
    
    // Записываем файл в системную папку временных файлов
    juce::File tempDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory);
    juce::File file = tempDir.getChildFile("ChiptunePattern.mid");
    
    if (file.existsAsFile())
        file.deleteFile();
        
    juce::FileOutputStream stream(file);
    if (stream.openedOk())
    {
        midiFile.writeTo(stream);
    }
    
    return file;
}

void MidiDragButton::calculateChordNotes(int baseNote, int key, int scale, int (&chordNotes)[3]) noexcept
{
    // Аналогично логике голоса
    chordNotes[0] = baseNote;
    
    int semitonesFromKey = (baseNote - key) % 12;
    if (semitonesFromKey < 0)
        semitonesFromKey += 12;
        
    int majorScale[7] = { 0, 2, 4, 5, 7, 9, 11 };
    int minorScale[7] = { 0, 2, 3, 5, 7, 8, 10 };
    int phrygianScale[7] = { 0, 1, 3, 5, 7, 8, 10 };
    
    const int* scalePattern = majorScale;
    if (scale == 1)
        scalePattern = minorScale;
    else if (scale == 2)
        scalePattern = phrygianScale;
        
    int rootDegree = -1;
    for (int i = 0; i < 7; ++i)
    {
        if (semitonesFromKey == scalePattern[i])
        {
            rootDegree = i;
            break;
        }
    }
    
    if (rootDegree != -1)
    {
        int deg3 = (rootDegree + 2) % 7;
        int deg5 = (rootDegree + 4) % 7;
        int octave3 = (rootDegree + 2) / 7;
        int octave5 = (rootDegree + 4) / 7;
        
        int semitones3 = scalePattern[deg3] + octave3 * 12;
        int semitones5 = scalePattern[deg5] + octave5 * 12;
        
        chordNotes[1] = baseNote + (semitones3 - semitonesFromKey);
        chordNotes[2] = baseNote + (semitones5 - semitonesFromKey);
    }
    else
    {
        if (scale == 0)
        {
            chordNotes[1] = baseNote + 4;
            chordNotes[2] = baseNote + 7;
        }
        else
        {
            chordNotes[1] = baseNote + 3;
            chordNotes[2] = baseNote + 7;
        }
    }
}
