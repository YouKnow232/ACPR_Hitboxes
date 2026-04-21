#include "modMenu.h"
#include <string>
#include <vector>
#include "gearLoader/ggxxacpr.hpp"
#include "baseMod/baseMod.hpp"
#include "settings.h"
#include "dataStore/dataStore.h"


using namespace ACPRHitboxes;
using Input = ggxxacpr::RawControllerInput;

struct MenuEntry {
    const char* Label;
    std::vector<const char*> ValueLabels;
    int value;
    int maxValue;

    bool IsEnumSetting() { return ValueLabels.size() > 0; }
    bool IsGaugeSetting() { return ValueLabels.size() == 0 && maxValue > 0; }
};

struct ScrollingSelectionState {
    int Selection;
    int ScrollOffset;
    int MaxEntries;
    int MaxDisplayable;

    int GetVisualIndex() { return Selection - ScrollOffset; }
    int GetIndex(int visualIndex) { return visualIndex + ScrollOffset; }
    void UpdateScrollingSelection(int selection) {
        Selection = selection;

        if (Selection > ScrollOffset + MaxDisplayable - 1) ScrollOffset = Selection - MaxDisplayable + 1;
        if (Selection < ScrollOffset) ScrollOffset = Selection;
    }
    bool CanScrollUp() { return ScrollOffset > 0; }
    bool CanScrollDown() { return ScrollOffset + MaxDisplayable < MaxEntries; }
};

enum Setting {
    DISPLAY,
    THROW_BOXES,
    COMBINE,
    OPACITY,
    BORDER,
    PIVOT_SIZE,
    PIVOT_THICKNESS,
    CLIPPING,
    TOGGLE_HEADER,
    TOGGLE_PUSH,
    TOGGLE_HURT,
    TOGGLE_HIT,
    TOGGLE_CLEAN_HIT,
    TOGGLE_THROW,
    TOGGLE_PIVOT,
    TOGGLE_MISC_RANGE,
    DEFAULT
};

enum CommonSoundEffectIds {
    SE_GAUGE    = 0x06,
    SE_SELECT   = 0x37,
    SE_ACCEPT   = 0x39,
    SE_EXIT     = 0x3B,
};

inline ggxxacpr::ColorVertex* GetColorPickerGeometry(int yPos) {
    static const ggxxacpr::ColorVertex base[4] = {
        {{330.0f, -10.0f, 4.0f}, 0xFFFF0000},
        {{370.0f, -10.0f, 4.0f}, 0xFFFF0000},
        {{330.0f,  20.0f, 4.0f}, 0xFF0000FF},
        {{370.0f,  20.0f, 4.0f}, 0xFF00FF00},
    };
    static ggxxacpr::ColorVertex buffer[4] = {
        {{330.0f, -10.0f, 4.0f}, 0xFFFF0000},
        {{370.0f, -10.0f, 4.0f}, 0xFFFF0000},
        {{330.0f,  20.0f, 4.0f}, 0xFF0000FF},
        {{370.0f,  20.0f, 4.0f}, 0xFF00FF00},
    };
    for (int i = 0; i < 4; i++) {
        buffer[i].Vector.y = base[i].Vector.y + yPos;
    }
    return buffer;
}

inline bool EitherInputting(ggxxacpr::PlayerInput* inputArr, ggxxacpr::RawControllerInput input) {
    return  (
        (inputArr[0].InputRaw1 | inputArr[1].InputRaw1) &
        static_cast<uint32_t>(input)
    ) != 0;
}
inline bool EitherPressed(ggxxacpr::PlayerInput* inputArr, ggxxacpr::RawControllerInput input) {
    return  (
        (inputArr[0].InputPress1 | inputArr[1].InputPress1) &
        static_cast<uint32_t>(input)
    ) != 0;
}
inline void UpdateMenuEntryValues(MenuEntry* entries, SettingsManager& settings) {
    entries[DISPLAY].value = settings.HidePlayer;
    entries[THROW_BOXES].value = settings.AlwaysDisplayThrowRange;
    entries[COMBINE].value = settings.CombineBoxes;
    entries[OPACITY].value = settings.Palette.Alpha;
    entries[BORDER].value = settings.HitboxBorderThickness;
    entries[PIVOT_SIZE].value = settings.PivotCrossSize;
    entries[PIVOT_THICKNESS].value = settings.PivotCrossThickness;
    entries[CLIPPING].value = settings.WidescreenClipping;
    entries[TOGGLE_PUSH].value = settings.HidePush;
    entries[TOGGLE_HURT].value = settings.HideHurt;
    entries[TOGGLE_HIT].value = settings.HideHit;
    entries[TOGGLE_CLEAN_HIT].value = settings.HideCleanHit;
    entries[TOGGLE_THROW].value = settings.HideGrab;
    entries[TOGGLE_PIVOT].value = settings.HidePivot;
    entries[TOGGLE_MISC_RANGE].value = settings.HideMiscRanges;
}

void BASEMOD_CALL CustomMenuHandler(ggxxacpr::PlayerInput* inputArr) {
    constexpr int numEntries = 17;
    constexpr int margin = 24;
    constexpr int entrySpacing = 32;
    constexpr float zPos = 4.0f;

    auto& settings = SettingsManager::GetInstance();
    
    static MenuEntry entries[numEntries] = {
        {"DISPLAY", {"ALL", "P2 ONLY", "P1 ONLY", "NONE"}, settings.HidePlayer, 3},
        {"THROW BOXES", {"NORMAL", "ALWAYS"}, settings.AlwaysDisplayThrowRange, 1},
        {"COMBINE BOXES", {"OFF", "ON"}, settings.CombineBoxes, 1},
        {"OPACITY", {}, static_cast<int>(settings.Palette.Alpha), 255},
        {"BORDER SIZE", {}, static_cast<int>(settings.HitboxBorderThickness), 100},
        {"PIVOT SIZE", {}, static_cast<int>(settings.PivotCrossSize), 100},
        {"PIVOT THICKNESS", {}, static_cast<int>(settings.PivotCrossThickness), 20},
        {"WIDESCREEN CLIP", {"OFF", "ON"}, settings.WidescreenClipping, 1},
        {"TOGGLE BOXES:", {}, 0, 0},
        {"  PUSH", {"SHOW", "HIDE"}, settings.HidePush, 1},
        {"  HURT", {"SHOW", "HIDE"}, settings.HideHurt, 1},
        {"  HIT", {"SHOW", "HIDE"}, settings.HideHit, 1},
        {"  CLEAN HIT", {"SHOW", "HIDE"}, settings.HideCleanHit, 1},
        {"  THROW", {"SHOW", "HIDE"}, settings.HideGrab, 1},
        {"  PIVOT", {"SHOW", "HIDE"}, settings.HidePivot, 1},
        {"  MISC RANGE", {"SHOW", "HIDE"}, settings.HideMiscRanges, 1},
        {"DEFAULT", {}, 0, 0},
    };

    BaseMod::Api api = BMApi();

    if (!api.IsValid()) return;

    static auto dim = api.ModMenu.GetDrawableAreaDimensions();
    static ScrollingSelectionState menuState = {
        Selection:      0,
        ScrollOffset:   0,
        MaxEntries:     numEntries,
        MaxDisplayable: (static_cast<int>(dim.bottom - dim.top - margin * 2) / entrySpacing)
    };

    menuState.UpdateScrollingSelection(api.ModMenu.HelperFunctions.SelectionHandler(menuState.Selection, numEntries));
    if (menuState.Selection == TOGGLE_HEADER) {
        if (EitherInputting(inputArr, Input::UP))
            menuState.UpdateScrollingSelection(TOGGLE_HEADER - 1);
        else if (EitherInputting(inputArr, Input::DOWN))
            menuState.UpdateScrollingSelection(TOGGLE_HEADER + 1);
    }

    // Left/Right Functionality
    bool leftInputted = api.ModMenu.HelperFunctions.HoldDirectionInputHandler(ggxxacpr::RawControllerInput::LEFT);
    bool rightInputted = api.ModMenu.HelperFunctions.HoldDirectionInputHandler(ggxxacpr::RawControllerInput::RIGHT);
    if (leftInputted || rightInputted) {
        auto& entry = entries[menuState.Selection];

        if (entry.IsEnumSetting() || entry.IsGaugeSetting()) {
            int increment = leftInputted ? -1 : 1;
            if (EitherInputting(inputArr, Input::BOTTOM_FACE)) increment *= 10;
            entry.value += increment;
            if (entry.IsEnumSetting()) {
                if (entry.value < 0) entry.value = entry.maxValue;
                if (entry.value > entry.maxValue) entry.value = 0;
            } else if (entry.IsGaugeSetting()) {
                if (entry.value < 0) entry.value = 0;
                if (entry.value > entry.maxValue) entry.value = entry.maxValue;
            }
            api.NativeFunctions.PlayCommonSoundEffect(
                entry.IsEnumSetting() ? SE_ACCEPT : SE_GAUGE
            );
        }

        switch (menuState.Selection) {
            case DISPLAY:
                settings.HidePlayer = entry.value;
                break;
            case THROW_BOXES:
                settings.AlwaysDisplayThrowRange = entry.value;
                break;
            case COMBINE:
                settings.CombineBoxes = entry.value;
                break;
            case OPACITY:
                settings.Palette.Alpha = entry.value;
                break;
            case BORDER:
                settings.HitboxBorderThickness = static_cast<float>(entry.value);
                break;
            case PIVOT_SIZE:
                settings.PivotCrossSize = static_cast<float>(entry.value);
                break;
            case PIVOT_THICKNESS:
                settings.PivotCrossThickness = static_cast<float>(entry.value);
                break;
            case CLIPPING:
                settings.WidescreenClipping = entry.value;
                break;
            case TOGGLE_PUSH:
                settings.HidePush = entry.value;
                break;
            case TOGGLE_HURT:
                settings.HideHurt = entry.value;
                break;
            case TOGGLE_HIT:
                settings.HideHit = entry.value;
                break;
            case TOGGLE_CLEAN_HIT:
                settings.HideCleanHit = entry.value;
                break;
            case TOGGLE_THROW:
                settings.HideGrab = entry.value;
                break;
            case TOGGLE_PIVOT:
                settings.HidePivot = entry.value;
                break;
            case TOGGLE_MISC_RANGE:
                settings.HideMiscRanges = entry.value;
                break;
        }
    }

    // Extended functionality
    if (EitherPressed(inputArr, Input::BOTTOM_FACE)) {
        switch (menuState.Selection) {
            case DEFAULT:
                api.NativeFunctions.PlayCommonSoundEffect(SE_ACCEPT);
                settings.SetToDefaults();
                UpdateMenuEntryValues(entries, settings);
                break;
        }
    }

    // Drawing
    const int labelX = dim.left + 32;
    for (int visualIndex = 0; visualIndex < menuState.MaxDisplayable; visualIndex++) {
        if (menuState.GetIndex(visualIndex) >= numEntries) break;
        auto& entry = entries[menuState.GetIndex(visualIndex)];
        int yPos = dim.top + margin + visualIndex * entrySpacing;
        api.NativeFunctions.RenderMenuText(
            entry.Label,
            labelX,
            yPos,
            zPos,
            menuState.Selection == menuState.GetIndex(visualIndex) ? 1.0f : 0.5f,
            nullptr, 0,
            false, 0xFFFFFFFF
        );

        // Is standard enum setting
        if (entry.IsEnumSetting()) {
            api.ModMenu.HelperFunctions.DrawEnumSettingUI(
                entry.ValueLabels[entry.value],
                0,
                yPos,
                menuState.Selection == menuState.GetIndex(visualIndex)
            );
        }
        // Is standard gauge setting
        else if (entry.IsGaugeSetting()) {
            api.ModMenu.HelperFunctions.DrawGaugeSettingUI(
                entry.value,
                yPos,
                menuState.Selection == menuState.GetIndex(visualIndex),
                entry.maxValue
            );
        }
        // Custom setting UI
        else {
            switch(menuState.GetIndex(visualIndex)) {
                //
            }
        }
    }

    if (menuState.CanScrollUp()) {
        api.ModMenu.HelperFunctions.DrawScrollArrow(BaseMod::DrawScrollArrowFlags::UP);
    }
    if (menuState.CanScrollDown()) {
        api.ModMenu.HelperFunctions.DrawScrollArrow(BaseMod::DrawScrollArrowFlags::DOWN);
    }
}

void RegisterModMenu(BaseMod::ModMenuApi& api) {
    api.RegisterCustomMenuTab("HITBOXES", CustomMenuHandler);
}
