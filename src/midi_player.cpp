#include "midi_player.h"
#include "MidiFile.h"
#include <windows.h>
#include <chrono>
#include <algorithm>

using namespace smf;

const std::map<int, std::string> MidiPlayer::KEY_MAP = {
    {36, "1"}, {37, "shift+1"}, {38, "2"}, {39, "shift+2"}, {40, "3"}, {41, "4"},
    {42, "shift+4"}, {43, "5"}, {44, "shift+5"}, {45, "6"}, {46, "shift+6"}, {47, "7"},
    {48, "8"}, {49, "shift+8"}, {50, "9"}, {51, "shift+9"}, {52, "0"}, {53, "q"},
    {54, "shift+q"}, {55, "w"}, {56, "shift+w"}, {57, "e"}, {58, "shift+e"}, {59, "r"},
    {60, "t"}, {61, "shift+t"}, {62, "y"}, {63, "shift+y"}, {64, "u"}, {65, "i"},
    {66, "shift+i"}, {67, "o"}, {68, "shift+o"}, {69, "p"}, {70, "shift+p"}, {71, "a"},
    {72, "s"}, {73, "shift+s"}, {74, "d"}, {75, "shift+d"}, {76, "f"}, {77, "g"},
    {78, "shift+g"}, {79, "h"}, {80, "shift+h"}, {81, "j"}, {82, "shift+j"}, {83, "k"},
    {84, "l"}, {85, "shift+l"}, {86, "z"}, {87, "shift+z"}, {88, "x"}, {89, "c"},
    {90, "shift+c"}, {91, "v"}, {92, "shift+v"}, {93, "b"}, {94, "shift+b"}, {95, "n"},
    {96, "m"}
};

MidiPlayer::MidiPlayer() {}

MidiPlayer::~MidiPlayer() {
    Stop();
}

bool MidiPlayer::LoadFile(const std::string& filePath) {
    Stop();
    
    MidiFile midifile;
    if (!midifile.read(filePath)) {
        return false;
    }
    
    midifile.doTimeAnalysis();
    midifile.linkNotePairs();
    
    m_events.clear();
    
    for (int track = 0; track < midifile.getTrackCount(); track++) {
        for (int i = 0; i < midifile[track].size(); i++) {
            if (midifile[track][i].isNoteOn() && midifile[track][i].getVelocity() > 0) {
                MidiEvent ev;
                ev.timeSec = midifile[track][i].seconds;
                ev.note = midifile[track][i].getKeyNumber();
                ev.velocity = midifile[track][i].getVelocity();
                m_events.push_back(ev);
            }
        }
    }
    
    std::sort(m_events.begin(), m_events.end(), [](const MidiEvent& a, const MidiEvent& b) {
        return a.timeSec < b.timeSec;
    });
    
    m_currentFileName = filePath;
    size_t lastSlash = m_currentFileName.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        m_currentFileName = m_currentFileName.substr(lastSlash + 1);
    }
    
    m_isLoaded = true;
    m_progress = 0.0f;
    m_currentEventIndex = 0;
    m_elapsedTimeSec = 0.0;
    return true;
}

void MidiPlayer::LoadFromQueue(size_t index) {
    if (index < m_queue.size()) {
        LoadFile(m_queue[index]);
    }
}

void MidiPlayer::NextTrack() {
    if (m_queue.empty()) return;
    auto it = std::find(m_queue.begin(), m_queue.end(), m_currentFileName);
    if (it != m_queue.end()) {
        size_t idx = std::distance(m_queue.begin(), it);
        if (idx + 1 < m_queue.size()) {
            LoadFromQueue(idx + 1);
            Play();
        } else if (loopQueue) {
            LoadFromQueue(0);
            Play();
        } else {
            Stop();
        }
    } else {
        LoadFromQueue(0);
        Play();
    }
}

void MidiPlayer::PreviousTrack() {
    if (m_queue.empty()) return;
    auto it = std::find(m_queue.begin(), m_queue.end(), m_currentFileName);
    if (it != m_queue.end()) {
        size_t idx = std::distance(m_queue.begin(), it);
        if (idx > 0) {
            LoadFromQueue(idx - 1);
            Play();
        } else if (loopQueue) {
            LoadFromQueue(m_queue.size() - 1);
            Play();
        } else {
            Stop();
        }
    } else {
        LoadFromQueue(0);
        Play();
    }
}

void MidiPlayer::AddToLibrary(const std::string& path) {
    if (std::find(m_library.begin(), m_library.end(), path) == m_library.end()) {
        m_library.push_back(path);
    }
}

void MidiPlayer::AddToQueue(const std::string& path) {
    m_queue.push_back(path);
}

void MidiPlayer::ToggleFavorite(const std::string& path) {
    auto it = std::find(m_favorites.begin(), m_favorites.end(), path);
    if (it != m_favorites.end()) {
        m_favorites.erase(it);
    } else {
        m_favorites.push_back(path);
    }
}

bool MidiPlayer::IsFavorite(const std::string& path) {
    return std::find(m_favorites.begin(), m_favorites.end(), path) != m_favorites.end();
}

void MidiPlayer::UpdateGlobalHotkeys() {
    static bool wasStartPressed = false;
    static bool wasStopPressed = false;
    static bool wasSpeedUpPressed = false;
    static bool wasSpeedDownPressed = false;

    bool isStartPressed = (GetAsyncKeyState(startKey) & 0x8000) != 0;
    bool isStopPressed = (GetAsyncKeyState(stopKey) & 0x8000) != 0;
    bool isSpeedUpPressed = (GetAsyncKeyState(speedUpKey) & 0x8000) != 0;
    bool isSpeedDownPressed = (GetAsyncKeyState(speedDownKey) & 0x8000) != 0;

    if (isStartPressed && !wasStartPressed) {
        if (!m_isPlaying) Play();
        else if (m_isPaused) Play();
    }
    
    if (isStopPressed && !wasStopPressed) {
        if (m_isPlaying && !m_isPaused) Pause();
        else Stop();
    }
    
    if (isSpeedUpPressed && !wasSpeedUpPressed) {
        m_speed = std::min(m_speed + 0.1f, 5.0f);
    }
    
    if (isSpeedDownPressed && !wasSpeedDownPressed) {
        m_speed = std::max(m_speed - 0.1f, 0.1f);
    }

    wasStartPressed = isStartPressed;
    wasStopPressed = isStopPressed;
    wasSpeedUpPressed = isSpeedUpPressed;
    wasSpeedDownPressed = isSpeedDownPressed;
}

void MidiPlayer::Pause() {
    if (m_isPlaying) {
        m_stopRequested = true;
        if (m_playThread.joinable()) {
            m_playThread.join();
        }
        m_isPlaying = false;
        m_isPaused = true;
        
        // Ensure shift is released
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = MapVirtualKeyA(VK_SHIFT, MAPVK_VK_TO_VSC);
        input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}

void MidiPlayer::Play() {
    if (!m_isLoaded || m_isPlaying) return;
    m_stopRequested = false;
    m_isPlaying = true;
    m_isPaused = false;
    m_playThread = std::thread(&MidiPlayer::PlayThread, this);
}

void MidiPlayer::Stop() {
    m_isPaused = false;
    m_currentEventIndex = 0;
    m_elapsedTimeSec = 0.0;
    m_progress = 0.0f;
    
    if (m_isPlaying) {
        m_stopRequested = true;
        if (m_playThread.joinable()) {
            m_playThread.join();
        }
        m_isPlaying = false;
        
        // Ensure shift is released
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = MapVirtualKeyA(VK_SHIFT, MAPVK_VK_TO_VSC);
        input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}

void MidiPlayer::SetProgress(float progress) {
    if (!m_isLoaded || m_events.empty()) return;
    
    bool wasPlaying = m_isPlaying;
    if (wasPlaying) Pause();
    
    m_progress = progress;
    if (m_progress < 0.0f) m_progress = 0.0f;
    if (m_progress > 1.0f) m_progress = 1.0f;
    
    double totalDuration = m_events.back().timeSec;
    m_elapsedTimeSec = totalDuration * m_progress;
    
    // Find the next event
    m_currentEventIndex = 0;
    for (size_t i = 0; i < m_events.size(); ++i) {
        if (m_events[i].timeSec >= m_elapsedTimeSec) {
            m_currentEventIndex = i;
            break;
        }
    }
    
    if (wasPlaying) Play();
}

float MidiPlayer::GetProgress() const {
    return m_progress.load();
}

int MidiPlayer::FitNoteToPiano(int note) {
    while (note > 96) note -= 12;
    while (note < 36) note += 12;
    return note;
}

void MidiPlayer::SendKey(const std::string& keyStr) {
    bool shift = false;
    std::string key = keyStr;
    
    if (keyStr.find("shift+") == 0) {
        shift = true;
        key = keyStr.substr(6);
    }
    
    char c = key[0];
    short vkCode = VkKeyScanA(c) & 0xFF;
    UINT scanCode = MapVirtualKeyA(vkCode, MAPVK_VK_TO_VSC);
    UINT shiftScan = MapVirtualKeyA(VK_SHIFT, MAPVK_VK_TO_VSC);
    
    std::vector<INPUT> inputs;
    
    if (shift) {
        INPUT iShift = {0};
        iShift.type = INPUT_KEYBOARD;
        iShift.ki.wScan = shiftScan;
        iShift.ki.dwFlags = KEYEVENTF_SCANCODE;
        inputs.push_back(iShift);
    }
    
    INPUT iKey = {0};
    iKey.type = INPUT_KEYBOARD;
    iKey.ki.wScan = scanCode;
    iKey.ki.dwFlags = KEYEVENTF_SCANCODE;
    inputs.push_back(iKey);
    
    iKey.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
    inputs.push_back(iKey);
    
    if (shift) {
        INPUT iShift = {0};
        iShift.type = INPUT_KEYBOARD;
        iShift.ki.wScan = shiftScan;
        iShift.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
        inputs.push_back(iShift);
    }
    
    SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT));
}

void MidiPlayer::PlayThread() {
    if (m_events.empty() || m_currentEventIndex >= m_events.size()) {
        m_isPlaying = false;
        return;
    }
    
    auto startTime = std::chrono::steady_clock::now();
    double startOffset = m_elapsedTimeSec;
    double totalDuration = m_events.back().timeSec;
    
    for (size_t i = m_currentEventIndex; i < m_events.size(); ++i) {
        if (m_stopRequested) {
            m_currentEventIndex = i;
            break;
        }
        
        const auto& ev = m_events[i];
        
        while (!m_stopRequested) {
            auto now = std::chrono::steady_clock::now();
            double elapsed_since_resume = std::chrono::duration<double>(now - startTime).count() * m_speed;
            m_elapsedTimeSec = startOffset + elapsed_since_resume;
            
            m_progress = (float)(m_elapsedTimeSec / totalDuration);
            if (m_progress > 1.0f) m_progress = 1.0f;
            
            if (m_elapsedTimeSec >= ev.timeSec) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        
        if (m_stopRequested) break;
        
        int note = FitNoteToPiano(ev.note);
        auto it = KEY_MAP.find(note);
        if (it != KEY_MAP.end()) {
            SendKey(it->second);
        }
    }
    
    if (!m_stopRequested) {
        m_progress = 1.0f;
        m_elapsedTimeSec = totalDuration;
        m_currentEventIndex = m_events.size();
        m_isPlaying = false;
        m_isPaused = false;
        if (loopQueue && !m_queue.empty()) {
            NextTrack();
        }
    }
}
