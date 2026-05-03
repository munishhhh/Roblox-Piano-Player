#include "ui.h"
#include "imgui.h"
#include <windows.h>
#include <commdlg.h>
#include <string>
#include <vector>
#include <algorithm>
#include <random>

namespace UI {

    enum class Tab { Dashboard, Library, Favorites, Settings };
    static Tab currentTab = Tab::Dashboard;

    // For hotkey capturing
    static bool capturingStartKey = false;
    static bool capturingStopKey = false;
    static bool capturingSpeedUpKey = false;
    static bool capturingSpeedDownKey = false;
    static bool capturingNextKey = false;
    static bool capturingPrevKey = false;
    static bool capturingLoopKey = false;
    static bool capturingShuffleKey = false;

    std::string GetKeyName(int vk);

    ImTextureID texPlay = (ImTextureID)0;
    ImTextureID texPause = (ImTextureID)0;
    ImTextureID texStop = (ImTextureID)0;
    ImTextureID texClose = (ImTextureID)0;
    ImTextureID texMinimize = (ImTextureID)0;
    ImTextureID texMaximize = (ImTextureID)0;
    ImTextureID texLogo = (ImTextureID)0;
    ImTextureID texStarFilled = (ImTextureID)0;
    ImTextureID texStarEmpty = (ImTextureID)0;
    ImTextureID texPrev = (ImTextureID)0;
    ImTextureID texNext = (ImTextureID)0;
    ImTextureID texTabDash = (ImTextureID)0;
    ImTextureID texTabMidi = (ImTextureID)0;
    ImTextureID texTabSettings = (ImTextureID)0;
    ImTextureID texLoop = (ImTextureID)0;
    ImTextureID texDiscord = (ImTextureID)0;
    ImTextureID texGithub = (ImTextureID)0;
    ImTextureID texInstagram = (ImTextureID)0;

    void ApplyPremiumStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        style.Alpha = 1.0f;
        style.WindowRounding = 12.0f;
        style.ChildRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;
        
        style.WindowBorderSize = 1.0f;
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 0.0f;
        
        // Colors for a full transparent glass theme
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text]                   = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.10f); // Highly transparent
        colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.10f, 0.10f, 0.12f, 0.85f);
        colors[ImGuiCol_Border]                 = ImVec4(0.40f, 0.40f, 0.45f, 0.30f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.25f, 0.40f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.30f, 0.30f, 0.35f, 0.60f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.40f, 0.40f, 0.45f, 0.80f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.10f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.10f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.40f, 0.40f, 0.45f, 0.60f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.50f, 0.50f, 0.55f, 0.80f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.60f, 0.60f, 0.65f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.30f, 0.90f, 0.50f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.40f, 0.70f, 1.00f, 0.80f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.50f, 0.80f, 1.00f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.30f, 0.30f, 0.35f, 0.50f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.40f, 0.40f, 0.45f, 0.70f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.50f, 0.50f, 0.55f, 0.90f);
        colors[ImGuiCol_Header]                 = ImVec4(0.30f, 0.30f, 0.35f, 0.45f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.40f, 0.40f, 0.45f, 0.70f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.50f, 0.50f, 0.55f, 0.90f);
        colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.50f, 0.50f, 0.55f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.60f, 0.60f, 0.65f, 1.00f);
    }

    void Init() {
        ApplyPremiumStyle();
    }

    std::vector<std::string> OpenMidiDialogMulti() {
        std::vector<std::string> results;
        char filename[8192]; // Large buffer for multiple files
        ZeroMemory(filename, sizeof(filename));
        
        OPENFILENAMEA ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = NULL;
        ofn.lpstrFilter = "MIDI Files\0*.mid;*.midi\0All Files\0*.*\0";
        ofn.lpstrFile = filename;
        ofn.nMaxFile = sizeof(filename);
        ofn.lpstrTitle = "Select MIDI Files";
        ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;
        
        if (GetOpenFileNameA(&ofn)) {
            std::string dirOrFile = filename;
            if (filename[dirOrFile.length() + 1] == '\0') {
                results.push_back(dirOrFile);
            } else {
                char* p = filename + dirOrFile.length() + 1;
                while (*p) {
                    results.push_back(dirOrFile + "\\" + p);
                    p += strlen(p) + 1;
                }
            }
        }
        return results;
    }

    std::string GetFileNameFromPath(const std::string& path) {
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            return path.substr(lastSlash + 1);
        }
        return path;
    }

    void RenderDashboard(MidiPlayer& player) {
        ImGui::Text("Dashboard");
        ImGui::Spacing();
        
        if (ImGui::Button("Upload MIDI File", ImVec2(0, 35))) {
            auto files = OpenMidiDialogMulti();
            for (const auto& f : files) {
                player.AddToQueue(f);
            }
            if (!files.empty() && player.GetQueue().size() == files.size()) {
                player.LoadFromQueue(0);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear Queue", ImVec2(0, 35))) {
            player.GetQueue().clear();
            player.Stop();
        }
        ImGui::SameLine();
        if (ImGui::Button("Shuffle", ImVec2(0, 35))) {
            auto& q = player.GetQueue();
            if (q.size() > 1) {
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(q.begin(), q.end(), g);
            }
        }
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "Current Hotkeys: Play/Resume [%s] | Stop/Pause [%s]", 
            GetKeyName(player.startKey).c_str(), GetKeyName(player.stopKey).c_str());
        ImGui::Spacing();

        // Speed Control
        float speed = player.GetSpeed();
        ImGui::SetNextItemWidth(300);
        if (ImGui::SliderFloat("Speed Multiplier", &speed, 0.1f, 3.0f, "%.2fx")) {
            player.SetSpeed(speed);
        }

        ImGui::Spacing(); ImGui::Spacing();

        auto& queue = player.GetQueue();

        // Dropdown style track selector
        ImGui::SetNextItemWidth(300);
        std::string preview_value = "Select Track...";
        if (player.IsLoaded()) {
            preview_value = player.GetCurrentFileName();
        }
        
        if (ImGui::BeginCombo("Active Track", preview_value.c_str())) {
            if (queue.empty()) {
                ImGui::Selectable("No tracks uploaded", false);
            } else {
                for (size_t i = 0; i < queue.size(); i++) {
                    std::string name = GetFileNameFromPath(queue[i]);
                    bool is_selected = (player.IsLoaded() && player.GetCurrentFileName() == name);
                    if (ImGui::Selectable(name.c_str(), is_selected)) {
                        player.LoadFromQueue(i);
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }
            ImGui::EndCombo();
        }

        ImGui::Spacing(); ImGui::Spacing();

        // Playlist View (drag to reorder)
        ImGui::Text("Current Queue");
        ImGui::BeginChild("PlaylistQueue", ImVec2(0, -90), true);
        
        if (queue.empty()) {
            ImGui::TextDisabled("Playlist is empty. Add files from the MIDI Files tab or drop files here.");
        } else {
            for (size_t i = 0; i < queue.size(); i++) {
                ImGui::PushID((int)i);
                std::string name = GetFileNameFromPath(queue[i]);
                bool isPlayingThis = player.IsLoaded() && player.GetCurrentFileName() == name;
                
                if (isPlayingThis) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.8f, 0.4f, 1.0f));
                }

                ImGui::Selectable((name + "##" + std::to_string(i)).c_str(), false, 0, ImVec2(ImGui::GetWindowWidth() - 100, 0));
                
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
                    player.LoadFromQueue(i);
                }

                if (isPlayingThis) {
                    ImGui::PopStyleColor();
                }

                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                    ImGui::SetDragDropPayload("PLAYLIST_ITEM", &i, sizeof(size_t));
                    ImGui::Text("Move %s", name.c_str());
                    ImGui::EndDragDropSource();
                }
                
                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PLAYLIST_ITEM")) {
                        IM_ASSERT(payload->DataSize == sizeof(size_t));
                        size_t sourceIdx = *(const size_t*)payload->Data;
                        size_t targetIdx = i;
                        if (sourceIdx != targetIdx) {
                            std::string temp = queue[sourceIdx];
                            queue.erase(queue.begin() + sourceIdx);
                            queue.insert(queue.begin() + targetIdx, temp);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::SameLine(ImGui::GetWindowWidth() - 80);
                if (texStarFilled && texStarEmpty) {
                    if (ImGui::ImageButton("fav", player.IsFavorite(queue[i]) ? texStarFilled : texStarEmpty, ImVec2(20, 20))) {
                        player.ToggleFavorite(queue[i]);
                    }
                } else {
                    if (ImGui::Button(player.IsFavorite(queue[i]) ? "★" : "☆", ImVec2(30, 20))) {
                        player.ToggleFavorite(queue[i]);
                    }
                }
                ImGui::SameLine();
                if (texClose) {
                    if (ImGui::ImageButton("btnRemove", texClose, ImVec2(20, 20))) {
                        queue.erase(queue.begin() + i);
                        i--;
                    }
                } else {
                    if (ImGui::Button("✕", ImVec2(30, 20))) {
                        queue.erase(queue.begin() + i);
                        i--;
                    }
                }
                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }

    void RenderLibrary(MidiPlayer& player) {
        ImGui::Text("MIDI Files Library & Saved");
        ImGui::Separator();
        ImGui::Spacing();
        
        auto& library = player.GetLibrary();

        if (ImGui::Button("Add Files", ImVec2(120, 30))) {
            auto files = OpenMidiDialogMulti();
            for (const auto& f : files) {
                player.AddToLibrary(f);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear All", ImVec2(120, 30))) {
            library.clear();
        }

        ImGui::Spacing();
        ImGui::BeginChild("LibraryList", ImVec2(0, 0), true);
        for (size_t i = 0; i < library.size(); i++) {
            ImGui::PushID((int)i);
            ImGui::Text("%s", GetFileNameFromPath(library[i]).c_str());
            
            // Push elements to the right
            ImGui::SameLine(ImGui::GetWindowWidth() - 160);
            
            if (texStarFilled && texStarEmpty) {
                if (ImGui::ImageButton("fav_lib", player.IsFavorite(library[i]) ? texStarFilled : texStarEmpty, ImVec2(20, 20))) {
                    player.ToggleFavorite(library[i]);
                }
            } else {
                if (ImGui::Button(player.IsFavorite(library[i]) ? "★" : "☆", ImVec2(30, 20))) {
                    player.ToggleFavorite(library[i]);
                }
            }
            ImGui::SameLine();
            
            if (ImGui::Button("Load", ImVec2(40, 20))) {
                player.AddToQueue(library[i]);
                player.LoadFromQueue(player.GetQueue().size() - 1);
            }
            ImGui::SameLine();
            if (ImGui::Button("+ Queue", ImVec2(60, 20))) {
                player.AddToQueue(library[i]);
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove", ImVec2(60, 20))) {
                library.erase(library.begin() + i);
                i--;
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    void RenderFavorites(MidiPlayer& player) {
        ImGui::Text("Favorite Tracks");
        ImGui::Separator();
        ImGui::Spacing();
        
        auto& favorites = player.GetFavorites();
        if (favorites.empty()) {
            ImGui::TextDisabled("You have no favorites yet. Click the empty star next to a song to add it here.");
            return;
        }

        ImGui::BeginChild("FavoritesList", ImVec2(0, 0), true);
        for (size_t i = 0; i < favorites.size(); i++) {
            ImGui::PushID((int)i);
            ImGui::Text("%s", GetFileNameFromPath(favorites[i]).c_str());
            ImGui::SameLine(ImGui::GetWindowWidth() - 160);
            
            if (texStarFilled) {
                if (ImGui::ImageButton("fav_list", texStarFilled, ImVec2(20, 20))) {
                    player.ToggleFavorite(favorites[i]);
                    i--;
                    ImGui::PopID();
                    continue;
                }
            } else {
                if (ImGui::Button("★", ImVec2(30, 20))) {
                    player.ToggleFavorite(favorites[i]);
                    i--;
                    ImGui::PopID();
                    continue;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Load", ImVec2(40, 20))) {
                player.AddToQueue(favorites[i]);
                player.LoadFromQueue(player.GetQueue().size() - 1);
            }
            ImGui::SameLine();
            if (ImGui::Button("+ Queue", ImVec2(60, 20))) {
                player.AddToQueue(favorites[i]);
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }

    std::string GetKeyName(int vk) {
        if (vk >= 0x70 && vk <= 0x87) return "F" + std::to_string(vk - 0x6F); // F1 - F24
        char name[128];
        UINT scanCode = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
        if (GetKeyNameTextA(scanCode << 16, name, sizeof(name)) > 0) return std::string(name);
        return "Unknown";
    }

    void RenderSettings(MidiPlayer& player) {
        ImGui::Text("Settings");
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Text("Hotkeys");
        
        // Start Key
        ImGui::Text("Start/Resume Playback:"); ImGui::SameLine(200);
        if (capturingStartKey) {
            ImGui::Button("Press any key...", ImVec2(150, 30));
            for (int i = 8; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    player.startKey = i;
                    capturingStartKey = false;
                    break;
                }
            }
        } else {
            if (ImGui::Button(GetKeyName(player.startKey).c_str(), ImVec2(150, 30))) {
                capturingStartKey = true;
                capturingStopKey = false;
            }
        }

        // Stop Key
        ImGui::Text("Stop/Pause Playback:"); ImGui::SameLine(200);
        if (capturingStopKey) {
            ImGui::Button("Press any key...", ImVec2(150, 30));
            for (int i = 8; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    player.stopKey = i;
                    capturingStopKey = false;
                    break;
                }
            }
        } else {
            if (ImGui::Button(GetKeyName(player.stopKey).c_str(), ImVec2(150, 30))) {
                capturingStopKey = true;
                capturingStartKey = false;
                capturingSpeedUpKey = false;
                capturingSpeedDownKey = false;
            }
        }

        ImGui::Spacing();

        // Speed Up Key
        ImGui::Text("Increase Speed (+):"); ImGui::SameLine(200);
        if (capturingSpeedUpKey) {
            ImGui::Button("Press any key...##1", ImVec2(150, 30));
            for (int i = 8; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    player.speedUpKey = i;
                    capturingSpeedUpKey = false;
                    break;
                }
            }
        } else {
            if (ImGui::Button(GetKeyName(player.speedUpKey).c_str(), ImVec2(150, 30))) {
                capturingSpeedUpKey = true;
                capturingStartKey = false;
                capturingStopKey = false;
                capturingSpeedDownKey = false;
            }
        }

        // Speed Down Key
        ImGui::Text("Decrease Speed (-):"); ImGui::SameLine(200);
        if (capturingSpeedDownKey) {
            ImGui::Button("Press any key...##2", ImVec2(150, 30));
            for (int i = 8; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    player.speedDownKey = i;
                    capturingSpeedDownKey = false;
                    break;
                }
            }
        } else {
            if (ImGui::Button(GetKeyName(player.speedDownKey).c_str(), ImVec2(150, 30))) {
                capturingSpeedDownKey = true;
                capturingStartKey = false;
                capturingStopKey = false;
                capturingSpeedUpKey = false;
                capturingNextKey = false;
                capturingPrevKey = false;
                capturingLoopKey = false;
                capturingShuffleKey = false;
            }
        }

        ImGui::Spacing();

        // Next Track Key
        ImGui::Text("Next Track:"); ImGui::SameLine(200);
        if (capturingNextKey) {
            ImGui::Button("Press any key...##3", ImVec2(150, 30));
            for (int i = 8; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    player.nextTrackKey = i;
                    capturingNextKey = false;
                    break;
                }
            }
        } else {
            if (ImGui::Button(GetKeyName(player.nextTrackKey).c_str(), ImVec2(150, 30))) {
                capturingNextKey = true;
                capturingStartKey = false;
                capturingStopKey = false;
                capturingSpeedUpKey = false;
                capturingSpeedDownKey = false;
                capturingPrevKey = false;
                capturingLoopKey = false;
                capturingShuffleKey = false;
            }
        }

        // Previous Track Key
        ImGui::Text("Previous Track:"); ImGui::SameLine(200);
        if (capturingPrevKey) {
            ImGui::Button("Press any key...##4", ImVec2(150, 30));
            for (int i = 8; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    player.prevTrackKey = i;
                    capturingPrevKey = false;
                    break;
                }
            }
        } else {
            if (ImGui::Button(GetKeyName(player.prevTrackKey).c_str(), ImVec2(150, 30))) {
                capturingPrevKey = true;
                capturingStartKey = false;
                capturingStopKey = false;
                capturingSpeedUpKey = false;
                capturingSpeedDownKey = false;
                capturingNextKey = false;
                capturingLoopKey = false;
                capturingShuffleKey = false;
            }
        }

        ImGui::Spacing();

        // Loop Toggle Key
        ImGui::Text("Toggle Loop:"); ImGui::SameLine(200);
        if (capturingLoopKey) {
            ImGui::Button("Press any key...##5", ImVec2(150, 30));
            for (int i = 8; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    player.loopKey = i;
                    capturingLoopKey = false;
                    break;
                }
            }
        } else {
            if (ImGui::Button(GetKeyName(player.loopKey).c_str(), ImVec2(150, 30))) {
                capturingLoopKey = true;
                capturingStartKey = false;
                capturingStopKey = false;
                capturingSpeedUpKey = false;
                capturingSpeedDownKey = false;
                capturingNextKey = false;
                capturingPrevKey = false;
                capturingShuffleKey = false;
            }
        }

        // Shuffle Key
        ImGui::Text("Shuffle Queue:"); ImGui::SameLine(200);
        if (capturingShuffleKey) {
            ImGui::Button("Press any key...##6", ImVec2(150, 30));
            for (int i = 8; i < 256; i++) {
                if (GetAsyncKeyState(i) & 0x8000) {
                    player.shuffleKey = i;
                    capturingShuffleKey = false;
                    break;
                }
            }
        } else {
            if (ImGui::Button(GetKeyName(player.shuffleKey).c_str(), ImVec2(150, 30))) {
                capturingShuffleKey = true;
                capturingStartKey = false;
                capturingStopKey = false;
                capturingSpeedUpKey = false;
                capturingSpeedDownKey = false;
                capturingNextKey = false;
                capturingPrevKey = false;
                capturingLoopKey = false;
            }
        }

        ImGui::Spacing();
        if (ImGui::Button("Reset Hotkeys", ImVec2(150, 30))) {
            player.ResetHotkeys();
        }

        ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        ImGui::Text("External");
        if (ImGui::Button("Launch Roblox", ImVec2(200, 40))) {
            ShellExecuteA(NULL, "open", "roblox-player:", NULL, NULL, SW_SHOWNORMAL);
        }

        ImGui::Spacing(); ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
        
        ImGui::Text("Contact Me");
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.1f));

        if (texDiscord) {
            if (ImGui::ImageButton("btnDiscord", texDiscord, ImVec2(32, 32))) {
                ShellExecuteA(NULL, "open", "https://discord.gg/JPZgv2efjq", NULL, NULL, SW_SHOWNORMAL);
            }
        } else {
            if (ImGui::Button("Discord", ImVec2(80, 30))) {
                ShellExecuteA(NULL, "open", "https://discord.gg/JPZgv2efjq", NULL, NULL, SW_SHOWNORMAL);
            }
        }
        ImGui::SameLine();

        if (texGithub) {
            if (ImGui::ImageButton("btnGithub", texGithub, ImVec2(32, 32))) {
                ShellExecuteA(NULL, "open", "https://github.com/munishhhh", NULL, NULL, SW_SHOWNORMAL);
            }
        } else {
            if (ImGui::Button("GitHub", ImVec2(80, 30))) {
                ShellExecuteA(NULL, "open", "https://github.com/munishhhh", NULL, NULL, SW_SHOWNORMAL);
            }
        }
        ImGui::SameLine();

        if (texInstagram) {
            if (ImGui::ImageButton("btnInstagram", texInstagram, ImVec2(32, 32))) {
                ShellExecuteA(NULL, "open", "https://instagram.com/Munishhhh_", NULL, NULL, SW_SHOWNORMAL);
            }
        } else {
            if (ImGui::Button("Instagram", ImVec2(80, 30))) {
                ShellExecuteA(NULL, "open", "https://instagram.com/Munishhhh_", NULL, NULL, SW_SHOWNORMAL);
            }
        }

        ImGui::PopStyleColor(2);
    }

    void RenderPlayerControls(MidiPlayer& player) {
        ImGui::BeginChild("PlayerControls", ImVec2(0, 80), true);

        // Progress Bar/Slider
        float progress = player.GetProgress();
        ImGui::SetNextItemWidth(-10); // Fill width
        if (ImGui::SliderFloat("##progress", &progress, 0.0f, 1.0f, "")) {
            player.SetProgress(progress);
        }

        ImGui::Spacing();

        // Now file info and buttons
        ImGui::BeginGroup();
        if (player.IsLoaded()) {
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.9f, 1.0f), "%s", player.GetCurrentFileName().c_str());
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.6f, 1.0f), player.IsPlaying() ? "Playing" : (player.IsPaused() ? "Paused" : "Stopped"));
        } else {
            ImGui::TextDisabled("No Track Playing");
        }
        ImGui::EndGroup();

        ImGui::SameLine(ImGui::GetWindowWidth() / 2 - 120);

        // Buttons
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0)); // Transparent buttons
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.1f));
        
        if (texPrev) {
            if (ImGui::ImageButton("btnPrev", texPrev, ImVec2(32, 32))) player.PreviousTrack();
        } else {
            if (ImGui::Button("|<", ImVec2(32, 32))) player.PreviousTrack();
        }
        ImGui::SameLine();

        if (texPlay && texPause && texStop) {
            bool isPlaying = player.IsPlaying();
            ImTextureID playPauseTex = isPlaying ? texPause : texPlay;
            if (ImGui::ImageButton("btnPlayPause", playPauseTex, ImVec2(32, 32))) {
                if (isPlaying) player.Pause();
                else player.Play();
            }
            ImGui::SameLine();
            if (ImGui::ImageButton("btnStop", texStop, ImVec2(32, 32))) {
                player.Stop();
            }
        } else {
            if (ImGui::Button(player.IsPlaying() ? "||" : ">", ImVec2(40, 40))) {
                if (player.IsPlaying()) player.Pause();
                else player.Play();
            }
            ImGui::SameLine();
            if (ImGui::Button("[]", ImVec2(40, 40))) {
                player.Stop();
            }
        }

        ImGui::SameLine();
        if (texNext) {
            if (ImGui::ImageButton("btnNext", texNext, ImVec2(32, 32))) player.NextTrack();
        } else {
            if (ImGui::Button(">|", ImVec2(32, 32))) player.NextTrack();
        }
        ImGui::SameLine();
        
        if (player.loopQueue) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.5f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 0.4f)); // slight green highlight
        }
        if (texLoop) {
            if (ImGui::ImageButton("btnLoop", texLoop, ImVec2(32, 32))) {
                player.loopQueue = !player.loopQueue;
            }
        } else {
            if (ImGui::Button("LOOP", ImVec2(40, 32))) {
                player.loopQueue = !player.loopQueue;
            }
        }
        if (player.loopQueue) {
            ImGui::PopStyleColor(2);
        }

        ImGui::PopStyleColor(2);

        ImGui::EndChild();
    }

    void Render(MidiPlayer& player) {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        
        ImGuiWindowFlags window_flags = 0
            | ImGuiWindowFlags_NoTitleBar 
            | ImGuiWindowFlags_NoResize 
            | ImGuiWindowFlags_NoMove 
            | ImGuiWindowFlags_NoCollapse 
            | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoBackground; 

        ImGui::Begin("Main UI", nullptr, window_flags);
        
        // Custom background
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        // Fully transparent inner fill to rely only on OS DWM blur + ImGui Style WindowBg
        draw_list->AddRectFilled(viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y), IM_COL32(0, 0, 0, 10)); // Barely visible tint
        draw_list->AddRect(viewport->Pos, ImVec2(viewport->Pos.x + viewport->Size.x, viewport->Pos.y + viewport->Size.y), IM_COL32(255, 255, 255, 30), 0.0f, 0, 1.5f);

        // Title at Top Center
        ImGui::SetCursorPos(ImVec2(10, 12));
        ImGui::TextColored(ImVec4(0.8f, 0.9f, 1.0f, 1.0f), "Brothers Regedit Roblox Midi Player");

        // Window Controls (Top Right)
        HWND hwnd = (HWND)viewport->PlatformHandleRaw;
        ImGui::SetCursorPos(ImVec2(viewport->Size.x - 120, 10));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1,1,1,0.2f));
        if (texMinimize && ImGui::ImageButton("btnMin", texMinimize, ImVec2(20, 20))) {
            PostMessageA(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        } else if (!texMinimize && ImGui::Button("_", ImVec2(30, 20))) {
            PostMessageA(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
        }
        ImGui::SameLine();
        if (texMaximize && ImGui::ImageButton("btnMax", texMaximize, ImVec2(20, 20))) {
            if (IsZoomed(hwnd)) PostMessageA(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
            else PostMessageA(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        } else if (!texMaximize && ImGui::Button("[ ]", ImVec2(30, 20))) {
            if (IsZoomed(hwnd)) PostMessageA(hwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
            else PostMessageA(hwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 0.8f));
        if (texClose && ImGui::ImageButton("btnClose", texClose, ImVec2(20, 20))) {
            PostMessageA(hwnd, WM_CLOSE, 0, 0);
        } else if (!texClose && ImGui::Button("✕", ImVec2(30, 20))) {
            PostMessageA(hwnd, WM_CLOSE, 0, 0);
        }
        ImGui::PopStyleColor(); // End close hovered
        ImGui::PopStyleColor(2);

        // Leave top 40 pixels empty for dragging
        ImGui::SetCursorPos(ImVec2(0, 40));

        // Sidebar
        ImGui::BeginChild("Sidebar", ImVec2(200, -85), true);
        ImGui::Spacing();
        
        if (texLogo) {
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 40);
            ImGui::Image(texLogo, ImVec2(100, 100));
            ImGui::Spacing();
        }
        
        ImGui::Separator();
        ImGui::Spacing();

        if (texTabDash) { ImGui::Image(texTabDash, ImVec2(20, 20)); ImGui::SameLine(); }
        if (ImGui::Selectable(" Dashboard", currentTab == Tab::Dashboard)) currentTab = Tab::Dashboard;
        
        if (texTabMidi) { ImGui::Image(texTabMidi, ImVec2(20, 20)); ImGui::SameLine(); }
        if (ImGui::Selectable(" MIDI Files", currentTab == Tab::Library)) currentTab = Tab::Library;
        
        if (texStarFilled) { ImGui::Image(texStarFilled, ImVec2(20, 20)); ImGui::SameLine(); }
        if (ImGui::Selectable(" Favorites", currentTab == Tab::Favorites)) currentTab = Tab::Favorites;
        
        if (texTabSettings) { ImGui::Image(texTabSettings, ImVec2(20, 20)); ImGui::SameLine(); }
        if (ImGui::Selectable(" Settings", currentTab == Tab::Settings)) currentTab = Tab::Settings;
        
        ImGui::EndChild();
        
        ImGui::SameLine();

        // Main Content Area
        ImGui::BeginChild("MainContent", ImVec2(0, -85), false);
        switch(currentTab) {
            case Tab::Dashboard: RenderDashboard(player); break;
            case Tab::Library: RenderLibrary(player); break;
            case Tab::Favorites: RenderFavorites(player); break;
            case Tab::Settings: RenderSettings(player); break;
        }
        ImGui::EndChild();

        // Bottom Player
        RenderPlayerControls(player);

        ImGui::End();
    }
}
