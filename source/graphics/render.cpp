#include "render.h"
#include <map>
#include <cstdint>
#include "directx/DirectXMath.h"
#include "baseMod/baseMod.hpp"
#include "graphics.h"
#include "gameData/hardCodedConstants.h"
#include "gameData/gameData.h"
#include "cleanHitRecorder/cleanHitRecorder.h"
#include "settings/settings.h"
#include "logging/logging.h"

using namespace DirectX;


namespace ACPRHitboxes {
    inline bool ShouldRender(BaseMod::Api* bmApi) {
        return bmApi->GameData.IsInGame() &&
            (bmApi->GameData.GetPauseState() == 0 || bmApi->GameData.GetPauseDisplayState() == 0) &&
            bmApi->GameData.GetCamera().size().x != 0;
    }
    inline bool ShouldHideHurtBoxes(GGXXACPR_Entity* e) {
        constexpr uint32_t mask = ACTION_STATE_STRIKE_INVLUN & ACTION_STATE_PROJECTILE_INVULN & ACTION_STATE_DISABLE_HURTBOXES;
        return (e->actionState & mask) != 0 ||
            (e->playerEntityDataPtr != nullptr && e->playerEntityDataPtr->invulnCounter > 0);
    }
    inline bool ShouldHideHitBoxes(GGXXACPR_Entity* e) {
        return (e->actionState & ACTION_STATE_DISABLE_HITBOXES) != 0 &&
            !(e->hitstopTime > 0 && ((e->attackState & ATTACK_STATE_HAS_CONNECTED) != 0));
    }

    inline void LoadVertBufferWithQuad(
        Vertex* buffer, float left, float right,
        float top, float bot, D3DCOLOR color
    ) {
        buffer[0] = {{left , top, 1.0f}, color, {0.0f, 0.0f}};
        buffer[1] = {{right, top, 1.0f}, color, {1.0f, 0.0f}};
        buffer[2] = {{left , bot, 1.0f}, color, {0.0f, 1.0f}};
        buffer[3] = {{left , bot, 1.0f}, color, {0.0f, 1.0f}};
        buffer[4] = {{right, top, 1.0f}, color, {1.0f, 0.0f}};
        buffer[5] = {{right, bot, 1.0f}, color, {1.0f, 1.0f}};
    }
    inline void LoadVertBufferWithQuad(Vertex* buffer, Quad quad, D3DCOLOR color) {
        LoadVertBufferWithQuad(buffer,
            static_cast<float>(quad.left),
            static_cast<float>(quad.right),
            static_cast<float>(quad.top),
            static_cast<float>(quad.bottom),
            color);
    }

    inline XMMATRIX RawEntityPosTransform(GGXXACPR_Entity* e) {
        XMMATRIX matrix = XMMatrixIdentity();

        // flip
        matrix *= XMMatrixScaling(
            e->bIsFacingRight == 0 ? 1.0f : -1.0f,
            1.0f,
            1.0f);

        // position
        matrix *= XMMatrixTranslation(
            static_cast<float>(e->xPos),
            static_cast<float>(e->yPos),
            0.0f);

        return matrix;
    }
    inline XMMATRIX PlayerPosTransform(ggxxacpr::Player& p) {
        return RawEntityPosTransform(p.getRaw());
    }
    inline XMMATRIX EntityPosTransform(ggxxacpr::Entity& e) {
        return RawEntityPosTransform(e.getRaw());
    }

    inline XMMATRIX RawEntityColliderTransform(GGXXACPR_Entity* e) {
        XMMATRIX matrix = XMMatrixIdentity();

        // model to world
        matrix *= XMMatrixScaling(100.0f, 100.0f, 1.0f);

        // flip
        matrix *= XMMatrixScaling(
            e->bIsFacingRight == 0 ? 1.0f : -1.0f,
            1.0f,
            1.0f);
        
        // scale
        if (e->scale > 0 || e->scaleY > 0) {
            ggxxacpr::Scale scale = {e->scale, e->scaleY};
            matrix *= XMMatrixScaling(scale.x(), scale.y(), 1.0f);
        }

        // position
        matrix *= XMMatrixTranslation(
            static_cast<float>(e->xPos),
            static_cast<float>(e->yPos),
            0.0f);

        return matrix;
    }
    inline XMMATRIX PlayerColliderTransform(ggxxacpr::Player& p) {
        return RawEntityColliderTransform(p.getRaw());
    }
    inline XMMATRIX EntityColliderTransform(ggxxacpr::Entity& e) {
        return RawEntityColliderTransform(e.getRaw());
    }

    inline XMMATRIX WideScreenCorrection(BaseMod::Api* api, ggxxacpr::Camera& c) {
        return XMMatrixScaling(
            4 * api->GameData.GetViewHeight() * 1.0f / (3 * api->GameData.GetViewWidth()),
            1.0f,
            1.0f
        );
    }

    inline XMMATRIX ProjectionMatrix(BaseMod::Api* api, ggxxacpr::Camera& c) {
        XMMATRIX matrix = XMMatrixIdentity();

        // half pixel correction
        matrix *= XMMatrixTranslation(-50.0f, -50.0f, 0.0f);

        if (c.size().x != 0 && c.size().y != 0) {
            matrix *= XMMatrixOrthographicOffCenterLH(
                static_cast<float>(c.left()),
                static_cast<float>(c.right()),
                static_cast<float>(c.bottom()),
                static_cast<float>(c.top()),
                0.0f,
                1.0f
            );
        }

        matrix *= WideScreenCorrection(api, c);

        return matrix;
    }

    /* ==================== Draw delegates functions ==================== */

    inline ggxxacpr::WorldCoordinate GetProximityDimensions(BaseMod::Api* api, ggxxacpr::Player& p) {
        if (p.id() == ggxxacpr::EntityId::POTEMKIN &&
            p.actId() == Constants::potemkinSlideHeadActId &&
            p.getRaw()->mark != 0
        ) {
            auto pushDim = GetPlayerPushBoxDimensions(api, p);
            return {pushDim.x + Constants::potemkinSlideHeadRange, pushDim.y};
        }
        else if (p.id() == ggxxacpr::EntityId::FAUST &&
            (p.actId() == Constants::faustHackNSlashFailActId ||
            p.actId() == Constants::faustHackNSlashUnblockableActId) &&
            p.actTimer() == 1
        ) {
            auto pushDim = GetPlayerPushBoxDimensions(api, p);
            return {Constants::faustHackNSlashRange, pushDim.y};
        }
        return {0, 0};
    }
    inline ggxxacpr::WorldCoordinate GetProximityDimensions(BaseMod::Api* api, ggxxacpr::Entity& e) {
        if (e.id() == ggxxacpr::EntityId::ROBO_KY_ENTITY_1 &&
            e.actId() == Constants::roboKyMatActId &&
            e.getRaw()->mark != 0
        ) {
            return {
                Constants::roboKyMatCollisionRange,
                SettingsManager::GetInstance().MinimumBoxHeight
            };
        }
        else if (e.id() == ggxxacpr::EntityId::TESTAMENT_ENTITY_2 &&
            (e.actId() == Constants::testamentHitomiSetActId ||
            e.actId() == Constants::testamentEnhancedHitomiSetActId)
        ) {
            int range = api->GameData.GetGameVersion() == ggxxacpr::GameVersion::ACCENT_CORE ?
                Constants::testamentHitmoiActivationRangeAC :
                Constants::testamentHitomiActivationRangePR;
            return {range, SettingsManager::GetInstance().MaximumBoxHeight};
        }
        else if (e.id() == ggxxacpr::EntityId::FAUST_ENTITY_1 &&
            (
                e.actId() == Constants::faustDonutPickupActId ||
                e.actId() == Constants::faustChocolatePickupActId ||
                e.actId() == Constants::faustChikuwaPickupActId
            )
        ) {
            return {
                Constants::faustFoodPickupRange,
                SettingsManager::GetInstance().MinimumBoxHeight
            };
        }
        else if (e.id() == ggxxacpr::EntityId::EDDIE_ENTITY_SHADOW &&
            e.getRaw()->localId == 1 && e.getRaw()->trans == 0
        ) {
            return {
                e.getRaw()->mark * 100,
                Constants::eddiePuddleVerticalRange
            };
        }
        return {0, 0};
    }
    void RenderPlayerProximity(BaseMod::Api* api, IDirect3DDevice9* device, ggxxacpr::Player& p) {
        constexpr int bufferSize = 6;
        static Vertex buffer[bufferSize];

        auto proxDim = GetProximityDimensions(api, p);
        if (proxDim.x == 0 && proxDim.y == 0) return;

        LoadVertBufferWithQuad(buffer,
            -proxDim.x, proxDim.x, proxDim.y, 0.0f,
            SettingsManager::GetInstance().Palette.MiscPushRange
        );

        auto cam = api->GameData.GetCamera();
        XMMATRIX transform = 
            XMMatrixScaling(1.0f, -1.0f, 1.0f) *
            PlayerPosTransform(p) *
            ProjectionMatrix(api, cam);
        DrawTriListPrimitive(device, buffer, bufferSize, transform);
    }
    void RenderEntityProximity(BaseMod::Api* api, IDirect3DDevice9* device, ggxxacpr::Entity& e) {
        constexpr int bufferSize = 6;
        static Vertex buffer[bufferSize];

        auto proxDim = GetProximityDimensions(api, e);
        if (proxDim.x == 0 && proxDim.y == 0) return;

        LoadVertBufferWithQuad(buffer,
            -proxDim.x, proxDim.x, proxDim.y, 0.0f,
            SettingsManager::GetInstance().Palette.MiscPivotRange
        );

        auto cam = api->GameData.GetCamera();
        XMMATRIX transform =
            XMMatrixScaling(1.0f, -1.0f, 1.0f) *
            EntityPosTransform(e) *
            ProjectionMatrix(api, cam);
        DrawTriListPrimitive(device, buffer, bufferSize, transform);
    }
    void RenderMiscRanges(BaseMod::Api* api, IDirect3DDevice9* device) {
        auto& settings = SettingsManager::GetInstance();
        if (settings.HideMiscRanges) return;
        int hide = settings.HidePlayer;
        ggxxacpr::Entity headNode = api->GameData.GetRootEntity();

        for (int i = 0; i < 2; i++) {
            if ((hide & (i+1)) != 0) continue;

            auto p = api->GameData.GetPlayer(i);
            if (!p.isValid()) continue;
            RenderPlayerProximity(api, device, p);

            // check entities
            ggxxacpr::Entity iEntity = headNode.next();
            while(iEntity.getRaw() != headNode.getRaw()) {
                if (iEntity.playerIndex() == i) {
                    RenderEntityProximity(api, device, iEntity);
                }
                iEntity = iEntity.next();
            }
        }
    }
    void RenderPushboxes(BaseMod::Api* api, IDirect3DDevice9* device) {
        constexpr int bufferSize = 6;
        static Vertex buffer[bufferSize];

        if (SettingsManager::GetInstance().HidePush) return;
        int hide = SettingsManager::GetInstance().HidePlayer;
        D3DCOLOR pushColor = SettingsManager::GetInstance().Palette.Push;
        D3DCOLOR noCollisionPushColor = pushColor & 0x00FFFFFF;
        ggxxacpr::Camera cam = api->GameData.GetCamera();
        ggxxacpr::GameVersion gameVer = api->GameData.GetGameVersion();

        for (int i = 0; i < 2; i++) {
            if ((hide & (i+1)) != 0) continue;
            ggxxacpr::Player p = api->GameData.GetPlayer(i);
            if (!p.isValid()) continue;
            
            ggxxacpr::WorldCoordinate pushDim = GetPlayerPushBoxDimensions(api, p);

            int yOffset = 0;
            if ((p.actionState() & ggxxacpr::ActionState::IS_AIRBORNE) != ggxxacpr::ActionState::NONE) {
                int16_t* offsets = api->GameData.CharacterData.GetPushboxAirborneOffsetArray(gameVer);
                yOffset = offsets[static_cast<short>(p.id())];
            }
            float halfWidth = static_cast<float>(pushDim.x);
            float top = static_cast<float>(-pushDim.y - yOffset);
            float bot = static_cast<float>(-yOffset);
            float xOffset = -halfWidth;

            // apply push adjust collider
            for (auto& collider : p.colliders()) {
                if (collider.boxTypeId == static_cast<short>(ggxxacpr::ColliderId::ADJUST_PUSH)) {
                    xOffset = collider.xOffset * 100.0f;
                    halfWidth = collider.width * 50.0f;
                    break;
                }
            }
            // hardcoded Bridget shoot adjustment
            if (gameVer == ggxxacpr::GameVersion::PLUS_R &&
                p.id() == ggxxacpr::EntityId::BRIDGET &&
                p.actId() == Constants::bridgetShootActId
            ) {
                bot += Constants::bridgetShootPushDownwardAdjustment;
            }
            
            D3DCOLOR color = pushColor;
            if ((p.actionState() & ggxxacpr::ActionState::NO_COLLISION) != ggxxacpr::ActionState::NONE) {
                color = noCollisionPushColor;
            }
            LoadVertBufferWithQuad(buffer, xOffset, xOffset + halfWidth * 2.0f, top, bot, color);

            XMMATRIX transform = PlayerPosTransform(p) * ProjectionMatrix(api, cam);
            DrawTriListPrimitive(device, buffer, bufferSize, transform);
        }
    }
    void RenderRawEntityColliders(
        BaseMod::Api* api,
        IDirect3DDevice9* device,
        GGXXACPR_Entity* entity,
        ggxxacpr::ColliderId colliderType,
        ggxxacpr::Camera& cam
    ) {
        constexpr int bufferSize = 6 * 64;
        static Vertex buffer[bufferSize];

        if (entity == nullptr ||
            (colliderType == ggxxacpr::ColliderId::HIT_BOX && ShouldHideHitBoxes(entity)) ||
            (colliderType == ggxxacpr::ColliderId::HURT_BOX && ShouldHideHurtBoxes(entity))
        ) {
            return;
        }

        auto& settings = SettingsManager::GetInstance();

        auto color = settings.Palette.Default;
        if (colliderType == ggxxacpr::ColliderId::HIT_BOX) {
            color = settings.Palette.Hitbox;
        } else if (colliderType == ggxxacpr::ColliderId::HURT_BOX) {
            color = settings.Palette.Hurtbox;
        }

        short colliderTypeRaw = static_cast<short>(colliderType);
        int bufferIndex = 0;

        // TODO: Test if always rendering the extra colliders is appropriate
        GGXXACPR_Collider* hitBoxSets[] = {
            entity->colliderSetPtr,
            entity->extraColliderSetPtr
        };
        for (auto hitboxSet : hitBoxSets) {
            if (hitboxSet == nullptr) continue;
            for (int i = 0; i < entity->boxCount; i++) {
                if (hitboxSet[i].boxTypeId != colliderTypeRaw) continue;
                
                LoadVertBufferWithQuad(&buffer[bufferIndex*6],
                    /*left */ hitboxSet[i].xOffset,
                    /*right*/ hitboxSet[i].xOffset + hitboxSet[i].width,
                    /*top  */ hitboxSet[i].yOffset,
                    /*bot  */ hitboxSet[i].yOffset + hitboxSet[i].height,
                    color
                );

                bufferIndex++;
                if (bufferIndex*6 >= bufferSize) break;
            }
        }

        XMMATRIX transform =  RawEntityColliderTransform(entity) * ProjectionMatrix(api, cam);
        DrawTriListPrimitive(device, buffer, bufferIndex*6, transform);
    }
    void RenderCollidersByType(
        BaseMod::Api* api,
        IDirect3DDevice9* device,
        ggxxacpr::ColliderId boxType
    ) {
        int hide = SettingsManager::GetInstance().HidePlayer;
        auto cam = api->GameData.GetCamera();
        for (int i = 0; i < 2; i++) {
            if ((hide & (i+1)) != 0) continue;
            auto player = api->GameData.GetPlayer(i);
            if (!player.isValid()) continue;

            RenderRawEntityColliders(
                api,
                device,
                player.getRaw(),
                boxType,
                cam);

            ggxxacpr::Entity headNode = api->GameData.GetRootEntity();
            ggxxacpr::Entity iEntity = headNode.next();
            while(iEntity.getRaw() != headNode.getRaw()) {
                if (iEntity.playerIndex() == i) {
                    RenderRawEntityColliders(
                        api,
                        device,
                        iEntity.getRaw(),
                        boxType,
                        cam);
                    }
                iEntity = iEntity.next();
            }
        }
    }
    void RenderHurtboxes(BaseMod::Api* api, IDirect3DDevice9* device) {
        if (SettingsManager::GetInstance().HideHurt) return;
        RenderCollidersByType(api, device, ggxxacpr::ColliderId::HURT_BOX);
    }
    void RenderHitboxes(BaseMod::Api* api, IDirect3DDevice9* device) {
        if (SettingsManager::GetInstance().HideHit) return;
        RenderCollidersByType(api, device, ggxxacpr::ColliderId::HIT_BOX);
    }
    void RenderCleanHitboxes(BaseMod::Api* api, IDirect3DDevice9* device) {
        constexpr int bufferSize = 6;
        static Vertex buffer[bufferSize];

        auto& settings = SettingsManager::GetInstance();
        if (settings.HideCleanHit) return;
        int hide = settings.HidePlayer;
        auto color = settings.Palette.CLHitbox;
        auto cam = api->GameData.GetCamera();
        
        for (int i = 0; i < 2; i++) {
            if ((hide & (i+1)) != 0) continue;

            auto p = api->GameData.GetPlayer(i);
            auto opp = api->GameData.GetPlayer(1-i);
            if (!p.isValid() || !opp.isValid()) break;

            auto hp = reinterpret_cast<GGXXACPR_HitParam*>(p.getRaw()->hitParamPtr);
            if (hp == nullptr || hp->cleanHitCheckScale == -1) continue;

            int clCounter = opp.cleanHitCounter();
            if (IsCLHitstop(opp.playerIndex())) clCounter--;
            int shrinkage = clCounter * hp->cleanHitCheckScale;
            int scaledWidth = std::max(hp->cleanHitCheckHalfWidth - shrinkage, 1);
            int scaledheight = std::max(hp->cleanHitCheckHalfHeight - shrinkage, 1);

            LoadVertBufferWithQuad(buffer,
                /*left */ static_cast<float>(hp->cleanHitCheckXDist - scaledWidth),
                /*right*/ static_cast<float>(hp->cleanHitCheckXDist + scaledWidth),
                /*top  */ static_cast<float>(hp->cleanHitCheckYDist - scaledheight + Constants::cleanHitYOffset),
                /*bot  */ static_cast<float>(hp->cleanHitCheckYDist + scaledheight + Constants::cleanHitYOffset),
                color
            );

            XMMATRIX transform =
                PlayerColliderTransform(p) *
                ProjectionMatrix(api, cam);
            DrawTriListPrimitive(device, buffer, bufferSize, transform);
        }
    }
    void RenderGrabboxes(BaseMod::Api* api, IDirect3DDevice9* device) {
        constexpr int bufferSize = 6;
        static Vertex buffer[bufferSize];
        static uint16_t* commandGrabRanges = api->GameData.CharacterData.GetCommandGrabRangeArray();

        auto& settings = SettingsManager::GetInstance();
        if (settings.HideGrab) return;
        int hide = settings.HidePlayer;
        D3DCOLOR color = settings.Palette.Grab;

        for (int i = 0; i < 2; i++) {
            if ((hide & (i+1)) != 0) continue;
            auto p = api->GameData.GetPlayer(i);
            if (!p.isValid()) continue;

            int cmdGrabId = CommandThrowIsActive(p);
            if (cmdGrabId != 0) {
                short range = commandGrabRanges[static_cast<short>(p.id())];
                auto pushDim = GetPlayerPushBoxDimensions(api, p);
                LoadVertBufferWithQuad(buffer,
                    -(pushDim.x + range),
                    pushDim.x + range,
                    -pushDim.y,
                    0.0f,
                    color
                );
                auto cam = api->GameData.GetCamera();
                XMMATRIX transform = PlayerPosTransform(p) * ProjectionMatrix(api, cam);
                DrawTriListPrimitive(device, buffer, bufferSize, transform);
            } else if (IsUniversalThrowActive(api, p) || settings.AlwaysDisplayThrowRange) {
                Quad quad = (p.actionState() & ggxxacpr::ActionState::IS_AIRBORNE) == ggxxacpr::ActionState::NONE ?
                    GetUniversalGroundThrowQuad(api, p) :
                    GetUniversalAirThrowQuad(api, p);
                
                LoadVertBufferWithQuad(buffer, quad, color);
                auto cam = api->GameData.GetCamera();
                XMMATRIX transform = PlayerPosTransform(p) * ProjectionMatrix(api, cam);
                DrawTriListPrimitive(device, buffer, bufferSize, transform);
            }
        }
    }
    // TODO: Render this in screen space instead of world coord
    void RenderPivotboxes(BaseMod::Api* api, IDirect3DDevice9* device) {
        constexpr int bufferSize = 6 * 2;
        static Vertex buffer[bufferSize];
        static bool bufferInitialized = false;
        static float curCrossSize = 0.0f;
        static float curCrossThickness = 0.0f;
        
        SettingsManager& settings = SettingsManager::GetInstance();
        if (settings.HidePivot) return;
        int hide = settings.HidePlayer;
        float sampledCrossSize = settings.PivotCrossSize;
        float sampledCrossThickness = settings.PivotCrossThickness;
        D3DCOLOR color = settings.Palette.PivotCross;

        // Update geometry
        if (!bufferInitialized || 
            // display settings have changed
            sampledCrossSize != curCrossSize ||
            sampledCrossThickness != curCrossThickness
        ) {
            float halfS = sampledCrossSize / 2.0f;
            float halfT = sampledCrossThickness / 2.0f;

            // horizontal line
            LoadVertBufferWithQuad(&buffer[0], -halfS, halfS, halfT, -halfT, color);
            // vertical line
            LoadVertBufferWithQuad(&buffer[6], -halfT, halfT, halfS, -halfS, color);

            curCrossSize = sampledCrossSize;
            curCrossThickness = sampledCrossThickness;
            bufferInitialized = true;
        }
        
        // Draw
        for (int i = 0; i < 2; i++) {
            if ((hide & (i+1)) != 0) continue;
            auto player = api->GameData.GetPlayer(i);
            if (!player.isValid()) continue;
            auto camera  = api->GameData.GetCamera();
            
            XMMATRIX transform = PlayerColliderTransform(player) * ProjectionMatrix(api, camera);
            DrawTriListPrimitive(device, buffer, bufferSize, transform);
        }
    }


    void RenderFrame(BaseMod::Api* bmApi, IDirect3DDevice9* device) {
        if(!ShouldRender(bmApi)) return;

        static std::map<DrawOperation, DrawDelegate> drawOpMap = {
            {DrawOperation::MiscRange,  RenderMiscRanges},
            {DrawOperation::Push,       RenderPushboxes},
            {DrawOperation::Hurt,       RenderHurtboxes},
            {DrawOperation::Hit,        RenderHitboxes},
            {DrawOperation::CleanHit,   RenderCleanHitboxes},
            {DrawOperation::Grab,       RenderGrabboxes},
            {DrawOperation::Pivot,      RenderPivotboxes},
        };

        SetRenderContext(device);

        for (DrawOperation op : SettingsManager::GetInstance().DrawOrder) {
            if (drawOpMap.contains(op)) {
                drawOpMap[op](bmApi, device);
            }
        }
    }
}
