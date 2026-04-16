#pragma once

#include <d3d9.h>
#include <filesystem>

namespace ACPRHitboxes {

    enum class DrawOperation {
        None, MiscRange, Push, Hurt, Hit, CleanHit, Grab, Pivot
    };

    void SaveSettingsToFile(std::filesystem::path path);
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
        bool HideMiscRanges = false;
        bool HidePush = false;
        bool HideHurt = false;
        bool HideHit = false;
        bool HideCleanHit = false;
        bool HideGrab = false;
        bool HidePivot = false;
        Palette Palette;

        // TODO: Better way to do this without copying default values?
        void SetToDefaults() {
            Display = true;
            CombineBoxes = true;
            PivotCrossThickness = 1.0f;
            HitboxBorderThickness = 2.0f;
            MinimumBoxHeight = 500;
            MaximumBoxHeight = 100000;
            WidescreenClipping = true;
            HidePlayer = 0;
            AlwaysDisplayThrowRange = false;
            DrawOrder[0] = DrawOperation::MiscRange;
            DrawOrder[1] = DrawOperation::Push;
            DrawOrder[2] = DrawOperation::Hurt;
            DrawOrder[3] = DrawOperation::Hit;
            DrawOrder[4] = DrawOperation::CleanHit;
            DrawOrder[5] = DrawOperation::Grab;
            DrawOrder[6] = DrawOperation::Pivot;
            Palette.Default = 0x00FF0000;
            Palette.Hitbox = 0x80FF0000;
            Palette.Hurtbox = 0x8000FF00;
            Palette.Push = 0x8000FFFF;
            Palette.Grab = 0x80FF00FF;
            Palette.CLHitbox = 0x80FF8000;
            Palette.MiscPushRange = 0x80FF00FF;
            Palette.MiscPivotRange = 0x80FF8000;
            Palette.PivotCross = 0xFF800080;
            HideMiscRanges = false;
            HidePush = false;
            HideHurt = false;
            HideHit = false;
            HideCleanHit = false;
            HideGrab = false;
            HidePivot = false;
        }

        protected:
        SettingsManager() = default;
        SettingsManager(const SettingsManager&) = default;
        SettingsManager(SettingsManager&&) = default;
        SettingsManager& operator=(const SettingsManager&) = delete;
        SettingsManager& operator=(SettingsManager&&) = delete;
        ~SettingsManager() = default;
    };

}