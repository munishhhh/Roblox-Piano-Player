#pragma once
#include <string>
#include <atomic>
#include <thread>
#include <vector>
#include <map>

class MidiPlayer {
public:
    MidiPlayer();
    ~MidiPlayer();

    bool LoadFile(const std::string& filePath);
    void Play();
    void Pause();
    void Stop();
    
    bool IsPlaying() const { return m_isPlaying; }
    bool IsPaused() const { return m_isPaused; }
    bool IsLoaded() const { return m_isLoaded; }
    std::string GetCurrentFileName() const { return m_currentFileName; }
    
    float GetProgress() const; // 0.0 to 1.0
    void SetProgress(float progress); // Seek
    void SetSpeed(float speed) { m_speed = speed; }
    float GetSpeed() const { return m_speed; }

    // Playback Queue
    std::vector<std::string>& GetQueue() { return m_queue; }
    void LoadFromQueue(size_t index);
    void NextTrack();
    void PreviousTrack();
    
    bool loopQueue = false;
    
    // Library and Playlists
    std::vector<std::string>& GetLibrary() { return m_library; }
    std::vector<std::string>& GetFavorites() { return m_favorites; }
    std::map<std::string, std::vector<std::string>>& GetPlaylists() { return m_playlists; }
    
    void AddToLibrary(const std::string& path);
    void AddToQueue(const std::string& path);
    void ToggleFavorite(const std::string& path);
    bool IsFavorite(const std::string& path);

    int startKey = 0x70; // VK_F1
    int stopKey = 0x79;  // VK_F10
    int speedUpKey = 0xBB; // VK_OEM_PLUS (=/+)
    int speedDownKey = 0xBD; // VK_OEM_MINUS (-/_)
    int nextTrackKey = 0x27; // VK_RIGHT
    int prevTrackKey = 0x25; // VK_LEFT
    int loopKey = 0x4C; // L key
    int shuffleKey = 0x53; // S key
    
    void ResetHotkeys() { 
        startKey = 0x70; 
        stopKey = 0x79; 
        speedUpKey = 0xBB; 
        speedDownKey = 0xBD; 
        nextTrackKey = 0x27;
        prevTrackKey = 0x25;
        loopKey = 0x4C;
        shuffleKey = 0x53;
    }

    void Shuffle();

    void UpdateGlobalHotkeys();

private:
    void PlayThread();
    void SendKey(const std::string& keyStr);
    int FitNoteToPiano(int note);

    std::atomic<bool> m_isPlaying{false};
    std::atomic<bool> m_isPaused{false};
    std::atomic<bool> m_stopRequested{false};
    bool m_isLoaded{false};
    
    std::string m_currentFileName;
    float m_speed = 1.0f;
    size_t m_currentEventIndex = 0;
    double m_elapsedTimeSec = 0.0;
    
    std::thread m_playThread;
    std::vector<std::string> m_queue;
    std::vector<std::string> m_library;
    std::vector<std::string> m_favorites;
    std::map<std::string, std::vector<std::string>> m_playlists;
    
    // Internal midi data
    struct MidiEvent {
        double timeSec;
        int note;
        int velocity;
    };
    std::vector<MidiEvent> m_events;
    
    std::atomic<float> m_progress{0.0f};

    static const std::map<int, std::string> KEY_MAP;
};
