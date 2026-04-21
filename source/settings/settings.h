#pragma once

#include <d3d9.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>

constexpr int _serializationVersion = 1;

namespace ACPRHitboxes {

    enum class DrawOperation {
        None, MiscRange, Push, Hurt, Hit, CleanHit, Grab, Pivot
    };

    struct Palette {
        int Alpha = 0x80;
        D3DCOLOR Default = 0xFF0000;
        D3DCOLOR Hitbox = 0xFF0000;
        D3DCOLOR Hurtbox = 0x00FF00;
        D3DCOLOR Push = 0x00FFFF;
        D3DCOLOR Grab = 0xFF00FF;
        D3DCOLOR CLHitbox = 0xFF8000;
        D3DCOLOR MiscPushRange = 0xFF00FF;
        D3DCOLOR MiscPivotRange = 0xFF8000;
        D3DCOLOR PivotCross = 0xFF800080;
    };

    class SettingsManager {
        public:
        static SettingsManager& GetInstance() {
            static SettingsManager instance;
            return instance;
        }

        public:
        bool Display = true;
        bool CombineBoxes = false;
        float PivotCrossSize = 15.0f;
        float PivotCrossThickness = 1.0f;
        float HitboxBorderThickness = 2.0f;
        int MinimumBoxHeight = 500;
        int MaximumBoxHeight = 100000;
        bool WidescreenClipping = true;
        int HidePlayer = 0;
        bool AlwaysDisplayThrowRange = false;
        DrawOperation DrawOrder[7] = {
            DrawOperation::MiscRange,
            DrawOperation::Push,
            DrawOperation::Hurt,
            DrawOperation::Hit,
            DrawOperation::CleanHit,
            DrawOperation::Grab,
            DrawOperation::Pivot,
        };
        bool HideMiscRanges = false;
        bool HidePush = false;
        bool HideHurt = false;
        bool HideHit = false;
        bool HideCleanHit = false;
        bool HideGrab = false;
        bool HidePivot = false;
        Palette Palette;

        void SetToDefaults() {
            *this = SettingsManager();
        }

        void VisitFields(auto lambda) {
            lambda(Display);
            lambda(CombineBoxes);
            lambda(PivotCrossSize);
            lambda(PivotCrossThickness);
            lambda(HitboxBorderThickness);
            lambda(MinimumBoxHeight);
            lambda(MaximumBoxHeight);
            lambda(WidescreenClipping);
            lambda(AlwaysDisplayThrowRange);
            lambda(DrawOrder);
            lambda(HideMiscRanges);
            lambda(HidePush);
            lambda(HideHurt);
            lambda(HideHit);
            lambda(HideCleanHit);
            lambda(HideGrab);
            lambda(HidePivot);
            lambda(Palette.Alpha);
            lambda(Palette.Default);
            lambda(Palette.Hitbox);
            lambda(Palette.Hurtbox);
            lambda(Palette.Push);
            lambda(Palette.Grab);
            lambda(Palette.CLHitbox);
            lambda(Palette.MiscPushRange);
            lambda(Palette.MiscPivotRange);
            lambda(Palette.PivotCross);
        }

        void Serialize(const std::string filename) {
            std::filesystem::path path = filename;
            std::ofstream file(path.string(), std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "[Hitboxes] Error saving settings file: " << filename.c_str() << std::endl;
                return;
            }
            auto write = [&file](auto&& data) {
                file.write(reinterpret_cast<char*>(&data), sizeof(data));
            };

            int serializationVersion = _serializationVersion;
            write(serializationVersion);
            VisitFields(write);
        }

        void Deserialize(const std::string filename) {
            std::filesystem::path path = filename;
            if (!std::filesystem::exists(path)) {
                std::cout << "[Hitboxes] No saved settings found." << std::endl;
                return;
            }
            std::ifstream file(path.string(), std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "[Hitboxes] Error loading settings file: " << filename.c_str() << std::endl;
                return;
            }
            
            char buffer[32];
            auto load = [&file, &buffer](auto&& data){
                file.read(reinterpret_cast<char*>(&data), sizeof(data));
            };

            int serializedVersion;
            load(serializedVersion);
            if (serializedVersion != _serializationVersion) {
                std::cerr << "[Hitboxes] Serialized settings are not a supported version! They will be ignored" << std::endl;
                return;
            }
            VisitFields(load);
        }

        protected:
        SettingsManager() = default;
        SettingsManager(const SettingsManager&) = default;
        SettingsManager(SettingsManager&&) = default;
        SettingsManager& operator=(const SettingsManager&) = default;
        SettingsManager& operator=(SettingsManager&&) = default;
        ~SettingsManager() = default;
    };
    static_assert(std::is_trivially_copyable_v<SettingsManager>);

}