#include <windows.h>
#include <iostream>
#include "HavokScript.h"
#include <set>
#include <cstring>
#include "Runtime.h"
#include "MemoryManipulation.h"
#include "ProxyTypes.h"
#include <cmath>

HANDLE mainThread;

ProxyTypes::DllCreateGameContext base_DllCreateGameContext;

ProxyTypes::PushMethods base_Plot_PushMethods;
ProxyTypes::PushMethods orig_Plot_PushMethods;

ProxyTypes::PushMethods base_Cities_PushMethods;
ProxyTypes::PushMethods orig_Cities_PushMethods;

ProxyTypes::PushMethods base_Influence_PushMethods;
ProxyTypes::PushMethods orig_Influence_PushMethods;

ProxyTypes::IMapPlot_GetInstance IMapPlot_GetInstance;
ProxyTypes::IPlayerCities_GetInstance IPlayerCities_GetInstance;
ProxyTypes::IPlayerInfluence_GetInstance IPlayerInfluence_GetInstance;
ProxyTypes::DiplomaticRelations_Edit DiplomaticRelations_Edit;

ProxyTypes::DiplomaticRelations_ChangeGrievanceScore DiplomaticRelations_ChangeGrievanceScore;
ProxyTypes::Cities_AddGreatWork Cities_AddGreatWork;
ProxyTypes::Influence_SetTokensToGive Influence_SetTokensToGive;

ProxyTypes::CCallWithErrorHandling CCallWithErrorHandling;

ProxyTypes::SetAppeal base_SetAppeal;
ProxyTypes::SetAppeal orig_SetAppeal;

ProxyTypes::RegisterScriptData base_RegisterScriptData;
ProxyTypes::RegisterScriptData orig_RegisterScriptData;

std::set<short*> lockedAppeals;

void __cdecl Hook_SetAppeal(void* plot, int appeal) {
    std::cout << "Hooked SetAppeal!\n";
    if (lockedAppeals.find((short*)((uintptr_t)plot + 0x4a)) == lockedAppeals.end()) {
        base_SetAppeal(plot, appeal);
    }
}

static int lSetAppeal(hks::lua_State* L) {
    void* plot = IMapPlot_GetInstance(L, 1, true);
    int appeal = hks::checkinteger(L, 2);
    std::cout << plot << ' ' << appeal << '\n';
    Hook_SetAppeal(plot, appeal);
    return 0;
}

static int lLockAppeal(hks::lua_State* L) {
    void* plot = IMapPlot_GetInstance(L, 1, true);
    bool setToLock = hks::toboolean(L, 2);
    short* appealAddress = (short*)((uintptr_t)plot + 0x4a);
    std::cout << "Plot adr: " << plot << "\nAppeal adr: " << appealAddress << "\nAppeal: " << *appealAddress << "\nsetToLock: " << setToLock << '\n';
    if (setToLock) {
        lockedAppeals.insert(appealAddress);
    }
    else {
        lockedAppeals.erase(appealAddress);
    }
    return 0;
}

static int lChangeGrievanceScore(hks::lua_State* L) {
    int player1Id = hks::checkinteger(L, 1);
    int player2Id = hks::checkinteger(L, 2);
    int amount = hks::checkinteger(L, 3);

    void* diplomaticRelations = DiplomaticRelations_Edit();
    DiplomaticRelations_ChangeGrievanceScore(diplomaticRelations, player1Id, player2Id, amount);
    return 0;
}

static void __cdecl Hook_Plot_PushMethods(hks::lua_State* L, int stackOffset) {
    std::cout << "Hooked Plot::PushMethods!\n";

    hks::pushnamedcclosure(L, lSetAppeal, 0, "lSetAppeal", 0);
    hks::setfield(L, stackOffset, "SetAppeal");
    hks::pushnamedcclosure(L, lLockAppeal, 0, "lLockAppeal", 0);
    hks::setfield(L, stackOffset, "LockAppeal");

    base_Plot_PushMethods(L, stackOffset);
}

static int __cdecl lCities_AddGreatWork(hks::lua_State* L) {
    void* cities = IPlayerCities_GetInstance(L, 1, true);
    int greatWorkIndex = hks::checkinteger(L, 2);
    std::cout << cities << ' ' << greatWorkIndex << '\n';

    Cities_AddGreatWork(cities, greatWorkIndex);
    std::cout << "After\n";
    return 0;
}

static void __cdecl Hook_Cities_PushMethods(hks::lua_State* L, int stackOffset) {
    std::cout << "Hooked Cities::PushMethods!\n";

    hks::pushnamedcclosure(L, lCities_AddGreatWork, 0, "lAddGreatWork", 0);
    hks::setfield(L, stackOffset, "AddGreatWork");

    base_Cities_PushMethods(L, stackOffset);
}

static int RegisterDiplomaticRelations(hks::lua_State* L) {
    std::cout << "Registering DiplomaticRelations\n";

    hks::createtable(L, 0, 1);

    hks::pushnamedcclosure(L, lChangeGrievanceScore, 0, "lChangeGrievanceScore", 0);
    hks::setfield(L, -2, "ChangeGrievanceScore");

    hks::setfield(L, hks::LUA_GLOBAL, "DiplomaticRelations");
    return 0;
}

void __cdecl Hook_RegisterScriptData(hks::lua_State* L) {
    std::cout << "Registering lua globals\n";

    hks::pushnamedcclosure(L, MemoryManipulation::LuaExport::lMem, 0, "lMem", 0);
    hks::setfield(L, hks::LUA_GLOBAL, "Mem");
    hks::pushnamedcclosure(L, MemoryManipulation::LuaExport::lObjMem, 0, "lObjMem", 0);
    hks::setfield(L, hks::LUA_GLOBAL, "ObjMem");

    CCallWithErrorHandling(L, RegisterDiplomaticRelations, NULL);

    base_RegisterScriptData(L);
}

static int lSetTokensToGive(hks::lua_State* L) {
    void* influence = IPlayerInfluence_GetInstance(L, 1, true);
    int tokens = hks::checkinteger(L, 2);

    Influence_SetTokensToGive(influence, tokens);
    return 0;
}

static int lSetPoints(hks::lua_State* L) {
    void* influence = IPlayerInfluence_GetInstance(L, 1, true);
    double points = hks::checknumber(L, 2);

    *(unsigned int*)((uintptr_t)influence + 0xb8) = static_cast<unsigned int>(std::round(points * 256.0));
    return 0;
}

static int lAdjustPoints(hks::lua_State* L) {
    void* influence = IPlayerInfluence_GetInstance(L, 1, true);
    double amountPoints = hks::checknumber(L, 2);

    *(unsigned int*)((uintptr_t)influence + 0xb8) += static_cast<unsigned int>(std::round(amountPoints * 256.0));
    return 0;
}

static void __cdecl Hook_Influence_PushMethods(hks::lua_State* L, int stackOffset) {
    std::cout << "Hooked Influence::PushMethods!\n";

    hks::pushnamedcclosure(L, lSetTokensToGive, 0, "lSetTokensToGive", 0);
    hks::setfield(L, stackOffset, "SetTokensToGive");
    hks::pushnamedcclosure(L, lSetPoints, 0, "lSetPoints", 0);
    hks::setfield(L, stackOffset, "SetPoints");
    hks::pushnamedcclosure(L, lAdjustPoints, 0, "lAdjustPoints", 0);
    hks::setfield(L, stackOffset, "AdjustPoints");

    base_Influence_PushMethods(L, stackOffset);
}

#pragma region Offsets
constexpr uintptr_t SET_APPEAL_OFFSET = 0x61270;
constexpr uintptr_t PLOT_PUSH_METHODS_OFFSET = 0x1b2e0;
constexpr uintptr_t REGISTER_SCRIPT_DATA_OFFSET = 0x5bdac0;
constexpr uintptr_t DIPLOMATIC_RELATIONS_CHANGE_GRIEVANCE_SCORE_OFFSET = 0x1cea40;
constexpr uintptr_t IMAP_PLOT_GET_INSTANCE_OFFSET = 0x15d60;
constexpr uintptr_t IPLAYER_CITIES_GET_INSTANCE_OFFSET = 0x6ee9b0;
constexpr uintptr_t DIPLOMATIC_RELATIONS_EDIT = 0x1d0220;
constexpr uintptr_t C_CALL_WITH_ERROR_HANDLING_OFFSET = 0x9ad880;
constexpr uintptr_t CITIES_ADD_GREAT_WORK_OFFSET = 0x2643b0;
constexpr uintptr_t CITIES_PUSH_METHODS_OFFSET = 0x6eeb10;
constexpr uintptr_t IPLAYER_INFLUENCE_GET_INSTANCE_OFFSET = 0x6f34f0;
constexpr uintptr_t SET_TOKENS_TO_GIVE_OFFSET = 0x2edaf0;
constexpr uintptr_t INFLUENCE_PUSH_METHODS_OFFSET = 0x6f3650;
#pragma endregion

static void InitHooks() {
    std::cout << "Initializing hooks!\n";
    using namespace Runtime;

    CCallWithErrorHandling = GetGameCoreFunctionAt<ProxyTypes::CCallWithErrorHandling>(C_CALL_WITH_ERROR_HANDLING_OFFSET);

    IPlayerCities_GetInstance = GetGameCoreFunctionAt<ProxyTypes::IPlayerCities_GetInstance>(IPLAYER_CITIES_GET_INSTANCE_OFFSET);
    IMapPlot_GetInstance = GetGameCoreFunctionAt<ProxyTypes::IMapPlot_GetInstance>(IMAP_PLOT_GET_INSTANCE_OFFSET);
    IPlayerInfluence_GetInstance = GetGameCoreFunctionAt<ProxyTypes::IPlayerInfluence_GetInstance>(IPLAYER_INFLUENCE_GET_INSTANCE_OFFSET);
    DiplomaticRelations_Edit = GetGameCoreFunctionAt<ProxyTypes::DiplomaticRelations_Edit>(DIPLOMATIC_RELATIONS_EDIT);

    Influence_SetTokensToGive = GetGameCoreFunctionAt<ProxyTypes::Influence_SetTokensToGive>(SET_TOKENS_TO_GIVE_OFFSET);
    Cities_AddGreatWork = GetGameCoreFunctionAt<ProxyTypes::Cities_AddGreatWork>(CITIES_ADD_GREAT_WORK_OFFSET);
    DiplomaticRelations_ChangeGrievanceScore = GetGameCoreFunctionAt
        <ProxyTypes::DiplomaticRelations_ChangeGrievanceScore>(DIPLOMATIC_RELATIONS_CHANGE_GRIEVANCE_SCORE_OFFSET);

    orig_RegisterScriptData = GetGameCoreFunctionAt<ProxyTypes::RegisterScriptData>(REGISTER_SCRIPT_DATA_OFFSET);
    CreateHook(orig_RegisterScriptData, &Hook_RegisterScriptData, &base_RegisterScriptData);

    lockedAppeals = {};
    orig_SetAppeal = GetGameCoreFunctionAt<ProxyTypes::SetAppeal>(SET_APPEAL_OFFSET);
    CreateHook(orig_SetAppeal, &Hook_SetAppeal, &base_SetAppeal);

    orig_Plot_PushMethods = GetGameCoreFunctionAt<ProxyTypes::PushMethods>(PLOT_PUSH_METHODS_OFFSET);
    CreateHook(orig_Plot_PushMethods, &Hook_Plot_PushMethods, &base_Plot_PushMethods);

    orig_Cities_PushMethods = GetGameCoreFunctionAt<ProxyTypes::PushMethods>(CITIES_PUSH_METHODS_OFFSET);
    CreateHook(orig_Cities_PushMethods, &Hook_Cities_PushMethods, &base_Cities_PushMethods);

    orig_Influence_PushMethods = GetGameCoreFunctionAt<ProxyTypes::PushMethods>(INFLUENCE_PUSH_METHODS_OFFSET);
    CreateHook(orig_Influence_PushMethods, &Hook_Influence_PushMethods, &base_Influence_PushMethods);
}

static void InitConsole() {
    if (!AllocConsole()) {
        MessageBoxW(NULL, L"Failed to create the console!", L"Error", MB_ICONERROR);
        return;
    }

    // Redirect standard input/output streams to the console
    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);

    std::cout << "Console initialized successfully." << std::endl;
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    Runtime::Create();
    base_DllCreateGameContext = (ProxyTypes::DllCreateGameContext)GetProcAddress(Runtime::GameCore, "DllCreateGameContext");
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        InitConsole();
        mainThread = CreateThread(0, 0, &MainThread, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
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
#pragma comment(linker,"/export:??4HksCompilerSettings@@QEAAAEAU0@$$QEAU0@@Z=proxy.??4HksCompilerSettings@@QEAAAEAU0@$$QEAU0@@Z,@1")
#pragma comment(linker,"/export:??4HksCompilerSettings@@QEAAAEAU0@AEBU0@@Z=proxy.??4HksCompilerSettings@@QEAAAEAU0@AEBU0@@Z,@2")
#pragma comment(linker,"/export:??4HksFixedHeapSettings@@QEAAAEAV0@$$QEAV0@@Z=proxy.??4HksFixedHeapSettings@@QEAAAEAV0@$$QEAV0@@Z,@3")
#pragma comment(linker,"/export:??4HksFixedHeapSettings@@QEAAAEAV0@AEBV0@@Z=proxy.??4HksFixedHeapSettings@@QEAAAEAV0@AEBV0@@Z,@4")
#pragma comment(linker,"/export:??4HksStateSettings@@QEAAAEAV0@$$QEAV0@@Z=proxy.??4HksStateSettings@@QEAAAEAV0@$$QEAV0@@Z,@5")
#pragma comment(linker,"/export:??4HksStateSettings@@QEAAAEAV0@AEBV0@@Z=proxy.??4HksStateSettings@@QEAAAEAV0@AEBV0@@Z,@6")
#pragma comment(linker,"/export:?_isHksGlobalMemoTestingMode@HksCompilerSettings@@QEBAHXZ=proxy.?_isHksGlobalMemoTestingMode@HksCompilerSettings@@QEBAHXZ,@7")
#pragma comment(linker,"/export:?_setHksGlobalMemoTestingMode@HksCompilerSettings@@QEAAXH@Z=proxy.?_setHksGlobalMemoTestingMode@HksCompilerSettings@@QEAAXH@Z,@8")
#pragma comment(linker,"/export:?getBytecodeSharingFormat@HksCompilerSettings@@QEBA?AW4BytecodeSharingFormat@1@XZ=proxy.?getBytecodeSharingFormat@HksCompilerSettings@@QEBA?AW4BytecodeSharingFormat@1@XZ,@9")
#pragma comment(linker,"/export:?getIntLiteralsEnabled@HksCompilerSettings@@QEBA?AW4IntLiteralOptions@1@XZ=proxy.?getIntLiteralsEnabled@HksCompilerSettings@@QEBA?AW4IntLiteralOptions@1@XZ,@10")
#pragma comment(linker,"/export:?getStrip@HksCompilerSettings@@QEBAPEAPEBDXZ=proxy.?getStrip@HksCompilerSettings@@QEBAPEAPEBDXZ,@11")
#pragma comment(linker,"/export:?hks_getcontext@@YAPEAXPEAUlua_State@@@Z=proxy.?hks_getcontext@@YAPEAXPEAUlua_State@@@Z,@12")
#pragma comment(linker,"/export:?hks_isstruct@@YAHPEAUlua_State@@H@Z=proxy.?hks_isstruct@@YAHPEAUlua_State@@H@Z,@13")
#pragma comment(linker,"/export:?hks_rawgetslot@@YAXPEAUlua_State@@HPEBDH@Z=proxy.?hks_rawgetslot@@YAXPEAUlua_State@@HPEBDH@Z,@14")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAF@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAF@Z,@15")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAG@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAG@Z,@16")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAH@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAH@Z,@17")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAI@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAI@Z,@18")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAJ@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAJ@Z,@19")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAK@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAK@Z,@20")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAM@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAM@Z,@21")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAN@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAN@Z,@22")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAPEBD@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAPEBD@Z,@23")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_J@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_J@Z,@24")
#pragma comment(linker,"/export:?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_K@Z=proxy.?hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_K@Z,@25")
#pragma comment(linker,"/export:?hks_rawsetslot@@YAXPEAUlua_State@@HPEBDH@Z=proxy.?hks_rawsetslot@@YAXPEAUlua_State@@HPEBDH@Z,@26")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH1@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH1@Z,@27")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHF@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHF@Z,@28")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHG@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHG@Z,@29")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHH@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHH@Z,@30")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHI@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHI@Z,@31")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHJ@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHJ@Z,@32")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHK@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHK@Z,@33")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHM@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHM@Z,@34")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHN@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHN@Z,@35")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHPEBX@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHPEBX@Z,@36")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_J@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_J@Z,@37")
#pragma comment(linker,"/export:?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_K@Z=proxy.?hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_K@Z,@38")
#pragma comment(linker,"/export:?hks_setcontext@@YAXPEAUlua_State@@PEAX@Z=proxy.?hks_setcontext@@YAXPEAUlua_State@@PEAX@Z,@39")
#pragma comment(linker,"/export:?hksi_hks_getcontext@@YAPEAXPEAUlua_State@@@Z=proxy.?hksi_hks_getcontext@@YAPEAXPEAUlua_State@@@Z,@40")
#pragma comment(linker,"/export:?hksi_hks_isstruct@@YAHPEAUlua_State@@H@Z=proxy.?hksi_hks_isstruct@@YAHPEAUlua_State@@H@Z,@41")
#pragma comment(linker,"/export:?hksi_hks_rawgetslot@@YAXPEAUlua_State@@HPEBDH@Z=proxy.?hksi_hks_rawgetslot@@YAXPEAUlua_State@@HPEBDH@Z,@42")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAF@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAF@Z,@43")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAG@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAG@Z,@44")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAH@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAH@Z,@45")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAI@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAI@Z,@46")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAJ@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAJ@Z,@47")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAK@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAK@Z,@48")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAM@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAM@Z,@49")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAN@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAN@Z,@50")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAPEBD@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEAPEBD@Z,@51")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_J@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_J@Z,@52")
#pragma comment(linker,"/export:?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_K@Z=proxy.?hksi_hks_rawgetslotv@@YAHPEAUlua_State@@HPEBDHAEA_K@Z,@53")
#pragma comment(linker,"/export:?hksi_hks_rawsetslot@@YAXPEAUlua_State@@HPEBDH@Z=proxy.?hksi_hks_rawsetslot@@YAXPEAUlua_State@@HPEBDH@Z,@54")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHF@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHF@Z,@55")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHG@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHG@Z,@56")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHI@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHI@Z,@57")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHJ@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHJ@Z,@58")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHK@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHK@Z,@59")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHM@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHM@Z,@60")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHN@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDHN@Z,@61")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_J@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_J@Z,@62")
#pragma comment(linker,"/export:?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_K@Z=proxy.?hksi_hks_rawsetslotv@@YAXPEAUlua_State@@HPEBDH_K@Z,@63")
#pragma comment(linker,"/export:?hksi_hks_setcontext@@YAXPEAUlua_State@@PEAX@Z=proxy.?hksi_hks_setcontext@@YAXPEAUlua_State@@PEAX@Z,@64")
#pragma comment(linker,"/export:?hksi_lua_equal@@YAHPEAUlua_State@@HH@Z=proxy.?hksi_lua_equal@@YAHPEAUlua_State@@HH@Z,@65")
#pragma comment(linker,"/export:?hksi_lua_getfenv@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_getfenv@@YAXPEAUlua_State@@H@Z,@66")
#pragma comment(linker,"/export:?hksi_lua_getfield@@YAXPEAUlua_State@@HPEBD@Z=proxy.?hksi_lua_getfield@@YAXPEAUlua_State@@HPEBD@Z,@67")
#pragma comment(linker,"/export:?hksi_lua_getmetatable@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_getmetatable@@YAHPEAUlua_State@@H@Z,@68")
#pragma comment(linker,"/export:?hksi_lua_gettable@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_gettable@@YAXPEAUlua_State@@H@Z,@69")
#pragma comment(linker,"/export:?hksi_lua_gettop@@YAHPEAUlua_State@@@Z=proxy.?hksi_lua_gettop@@YAHPEAUlua_State@@@Z,@70")
#pragma comment(linker,"/export:?hksi_lua_insert@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_insert@@YAXPEAUlua_State@@H@Z,@71")
#pragma comment(linker,"/export:?hksi_lua_iscfunction@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_iscfunction@@YAHPEAUlua_State@@H@Z,@72")
#pragma comment(linker,"/export:?hksi_lua_isnumber@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_isnumber@@YAHPEAUlua_State@@H@Z,@73")
#pragma comment(linker,"/export:?hksi_lua_isstring@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_isstring@@YAHPEAUlua_State@@H@Z,@74")
#pragma comment(linker,"/export:?hksi_lua_isuserdata@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_isuserdata@@YAHPEAUlua_State@@H@Z,@75")
#pragma comment(linker,"/export:?hksi_lua_lessthan@@YAHPEAUlua_State@@HH@Z=proxy.?hksi_lua_lessthan@@YAHPEAUlua_State@@HH@Z,@76")
#pragma comment(linker,"/export:?hksi_lua_next@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_next@@YAHPEAUlua_State@@H@Z,@77")
#pragma comment(linker,"/export:?hksi_lua_objlen@@YA_KPEAUlua_State@@H@Z=proxy.?hksi_lua_objlen@@YA_KPEAUlua_State@@H@Z,@78")
#pragma comment(linker,"/export:?hksi_lua_pushinteger@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_pushinteger@@YAXPEAUlua_State@@H@Z,@79")
#pragma comment(linker,"/export:?hksi_lua_pushlightuserdata@@YAXPEAUlua_State@@PEAX@Z=proxy.?hksi_lua_pushlightuserdata@@YAXPEAUlua_State@@PEAX@Z,@80")
#pragma comment(linker,"/export:?hksi_lua_pushnil@@YAXPEAUlua_State@@@Z=proxy.?hksi_lua_pushnil@@YAXPEAUlua_State@@@Z,@81")
#pragma comment(linker,"/export:?hksi_lua_pushnumber@@YAXPEAUlua_State@@N@Z=proxy.?hksi_lua_pushnumber@@YAXPEAUlua_State@@N@Z,@82")
#pragma comment(linker,"/export:?hksi_lua_pushthread@@YAHPEAUlua_State@@@Z=proxy.?hksi_lua_pushthread@@YAHPEAUlua_State@@@Z,@83")
#pragma comment(linker,"/export:?hksi_lua_pushvalue@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_pushvalue@@YAXPEAUlua_State@@H@Z,@84")
#pragma comment(linker,"/export:?hksi_lua_rawequal@@YAHPEAUlua_State@@HH@Z=proxy.?hksi_lua_rawequal@@YAHPEAUlua_State@@HH@Z,@85")
#pragma comment(linker,"/export:?hksi_lua_rawget@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_rawget@@YAXPEAUlua_State@@H@Z,@86")
#pragma comment(linker,"/export:?hksi_lua_rawget_array_lud@@YAPEAXPEAUlua_State@@HH@Z=proxy.?hksi_lua_rawget_array_lud@@YAPEAXPEAUlua_State@@HH@Z,@87")
#pragma comment(linker,"/export:?hksi_lua_rawgeti@@YAXPEAUlua_State@@HH@Z=proxy.?hksi_lua_rawgeti@@YAXPEAUlua_State@@HH@Z,@88")
#pragma comment(linker,"/export:?hksi_lua_rawlength@@YA_KPEAUlua_State@@H@Z=proxy.?hksi_lua_rawlength@@YA_KPEAUlua_State@@H@Z,@89")
#pragma comment(linker,"/export:?hksi_lua_rawset@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_rawset@@YAXPEAUlua_State@@H@Z,@90")
#pragma comment(linker,"/export:?hksi_lua_rawset_array@@YAXPEAUlua_State@@HH@Z=proxy.?hksi_lua_rawset_array@@YAXPEAUlua_State@@HH@Z,@91")
#pragma comment(linker,"/export:?hksi_lua_rawset_array_lud@@YAXPEAUlua_State@@HHPEAX@Z=proxy.?hksi_lua_rawset_array_lud@@YAXPEAUlua_State@@HHPEAX@Z,@92")
#pragma comment(linker,"/export:?hksi_lua_rawseti@@YAXPEAUlua_State@@HH@Z=proxy.?hksi_lua_rawseti@@YAXPEAUlua_State@@HH@Z,@93")
#pragma comment(linker,"/export:?hksi_lua_remove@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_remove@@YAXPEAUlua_State@@H@Z,@94")
#pragma comment(linker,"/export:?hksi_lua_replace@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_replace@@YAXPEAUlua_State@@H@Z,@95")
#pragma comment(linker,"/export:?hksi_lua_setfenv@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_setfenv@@YAHPEAUlua_State@@H@Z,@96")
#pragma comment(linker,"/export:?hksi_lua_setfield@@YAXPEAUlua_State@@HPEBD@Z=proxy.?hksi_lua_setfield@@YAXPEAUlua_State@@HPEBD@Z,@97")
#pragma comment(linker,"/export:?hksi_lua_setmetatable@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_setmetatable@@YAHPEAUlua_State@@H@Z,@98")
#pragma comment(linker,"/export:?hksi_lua_settop@@YAXPEAUlua_State@@H@Z=proxy.?hksi_lua_settop@@YAXPEAUlua_State@@H@Z,@99")
#pragma comment(linker,"/export:?hksi_lua_toboolean@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_toboolean@@YAHPEAUlua_State@@H@Z,@100")
#pragma comment(linker,"/export:?hksi_lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z=proxy.?hksi_lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z,@101")
#pragma comment(linker,"/export:?hksi_lua_tointeger@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_tointeger@@YAHPEAUlua_State@@H@Z,@102")
#pragma comment(linker,"/export:?hksi_lua_tolstring@@YAPEBDPEAUlua_State@@HPEA_K@Z=proxy.?hksi_lua_tolstring@@YAPEBDPEAUlua_State@@HPEA_K@Z,@103")
#pragma comment(linker,"/export:?hksi_lua_tonumber@@YANPEAUlua_State@@H@Z=proxy.?hksi_lua_tonumber@@YANPEAUlua_State@@H@Z,@104")
#pragma comment(linker,"/export:?hksi_lua_topointer@@YAPEBXPEAUlua_State@@H@Z=proxy.?hksi_lua_topointer@@YAPEBXPEAUlua_State@@H@Z,@105")
#pragma comment(linker,"/export:?hksi_lua_tothread@@YAPEAUlua_State@@PEAU1@H@Z=proxy.?hksi_lua_tothread@@YAPEAUlua_State@@PEAU1@H@Z,@106")
#pragma comment(linker,"/export:?hksi_lua_touserdata@@YAPEAXPEAUlua_State@@H@Z=proxy.?hksi_lua_touserdata@@YAPEAXPEAUlua_State@@H@Z,@107")
#pragma comment(linker,"/export:?hksi_lua_type@@YAHPEAUlua_State@@H@Z=proxy.?hksi_lua_type@@YAHPEAUlua_State@@H@Z,@108")
#pragma comment(linker,"/export:?isEmitStruct@HksCompilerSettings@@QEBAHXZ=proxy.?isEmitStruct@HksCompilerSettings@@QEBAHXZ,@109")
#pragma comment(linker,"/export:?isGlobalMemoization@HksCompilerSettings@@QEBAHXZ=proxy.?isGlobalMemoization@HksCompilerSettings@@QEBAHXZ,@110")
#pragma comment(linker,"/export:?lua_equal@@YAHPEAUlua_State@@HH@Z=proxy.?lua_equal@@YAHPEAUlua_State@@HH@Z,@111")
#pragma comment(linker,"/export:?lua_getfenv@@YAXPEAUlua_State@@H@Z=proxy.?lua_getfenv@@YAXPEAUlua_State@@H@Z,@112")
#pragma comment(linker,"/export:?lua_getfield@@YAXPEAUlua_State@@HPEBD@Z=proxy.?lua_getfield@@YAXPEAUlua_State@@HPEBD@Z,@113")
#pragma comment(linker,"/export:?lua_getmetatable@@YAHPEAUlua_State@@H@Z=proxy.?lua_getmetatable@@YAHPEAUlua_State@@H@Z,@114")
#pragma comment(linker,"/export:?lua_gettable@@YAXPEAUlua_State@@H@Z=proxy.?lua_gettable@@YAXPEAUlua_State@@H@Z,@115")
#pragma comment(linker,"/export:?lua_gettop@@YAHPEAUlua_State@@@Z=proxy.?lua_gettop@@YAHPEAUlua_State@@@Z,@116")
#pragma comment(linker,"/export:?lua_insert@@YAXPEAUlua_State@@H@Z=proxy.?lua_insert@@YAXPEAUlua_State@@H@Z,@117")
#pragma comment(linker,"/export:?lua_iscfunction@@YAHPEAUlua_State@@H@Z=proxy.?lua_iscfunction@@YAHPEAUlua_State@@H@Z,@118")
#pragma comment(linker,"/export:?lua_isnumber@@YAHPEAUlua_State@@H@Z=proxy.?lua_isnumber@@YAHPEAUlua_State@@H@Z,@119")
#pragma comment(linker,"/export:?lua_isstring@@YAHPEAUlua_State@@H@Z=proxy.?lua_isstring@@YAHPEAUlua_State@@H@Z,@120")
#pragma comment(linker,"/export:?lua_isuserdata@@YAHPEAUlua_State@@H@Z=proxy.?lua_isuserdata@@YAHPEAUlua_State@@H@Z,@121")
#pragma comment(linker,"/export:?lua_lessthan@@YAHPEAUlua_State@@HH@Z=proxy.?lua_lessthan@@YAHPEAUlua_State@@HH@Z,@122")
#pragma comment(linker,"/export:?lua_next@@YAHPEAUlua_State@@H@Z=proxy.?lua_next@@YAHPEAUlua_State@@H@Z,@123")
#pragma comment(linker,"/export:?lua_objlen@@YA_KPEAUlua_State@@H@Z=proxy.?lua_objlen@@YA_KPEAUlua_State@@H@Z,@124")
#pragma comment(linker,"/export:?lua_pushinteger@@YAXPEAUlua_State@@H@Z=proxy.?lua_pushinteger@@YAXPEAUlua_State@@H@Z,@125")
#pragma comment(linker,"/export:?lua_pushlightuserdata@@YAXPEAUlua_State@@PEAX@Z=proxy.?lua_pushlightuserdata@@YAXPEAUlua_State@@PEAX@Z,@126")
#pragma comment(linker,"/export:?lua_pushnil@@YAXPEAUlua_State@@@Z=proxy.?lua_pushnil@@YAXPEAUlua_State@@@Z,@127")
#pragma comment(linker,"/export:?lua_pushnumber@@YAXPEAUlua_State@@N@Z=proxy.?lua_pushnumber@@YAXPEAUlua_State@@N@Z,@128")
#pragma comment(linker,"/export:?lua_pushthread@@YAHPEAUlua_State@@@Z=proxy.?lua_pushthread@@YAHPEAUlua_State@@@Z,@129")
#pragma comment(linker,"/export:?lua_pushvalue@@YAXPEAUlua_State@@H@Z=proxy.?lua_pushvalue@@YAXPEAUlua_State@@H@Z,@130")
#pragma comment(linker,"/export:?lua_rawequal@@YAHPEAUlua_State@@HH@Z=proxy.?lua_rawequal@@YAHPEAUlua_State@@HH@Z,@131")
#pragma comment(linker,"/export:?lua_rawget@@YAXPEAUlua_State@@H@Z=proxy.?lua_rawget@@YAXPEAUlua_State@@H@Z,@132")
#pragma comment(linker,"/export:?lua_rawgeti@@YAXPEAUlua_State@@HH@Z=proxy.?lua_rawgeti@@YAXPEAUlua_State@@HH@Z,@133")
#pragma comment(linker,"/export:?lua_rawset@@YAXPEAUlua_State@@H@Z=proxy.?lua_rawset@@YAXPEAUlua_State@@H@Z,@134")
#pragma comment(linker,"/export:?lua_rawseti@@YAXPEAUlua_State@@HH@Z=proxy.?lua_rawseti@@YAXPEAUlua_State@@HH@Z,@135")
#pragma comment(linker,"/export:?lua_remove@@YAXPEAUlua_State@@H@Z=proxy.?lua_remove@@YAXPEAUlua_State@@H@Z,@136")
#pragma comment(linker,"/export:?lua_replace@@YAXPEAUlua_State@@H@Z=proxy.?lua_replace@@YAXPEAUlua_State@@H@Z,@137")
#pragma comment(linker,"/export:?lua_setfenv@@YAHPEAUlua_State@@H@Z=proxy.?lua_setfenv@@YAHPEAUlua_State@@H@Z,@138")
#pragma comment(linker,"/export:?lua_setfield@@YAXPEAUlua_State@@HPEBD@Z=proxy.?lua_setfield@@YAXPEAUlua_State@@HPEBD@Z,@139")
#pragma comment(linker,"/export:?lua_setmetatable@@YAHPEAUlua_State@@H@Z=proxy.?lua_setmetatable@@YAHPEAUlua_State@@H@Z,@140")
#pragma comment(linker,"/export:?lua_settop@@YAXPEAUlua_State@@H@Z=proxy.?lua_settop@@YAXPEAUlua_State@@H@Z,@141")
#pragma comment(linker,"/export:?lua_toboolean@@YA_NPEAUlua_State@@H@Z=proxy.?lua_toboolean@@YA_NPEAUlua_State@@H@Z,@142")
#pragma comment(linker,"/export:?lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z=proxy.?lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z,@143")
#pragma comment(linker,"/export:?lua_tointeger@@YAHPEAUlua_State@@H@Z=proxy.?lua_tointeger@@YAHPEAUlua_State@@H@Z,@144")
#pragma comment(linker,"/export:?lua_tolstring@@YAPEBDPEAUlua_State@@HPEA_K@Z=proxy.?lua_tolstring@@YAPEBDPEAUlua_State@@HPEA_K@Z,@145")
#pragma comment(linker,"/export:?lua_tonumber@@YANPEAUlua_State@@H@Z=proxy.?lua_tonumber@@YANPEAUlua_State@@H@Z,@146")
#pragma comment(linker,"/export:?lua_topointer@@YAPEBXPEAUlua_State@@H@Z=proxy.?lua_topointer@@YAPEBXPEAUlua_State@@H@Z,@147")
#pragma comment(linker,"/export:?lua_tothread@@YAPEAUlua_State@@PEAU1@H@Z=proxy.?lua_tothread@@YAPEAUlua_State@@PEAU1@H@Z,@148")
#pragma comment(linker,"/export:?lua_touserdata@@YAPEAXPEAUlua_State@@H@Z=proxy.?lua_touserdata@@YAPEAXPEAUlua_State@@H@Z,@149")
#pragma comment(linker,"/export:?lua_type@@YAHPEAUlua_State@@H@Z=proxy.?lua_type@@YAHPEAUlua_State@@H@Z,@150")
#pragma comment(linker,"/export:?setBytecodeSharingFormat@HksCompilerSettings@@QEAAXW4BytecodeSharingFormat@1@@Z=proxy.?setBytecodeSharingFormat@HksCompilerSettings@@QEAAXW4BytecodeSharingFormat@1@@Z,@151")
#pragma comment(linker,"/export:?setEmitStruct@HksCompilerSettings@@QEAAXH@Z=proxy.?setEmitStruct@HksCompilerSettings@@QEAAXH@Z,@152")
#pragma comment(linker,"/export:?setGlobalMemoization@HksCompilerSettings@@QEAAXH@Z=proxy.?setGlobalMemoization@HksCompilerSettings@@QEAAXH@Z,@153")
#pragma comment(linker,"/export:?setIntLiteralsEnabled@HksCompilerSettings@@QEAAXW4IntLiteralOptions@1@@Z=proxy.?setIntLiteralsEnabled@HksCompilerSettings@@QEAAXW4IntLiteralOptions@1@@Z,@154")
// #pragma comment(linker,"/export:DllCreateGameContext=proxy.DllCreateGameContext,@155")
#pragma comment(linker,"/export:DllDestroyGameContext=proxy.DllDestroyGameContext,@156")
#pragma comment(linker,"/export:EXP_GetTelemetrySessionHash=proxy.EXP_GetTelemetrySessionHash,@157")
#pragma endregion
