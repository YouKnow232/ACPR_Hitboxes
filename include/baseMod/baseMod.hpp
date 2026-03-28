#ifndef BASEMOD_HPP
#define BASEMOD_HPP

#include "baseMod_c.h"
#include <string>
#include "gearLoader/ggxxacpr.hpp"

namespace BaseMod {

    using HookId = BaseMod_HookId;
    using HookContext = BaseMod_HookContext;
    using PeakMessageArgs = BaseMod_PeekMessageArgs;
    using PeekMessageInfo = BaseMod_PeekMessageInfo;
    using GameUpdateInfo = BaseMod_GameUpdateInfo;
    using DrawInfo = BaseMod_DrawInfo;

    enum class PushboxDimensionArrayType : int32_t {
        STANDING_WIDTH = BM_PD_STANDING_WIDTH,
        STANDING_HEIGHT_AC = BM_PD_STANDING_HEIGHT_AC,
        STANDING_HEIGHT_PR = BM_PD_STANDING_HEIGHT_PR,
        CROUCHING_WIDTH = BM_PD_CROUCHING_WIDTH,
        CROUCHING_HEIGHT = BM_PD_CROUCHING_HEIGHT,
        AIRBORNE_WIDTH = BM_PD_AIRBORNE_WIDTH,
        AIRBORNE_HEIGHT = BM_PD_AIRBORNE_HEIGHT,
    };
    enum class ThrowRangeArrayType : int32_t {
        GROUND_AC = BM_TR_GROUND_AC,
        GROUND_PR = BM_TR_GROUND_PR,
        AIR_HORIZONTAL_AC = BM_TR_AIR_HORIZONTAL_AC,
        AIR_HORIZONTAL_PR = BM_TR_AIR_HORIZONTAL_PR,
        AIR_UPPER = BM_TR_AIR_UPPER,    // This array is not split between Accent Core and Plus R
        AIR_LOWER = BM_TR_AIR_LOWER,    // This array is not split between Accent Core and Plus R
    };

    class NativeFunctionsApi {
    public:
        NativeFunctionsApi() : _ref(nullptr) {}
        NativeFunctionsApi(const BaseMod_NativeFunctionsApi* ref = nullptr)
            : _ref(ref) { }
        /**
         * Retruns true if there is a difference in major version number between
         *      actual and expected BaseMod API versions.
         */
        bool VersionError() {
            return (BASEMOD_API_VERSION_NUM & 0xFF0000) != (_ref->version & 0xFF0000);
        }
        const BaseMod_NativeFunctionsApi* GetCApi() { return _ref; }

        /**
         *  \brief Draws text to the screen during battle using the game's text glyph system.
         * 
         *  This function must be called before the game begins its drawing process.
         *      It is recommend to call this function in the `AfterGameUpdate` hook.
         * 
         *  \param text A pointer to the text string to be displayed. The string format is a subset of ASCII.
         *      The character set is limited to upper case letters, numbers, and the following special symbols: "-+.!?/():&"
         *      The following characters map to additional special characters:
         *          ",~acegiwxyz" maps to "•±on•utabyx".
         *          '>' = END character from the highscore initials screen.
         *          "bdfh" = down/left/right/up arrows
         *  \param xPos Internal resolution screen-space coordinate (640x480). Left edge is 0, right is 640.
         *  \param yPos Internal resolution screen-space coordinate (640x480). Top edge is 0, bottom is 480.
         *  \param zPos The draw order/depth buffer value. Lower values draw later / appear in front of other text and sprites.
         *  \param alpha Transparency value [0-255].
         *  \param size Scaling value, standard size is 1.0f which results in a text glyph of size 12x15px (internal resolution).
         *  \return zero if no error occurred, otherwise returns the error code.
         */
        uint32_t RenderText(std::string text, int32_t xPos, int32_t yPos, float zPos, uint8_t alpha, float size) {
            if (VersionError()) return 1;
            return _ref->RenderText(text.c_str(), xPos, yPos, zPos, alpha, size);
        }

        /**
         *  \brief A higher level text rendering function that triggers a text popup animation in-game (e.g. COUNTER HIT / RECOVERY).
         * 
         *  \param playerIndex Which side of the screen to display the pop up.
         *  \param text The text to be displayed. See `RenderText` for format and available characters.
         */
        uint32_t RenderPopUpText(int32_t playerIndex, std::string text) {
            if (VersionError()) return 1;
            return _ref->RenderPopUpText(playerIndex, text.c_str());
        }

        /**
         *  \brief Plays a sound effect.
         * 
         *  \param id The id of the sound effect. The id maps to the "COMMON SE" sound effect
         *      in the Sound menu. See github.com/youknow232/gearloader/docs/SoundEffectIdMap.txt
         *      For the id mappings.
         */
        bool PlayCommonSoundEffect(uint32_t id) {
            return _ref->PlayCommonSoundEffect(id);
        }

        /**
         *  \brief Draws a sprite to the screen.
         * 
         *  See `GGXXACPR_DrawSpriteParams`
         * 
         *  \param params Combined parameter struct
         *  \param flag Unkown
         */
        uint32_t DrawSprite(GGXXACPR_DrawSpriteParams* params, int32_t flag) {
            if (VersionError()) return 1;
            _ref->DrawSprite(params, flag);
            return 0;
        }

        /**
         *  \brief Draws a quad to the screen.
         * 
         *  A simple helper function that constructs four vertices from the given parameters
         *      and renders the resulting quad with the game's D3D9 setup.
         *      The edges of the quad are given in internal resolution screen space coordinates.
         *      The top left pixel is (0, 0) and the bottom right pixel is (640, 480).
         * 
         *  \param left edge position
         *  \param top edge position
         *  \param right edge position
         *  \param bottom edge position
         *  \param zPos The Z coordinate for each vertex
         *  \param color ARGB color value (i.e. `D3DCOLOR`)
         */
        void DrawQuad(int32_t left, int32_t top, int32_t right, int32_t bottom, int32_t zPos, uint32_t color) {
            _ref->DrawQuad(left, top, right, bottom, zPos, color);
        }

    private:
        const BaseMod_NativeFunctionsApi* _ref;
    };

    class CharDataApi {
    public:
        CharDataApi() : _ref(nullptr) {}
        CharDataApi(const BaseMod_CharDataApi* ref ) : _ref(ref) {}
        /**
         * Returns true if there is a difference in major version number
         *      between actual and expected BaseMod API versions.
         */
        bool VersionError() {
            return (BASEMOD_API_VERSION_NUM & 0xFF0000) != (_ref->version & 0xFF0000);
        }
        const BaseMod_CharDataApi* GetCApi() { return _ref; }

        uint16_t* GetPushboxDimensionArray(PushboxDimensionArrayType type) {
            return _ref->GetPushboxDimensionArray(static_cast<int32_t>(type));
        }
        int16_t* GetPushboxAirborneOffsetArray(ggxxacpr::GameVersion gameVer) {
            return _ref->GetPushboxAirborneOffsetArray(static_cast<int32_t>(gameVer));
        }
        int16_t* GetThrowRangeArray(ThrowRangeArrayType type) {
            return _ref->GetThrowRangeArray(static_cast<int32_t>(type));
        }
        uint16_t* GetCommandGrabRangeArray() {
            return _ref->GetCommandGrabRangeArray();
        }
    private:
        const BaseMod_CharDataApi* _ref;
    };

    class GameDataApi {
    public:
        GameDataApi() :
            _ref(nullptr),
            CharacterData(nullptr) { }
        GameDataApi(const BaseMod_GameDataApi* ref) :
            _ref(ref),
            CharacterData(ref->CharacterData) { }
        /**
         * Returns true if there is a difference in major version number
         *      between actual and expected BaseMod API versions.
         */
        bool VersionError() {
            return (BASEMOD_API_VERSION_NUM & 0xFF0000) != (_ref->version & 0xFF0000);
        }
        const BaseMod_GameDataApi* GetCApi() { return _ref; }

        CharDataApi CharacterData;

        /**
         *  \brief `0` for player 1, `1` for player 2.
         */
        ggxxacpr::Player GetPlayer(int playerIndex) {
            return _ref->GetPlayer(playerIndex);
        }
        /**
         *  \brief Gets the current state of a player's controller input
         */
        ggxxacpr::RawControllerInput GetPlayerInput(int player_index) {
            return static_cast<ggxxacpr::RawControllerInput>(_ref->GetPlayerInput(player_index));
        }
        /**
         *  \brief Gets the camera struct. See `GGXXACPR_Camera`.
         */
        ggxxacpr::Camera GetCamera() {
            return _ref->GetCamera();
        }
        /**
         *  \brief returns a non-zero value if the game is on the battle screen.
         */
        bool IsInGame() { return _ref->IsInGame() > 0; }
        /**
         *  \brief Returns a pointer to the current job mode, see enum `GGXXACPR_JobMode`. This variable
         *      determines what scene the game is set to such as "TitleScreen", "Battle", "MissionMenu".
         */
        ggxxacpr::JobMode GetJobMode() { return static_cast<ggxxacpr::JobMode>(_ref->GetJobMode()); }
        /**
         *  \brief See enum `GGXXACPR_GameModeFeatureFlags`. Returns the current game mode feature flags.
         */
        ggxxacpr::GameModeFeatureFlags GetGameModeFeatureFlags() { return static_cast<ggxxacpr::GameModeFeatureFlags>(_ref->GetGameModeFeatureFlags()); }
        /**
         *  \brief Enum `GGXXACPR_MainMenuItem`. Returns the item selected from the main menu.
         * 
         *  This value defaults to MAIN_MENU_ITEM_ARCADE and is
         *      set when selecting an option on the main menu.
         */
        ggxxacpr::MenuItem GetMainMenuSelection() { return static_cast<ggxxacpr::MenuItem>(_ref->GetMainMenuSelection()); }
        /**
         *  \brief Returns the D3D9 device pointer.
         * 
         *  Include `d3d9.h` and cast to IDirect3DDevice9 to use. This is
         *      a borrowed pointer. Do not call `Release()`.
         */
        void* GetD3D9Device() { return _ref->GetD3D9Device(); }
        ggxxacpr::GameVersion GetGameVersion() { return static_cast<ggxxacpr::GameVersion>(_ref->GetGameVersion()); }
        uint32_t GetViewWidth() { return _ref->GetViewWidth(); }
        uint32_t GetViewHeight() { return _ref->GetViewHeight(); }
        ggxxacpr::Entity GetRootEntity() { return ggxxacpr::Entity(_ref->GetRootEntity()); }
        uint32_t GetGlobalThrowFlags() { return _ref->GetGlobalThrowFlags(); }
        /**
         *  \brief Returns the pause state.
         * 
         *  The game is paused if this value is non zero. In training mode, this variable transitions from 0 to 1 to 2 when pausing.
         */
        int32_t GetPauseState() { return *_ref->GetPauseState(); }
        /**
         *  \brief Gets a pointer to the player input struct array. See `GGXXACPR_PlayerInput`.
         */
        GGXXACPR_PlayerInput* GetPlayerInputStructArr() { return _ref->GetPlayerInputStructArr(); }
        /**
         *  \brief Gets a pointer to the game's locale state. see `GGXXACPR_LocaleState`.
         */
        GGXXACPR_LocaleState* GetLocaleState() { return _ref->GetLocaleState(); }
    private:
        const BaseMod_GameDataApi* _ref;
    };

    template<typename T>
    using PeekMessageHook = void (__stdcall *)(T* userData, const BaseMod_HookContext* ctx, const BaseMod_PeekMessageInfo* info);
    template<typename T>
    using GameUpdateHook = void(__stdcall *)(T* userData, const HookContext* ctx, const GameUpdateInfo* info);
    template<typename T>
    using DrawHook = void(__stdcall *)(T* userData, const BaseMod_HookContext* ctx, const BaseMod_DrawInfo* info);

    class HookApi {
    public:
        HookApi() : _ref(nullptr) { }
        HookApi(const BaseMod_HookApi* ref = nullptr) : _ref(ref) { }
        /**
         * Returns true if there is a difference in major version number
         *      between actual and expected BaseMod API versions.
         */
        bool VersionError() {
            return (BASEMOD_API_VERSION_NUM & 0xFF0000) != (_ref->version & 0xFF0000);
        }
        const BaseMod_HookApi* GetCApi() { return _ref; }
        /**
         *  \brief Registers a hook to the PeekMessage hook.
         * 
         *  Use PeekMessage hooks to read windows message such as low level keyboard input.
         * 
         *  \param hookFn The callback function, see type `BaseMod_PeekMessageHook`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        HookId AfterPeekMessage(BaseMod_PeekMessageHook hookFn, void* userData) {
            return _ref->AfterPeekMessage(hookFn, userData);
        }
        /**
         *  \brief Registers a hook to the PeekMessage hook.
         * 
         *  Use PeekMessage hooks to read windows message such as low level keyboard input.
         *      Template version for type safety on the userData pointer.
         * 
         *  \param hookFn The callback function, see type `PeekMessageHook<T>`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        template<typename T>
        HookId AfterPeekMessage(PeekMessageHook<T> hookFn, T* userData) {
            return _ref->AfterPeekMessage(
                reinterpret_cast<BaseMod_PeekMessageHook>(hookFn),
                userData);
        }
        /**
         *  \brief Registers a hook to run before the game state updates.
         * 
         *  Use this to apply changes to the game state right before it runs an update.
         * 
         *  \param hookFn The callback function, see type `BaseMod_GameUpdateHook`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        HookId BeforeGameUpdate(BaseMod_GameUpdateHook hookFn, void* userData) {
            return _ref->BeforeGameUpdate(hookFn, userData);
        }
        /**
         *  \brief Registers a hook to run before the game state updates.
         * 
         *  Use this to apply changes to the game state right before it runs an update.
         *      Template version for type safety on the userData pointer.
         * 
         *  \param hookFn The callback function, see type `GameUpdateHook<T>`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        template<typename T>
        HookId BeforeGameUpdate(GameUpdateHook<T> hookFn, T* userData) {
            return _ref->BeforeGameUpdate(
                reinterpret_cast<BaseMod_GameUpdateHook>(hookFn),
                userData);
        }
        /**
         *  \brief Registers a hook to run after the game state updates.
         * 
         *  Use this to read game state as soon as it updates or to overwrite
         *      the game state right after the game updates it.
         * 
         *  \param hookFn The callback function, see type `BaseMod_GameUpdateHook`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        HookId AfterGameUpdate(BaseMod_GameUpdateHook hookFn, void* userData) {
            return _ref->AfterGameUpdate(hookFn, userData);
        }
        /**
         *  \brief Registers a hook to run after the game state updates.
         * 
         *  Use this to read game state as soon as it updates or to overwrite
         *      the game state right after the game updates it.
         *      Template version for type safety on the userData pointer.
         * 
         *  \param hookFn The callback function, see type `GameUpdateHook<T>`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        template<typename T>
        HookId AfterGameUpdate(GameUpdateHook<T> hookFn, T* userData) {
            return _ref->AfterGameUpdate(
                reinterpret_cast<BaseMod_GameUpdateHook>(hookFn),
                userData);
        }
        /**
         *  \brief Registers a hook to run before the call to `IDirect3DDevice9::EndScene`.
         * 
         *  Use this to add additional graphics logic to the game's main scene.
         * 
         *  \param hookFn The callback function, see type `BaseMod_DrawHook`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        HookId BeforeEndScene(BaseMod_DrawHook hookFn, void* userData) {
            return _ref->BeforeEndScene(hookFn, userData);
        }
        /**
         *  \brief Registers a hook to run before the call to `IDirect3DDevice9::EndScene`.
         * 
         *  Use this to add additional graphics logic to the game's main scene.
         *      Template version for type safety on the userData pointer.
         * 
         *  \param hookFn The callback function, see type `DrawHook<T>`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        template<typename T>
        HookId BeforeEndScene(DrawHook<T> hookFn, T* userData) {
            return _ref->BeforeEndScene(
                reinterpret_cast<BaseMod_DrawHook>(hookFn),
                userData);
        }
        /**
         *  \brief Registers a hook to run before the call to `IDirect3DDevice9::Present`.
         * 
         *  Use this to add a new scene to the current frame. Hooks registered must start and
         *      end their own scene with `IDirect3DDevice9::BeginScene` and `IDirect3DDevice9::EndScene`.
         * 
         *  \param hookFn The callback function, see type `BaseMod_DrawHook`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        HookId BeforePresent(BaseMod_DrawHook hookFn, void* userData) {
            return _ref->BeforePresent(hookFn, userData);
        }
        /**
         *  \brief Registers a hook to run before the call to `IDirect3DDevice9::Present`.
         * 
         *  Use this to add a new scene to the current frame. Hooks registered must start and
         *      end their own scene with `IDirect3DDevice9::BeginScene` and `IDirect3DDevice9::EndScene`.
         *      Template version for type safety on the userData pointer.
         * 
         *  \param hookFn The callback function, see type `DrawHook<T>`.
         *  \param userData A generic pointer to state data the callback function needs.
         * 
         *  \return A hook id value that can be passed to `RemoveHook`.
         */
        template<typename T>
        HookId BeforePresent(DrawHook<T> hookFn, T* userData) {
            return _ref->BeforePresent(
                reinterpret_cast<BaseMod_DrawHook>(hookFn),
                userData);
        }
        /**
         *  \brief Removes a hook from the registry.
         *  \param id The `BaseMod_HookId` of the hook to be removed.
         *  \return zero if no error occurred, otherwise returns the error code.
         */
        uint32_t RemoveHook(HookId id) {
            return _ref->RemoveHook(id);
        }

    private:
        const BaseMod_HookApi* _ref;
    };

    using MenuAction = BM_MenuAction;
    using ValueChangeCallback = BM_ValueChangeCallback;
    using CustomMenuHandler = BM_CustomMenuHandler;
    using ModMenuEntry = BaseMod_ModMenuEntry;
    class ModMenuApi {
    public:
        ModMenuApi() : _ref(nullptr) { }
        ModMenuApi(const BaseMod_ModMenuApi* ref = nullptr) : _ref(ref) {}
        /**
         *  \brief Registers a menu definition with the mod menu.
         * 
         *  \param title Tab name. Character limitations are similar to `BaseMod_NativeFunctionsApi::RenderText`
         *  \param entries An array of BaseMod_ModMenuEntry structures comprising the menu defintion.
         *      See `BaseMod_ModMenuEntry`. Callers must maintain the lifetime of values in the declaration.
         *  \return 0 if no error, else an error code.
         */
        const BaseMod_ModMenuApi* GetCApi() { return _ref; }
        uint32_t RegisterMenuTab(const char* title, const ModMenuEntry* entries, uint32_t numEntries) {
            return _ref->RegisterMenuTab(title, entries, numEntries);
        }
        uint32_t RegisterCustomMenuTab(const char* title, CustomMenuHandler handler) {
            return _ref->RegisterCustomMenuTab(title, handler);
        }

    private:
        const BaseMod_ModMenuApi* _ref;
    };

    class Api {
    public:
        Api()
            : _ref(nullptr)
            , NativeFunctions(nullptr)
            , GameData(nullptr)
            , Hooks(nullptr)
            , ModMenu(nullptr) { }
        Api(const BaseMod_Api* ref)
            : _ref(ref)
            , NativeFunctions(ref->NativeFunctions)
            , GameData(ref->GameData)
            , Hooks(ref->Hooks)
            , ModMenu(ref->ModMenu) { }
        /**
         * Returns true if there is a difference in major version number
         *      between actual and expected BaseMod API versions.
         */
        bool VersionError() {
            return (BASEMOD_API_VERSION_NUM & BASEMOD_MAJOR_VERSION_MASK) !=
                (_ref->version & BASEMOD_MAJOR_VERSION_MASK);
        }
        const BaseMod_Api* GetCApi() { return _ref; }
        /// \brief API for invoking native game functions
        NativeFunctionsApi NativeFunctions;
        /// \brief Access to notable game data
        GameDataApi GameData;
        /// \brief Function hooking manager
        HookApi Hooks;
        /// \brief Add options to mod menu
        ModMenuApi ModMenu;
    private:
        const BaseMod_Api* _ref;
    };
}


#endif