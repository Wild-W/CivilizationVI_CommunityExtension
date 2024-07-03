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

HANDLE mainThread;

ProxyTypes::DllCreateGameContext base_DllCreateGameContext;

ProxyTypes::PushMethods base_Cities_PushMethods;
ProxyTypes::PushMethods orig_Cities_PushMethods;

ProxyTypes::InstancedPushMethods base_GameDiplomacy_PushMethods;
ProxyTypes::InstancedPushMethods orig_GameDiplomacy_PushMethods;

ProxyTypes::IPlayerCities_GetInstance IPlayerCities_GetInstance;
ProxyTypes::DiplomaticRelations_GetInstance DiplomaticRelations_GetInstance;

ProxyTypes::FAutoVariable_edit FAutoVariable_edit;

ProxyTypes::SetMaxTurns SetMaxTurns;
ProxyTypes::SetHasConstructedTradingPost SetHasConstructedTradingPost;
ProxyTypes::DiplomaticRelations_ChangeGrievanceScore DiplomaticRelations_ChangeGrievanceScore;
ProxyTypes::Cities_AddGreatWork Cities_AddGreatWork;

ProxyTypes::Culture_Get Culture_Get;
ProxyTypes::FindOrAddGreatWork FindOrAddGreatWork;
ProxyTypes::SetGreatWorkPlayer SetGreatWorkPlayer;

ProxyTypes::CCallWithErrorHandling CCallWithErrorHandling;

ProxyTypes::RegisterScriptData base_RegisterScriptData;
ProxyTypes::RegisterScriptData orig_RegisterScriptData;

ProxyTypes::RegisterScriptDataForUI base_RegisterScriptDataForUI;
ProxyTypes::RegisterScriptDataForUI orig_RegisterScriptDataForUI;

ProxyTypes::EmergencyManager_ChangePlayerScore EmergencyManager_ChangePlayerScore;
ProxyTypes::EmergencyManager_ChangePlayerScore2 EmergencyManager_ChangePlayerScore2;
ProxyTypes::EmergencyManager_Get EmergencyManager_Get;

ProxyTypes::GlobalParameters_Initialize base_GlobalParameters_Initialize;
ProxyTypes::GlobalParameters_Initialize orig_GlobalParameters_Initialize;

ProxyTypes::GlobalParameters_Get GlobalParameters_Get;

ProxyTypes::ApplyTourism ApplyTourism;
ProxyTypes::GetPlayersToProcess GetPlayersToProcess;

static int lChangeGrievanceScore(hks::lua_State* L) {
    void* diplomaticRelations = DiplomaticRelations_GetInstance(L, 1, true);
    int player1Id = hks::checkinteger(L, 2);
    int player2Id = hks::checkinteger(L, 3);
    int amount = hks::checkinteger(L, 4);

    DiplomaticRelations_ChangeGrievanceScore(diplomaticRelations, player1Id, player2Id, amount);
    return 0;
}

static int __cdecl lCities_AddGreatWork(hks::lua_State* L) {
    void* cities = IPlayerCities_GetInstance(L, 1, true);
    int greatWorkListIndex = hks::checkinteger(L, 2);
    std::cout << cities << ' ' << greatWorkListIndex << '\n';

    Cities_AddGreatWork(cities, greatWorkListIndex);
    return 0;
}

static void __cdecl Hook_Cities_PushMethods(hks::lua_State* L, int stackOffset) {
    std::cout << "Hooked Cities::PushMethods!\n";

    PushLuaMethod(L, lCities_AddGreatWork, "lAddGreatWork", stackOffset, "AddGreatWork");

    base_Cities_PushMethods(L, stackOffset);
}

static int __cdecl lSetHasConstructedTradingPost(hks::lua_State* L) {
    hks::getfield(L, 1, "__instance");
    void* city = hks::touserdata(L, -1);
    if (city == NULL) {
        hks::error(L, "Failed to retrieve __instance field as userdata. Is your object NULL?");
        return 0;
    }
    hks::pop(L, 1);

    void* cityTrade = FAutoVariable_edit((void*)((uintptr_t)city + 0xe78));
    int playerId = hks::checkinteger(L, 2);
    bool didConstruct = hks::toboolean(L, 3);
    std::cout << cityTrade << ' ' << playerId << ' ' << didConstruct << '\n';

    SetHasConstructedTradingPost(cityTrade, playerId, didConstruct);
    return 0;
}

static int RegisterCityTradeManager(hks::lua_State* L) {
    std::cout << "Registering CityTradeManager!\n";

    hks::createtable(L, 0, 1);

    PushLuaMethod(L, lSetHasConstructedTradingPost, "lSetHasConstructedTradingPost", -2, "SetHasConstructedTradingPost");

    hks::setfield(L, hks::LUA_GLOBAL, "CityTradeManager");
    return 0;
}

static int lFindOrAddGreatWork(hks::lua_State* L) {
    void* culture = Culture_Get();
    unsigned int greatWorkIndex = hks::checkinteger(L, 1);

    int greatWorkListIndex = FindOrAddGreatWork(culture, greatWorkIndex);
    hks::pushinteger(L, greatWorkListIndex);
    return 1;
}

static int lSetGreatWorkPlayer(hks::lua_State* L) {
    void* culture = Culture_Get();
    int greatWorkListIndex = hks::checkinteger(L, 1);
    int playerId = hks::checkinteger(L, 2);

    SetGreatWorkPlayer(culture, greatWorkListIndex, playerId);
    return 0;
}

static int RegisterCultureManager(hks::lua_State* L) {
    std::cout << "Registering CultureManager!\n";

    hks::createtable(L, 0, 2);

    PushLuaMethod(L, lFindOrAddGreatWork, "lFindOrAddGreatWork", -2, "FindOrAddGreatWork");
    PushLuaMethod(L, lSetGreatWorkPlayer, "lSetGreatWorkPlayer", -2, "SetGreatWorkPlayer");

    hks::setfield(L, hks::LUA_GLOBAL, "CultureManager");
    return 0;
}

static int lEmergencyManager_ChangePlayerScore(hks::lua_State* L) {
    int argCount = hks::gettop(L);

    void* manager = EmergencyManager_Get();
    int playerId = hks::checkinteger(L, 1);

    if (argCount == 3) {
        int emergencyHash = hks::checkinteger(L, 2);
        int amount = hks::checkinteger(L, 3);
        EmergencyManager_ChangePlayerScore(manager, playerId, emergencyHash, amount);
    }
    else if (argCount == 4) {
        int otherPlayerId = hks::checkinteger(L, 2);
        int emergencyIndex = hks::checkinteger(L, 3);
        int amount = hks::checkinteger(L, 4);
        EmergencyManager_ChangePlayerScore2(manager, playerId, otherPlayerId, emergencyIndex, amount);
    }
    else {
        hks::error(L, "Incorrect number of arguments. Expected 3 or 4.");
    }

    return 0;
}

static int RegisterEmergencyManager(hks::lua_State* L) {
    std::cout << "Registering EmergencyManager!\n";

    hks::createtable(L, 0, 1);

    PushLuaMethod(L, lEmergencyManager_ChangePlayerScore, "lChangePlayerScore", -2, "ChangePlayerScore");

    hks::setfield(L, hks::LUA_GLOBAL, "EmergencyManager");
    return 0;
}

//std::vector<void*> getAlivePlayers() {
//    void* playerManager = PlayerManager_Edit();
//    std::vector<void*> alivePlayers;
//
//    uintptr_t* start = *(uintptr_t**)(playerManager + 0x50);
//    uintptr_t* end = *(uintptr_t**)(playerManager + 0x58);
//
//    while (start < end) {
//        void* player = reinterpret_cast<void*>(*start);
//        // Assuming some method or property to check if the player is alive
//        if (player->isAlive()) {
//            alivePlayers.push_back(player);
//        }
//        start++;
//    }
//
//    return alivePlayers;
//}

void PushSharedGlobals(hks::lua_State* L) {
    PushLuaMethod(L, MemoryManipulation::LuaExport::lMem, "lMem", hks::LUA_GLOBAL, "Mem");
    PushLuaMethod(L, MemoryManipulation::LuaExport::lObjMem, "lObjMem", hks::LUA_GLOBAL, "ObjMem");

    MemoryManipulation::LuaExport::PushFieldTypes(L);
}

void __cdecl Hook_RegisterScriptData(hks::lua_State* L) {
    std::cout << "Registering lua globals!\n";

    PushSharedGlobals(L);
    CCallWithErrorHandling(L, RegisterCityTradeManager, NULL);
    CCallWithErrorHandling(L, RegisterCultureManager, NULL);
    CCallWithErrorHandling(L, RegisterEmergencyManager, NULL);
    CCallWithErrorHandling(L, EconomicManager::Register, NULL);

    base_RegisterScriptData(L);
}

void __cdecl Hook_RegisterScriptDataForUI(hks::lua_State* _, hks::lua_State* L) {
    std::cout << "Registering cache lua globals!\n";

    PushSharedGlobals(L);

    base_RegisterScriptDataForUI(_, L);
}

static void __cdecl Hook_GameDiplomacy_PushMethods(void* _, hks::lua_State* L, int stackOffset) {
    std::cout << "Hooked GameDiplomacy::PushMethods!\n";

    PushLuaMethod(L, lChangeGrievanceScore, "lChangeGrievanceScore", stackOffset, "ChangeGrievanceScore");

    base_GameDiplomacy_PushMethods(_, L, stackOffset);
}

static void* Query(void* dbConnection, const char* query) {
    // return (**(databaseQueryFunction**)(*(uintptr_t*)databaseConnection + 0x18))(databaseConnection, query, 0xffffffff);
    typedef long long* (*DatabaseQueryFunction)(void* dbConnection, const char* query, int limit);

    long long* dbQuery = nullptr;

    if (dbConnection != nullptr) {
        uintptr_t funcPtrAddress = *(uintptr_t*)((char*)dbConnection + 0x18);
        
        if (funcPtrAddress != 0 && funcPtrAddress != 0xFFFFFFFF) {
            DatabaseQueryFunction queryFunc = (DatabaseQueryFunction)funcPtrAddress;
            dbQuery = queryFunc(dbConnection, query, -1);
        }
        else {
            std::cerr << "Invalid function pointer address: " << funcPtrAddress << '\n';
        }
    }

    return dbQuery;
}

// monopolyTourismModifier = *(int *)(*(longlong *)(economicManager + 0xb8) + 4 + playerId * 8);

// float* monopolyTourismModifier;

//static void __cdecl Hook_GlobalParameters_Initialize(void* globalParameters, void* databaseConnection) {
//    std::cout << "dbConnection: " << databaseConnection << '\n';
//    void* databaseQuery = Query(databaseConnection, "SELECT Value From GlobalParameters WHERE Name = ? LIMIT 1");
//    std::cout << "dbQuery: " << databaseQuery << '\n';
//    if (databaseQuery != NULL) {
//        GlobalParameters_Get(globalParameters, databaseQuery, "MONOPOLY_TOURISM_MODIFIER", monopolyTourismModifier, 1.0);
//        std::cout << *monopolyTourismModifier << '\n';
//    }
//
//    base_GlobalParameters_Initialize(globalParameters, databaseConnection);
//}

#pragma region Offsets
constexpr uintptr_t CURRENT_GAME_OFFSET = 0xb8aa60;

constexpr uintptr_t REGISTER_SCRIPT_DATA_OFFSET = 0x5bdac0;
constexpr uintptr_t DIPLOMATIC_RELATIONS_CHANGE_GRIEVANCE_SCORE_OFFSET = 0x1cea40;
constexpr uintptr_t IMAP_PLOT_GET_INSTANCE_OFFSET = 0x15d60;
constexpr uintptr_t IPLAYER_CITIES_GET_INSTANCE_OFFSET = 0x6ee9b0;
constexpr uintptr_t DIPLOMATIC_RELATIONS_EDIT = 0x1d0220;
constexpr uintptr_t C_CALL_WITH_ERROR_HANDLING_OFFSET = 0x9ad880;
constexpr uintptr_t CITIES_ADD_GREAT_WORK_OFFSET = 0x2643b0;
constexpr uintptr_t CITIES_PUSH_METHODS_OFFSET = 0x6eeb10;
constexpr uintptr_t SET_HAS_CONSTRUCTED_TRADING_POST_OFFSET = 0x14e720;
constexpr uintptr_t REGISTER_SCRIPT_DATA_FOR_UI_OFFSET = 0x5bdd80;
constexpr uintptr_t SET_MAX_TURNS_OFFSET = 0x597960;
constexpr uintptr_t GAME_F_AUTO_VARIABLE_EDIT_OFFSET = 0x72a920;
constexpr uintptr_t CULTURE_GET_OFFSET = 0x1c8250;
constexpr uintptr_t CULTURE_FIND_OR_ADD_GREAT_WORK_OFFSET = 0x1c80e0;
constexpr uintptr_t CULTURE_SET_GREAT_WORK_PLAYER_OFFSET = 0x1c8c80;
constexpr uintptr_t DIPLOMATIC_RELATIONS_GET_INSTANCE_OFFSET = 0x6d86e0;
constexpr uintptr_t IGAME_DIPLOMACY_PUSH_METHODS_OFFSET = 0x745660;
constexpr uintptr_t NEUTRALIAZE_GOVERNOR_OFFSET = 0x2df270;
constexpr uintptr_t EMERGENCY_MANAGER_GET_OFFSET = 0x19a7b0;
constexpr uintptr_t EMERGENCY_MANAGER_CHANGE_PLAYER_SCORE_OFFSET = 0x1991f0;
constexpr uintptr_t EMERGENCY_MANAGER_CHANGE_PLAYER_SCORE_2_OFFSET = 0x199140;
constexpr uintptr_t GLOBAL_PARAMETERS_INITIALIZE_OFFSET = 0x1f02a0;
constexpr uintptr_t GLOBAL_PARAMETERS_GET_OFFSET = 0x1f0120;
constexpr uintptr_t APPLY_TOURISM_OFFSET = 0x27a0e0;
constexpr uintptr_t GET_PLAYERS_TO_PROCESS_OFFSET = 0x49d40;
#pragma endregion

static void InitHooks() {
    std::cout << "Initializing hooks ...\n";
    using namespace Runtime;

    CCallWithErrorHandling = GetGameCoreGlobalAt<ProxyTypes::CCallWithErrorHandling>(C_CALL_WITH_ERROR_HANDLING_OFFSET);

    IPlayerCities_GetInstance = GetGameCoreGlobalAt<ProxyTypes::IPlayerCities_GetInstance>(IPLAYER_CITIES_GET_INSTANCE_OFFSET);
    DiplomaticRelations_GetInstance = GetGameCoreGlobalAt<ProxyTypes::DiplomaticRelations_GetInstance>(DIPLOMATIC_RELATIONS_GET_INSTANCE_OFFSET);

    Culture_Get = GetGameCoreGlobalAt<ProxyTypes::Culture_Get>(CULTURE_GET_OFFSET);
    FindOrAddGreatWork = GetGameCoreGlobalAt<ProxyTypes::FindOrAddGreatWork>(CULTURE_FIND_OR_ADD_GREAT_WORK_OFFSET);
    SetGreatWorkPlayer = GetGameCoreGlobalAt<ProxyTypes::SetGreatWorkPlayer>(CULTURE_SET_GREAT_WORK_PLAYER_OFFSET);

    SetMaxTurns = GetGameCoreGlobalAt<ProxyTypes::SetMaxTurns>(SET_MAX_TURNS_OFFSET);
    SetHasConstructedTradingPost = GetGameCoreGlobalAt
        <ProxyTypes::SetHasConstructedTradingPost>(SET_HAS_CONSTRUCTED_TRADING_POST_OFFSET);
    Cities_AddGreatWork = GetGameCoreGlobalAt<ProxyTypes::Cities_AddGreatWork>(CITIES_ADD_GREAT_WORK_OFFSET);
    DiplomaticRelations_ChangeGrievanceScore = GetGameCoreGlobalAt
        <ProxyTypes::DiplomaticRelations_ChangeGrievanceScore>(DIPLOMATIC_RELATIONS_CHANGE_GRIEVANCE_SCORE_OFFSET);

    FAutoVariable_edit = GetGameCoreGlobalAt<ProxyTypes::FAutoVariable_edit>(GAME_F_AUTO_VARIABLE_EDIT_OFFSET);

    EmergencyManager_ChangePlayerScore = GetGameCoreGlobalAt<ProxyTypes::EmergencyManager_ChangePlayerScore>(EMERGENCY_MANAGER_CHANGE_PLAYER_SCORE_OFFSET);
    EmergencyManager_ChangePlayerScore2 = GetGameCoreGlobalAt<ProxyTypes::EmergencyManager_ChangePlayerScore2>(EMERGENCY_MANAGER_CHANGE_PLAYER_SCORE_2_OFFSET);
    EmergencyManager_Get = GetGameCoreGlobalAt<ProxyTypes::EmergencyManager_Get>(EMERGENCY_MANAGER_GET_OFFSET);

    GlobalParameters_Get = GetGameCoreGlobalAt<ProxyTypes::GlobalParameters_Get>(GLOBAL_PARAMETERS_GET_OFFSET);
    ApplyTourism = GetGameCoreGlobalAt<ProxyTypes::ApplyTourism>(APPLY_TOURISM_OFFSET);
    GetPlayersToProcess = GetGameCoreGlobalAt<ProxyTypes::GetPlayersToProcess>(GET_PLAYERS_TO_PROCESS_OFFSET);

    orig_RegisterScriptData = GetGameCoreGlobalAt<ProxyTypes::RegisterScriptData>(REGISTER_SCRIPT_DATA_OFFSET);
    CreateHook(orig_RegisterScriptData, &Hook_RegisterScriptData, &base_RegisterScriptData);

    orig_RegisterScriptDataForUI = GetGameCoreGlobalAt<ProxyTypes::RegisterScriptDataForUI>(REGISTER_SCRIPT_DATA_FOR_UI_OFFSET);
    CreateHook(orig_RegisterScriptDataForUI, &Hook_RegisterScriptDataForUI, &base_RegisterScriptDataForUI);

    orig_Cities_PushMethods = GetGameCoreGlobalAt<ProxyTypes::PushMethods>(CITIES_PUSH_METHODS_OFFSET);
    CreateHook(orig_Cities_PushMethods, &Hook_Cities_PushMethods, &base_Cities_PushMethods);

    orig_GameDiplomacy_PushMethods = GetGameCoreGlobalAt<ProxyTypes::InstancedPushMethods>(IGAME_DIPLOMACY_PUSH_METHODS_OFFSET);
    CreateHook(orig_GameDiplomacy_PushMethods, &Hook_GameDiplomacy_PushMethods, &base_GameDiplomacy_PushMethods);

    EconomicManager::Create();
    Plot::Create();
    PlayerGovernors::Create();
    PlayerInfluence::Create();

    std::cout << "Hooks initialized!\n";
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    Runtime::Create();
    base_DllCreateGameContext = (ProxyTypes::DllCreateGameContext)GetProcAddress(Runtime::GameCore, "DllCreateGameContext");
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        Runtime::InitConsole();
        mainThread = CreateThread(0, 0, &MainThread, NULL, 0, NULL);
        break;
    case DLL_PROCESS_DETACH:
        CloseHandle(mainThread);
        Runtime::CloseConsole();
        Runtime::Destroy();
        break;
    }
    return TRUE;
}

extern "C" {
    __declspec(dllexport) void* __cdecl DllCreateGameContext(void) {
        std::cout << "Waiting for main thread ...\n";
        DWORD result = WaitForSingleObject(mainThread, INFINITE);
        if (result == WAIT_OBJECT_0) {
            std::cout << "Main thread completed successfully!\n";
        }
        else {
            std::cerr << "WaitForSingleObject failed: " << GetLastError() << '\n';
        }
        Runtime::InitMinHook();
        hks::InitHavokScript();
        InitHooks();
        return base_DllCreateGameContext();
    }
}

#pragma region ExportRedirectors
#pragma comment(linker,"/export:??4HksCompilerSettings@@QEAAAEAU0@$$QEAU0@@Z=GameCore_XP2_FinalRelease.??4HksCompilerSettings@@QEAAAEAU0@$$QEAU0@@Z,@1")
#pragma comment(linker,"/export:??4HksCompilerSettings@@QEAAAEAU0@AEBU0@@Z=GameCore_XP2_FinalRelease.??4HksCompilerSettings@@QEAAAEAU0@AEBU0@@Z,@2")
#pragma comment(linker,"/export:??4HksFixedHeapSettings@@QEAAAEAV0@$$QEAV0@@Z=GameCore_XP2_FinalRelease.??4HksFixedHeapSettings@@QEAAAEAV0@$$QEAV0@@Z,@3")
#pragma comment(linker,"/export:??4HksFixedHeapSettings@@QEAAAEAV0@AEBV0@@Z=GameCore_XP2_FinalRelease.??4HksFixedHeapSettings@@QEAAAEAV0@AEBV0@@Z,@4")
#pragma comment(linker,"/export:??4HksStateSettings@@QEAAAEAV0@$$QEAV0@@Z=GameCore_XP2_FinalRelease.??4HksStateSettings@@QEAAAEAV0@$$QEAV0@@Z,@5")
#pragma comment(linker,"/export:??4HksStateSettings@@QEAAAEAV0@AEBV0@@Z=GameCore_XP2_FinalRelease.??4HksStateSettings@@QEAAAEAV0@AEBV0@@Z,@6")
#pragma comment(linker,"/export:?_isHksGlobalMemoTestingMode@HksCompilerSettings@@QEBAHXZ=GameCore_XP2_FinalRelease.?_isHksGlobalMemoTestingMode@HksCompilerSettings@@QEBAHXZ,@7")
#pragma comment(linker,"/export:?_setHksGlobalMemoTestingMode@HksCompilerSettings@@QEAAXH@Z=GameCore_XP2_FinalRelease.?_setHksGlobalMemoTestingMode@HksCompilerSettings@@QEAAXH@Z,@8")
#pragma comment(linker,"/export:?getBytecodeSharingFormat@HksCompilerSettings@@QEBA?AW4BytecodeSharingFormat@1@XZ=GameCore_XP2_FinalRelease.?getBytecodeSharingFormat@HksCompilerSettings@@QEBA?AW4BytecodeSharingFormat@1@XZ,@9")
#pragma comment(linker,"/export:?getIntLiteralsEnabled@HksCompilerSettings@@QEBA?AW4IntLiteralOptions@1@XZ=GameCore_XP2_FinalRelease.?getIntLiteralsEnabled@HksCompilerSettings@@QEBA?AW4IntLiteralOptions@1@XZ,@10")
#pragma comment(linker,"/export:?getStrip@HksCompilerSettings@@QEBAPEAPEBDXZ=GameCore_XP2_FinalRelease.?getStrip@HksCompilerSettings@@QEBAPEAPEBDXZ,@11")
#pragma comment(linker,"/export:?hks_getcontext@@YAPEAXPEAUlua_State@@@Z=GameCore_XP2_FinalRelease.?hks_getcontext@@YAPEAXPEAUlua_State@@@Z,@12")
#pragma comment(linker,"/export:?hks_isstruct@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hks_isstruct@@YAHPEAUlua_State@@H@Z,@13")
#pragma comment(linker,"/export:?hks_rawgetslot@@YAXPEAUlua_State@@HPEBDH@Z=GameCore_XP2_FinalRelease.?hks_rawgetslot@@YAXPEAUlua_State@@HPEBDH@Z,@14")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAF@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAF@Z,@15")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAG@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAG@Z,@16")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAH@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAH@Z,@17")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAI@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAI@Z,@18")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAJ@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAJ@Z,@19")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAK@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAK@Z,@20")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAM@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAM@Z,@21")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAN@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAN@Z,@22")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAPEBD@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAPEBD@Z,@23")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_J@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_J@Z,@24")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_K@Z=GameCore_XP2_FinalRelease.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_K@Z,@25")
#pragma comment(linker,"/export:?hks_rawsetslot@@YAXPEAUlua_State@@HPEBDH@Z=GameCore_XP2_FinalRelease.?hks_rawsetslot@@YAXPEAUlua_State@@HPEBDH@Z,@26")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH1@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH1@Z,@27")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHF@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHF@Z,@28")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHG@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHG@Z,@29")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHH@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHH@Z,@30")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHI@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHI@Z,@31")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHJ@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHJ@Z,@32")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHK@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHK@Z,@33")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHM@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHM@Z,@34")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHN@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHN@Z,@35")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHPEBX@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHPEBX@Z,@36")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_J@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_J@Z,@37")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_K@Z=GameCore_XP2_FinalRelease.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_K@Z,@38")
#pragma comment(linker,"/export:?hks_setcontext@@YAXPEAUlua_State@@PEAX@Z=GameCore_XP2_FinalRelease.?hks_setcontext@@YAXPEAUlua_State@@PEAX@Z,@39")
#pragma comment(linker,"/export:?hksi_hks_getcontext@@YAPEAXPEAUlua_State@@@Z=GameCore_XP2_FinalRelease.?hksi_hks_getcontext@@YAPEAXPEAUlua_State@@@Z,@40")
#pragma comment(linker,"/export:?hksi_hks_isstruct@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_hks_isstruct@@YAHPEAUlua_State@@H@Z,@41")
#pragma comment(linker,"/export:?hksi_hks_rawgetslot@@YAXPEAUlua_State@@HPEBDH@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslot@@YAXPEAUlua_State@@HPEBDH@Z,@42")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAF@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAF@Z,@43")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAG@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAG@Z,@44")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAH@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAH@Z,@45")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAI@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAI@Z,@46")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAJ@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAJ@Z,@47")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAK@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAK@Z,@48")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAM@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAM@Z,@49")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAN@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAN@Z,@50")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAPEBD@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAPEBD@Z,@51")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_J@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_J@Z,@52")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_K@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_K@Z,@53")
#pragma comment(linker,"/export:?hksi_hks_rawsetslot@@YAXPEAUlua_State@@HPEBDH@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslot@@YAXPEAUlua_State@@HPEBDH@Z,@54")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHF@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHF@Z,@55")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHG@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHG@Z,@56")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHI@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHI@Z,@57")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHJ@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHJ@Z,@58")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHK@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHK@Z,@59")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHM@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHM@Z,@60")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHN@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHN@Z,@61")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_J@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_J@Z,@62")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_K@Z=GameCore_XP2_FinalRelease.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_K@Z,@63")
#pragma comment(linker,"/export:?hksi_hks_setcontext@@YAXPEAUlua_State@@PEAX@Z=GameCore_XP2_FinalRelease.?hksi_hks_setcontext@@YAXPEAUlua_State@@PEAX@Z,@64")
#pragma comment(linker,"/export:?hksi_lua_equal@@YAHPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?hksi_lua_equal@@YAHPEAUlua_State@@HH@Z,@65")
#pragma comment(linker,"/export:?hksi_lua_getfenv@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_getfenv@@YAXPEAUlua_State@@H@Z,@66")
#pragma comment(linker,"/export:?hksi_lua_getfield@@YAXPEAUlua_State@@HPEBD@Z=GameCore_XP2_FinalRelease.?hksi_lua_getfield@@YAXPEAUlua_State@@HPEBD@Z,@67")
#pragma comment(linker,"/export:?hksi_lua_getmetatable@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_getmetatable@@YAHPEAUlua_State@@H@Z,@68")
#pragma comment(linker,"/export:?hksi_lua_gettable@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_gettable@@YAXPEAUlua_State@@H@Z,@69")
#pragma comment(linker,"/export:?hksi_lua_gettop@@YAHPEAUlua_State@@@Z=GameCore_XP2_FinalRelease.?hksi_lua_gettop@@YAHPEAUlua_State@@@Z,@70")
#pragma comment(linker,"/export:?hksi_lua_insert@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_insert@@YAXPEAUlua_State@@H@Z,@71")
#pragma comment(linker,"/export:?hksi_lua_iscfunction@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_iscfunction@@YAHPEAUlua_State@@H@Z,@72")
#pragma comment(linker,"/export:?hksi_lua_isnumber@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_isnumber@@YAHPEAUlua_State@@H@Z,@73")
#pragma comment(linker,"/export:?hksi_lua_isstring@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_isstring@@YAHPEAUlua_State@@H@Z,@74")
#pragma comment(linker,"/export:?hksi_lua_isuserdata@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_isuserdata@@YAHPEAUlua_State@@H@Z,@75")
#pragma comment(linker,"/export:?hksi_lua_lessthan@@YAHPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?hksi_lua_lessthan@@YAHPEAUlua_State@@HH@Z,@76")
#pragma comment(linker,"/export:?hksi_lua_next@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_next@@YAHPEAUlua_State@@H@Z,@77")
#pragma comment(linker,"/export:?hksi_lua_objlen@@YA_KPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_objlen@@YA_KPEAUlua_State@@H@Z,@78")
#pragma comment(linker,"/export:?hksi_lua_pushinteger@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_pushinteger@@YAXPEAUlua_State@@H@Z,@79")
#pragma comment(linker,"/export:?hksi_lua_pushlightuserdata@@YAXPEAUlua_State@@PEAX@Z=GameCore_XP2_FinalRelease.?hksi_lua_pushlightuserdata@@YAXPEAUlua_State@@PEAX@Z,@80")
#pragma comment(linker,"/export:?hksi_lua_pushnil@@YAXPEAUlua_State@@@Z=GameCore_XP2_FinalRelease.?hksi_lua_pushnil@@YAXPEAUlua_State@@@Z,@81")
#pragma comment(linker,"/export:?hksi_lua_pushnumber@@YAXPEAUlua_State@@N@Z=GameCore_XP2_FinalRelease.?hksi_lua_pushnumber@@YAXPEAUlua_State@@N@Z,@82")
#pragma comment(linker,"/export:?hksi_lua_pushthread@@YAHPEAUlua_State@@@Z=GameCore_XP2_FinalRelease.?hksi_lua_pushthread@@YAHPEAUlua_State@@@Z,@83")
#pragma comment(linker,"/export:?hksi_lua_pushvalue@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_pushvalue@@YAXPEAUlua_State@@H@Z,@84")
#pragma comment(linker,"/export:?hksi_lua_rawequal@@YAHPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawequal@@YAHPEAUlua_State@@HH@Z,@85")
#pragma comment(linker,"/export:?hksi_lua_rawget@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawget@@YAXPEAUlua_State@@H@Z,@86")
#pragma comment(linker,"/export:?hksi_lua_rawget_array_lud@@YAPEAXPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawget_array_lud@@YAPEAXPEAUlua_State@@HH@Z,@87")
#pragma comment(linker,"/export:?hksi_lua_rawgeti@@YAXPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawgeti@@YAXPEAUlua_State@@HH@Z,@88")
#pragma comment(linker,"/export:?hksi_lua_rawlength@@YA_KPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawlength@@YA_KPEAUlua_State@@H@Z,@89")
#pragma comment(linker,"/export:?hksi_lua_rawset@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawset@@YAXPEAUlua_State@@H@Z,@90")
#pragma comment(linker,"/export:?hksi_lua_rawset_array@@YAXPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawset_array@@YAXPEAUlua_State@@HH@Z,@91")
#pragma comment(linker,"/export:?hksi_lua_rawset_array_lud@@YAXPEAUlua_State@@HHPEAX@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawset_array_lud@@YAXPEAUlua_State@@HHPEAX@Z,@92")
#pragma comment(linker,"/export:?hksi_lua_rawseti@@YAXPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?hksi_lua_rawseti@@YAXPEAUlua_State@@HH@Z,@93")
#pragma comment(linker,"/export:?hksi_lua_remove@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_remove@@YAXPEAUlua_State@@H@Z,@94")
#pragma comment(linker,"/export:?hksi_lua_replace@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_replace@@YAXPEAUlua_State@@H@Z,@95")
#pragma comment(linker,"/export:?hksi_lua_setfenv@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_setfenv@@YAHPEAUlua_State@@H@Z,@96")
#pragma comment(linker,"/export:?hksi_lua_setfield@@YAXPEAUlua_State@@HPEBD@Z=GameCore_XP2_FinalRelease.?hksi_lua_setfield@@YAXPEAUlua_State@@HPEBD@Z,@97")
#pragma comment(linker,"/export:?hksi_lua_setmetatable@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_setmetatable@@YAHPEAUlua_State@@H@Z,@98")
#pragma comment(linker,"/export:?hksi_lua_settop@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_settop@@YAXPEAUlua_State@@H@Z,@99")
#pragma comment(linker,"/export:?hksi_lua_toboolean@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_toboolean@@YAHPEAUlua_State@@H@Z,@100")
#pragma comment(linker,"/export:?hksi_lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z=GameCore_XP2_FinalRelease.?hksi_lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z,@101")
#pragma comment(linker,"/export:?hksi_lua_tointeger@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_tointeger@@YAHPEAUlua_State@@H@Z,@102")
#pragma comment(linker,"/export:?hksi_lua_tolstring@@YAPEBDPEAUlua_State@@HPEA_K@Z=GameCore_XP2_FinalRelease.?hksi_lua_tolstring@@YAPEBDPEAUlua_State@@HPEA_K@Z,@103")
#pragma comment(linker,"/export:?hksi_lua_tonumber@@YANPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_tonumber@@YANPEAUlua_State@@H@Z,@104")
#pragma comment(linker,"/export:?hksi_lua_topointer@@YAPEBXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_topointer@@YAPEBXPEAUlua_State@@H@Z,@105")
#pragma comment(linker,"/export:?hksi_lua_tothread@@YAPEAUlua_State@@PEAU1@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_tothread@@YAPEAUlua_State@@PEAU1@H@Z,@106")
#pragma comment(linker,"/export:?hksi_lua_touserdata@@YAPEAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_touserdata@@YAPEAXPEAUlua_State@@H@Z,@107")
#pragma comment(linker,"/export:?hksi_lua_type@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?hksi_lua_type@@YAHPEAUlua_State@@H@Z,@108")
#pragma comment(linker,"/export:?isEmitStruct@HksCompilerSettings@@QEBAHXZ=GameCore_XP2_FinalRelease.?isEmitStruct@HksCompilerSettings@@QEBAHXZ,@109")
#pragma comment(linker,"/export:?isGlobalMemoization@HksCompilerSettings@@QEBAHXZ=GameCore_XP2_FinalRelease.?isGlobalMemoization@HksCompilerSettings@@QEBAHXZ,@110")
#pragma comment(linker,"/export:?lua_equal@@YAHPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?lua_equal@@YAHPEAUlua_State@@HH@Z,@111")
#pragma comment(linker,"/export:?lua_getfenv@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_getfenv@@YAXPEAUlua_State@@H@Z,@112")
#pragma comment(linker,"/export:?lua_getfield@@YAXPEAUlua_State@@HPEBD@Z=GameCore_XP2_FinalRelease.?lua_getfield@@YAXPEAUlua_State@@HPEBD@Z,@113")
#pragma comment(linker,"/export:?lua_getmetatable@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_getmetatable@@YAHPEAUlua_State@@H@Z,@114")
#pragma comment(linker,"/export:?lua_gettable@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_gettable@@YAXPEAUlua_State@@H@Z,@115")
#pragma comment(linker,"/export:?lua_gettop@@YAHPEAUlua_State@@@Z=GameCore_XP2_FinalRelease.?lua_gettop@@YAHPEAUlua_State@@@Z,@116")
#pragma comment(linker,"/export:?lua_insert@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_insert@@YAXPEAUlua_State@@H@Z,@117")
#pragma comment(linker,"/export:?lua_iscfunction@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_iscfunction@@YAHPEAUlua_State@@H@Z,@118")
#pragma comment(linker,"/export:?lua_isnumber@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_isnumber@@YAHPEAUlua_State@@H@Z,@119")
#pragma comment(linker,"/export:?lua_isstring@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_isstring@@YAHPEAUlua_State@@H@Z,@120")
#pragma comment(linker,"/export:?lua_isuserdata@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_isuserdata@@YAHPEAUlua_State@@H@Z,@121")
#pragma comment(linker,"/export:?lua_lessthan@@YAHPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?lua_lessthan@@YAHPEAUlua_State@@HH@Z,@122")
#pragma comment(linker,"/export:?lua_next@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_next@@YAHPEAUlua_State@@H@Z,@123")
#pragma comment(linker,"/export:?lua_objlen@@YA_KPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_objlen@@YA_KPEAUlua_State@@H@Z,@124")
#pragma comment(linker,"/export:?lua_pushinteger@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_pushinteger@@YAXPEAUlua_State@@H@Z,@125")
#pragma comment(linker,"/export:?lua_pushlightuserdata@@YAXPEAUlua_State@@PEAX@Z=GameCore_XP2_FinalRelease.?lua_pushlightuserdata@@YAXPEAUlua_State@@PEAX@Z,@126")
#pragma comment(linker,"/export:?lua_pushnil@@YAXPEAUlua_State@@@Z=GameCore_XP2_FinalRelease.?lua_pushnil@@YAXPEAUlua_State@@@Z,@127")
#pragma comment(linker,"/export:?lua_pushnumber@@YAXPEAUlua_State@@N@Z=GameCore_XP2_FinalRelease.?lua_pushnumber@@YAXPEAUlua_State@@N@Z,@128")
#pragma comment(linker,"/export:?lua_pushthread@@YAHPEAUlua_State@@@Z=GameCore_XP2_FinalRelease.?lua_pushthread@@YAHPEAUlua_State@@@Z,@129")
#pragma comment(linker,"/export:?lua_pushvalue@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_pushvalue@@YAXPEAUlua_State@@H@Z,@130")
#pragma comment(linker,"/export:?lua_rawequal@@YAHPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?lua_rawequal@@YAHPEAUlua_State@@HH@Z,@131")
#pragma comment(linker,"/export:?lua_rawget@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_rawget@@YAXPEAUlua_State@@H@Z,@132")
#pragma comment(linker,"/export:?lua_rawgeti@@YAXPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?lua_rawgeti@@YAXPEAUlua_State@@HH@Z,@133")
#pragma comment(linker,"/export:?lua_rawset@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_rawset@@YAXPEAUlua_State@@H@Z,@134")
#pragma comment(linker,"/export:?lua_rawseti@@YAXPEAUlua_State@@HH@Z=GameCore_XP2_FinalRelease.?lua_rawseti@@YAXPEAUlua_State@@HH@Z,@135")
#pragma comment(linker,"/export:?lua_remove@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_remove@@YAXPEAUlua_State@@H@Z,@136")
#pragma comment(linker,"/export:?lua_replace@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_replace@@YAXPEAUlua_State@@H@Z,@137")
#pragma comment(linker,"/export:?lua_setfenv@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_setfenv@@YAHPEAUlua_State@@H@Z,@138")
#pragma comment(linker,"/export:?lua_setfield@@YAXPEAUlua_State@@HPEBD@Z=GameCore_XP2_FinalRelease.?lua_setfield@@YAXPEAUlua_State@@HPEBD@Z,@139")
#pragma comment(linker,"/export:?lua_setmetatable@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_setmetatable@@YAHPEAUlua_State@@H@Z,@140")
#pragma comment(linker,"/export:?lua_settop@@YAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_settop@@YAXPEAUlua_State@@H@Z,@141")
#pragma comment(linker,"/export:?lua_toboolean@@YA_NPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_toboolean@@YA_NPEAUlua_State@@H@Z,@142")
#pragma comment(linker,"/export:?lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z=GameCore_XP2_FinalRelease.?lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z,@143")
#pragma comment(linker,"/export:?lua_tointeger@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_tointeger@@YAHPEAUlua_State@@H@Z,@144")
#pragma comment(linker,"/export:?lua_tolstring@@YAPEBDPEAUlua_State@@HPEA_K@Z=GameCore_XP2_FinalRelease.?lua_tolstring@@YAPEBDPEAUlua_State@@HPEA_K@Z,@145")
#pragma comment(linker,"/export:?lua_tonumber@@YANPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_tonumber@@YANPEAUlua_State@@H@Z,@146")
#pragma comment(linker,"/export:?lua_topointer@@YAPEBXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_topointer@@YAPEBXPEAUlua_State@@H@Z,@147")
#pragma comment(linker,"/export:?lua_tothread@@YAPEAUlua_State@@PEAU1@H@Z=GameCore_XP2_FinalRelease.?lua_tothread@@YAPEAUlua_State@@PEAU1@H@Z,@148")
#pragma comment(linker,"/export:?lua_touserdata@@YAPEAXPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_touserdata@@YAPEAXPEAUlua_State@@H@Z,@149")
#pragma comment(linker,"/export:?lua_type@@YAHPEAUlua_State@@H@Z=GameCore_XP2_FinalRelease.?lua_type@@YAHPEAUlua_State@@H@Z,@150")
#pragma comment(linker,"/export:?setBytecodeSharingFormat@HksCompilerSettings@@QEAAXW4BytecodeSharingFormat@1@@Z=GameCore_XP2_FinalRelease.?setBytecodeSharingFormat@HksCompilerSettings@@QEAAXW4BytecodeSharingFormat@1@@Z,@151")
#pragma comment(linker,"/export:?setEmitStruct@HksCompilerSettings@@QEAAXH@Z=GameCore_XP2_FinalRelease.?setEmitStruct@HksCompilerSettings@@QEAAXH@Z,@152")
#pragma comment(linker,"/export:?setGlobalMemoization@HksCompilerSettings@@QEAAXH@Z=GameCore_XP2_FinalRelease.?setGlobalMemoization@HksCompilerSettings@@QEAAXH@Z,@153")
#pragma comment(linker,"/export:?setIntLiteralsEnabled@HksCompilerSettings@@QEAAXW4IntLiteralOptions@1@@Z=GameCore_XP2_FinalRelease.?setIntLiteralsEnabled@HksCompilerSettings@@QEAAXW4IntLiteralOptions@1@@Z,@154")
// #pragma comment(linker,"/export:DllCreateGameContext=GameCore_XP2_FinalRelease.DllCreateGameContext,@155")
#pragma comment(linker,"/export:DllDestroyGameContext=GameCore_XP2_FinalRelease.DllDestroyGameContext,@156")
#pragma comment(linker,"/export:EXP_GetTelemetrySessionHash=GameCore_XP2_FinalRelease.EXP_GetTelemetrySessionHash,@157")
#pragma endregion
