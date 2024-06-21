#include <windows.h>
#include <iostream>
#include <MinHook.h>
#include <set>
#include <cstring>
#pragma comment(lib, "libMinHook.x64.lib")

constexpr int LUA_GLOBAL = -10002;

HMODULE legitDll;
uintptr_t baseAddress;
HMODULE hksDll;
HANDLE mainThread;

typedef struct lua_State lua_State;
typedef class Instance Instance;

typedef void* (__cdecl* DllCreateGameContextType)(void);
DllCreateGameContextType base_DllCreateGameContext;
typedef void (__cdecl* DllDestroyGameContextType)(void*);
DllDestroyGameContextType base_DllDestroyGameContext;
typedef void* (__cdecl* EXP_GetTelementarySessionHashType)(void);
EXP_GetTelementarySessionHashType base_EXP_GetTelementarySessionHash;

typedef void (__cdecl* PushMethodsType)(lua_State* __ptr64, int);
PushMethodsType Base_Plot_PushMethods;
PushMethodsType orig_Plot_PushMethods;

typedef void (__cdecl* hks_pushnamedcclosureType)(lua_State* __ptr64, int(__cdecl*)(lua_State* __ptr64), int, const char* __ptr64, int);
hks_pushnamedcclosureType hks_pushnamedcclosure;
typedef int (__cdecl* luaL_checkintegerType)(lua_State* __ptr64, int);
luaL_checkintegerType luaL_checkinteger;
typedef double (__cdecl* luaL_checknumberType)(struct lua_State* __ptr64, int);
luaL_checknumberType luaL_checknumber;
typedef void (__cdecl* hksi_lua_setfieldType)(lua_State* __ptr64, int, const char* __ptr64);
hksi_lua_setfieldType hksi_lua_setfield;
typedef int (__thiscall* GetTopType)(lua_State* __ptr64);
GetTopType GetTop;
typedef int (__thiscall* DoStringType)(lua_State* __ptr64, const char* __ptr64);
DoStringType DoString;
typedef int (__cdecl* hksi_lua_tobooleanType)(struct lua_State* __ptr64, int);
hksi_lua_tobooleanType hksi_lua_toboolean;
typedef void (__cdecl* hksi_lua_pushnumberType)(struct lua_State* __ptr64, double);
hksi_lua_pushnumberType hksi_lua_pushnumber;
typedef void (__cdecl* hksi_lua_pushintegerType)(struct lua_State* __ptr64, int);
hksi_lua_pushintegerType hksi_lua_pushinteger;
typedef void (__cdecl* hksi_luaL_errorType)(struct lua_State* __ptr64, char const* __ptr64, ...);
hksi_luaL_errorType hksi_luaL_error;
typedef char const* __ptr64 (__cdecl* hksi_lua_pushfstringType)(struct lua_State* __ptr64, char const* __ptr64, ...);
hksi_lua_pushfstringType hksi_lua_pushfstring;
typedef char const* __ptr64 (__cdecl* CheckLStringType)(struct lua_State*, int, unsigned __int64* __ptr64);
CheckLStringType CheckLString;
typedef void* __ptr64 (__cdecl* hksi_lua_touserdataType)(struct lua_State* __ptr64, int);
hksi_lua_touserdataType hksi_lua_touserdata;
typedef void (__cdecl* hksi_lua_getfieldType)(struct lua_State* __ptr64, int, char const* __ptr64);
hksi_lua_getfieldType hksi_lua_getfield;
typedef void(__thiscall* PopType)(lua_State*, int);
PopType Pop;

typedef void* (__cdecl* IMapPlot_GetInstanceType)(lua_State* __ptr64, int, bool);
IMapPlot_GetInstanceType IMapPlot_GetInstance;
typedef void(__cdecl* DiplomaticRelations_ChangeGrievanceScoreType)(void* __ptr64 diplomaticRelations, int player1Id, int player2Id, int amount);
DiplomaticRelations_ChangeGrievanceScoreType DiplomaticRelations_ChangeGrievanceScore;

typedef void (__thiscall* SetAppealType)(void* __ptr64 plot, int appeal);
SetAppealType Base_SetAppeal;
SetAppealType orig_SetAppeal;

typedef void (__cdecl* RegisterScriptDataType)(struct lua_State* __ptr64);
RegisterScriptDataType base_RegisterScriptData;
RegisterScriptDataType orig_RegisterScriptData;

std::set<short*> lockedAppeals;

enum MemoryType {
    FIELD_BYTE,
    FIELD_SHORT,
    FIELD_UNSIGNED_SHORT,
    FIELD_INT,
    FIELD_UNSIGNED_INT,
    FIELD_LONG_LONG_INT,
    FIELD_UNSIGNED_LONG_LONG_INT,
    FIELD_CHAR,
    FIELD_FLOAT,
    FIELD_DOUBLE,
    FIELD_C_STRING,
    FIELD_BOOL
};

static int PushCValue(lua_State* L, MemoryType memoryType, uintptr_t address) {
    switch (memoryType) {
    case FIELD_BYTE: hksi_lua_pushinteger(L, *(byte*)address); break;
    case FIELD_SHORT: hksi_lua_pushinteger(L, *(short*)address); break;
    case FIELD_INT: hksi_lua_pushinteger(L, *(int*)address); break;
    case FIELD_LONG_LONG_INT: hksi_lua_pushinteger(L, *(long long int*)address); break;
    case FIELD_UNSIGNED_LONG_LONG_INT: hksi_lua_pushinteger(L, *(unsigned long long int*)address); break;
    case FIELD_CHAR: hksi_lua_pushinteger(L, *(char*)address); break;
    case FIELD_FLOAT: hksi_lua_pushnumber(L, *(float*)address); break;
    case FIELD_DOUBLE: hksi_lua_pushnumber(L, *(double*)address); break;
    case FIELD_C_STRING: hksi_lua_pushfstring(L, *(char**)address); break;
    default: hksi_luaL_error(L, "Invalid MemoryType parameter was passed!"); return 0;
    }
    return 1;
}

static void SetCValue(lua_State* L, MemoryType memoryType, uintptr_t address, int index) {
    switch (memoryType) {
    case FIELD_BYTE: *(byte*)address = static_cast<byte>(luaL_checkinteger(L, index)); break;
    case FIELD_SHORT: *(short*)address = static_cast<short>(luaL_checkinteger(L, index)); break;
    case FIELD_INT: *(int*)address = static_cast<int>(luaL_checkinteger(L, index)); break;
    case FIELD_LONG_LONG_INT: *(long long int*)address = static_cast<long long int>(luaL_checkinteger(L, index)); break;
    case FIELD_UNSIGNED_LONG_LONG_INT: *(unsigned long long int*)address = static_cast<unsigned long long int>(luaL_checkinteger(L, index)); break;
    case FIELD_CHAR: *(char*)address = static_cast<char>(luaL_checkinteger(L, index)); break;
    case FIELD_FLOAT: *(float*)address = static_cast<float>(luaL_checkinteger(L, index)); break;
    case FIELD_DOUBLE: *(double*)address = static_cast<double>(luaL_checkinteger(L, index)); break;
    case FIELD_C_STRING: {
        size_t length;
        const char* inputString = CheckLString(L, index, &length);
        char* newString = (char*)malloc(length + 1);
        if (!newString) {
            hksi_luaL_error(L, "String memory allocation failed");
            return;
        }
        strcpy_s(newString, length, inputString);
        *(char**)address = newString;
        break;
    }
    default: hksi_luaL_error(L, "Invalid MemoryType parameter was passed!");
    }
}

void __cdecl Hook_SetAppeal(void* __ptr64 plot, int appeal) {
    std::cout << "Hooked SetAppeal!\n";
    if (lockedAppeals.find((short*)((uintptr_t)plot + 0x4a)) == lockedAppeals.end()) {
        Base_SetAppeal(plot, appeal);
    }
}

static int __cdecl lMem(lua_State* __ptr64 L) {
    uintptr_t address = static_cast<uintptr_t>(luaL_checknumber(L, 1)); // Check for number because int too small to store x64 pointer
    auto memoryType = static_cast<MemoryType>(luaL_checkinteger(L, 2));
    if (GetTop(L) == 3) {
        SetCValue(L, memoryType, baseAddress + address, 3);
        return 0;
    }
    return PushCValue(L, memoryType, baseAddress + address);
}

static int __cdecl lObjMem(lua_State* __ptr64 L) {
    hksi_lua_getfield(L, 1, "__instance");
    auto objectAddress = reinterpret_cast<uintptr_t>(hksi_lua_touserdata(L, -1));
    if (objectAddress == NULL) {
        hksi_luaL_error(L, "Failed to retrieve __instance field as userdata. Is your object NULL?");
        return 0;
    }
    Pop(L, 1);
    uintptr_t offsetAddress = static_cast<uintptr_t>(luaL_checkinteger(L, 2));
    auto memoryType = static_cast<MemoryType>(luaL_checkinteger(L, 3));
    if (GetTop(L) == 4) {
        SetCValue(L, memoryType, objectAddress + offsetAddress, 4);
        return 0;
    }
    return PushCValue(L, memoryType, objectAddress + offsetAddress);
}

static int __cdecl lSetAppeal(lua_State* __ptr64 L) {
    void* plot = IMapPlot_GetInstance(L, 1, true);
    int appeal = luaL_checkinteger(L, 2);
    std::cout << plot << ' ' << appeal << '\n';
    Hook_SetAppeal(plot, appeal);
    return 0;
}

static int __cdecl lLockAppeal(lua_State* __ptr64 L) {
    void* plot = IMapPlot_GetInstance(L, 1, true);
    bool setToLock = hksi_lua_toboolean(L, 2);
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

static void __cdecl Hook_Plot_PushMethods(lua_State* __ptr64 L, int stackOffset) {
    std::cout << "Hooked pushmethods!!\n";

    hks_pushnamedcclosure(L, lSetAppeal, 0, "lSetAppeal", 0);
    hksi_lua_setfield(L, stackOffset, "SetAppeal");
    hks_pushnamedcclosure(L, lLockAppeal, 0, "lLockAppeal", 0);
    hksi_lua_setfield(L, stackOffset, "LockAppeal");

    DoString(L, "print(StateName)");

    Base_Plot_PushMethods(L, stackOffset);
}

void __cdecl Hook_RegisterScriptData(lua_State* L) {
    std::cout << "Registering Memory Functions\n";
    hks_pushnamedcclosure(L, lMem, 0, "lMem", 0);
    hksi_lua_setfield(L, LUA_GLOBAL, "Mem");
    hks_pushnamedcclosure(L, lObjMem, 0, "lObjMem", 0);
    hksi_lua_setfield(L, LUA_GLOBAL, "ObjMem");
    base_RegisterScriptData(L);
}

template <typename T>
inline MH_STATUS MH_CreateHookEx(LPVOID pTarget, LPVOID pDetour, T** ppOriginal) {
    return MH_CreateHook(pTarget, pDetour, reinterpret_cast<LPVOID*>(ppOriginal));
}

static void InitHavokScript() {
    if (MH_Initialize() != MH_OK) {
        std::cout << "MH failed to init\n";
    }

    hksDll = GetModuleHandle(TEXT("HavokScript_FinalRelease.dll"));
    if (hksDll == NULL) {
        std::cout << "Failed to load HavokScript! " << GetLastError() << '\n';
        return;
    }

    hks_pushnamedcclosure = (hks_pushnamedcclosureType)GetProcAddress(hksDll, "?hks_pushnamedcclosure@@YAXPEAUlua_State@@P6AH0@ZHPEBDH@Z");
    hksi_lua_setfield = (hksi_lua_setfieldType)GetProcAddress(hksDll, "?hksi_lua_setfield@@YAXPEAUlua_State@@HPEBD@Z");
    luaL_checkinteger = (luaL_checkintegerType)GetProcAddress(hksDll, "?luaL_checkinteger@@YAHPEAUlua_State@@H@Z");
    GetTop = (GetTopType)GetProcAddress(hksDll, "?GetTop@LuaState@LuaPlus@@QEBAHXZ");
    DoString = (DoStringType)GetProcAddress(hksDll, "?DoString@LuaState@LuaPlus@@QEAAHPEBD@Z");
    hksi_lua_toboolean = (hksi_lua_tobooleanType)GetProcAddress(hksDll, "?hksi_lua_toboolean@@YAHPEAUlua_State@@H@Z");
    hksi_lua_pushinteger = (hksi_lua_pushintegerType)GetProcAddress(hksDll, "?hksi_lua_pushinteger@@YAXPEAUlua_State@@H@Z");
    hksi_lua_pushnumber = (hksi_lua_pushnumberType)GetProcAddress(hksDll, "?hksi_lua_pushnumber@@YAXPEAUlua_State@@N@Z");
    hksi_luaL_error = (hksi_luaL_errorType)GetProcAddress(hksDll, "?hksi_luaL_error@@YAHPEAUlua_State@@PEBDZZ");
    luaL_checknumber = (luaL_checknumberType)GetProcAddress(hksDll, "?luaL_checknumber@@YANPEAUlua_State@@H@Z");
    hksi_lua_pushfstring = (hksi_lua_pushfstringType)GetProcAddress(hksDll, "?hksi_lua_pushfstring@@YAPEBDPEAUlua_State@@PEBDZZ");
    CheckLString = (CheckLStringType)GetProcAddress(hksDll, "");
    Pop = (PopType)GetProcAddress(hksDll, "?Pop@LuaState@LuaPlus@@QEAAXH@Z");
    hksi_lua_touserdata = (hksi_lua_touserdataType)GetProcAddress(hksDll, "?hksi_lua_touserdata@@YAPEAXPEAUlua_State@@H@Z");
    hksi_lua_getfield = (hksi_lua_getfieldType)GetProcAddress(hksDll, "?hksi_lua_getfield@@YAXPEAUlua_State@@HPEBD@Z");

    IMapPlot_GetInstance = reinterpret_cast<IMapPlot_GetInstanceType>(baseAddress + 0x15d60);
    DiplomaticRelations_ChangeGrievanceScore = reinterpret_cast<DiplomaticRelations_ChangeGrievanceScoreType>(baseAddress + 0x1cea40);

    orig_RegisterScriptData = reinterpret_cast<RegisterScriptDataType>(baseAddress + 0x5bdac0);
    if (MH_CreateHookEx(orig_RegisterScriptData, &Hook_RegisterScriptData, &base_RegisterScriptData)) {
        std::cout << "Failed to hook RegisterScriptData\n";
    }

    lockedAppeals = {};
    orig_SetAppeal = reinterpret_cast<SetAppealType>(baseAddress + 0x61270);
    if (MH_CreateHook(reinterpret_cast<void**>(orig_SetAppeal), &Hook_SetAppeal, reinterpret_cast<void**>(&Base_SetAppeal)) != MH_OK) {
        std::cout << "Failed to hook Plot::SetAppeal\n";
    }

    orig_Plot_PushMethods = reinterpret_cast<PushMethodsType>(baseAddress + 0x1b2e0);
    if (MH_CreateHook(reinterpret_cast<void**>(orig_Plot_PushMethods), &Hook_Plot_PushMethods, reinterpret_cast<void**>(&Base_Plot_PushMethods)) != MH_OK) {
        std::cout << "Failed to hook Plot::PushMethods\n";
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        std::cout << "Failed to enable hooks\n";
    }
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

static void Uninitialize(void) {
    if (legitDll) {
        MH_DisableHook(MH_ALL_HOOKS);
        MH_Uninitialize();
        FreeLibrary(legitDll);
        legitDll = NULL;
    }
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    std::cout << "Hello world\n";
    legitDll = LoadLibraryW(L"proxy.dll");
    if (!legitDll) {
        std::cout << "Legitimate DLL failed to load!\n";
        return FALSE;
    }
    base_DllCreateGameContext = (DllCreateGameContextType)GetProcAddress(legitDll, "DllCreateGameContext");
    baseAddress = reinterpret_cast<uintptr_t>(legitDll);
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
        Uninitialize();
        break;
    }
    return TRUE;
}

extern "C" {
    __declspec(dllexport) void* __cdecl DllCreateGameContext(void) {
        std::cout << "Waiting for main thread " << base_DllCreateGameContext << '\n';
        DWORD result = WaitForSingleObject(mainThread, INFINITE);
        if (result == WAIT_OBJECT_0) {
            std::cout << "Main thread completed successfully.\n";
        }
        else {
            std::cerr << "WaitForSingleObject failed: " << GetLastError() << '\n';
        }
        InitHavokScript();
        // CloseHandle(mainThread);
        return base_DllCreateGameContext();
    }

    // __declspec(dllexport) void DllDestroyGameContext(void* unknown) {
    //     std::cout << "2 " << unknown << ' ' << base_DllDestroyGameContext << '\n';
    //     base_DllDestroyGameContext(unknown);
    // }

    // __declspec(dllexport) void* EXP_GetTelementarySessionHash(void) {
    //     std::cout << "3 " << base_EXP_GetTelementarySessionHash << '\n';
    //     return base_EXP_GetTelementarySessionHash();
    // }
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
