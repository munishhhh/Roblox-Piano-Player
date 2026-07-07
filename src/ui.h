#pragma once
#include "midi_player.h"
#include "imgui.h"

namespace UI {
    void Init();
    void Render(MidiPlayer& player);

    extern ImTextureID texPlay;
    extern ImTextureID texPause;
    extern ImTextureID texStop;
    extern ImTextureID texClose;
    extern ImTextureID texMinimize;
    extern ImTextureID texMaximize;
    extern ImTextureID texLogo;
    extern ImTextureID texStarFilled;
    extern ImTextureID texStarEmpty;
    extern ImTextureID texPrev;
    extern ImTextureID texNext;
    extern ImTextureID texTabDash;
    extern ImTextureID texTabMidi;
    extern ImTextureID texTabSettings;
    extern ImTextureID texLoop;
    extern ImTextureID texLoad;
    extern ImTextureID texQueue;
    extern ImTextureID texUploadMidi;
    extern ImTextureID texClearQueue;
    extern ImTextureID texShuffleButton;
    extern ImTextureID texResetHotkeys;
    extern ImTextureID texRobloxLaunch;
    extern ImTextureID texDiscord;
    extern ImTextureID texGithub;
    extern ImTextureID texInstagram;
}
