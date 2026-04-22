#include "gearLoader/gearLoader.hpp"
#include "baseMod/baseMod.hpp"

#include "cleanHitRecorder/cleanHitRecorder.h"
#include "logging/logging.h"
#include "dataStore/dataStore.h"
#include "graphics/graphics.h"
#include "graphics/render.h"
#include "settings/settings.h"
#include "settings/modMenu.h"
#include <iostream>


using namespace ACPRHitboxes;

static BaseMod::HookId renderHookId;
static BaseMod::HookId saveHookId;
static BaseMod::HookId updateHookId;

static const char* settingsFile = "./mods/hitboxes/saved_settings.bin";

void BASEMOD_CALL RenderHook(
    BaseMod::Api* bmApi,
    const BaseMod_HookContext* ctx,
    const BaseMod_DrawInfo* info
) {
    RenderFrame(bmApi, reinterpret_cast<IDirect3DDevice9*>(info->device));
}

void BASEMOD_CALL SaveHook(
    BaseMod::Api* bmApi,
    const BaseMod::HookContext* ctx,
    const BaseMod::SaveGameInfo* info
) {
    SettingsManager::GetInstance().Serialize(settingsFile);
}

void BASEMOD_CALL UpdateHook(
    BaseMod::Api* bmApi,
    const BaseMod_HookContext* ctx,
    const BaseMod_GameUpdateInfo* info
) {
    if (!bmApi->GameData.IsInGame()) return;
    auto p1 = bmApi->GameData.GetPlayer(0);
    if (p1.isValid()) WatchHitstop(0, p1.hitstopTime());
    auto p2 = bmApi->GameData.GetPlayer(1);
    if (p2.isValid()) WatchHitstop(1, p2.hitstopTime());
}

GEARLOADER_EXPORT void GEARLOADER_CALL Init(GearLoaderContext* ctx, GearLoaderApi* api) {
    GearLoader::Api* glApi = new GearLoader::Api(api, ctx);
    SetLogger(glApi);

    const BaseMod_Api* baseModApi;
    SemanticVersion retVer;
    int errCode = glApi->RetrieveModApi<BaseMod_Api>(
        BASEMOD_NAME,
        BASEMOD_API_VERSION,
        &baseModApi,
        &retVer
    );
    if (errCode != 0 || baseModApi == nullptr) {
        glApi->Log(GearLoader::LogLevel::ERR, "BaseMod api was not found");
        return;
    }

    BaseMod::Api bmApi = SetBMApi(baseModApi);
    BaseMod::Api* bmApiPtr = new BaseMod::Api(baseModApi);

    SettingsManager::GetInstance().Deserialize(settingsFile);

    int result = InitGraphics(reinterpret_cast<IDirect3DDevice9*>(bmApi.GameData.GetD3D9Device()));
    if (result != D3D_OK) {
        std::stringstream ss;
        ss << "Graphics failed to initiailze: 0x" << std::hex << result;
        glApi->Log(GearLoader::LogLevel::ERR, ss.str());
        return;
    }

    RegisterModMenu(bmApi.ModMenu);

    // Register hooks
    renderHookId = bmApi.Hooks.BeforePresent<BaseMod::Api>(RenderHook, bmApiPtr);
    saveHookId = bmApi.Hooks.AfterSaveGame<BaseMod::Api>(SaveHook, bmApiPtr);
    updateHookId = bmApi.Hooks.AfterGameUpdate<BaseMod::Api>(UpdateHook, bmApiPtr);
    SetCLHook();

    std::cout << "[Hitboxes] Initialized" << std::endl;
}
