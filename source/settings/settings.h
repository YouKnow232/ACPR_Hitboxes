#pragma once

#include <d3d9.h>
#include <filesystem>

namespace ACPRHitboxes {

    enum class DrawOperation {
        None, MiscRange, Push, Hurt, Hit, CleanHit, Grab, Pivot
    };

    void LoadSettingsFromFile(std::filesystem::path path);

    struct Palette {
        D3DCOLOR Default = 0x00FF0000;
        D3DCOLOR Hitbox = 0x80FF0000;
        D3DCOLOR Hurtbox = 0x8000FF00;
        D3DCOLOR Push = 0x8000FFFF;
        D3DCOLOR Grab = 0x80FF00FF;
        D3DCOLOR CLHitbox = 0x80FF8000;
        D3DCOLOR MiscPushRange = 0x80FF00FF;
        D3DCOLOR MiscPivotRange = 0x80FF8000;
        D3DCOLOR PivotCross = 0xFF800080;
    };

    class SettingsManager {
        public:
        static SettingsManager& GetInstance() {
            static SettingsManager instance;
            return instance;
        }

        bool Display = true;
        bool CombineBoxes = true;
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
        Palette Palette;

        protected:
        SettingsManager() = default;
        SettingsManager(const SettingsManager&) = default;
        SettingsManager(SettingsManager&&) = default;
        SettingsManager& operator=(const SettingsManager&) = delete;
        SettingsManager& operator=(SettingsManager&&) = delete;
        ~SettingsManager() = default;
    };

}