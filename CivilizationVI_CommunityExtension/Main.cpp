#include <windows.h>
#include <iostream>
#include "HavokScript.h"
#include <set>
#include <cstring>
#include "Runtime.h"
#include "MemoryManipulation.h"
#include "ProxyTypes.h"
#include <cmath>
#include <algorithm>
#include "EconomicManager.h"
#include "Plot.h"
#include "PlayerGovernors.h"
#include "PlayerInfluence.h"
#include "GameDiplomacy.h"
#include "EmergencyManager.h"
#include "CultureManager.h"
#include "Game.h"
#include "CityTradeManager.h"
#include "PlayerCities.h"
#include "GovernorManager.h"
#include "Player.h"
#include "UnitManager.h"
#include "Unit.h"
#include "AI.h"
#include "EventSystems.h"
#include "NationalParks.h"
#include "CivMap.h"
#include "Rules.h"
#include "ForgeUI.h"

static std::once_flag g_initFlag;
static bool g_consoleAllocated = false;

ProxyTypes::DllCreateGameContext base_DllCreateGameContext;
ProxyTypes::DllDestroyGameContext base_DllDestroyGameContext;

ProxyTypes::SetMaxTurns SetMaxTurns;

ProxyTypes::CCallWithErrorHandling CCallWithErrorHandling;

ProxyTypes::RegisterScriptData base_RegisterScriptData;
ProxyTypes::RegisterScriptData orig_RegisterScriptData;

ProxyTypes::RegisterScriptDataForUI base_RegisterScriptDataForUI;
ProxyTypes::RegisterScriptDataForUI orig_RegisterScriptDataForUI;

ProxyTypes::GlobalParameters_Initialize base_GlobalParameters_Initialize;
ProxyTypes::GlobalParameters_Initialize orig_GlobalParameters_Initialize;

ProxyTypes::GlobalParameters_Get GlobalParameters_Get;

ProxyTypes::ApplyTourism ApplyTourism;
ProxyTypes::GetPlayersToProcess GetPlayersToProcess;

static int lGetCurrentThreadId(hks::lua_State* L) {
    hks::pushinteger(L, GetCurrentThreadId());
    return 1;
}

static void PushDebugLibrary(hks::lua_State* L) {
    hks::pushnamedcclosure(L, hks::luaopen_debug, 0, "luaopen_debug", 0);
    hks::pcall(L, 0, 1, NULL);

    // From PUC Lua 5.1
    // Supposedly deprecated in havokscript and removed in favor of using their in-house debugger
    // debug.c_breakpoint() exists which is equivalent to the `int3` opcode.
    //
    // They don't actually work though. Would appreciate contributions to fix that.
    PushLuaMethod(L, hks::l_gethook, "l_gethook", -2, "gethook");
    PushLuaMethod(L, hks::l_sethook, "l_sethook", -2, "sethook");

    // Custom
    PushLuaMethod(L, lGetCurrentThreadId, "lGetCurrentThreadId", -2, "GetCurrentThreadId");

    hks::setfield(L, hks::LUA_GLOBAL, "debug");
}

static void PushSharedGlobals(hks::lua_State* L) {
    PushDebugLibrary(L);

    PushLuaMethod(L, MemoryManipulation::LuaExport::lMem, "lMem", hks::LUA_GLOBAL, "Mem");
    PushLuaMethod(L, MemoryManipulation::LuaExport::lObjMem, "lObjMem", hks::LUA_GLOBAL, "ObjMem");
    PushLuaMethod(L, MemoryManipulation::LuaExport::lRegisterCallEvent, "lRegisterCallEvent", hks::LUA_GLOBAL, "RegisterCallEvent");
    PushLuaMethod(L, EventSystems::lRegisterProcessor, "lRegisterProcessor", hks::LUA_GLOBAL, "RegisterProcessor");

    MemoryManipulation::LuaExport::PushFieldTypes(L);
}

static void Hook_RegisterScriptData(hks::lua_State* L) {
    Runtime::MainLogger->Info("Registering lua globals!");

    PushSharedGlobals(L);
    CCallWithErrorHandling(L, CityTradeManager::Register, NULL);
    CCallWithErrorHandling(L, CultureManager::Register, NULL);
    CCallWithErrorHandling(L, EmergencyManager::Register, NULL);
    CCallWithErrorHandling(L, EconomicManager::Register, NULL);
    CCallWithErrorHandling(L, GovernorManager::Register, NULL);
    CCallWithErrorHandling(L, AI::CongressSupport::RegisterOutcomeTypes, NULL);
    CCallWithErrorHandling(L, AI::Espionage::RegisterAIEspionageManager, NULL);
    CCallWithErrorHandling(L, NationalParks::Register, NULL);
    CCallWithErrorHandling(L, Rules::Players::Register, NULL);

    base_RegisterScriptData(L);
}

static void Hook_RegisterScriptDataForUI(hks::lua_State* _, hks::lua_State* L) {
    Runtime::MainLogger->Info("Registering cache lua globals!");

    PushSharedGlobals(L);
    CCallWithErrorHandling(L, NationalParks::Cache::Register, NULL);

    base_RegisterScriptDataForUI(_, L);
}

#pragma region Offsets
constexpr uintptr_t CURRENT_GAME_OFFSET = 0xb8aa60;

constexpr uintptr_t REGISTER_SCRIPT_DATA_OFFSET = 0x5bdac0;
constexpr uintptr_t C_CALL_WITH_ERROR_HANDLING_OFFSET = 0x9ad880;
constexpr uintptr_t REGISTER_SCRIPT_DATA_FOR_UI_OFFSET = 0x5bdd80;
constexpr uintptr_t SET_MAX_TURNS_OFFSET = 0x597960;
constexpr uintptr_t GLOBAL_PARAMETERS_INITIALIZE_OFFSET = 0x1f02a0;
constexpr uintptr_t GLOBAL_PARAMETERS_GET_OFFSET = 0x1f0120;
constexpr uintptr_t APPLY_TOURISM_OFFSET = 0x27a0e0;
constexpr uintptr_t GET_PLAYERS_TO_PROCESS_OFFSET = 0x49d40;
#pragma endregion

static void InitHooks() {
    using namespace Runtime;

    MainLogger->Info("Initializing Hooks...");

    ForgeUI::Create();
    EventSystems::Create();

    CCallWithErrorHandling = GetGameCoreGlobalAt<ProxyTypes::CCallWithErrorHandling>(C_CALL_WITH_ERROR_HANDLING_OFFSET);

    SetMaxTurns = GetGameCoreGlobalAt<ProxyTypes::SetMaxTurns>(SET_MAX_TURNS_OFFSET);

    GlobalParameters_Get = GetGameCoreGlobalAt<ProxyTypes::GlobalParameters_Get>(GLOBAL_PARAMETERS_GET_OFFSET);
    ApplyTourism = GetGameCoreGlobalAt<ProxyTypes::ApplyTourism>(APPLY_TOURISM_OFFSET);
    GetPlayersToProcess = GetGameCoreGlobalAt<ProxyTypes::GetPlayersToProcess>(GET_PLAYERS_TO_PROCESS_OFFSET);

    orig_RegisterScriptData = GetGameCoreGlobalAt<ProxyTypes::RegisterScriptData>(REGISTER_SCRIPT_DATA_OFFSET);
    CreateHook(orig_RegisterScriptData, &Hook_RegisterScriptData, &base_RegisterScriptData);

    orig_RegisterScriptDataForUI = GetGameCoreGlobalAt<ProxyTypes::RegisterScriptDataForUI>(REGISTER_SCRIPT_DATA_FOR_UI_OFFSET);
    CreateHook(orig_RegisterScriptDataForUI, &Hook_RegisterScriptDataForUI, &base_RegisterScriptDataForUI);

    EconomicManager::Create();
    Plot::Create();
    PlayerGovernors::Create();
    PlayerInfluence::Create();
    GameDiplomacy::Create();
    EmergencyManager::Create();
    CultureManager::Create();
    Game::Create();
    CityTradeManager::Create();
    PlayerCities::Create();
    Player::Create();
    UnitManager::Create();
    Unit::Create();
    AI::CongressSupport::Create();
    AI::Espionage::Create();
    NationalParks::Create();
    CivMap::Create();
    Rules::Players::Create();

    MainLogger->Info("Hooks initialized!");
}

static void InitializeCommunityExtension() {
    // 1. Initialize Runtime (including logging settings) first
    Runtime::Create();

    // 2. Then create the console
    int enableConsole = GetPrivateProfileIntA("Settings", "EnableConsole", 0, "./CommunityExtension.ini");
    if (enableConsole) {
        Runtime::InitConsole();
        g_consoleAllocated = true;
    }

    // 3. Get base GameCore create and destroy functions
    base_DllCreateGameContext = reinterpret_cast<ProxyTypes::DllCreateGameContext>(
        GetProcAddress(Runtime::GameCore, "DllCreateGameContext"));
    base_DllDestroyGameContext = reinterpret_cast<ProxyTypes::DllDestroyGameContext>(
        GetProcAddress(Runtime::GameCore, "DllDestroyGameContext"));
    
    if (!base_DllCreateGameContext) {
        throw std::runtime_error("Could not find function: GameCore_XP2::DllCreateGameContext!");
    }
    if (!base_DllDestroyGameContext) {
        throw std::runtime_error("Could not find function: GameCore_XP2::DllDestroyGameContext!");
    }

    // Do stuff to the game
    Runtime::InitMinHook();
    hks::InitHavokScript();
    InitHooks();
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        break;

    case DLL_PROCESS_DETACH:
        // lpReserved is nullptr only during a clean FreeLibrary/normal exit
        if (!lpReserved) {
            if (g_consoleAllocated) {
                Runtime::CloseConsole();
            }
            Runtime::Destroy();
        }
        break;
    }
    return TRUE;
}

extern "C" {
    __declspec(dllexport) void* __cdecl DllCreateGameContext(void) {
        std::call_once(g_initFlag, InitializeCommunityExtension);

        // Normal game stuff happens
        return base_DllCreateGameContext();
    }

    // WildW, 11-3-2026
    //
    // The address for this function has to exist before any GameCore initialization is done.
    // This is because export forwarding (see Exports.h) is resolved dynamically. (at the moment the address is polled)
    // So, if we haven't yet called LoadLibrary on the original GameCore, then the call to GetProcAddress at Base+0xf85c3
    // will result in error code 127 (not found).
    //
    // Originally, this problem was solved when I brute forced initialization with a bunch of different techniques,
    // eventually landing on creating a new WINAPI thread which immediately started loading the original GameCore
    // into memory. The old solution is a race condition.
    __declspec(dllexport) void __cdecl DllDestroyGameContext(void* _) {
        base_DllDestroyGameContext(_);
    }
}

// Export all functions not handled here
#include "Exports.h"
