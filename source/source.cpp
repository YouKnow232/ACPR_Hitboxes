#include "gearLoader/gearLoader.hpp"
#include "baseMod/baseMod.hpp"

#include "logging/logging.h"
#include "graphics/graphics.h"
#include "graphics/render.h"
#include "settings/settings.h"

#include <iostream>

using namespace ACPRHitboxes;


static BaseMod::HookId renderHookId;
static BaseMod::HookId clRecordHookId;

void __stdcall RenderHook(
    BaseMod::Api* bmApi,
    const BaseMod_HookContext* ctx,
    const BaseMod_DrawInfo* info
) {
    RenderFrame(bmApi, reinterpret_cast<IDirect3DDevice9*>(info->device));
}

void __stdcall CleanHitRecordHit(
    BaseMod::Api* bmApi,
    const BaseMod_HookContext* ctx,
    const BaseMod_GameUpdateInfo* info
) {
    if (!bmApi->GameData.IsInGame()) return;

    // Hook CL func or something
}

GEARLOADER_EXPORT void GEARLOADER_CALL Init(GearLoaderContext* ctx, GearLoaderApi* api) {
    GearLoader::Api* glApi = new GearLoader::Api(api, ctx);
    SetLogger(glApi);

    const BaseMod_Api* baseModApi;
    SemanticVersion retVer;
    int errCode = glApi->RetrieveModApi<BaseMod_Api>(
        BASEMOD_NAME,
        ">=0.1.0",
        &baseModApi,
        &retVer
    );
    if (errCode != 0 || baseModApi == nullptr) {
        glApi->Log(GearLoader::LogLevel::ERR, "BaseMod api was not found");
        return;
    }

    BaseMod::Api* bmApi = new BaseMod::Api(baseModApi);

    LoadSettingsFromFile("./mods/hitboxes/settings.ini");

    int result = InitGraphics(reinterpret_cast<IDirect3DDevice9*>(bmApi->GameData.GetD3D9Device()));
    if (result != D3D_OK) {
        std::stringstream ss;
        ss << "Graphics failed to initiailze: 0x" << std::hex << result;
        glApi->Log(GearLoader::LogLevel::ERR, ss.str());
        return;
    }

    // Register hooks
    renderHookId = bmApi->Hooks.BeforePresent<BaseMod::Api>(RenderHook, bmApi);
    clRecordHookId = bmApi->Hooks.AfterGameUpdate<BaseMod::Api>(CleanHitRecordHit, bmApi);
}