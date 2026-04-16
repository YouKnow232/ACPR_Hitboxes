#include "cleanHitRecorder.h"
#include <windows.h>
#include <cstdint>
#include "gearLoader/ggxxacpr_c.h"
#include "patch.h"

/**
 *  It's surprisingly hard to detect when a player is in clean hit hitstop.
 *  These functions exist to detect a clean hit and store its own state
 *  variable for clean hit hitstop.
 * 
 *  This is required to render clean hit boxes during hitstop. Since the scaling
 *  factor takes affect immediately after the hit, the clean hit box would appear
 *  smaller than expected during hitstop when we expect to see how the box was
 *  before the hit connected.
 */


constexpr intptr_t CLEAN_HIT_FN = 0x132090;
constexpr intptr_t CLEAN_HIT_FN_CALL = 0x134f91;

intptr_t base() {
    static intptr_t base = reinterpret_cast<intptr_t>(GetModuleHandle(nullptr));
    return base;
}

static bool _inCLHitstun[2] = {false, false};

static const intptr_t NativeCLCheckFnAddr = base() + CLEAN_HIT_FN;
// There is a parameter passed via EDX (GGXXACPR_Entity* attacker),
//  but it should be forwarded to the native call as long as the EDX
//  register doesn't get clobbered before then.
int32_t __stdcall CLCheckWrapper(GGXXACPR_Entity* defender) {
    int32_t result;
    // Don't clobber EDX before asm block
    asm(
        "push %[aDefender]\n\t"
        "call *%[fn]\n\t"
        "sub $4, %%esp\n\t"
        "movl %%eax, %[aResult]"
        : [aResult] "=r" (result)
        : [fn] "g" (NativeCLCheckFnAddr),
          [aDefender] "g" (defender)
        : "memory", "cc"
    );

    if (result) {
        _inCLHitstun[defender->playerIndex] = true;
    }

    return result;
}

void SetCLHook() {
    void* patchAddress = reinterpret_cast<void*>(base() + CLEAN_HIT_FN_CALL + 1);
    void* hook = reinterpret_cast<void*>(CLCheckWrapper);
    Patch_RelativeJump(
        patchAddress,
        hook,
        nullptr
    );
}
void WatchHitstop(int playerIndex, int hitStop) {
    if (hitStop == 0) _inCLHitstun[playerIndex] = false;
}
bool IsCLHitstop(int playerIndex) {
    if (playerIndex != 0 && playerIndex != 1) return false;

    return _inCLHitstun[playerIndex];
}
