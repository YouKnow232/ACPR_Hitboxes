#pragma once

#include <windows.h>
#include "baseMod/baseMod.hpp"


struct Quad {
    int left, right, top, bottom;

    inline int x() {return left;}
    inline int y() {return top;}
    inline int width() {return right - left;}
    inline int height() {return top - bottom;}
};

ggxxacpr::WorldCoordinate GetPlayerPushBoxDimensions(BaseMod::Api* api, ggxxacpr::Player& p) {
    ggxxacpr::WorldCoordinate output;

    ggxxacpr::GameVersion version = api->GameData.GetGameVersion();
    ggxxacpr::ActionState actState = p.actionState();
    
    BaseMod::PushboxDimensionArrayType widthArrType;
    BaseMod::PushboxDimensionArrayType heightArrType;
    if ((actState & ggxxacpr::ActionState::IS_AIRBORNE) != ggxxacpr::ActionState::NONE) {
        widthArrType = BaseMod::PushboxDimensionArrayType::AIRBORNE_WIDTH;
        heightArrType = BaseMod::PushboxDimensionArrayType::AIRBORNE_HEIGHT;
    } else if ((actState & ggxxacpr::ActionState::IS_CROUCHING) != ggxxacpr::ActionState::NONE) {
        widthArrType = BaseMod::PushboxDimensionArrayType::CROUCHING_WIDTH;
        heightArrType = BaseMod::PushboxDimensionArrayType::CROUCHING_HEIGHT;
    } else if ((actState & ggxxacpr::ActionState::WAKEUP) != ggxxacpr::ActionState::NONE) {
        widthArrType = BaseMod::PushboxDimensionArrayType::AIRBORNE_WIDTH;
        heightArrType = version == ggxxacpr::GameVersion::ACCENT_CORE ?
            BaseMod::PushboxDimensionArrayType::STANDING_HEIGHT_AC :
            BaseMod::PushboxDimensionArrayType::STANDING_HEIGHT_PR;
    } else {    // standing
        widthArrType = BaseMod::PushboxDimensionArrayType::STANDING_WIDTH;
        heightArrType = version == ggxxacpr::GameVersion::ACCENT_CORE ?
            BaseMod::PushboxDimensionArrayType::STANDING_HEIGHT_AC :
            BaseMod::PushboxDimensionArrayType::STANDING_HEIGHT_PR;
    }
    
    uint16_t* widths = api->GameData.CharacterData.GetPushboxDimensionArray(widthArrType);
    uint16_t* heights = api->GameData.CharacterData.GetPushboxDimensionArray(heightArrType);
    
    short rawId = static_cast<short>(p.id());
    output.x = widths[rawId];
    output.y = heights[rawId];

    return output;
}

int CommandThrowIsActive(ggxxacpr::Player& p) {
    // TODO: Is there a way to check for this without hard coding data?
    // Could just invoke function GGXXACPR_Win.exe+1201d0
    struct CharAct {ggxxacpr::EntityId charId; uint16_t actId; uint32_t cmdGrabId;};
    static CharAct CommandGrabActIds[] = {
        {ggxxacpr::EntityId::SOL,       0x088,  1}, // Wild Throw
        {ggxxacpr::EntityId::KY,        0x109,  2}, // EX Ky Elegant Slash
        {ggxxacpr::EntityId::MAY,       0x06B,  4}, // Overhead Kiss
        {ggxxacpr::EntityId::MAY,       0x0A2, 15}, // IK
        {ggxxacpr::EntityId::POTEMKIN,  0x07A,  3}, // Potbuster
        {ggxxacpr::EntityId::CHIPP,     0x08F,  6}, // Leaf Grab
        {ggxxacpr::EntityId::CHIPP,     0x130, 22}, // EX Chipp grab super
        {ggxxacpr::EntityId::EDDIE,     0x07C,  5}, // Damned Fang
        {ggxxacpr::EntityId::BAIKEN,    0x109, 23}, // EX Baiken grab super
        {ggxxacpr::EntityId::JAM,       0x0EE, 14}, // Unknown (Unused)
        {ggxxacpr::EntityId::SLAYER,    0x070, 18}, // BSU
        {ggxxacpr::EntityId::ZAPPA,     0x07E, 20}, // IK
        {ggxxacpr::EntityId::BRIDGET,   0x11B, 24}, // EX Bridget grab super
        {ggxxacpr::EntityId::ROBO_KY,   0x0DF, 21}, // S-KY-line
        {ggxxacpr::EntityId::ABA,       0x112, 25}, // Close Key Grab
        {ggxxacpr::EntityId::ABA,       0x113, 25}, // Moroha/ABA EX Close Key Grab

        // These two cmdGrabIds are associated with ABA's air keygrab (one act for normal and one for moroha mode).
        //      The `mark` var is set to 1 while ABA is falling after a successful air keygrab, but there's no
        //      known use for this cmdGrabId here.

        // {ggxxacpr::EntityId::ABA,       0x11A, 26}, // Unknown (Unused Air keygrab?) actId 282, cmdGrabId 26
        // {ggxxacpr::EntityId::ABA,       0x11B, 26}, // Unknown (Unused Air keygrab?) actId 283, cmdGrabId 26
    };

    if (p.getRaw()->mark == 1) {
        for (auto& entry : CommandGrabActIds) {
            if (entry.charId == p.id() && entry.actId == p.actId()) {
                return entry.cmdGrabId;
            }
        }
    }

    return 0;
}

bool IsUniversalThrowActive(BaseMod::Api* api, ggxxacpr::Player& p) {
    uint32_t universalThrowFlags = api->GameData.GetGlobalThrowFlags();
    
    auto opponent = api->GameData.GetPlayer(!p.playerIndex());
    auto pRaw = p.getRaw();

    return (pRaw->commandFlags & COMMAND_STATE_DISABLE_TRHOW) == 0 &&
        universalThrowFlags & (p.playerIndex() + 1) != 0 &&
        (opponent.actionState() & ggxxacpr::ActionState::IS_IN_HITSTUN) != ggxxacpr::ActionState::NONE;
}

Quad GetUniversalGroundThrowQuad(BaseMod::Api* api, ggxxacpr::Player& p) {
    static const int16_t* range_AC = api->GameData.CharacterData
        .GetThrowRangeArray(BaseMod::ThrowRangeArrayType::GROUND_AC);
    static const int16_t* range_PR = api->GameData.CharacterData
        .GetThrowRangeArray(BaseMod::ThrowRangeArrayType::GROUND_PR);

    ggxxacpr::GameVersion gameVer = api->GameData.GetGameVersion();

    auto pushDim = GetPlayerPushBoxDimensions(api, p);
    short rawId = static_cast<short>(p.id());
    short throwRange = gameVer == ggxxacpr::GameVersion::ACCENT_CORE ?
        range_AC[rawId] : range_PR[rawId];

    return {
        -(pushDim.x + throwRange),
        pushDim.x + throwRange,
        -pushDim.y,
        0
    };
}

Quad GetUniversalAirThrowQuad(BaseMod::Api* api, ggxxacpr::Player& p) {
    static const int16_t* horiArr_AC = api->GameData.CharacterData
        .GetThrowRangeArray(BaseMod::ThrowRangeArrayType::AIR_HORIZONTAL_AC);
    static const int16_t* horiArr_PR = api->GameData.CharacterData
        .GetThrowRangeArray(BaseMod::ThrowRangeArrayType::AIR_HORIZONTAL_PR);
    static const int16_t* upperArr = api->GameData.CharacterData
        .GetThrowRangeArray(BaseMod::ThrowRangeArrayType::AIR_UPPER);
    static const int16_t* lowerArr = api->GameData.CharacterData
        .GetThrowRangeArray(BaseMod::ThrowRangeArrayType::AIR_LOWER);

    static const int16_t* offsetArr_AC = api->GameData.CharacterData
        .GetPushboxAirborneOffsetArray(ggxxacpr::GameVersion::ACCENT_CORE);
    static const int16_t* offsetArr_PR = api->GameData.CharacterData
        .GetPushboxAirborneOffsetArray(ggxxacpr::GameVersion::PLUS_R);


    ggxxacpr::GameVersion gameVer = api->GameData.GetGameVersion();
    auto pushDim = GetPlayerPushBoxDimensions(api, p);

    short rawId = static_cast<short>(p.id());
    short yOffset = gameVer == ggxxacpr::GameVersion::ACCENT_CORE ?
        offsetArr_AC[rawId] : offsetArr_PR[rawId];
    short horiRange = gameVer == ggxxacpr::GameVersion::ACCENT_CORE ?
        horiArr_AC[rawId] : horiArr_PR[rawId];
    short upper = upperArr[rawId];
    short lower = lowerArr[rawId];

    return {
        -(pushDim.x + horiRange),
        pushDim.x + horiRange,
        upper - yOffset,
        lower - yOffset
    };
}