#pragma once

#include <cstdint>

namespace ACPRHitboxes::Constants {
    // push boxes
    constexpr uint16_t bridgetShootActId = 134;
    constexpr float bridgetShootPushDownwardAdjustment = 7000.0f;

    // clean hit checks
    constexpr int cleanHitYOffset = 170;

    // misc range checks
    constexpr uint16_t potemkinSlideHeadActId = 120;
    constexpr int potemkinSlideHeadRange = 17000;
    constexpr uint16_t faustHackNSlashFailActId = 190;
    constexpr uint16_t faustHackNSlashUnblockableActId = 122;
    constexpr int faustHackNSlashRange = 10000;
    constexpr uint16_t roboKyMatActId = 13;
    constexpr int roboKyMatCollisionRange = 13200;
    constexpr uint16_t testamentHitomiSetActId = 10;
    constexpr uint16_t testamentEnhancedHitomiSetActId = 17;
    constexpr int testamentHitmoiActivationRangeAC = 7000;
    constexpr int testamentHitomiActivationRangePR = 5500;
    constexpr uint16_t faustDonutPickupActId = 18;
    constexpr uint16_t faustChocolatePickupActId = 19;
    constexpr uint16_t faustChikuwaPickupActId = 53;
    constexpr int faustFoodPickupRange = 4800;
    constexpr int eddiePuddleVerticalRange = 4000;
}