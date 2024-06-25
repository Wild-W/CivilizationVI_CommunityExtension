#include <iostream>
#include <Windows.h>

namespace hks {
	constexpr int LUA_GLOBAL = -10002;
	
	typedef struct lua_State lua_State;

	typedef int(__cdecl* luaFunc)(lua_State*);

	typedef void(__cdecl* hks_pushnamedcclosureType)(lua_State*, luaFunc, int, const char*, int);
	hks_pushnamedcclosureType pushnamedcclosure;
	typedef int(__cdecl* luaL_checkintegerType)(lua_State*, int);
	luaL_checkintegerType checkinteger;
	typedef double(__cdecl* luaL_checknumberType)(lua_State*, int);
	luaL_checknumberType checknumber;
	typedef void(__cdecl* hksi_lua_setfieldType)(lua_State*, int, const char*);
	hksi_lua_setfieldType setfield;
	typedef int(__thiscall* GetTopType)(lua_State*);
	GetTopType GetTop;
	typedef int(__thiscall* DoStringType)(lua_State*, const char*);
	DoStringType DoString;
	typedef int(__cdecl* hksi_lua_tobooleanType)(lua_State*, int);
	hksi_lua_tobooleanType hksi_lua_toboolean;
	typedef void(__cdecl* hksi_lua_pushnumberType)(lua_State*, double);
	hksi_lua_pushnumberType hksi_lua_pushnumber;
	typedef void(__cdecl* hksi_lua_pushintegerType)(lua_State*, int);
	hksi_lua_pushintegerType hksi_lua_pushinteger;
	typedef void(__cdecl* hksi_luaL_errorType)(lua_State*, char const*, ...);
	hksi_luaL_errorType hksi_luaL_error;
	typedef char const* (__cdecl* hksi_lua_pushfstringType)(lua_State*, char const*, ...);
	hksi_lua_pushfstringType hksi_lua_pushfstring;
	typedef char const* (__cdecl* CheckLStringType)(lua_State*, int, unsigned __int64*);
	CheckLStringType CheckLString;
	typedef void* (__cdecl* hksi_lua_touserdataType)(lua_State*, int);
	hksi_lua_touserdataType hksi_lua_touserdata;
	typedef void(__cdecl* hksi_lua_getfieldType)(lua_State*, int, char const*);
	hksi_lua_getfieldType hksi_lua_getfield;
	typedef void(__thiscall* PopType)(lua_State*, int);
	PopType Pop;
	typedef void(__cdecl* hksi_lua_createtableType)(lua_State*, int, int);
	hksi_lua_createtableType createtable;

	namespace {
		static void InitHavokScriptImports(HMODULE hksDll) {
			pushnamedcclosure = (hks_pushnamedcclosureType)GetProcAddress(hksDll, "?hks_pushnamedcclosure@@YAXPEAUlua_State@@P6AH0@ZHPEBDH@Z");
			setfield = (hksi_lua_setfieldType)GetProcAddress(hksDll, "?hksi_lua_setfield@@YAXPEAUlua_State@@HPEBD@Z");
			checkinteger = (luaL_checkintegerType)GetProcAddress(hksDll, "?luaL_checkinteger@@YAHPEAUlua_State@@H@Z");
			GetTop = (GetTopType)GetProcAddress(hksDll, "?GetTop@LuaState@LuaPlus@@QEBAHXZ");
			DoString = (DoStringType)GetProcAddress(hksDll, "?DoString@LuaState@LuaPlus@@QEAAHPEBD@Z");
			hksi_lua_toboolean = (hksi_lua_tobooleanType)GetProcAddress(hksDll, "?hksi_lua_toboolean@@YAHPEAUlua_State@@H@Z");
			hksi_lua_pushinteger = (hksi_lua_pushintegerType)GetProcAddress(hksDll, "?hksi_lua_pushinteger@@YAXPEAUlua_State@@H@Z");
			hksi_lua_pushnumber = (hksi_lua_pushnumberType)GetProcAddress(hksDll, "?hksi_lua_pushnumber@@YAXPEAUlua_State@@N@Z");
			hksi_luaL_error = (hksi_luaL_errorType)GetProcAddress(hksDll, "?hksi_luaL_error@@YAHPEAUlua_State@@PEBDZZ");
			checknumber = (luaL_checknumberType)GetProcAddress(hksDll, "?luaL_checknumber@@YANPEAUlua_State@@H@Z");
			hksi_lua_pushfstring = (hksi_lua_pushfstringType)GetProcAddress(hksDll, "?hksi_lua_pushfstring@@YAPEBDPEAUlua_State@@PEBDZZ");
			CheckLString = (CheckLStringType)GetProcAddress(hksDll, "");
			Pop = (PopType)GetProcAddress(hksDll, "?Pop@LuaState@LuaPlus@@QEAAXH@Z");
			hksi_lua_touserdata = (hksi_lua_touserdataType)GetProcAddress(hksDll, "?hksi_lua_touserdata@@YAPEAXPEAUlua_State@@H@Z");
			hksi_lua_getfield = (hksi_lua_getfieldType)GetProcAddress(hksDll, "?hksi_lua_getfield@@YAXPEAUlua_State@@HPEBD@Z");
			createtable = (hksi_lua_createtableType)GetProcAddress(hksDll, "?lua_createtable@@YAXPEAUlua_State@@HH@Z");
		}
	}

	// Should only ever be called once.
    void InitHavokScript() {
        HMODULE hksDll = GetModuleHandle(TEXT("HavokScript_FinalRelease.dll"));
        if (hksDll == NULL) {
            std::cout << "Failed to load HavokScript! " << GetLastError() << '\n';
            return;
        }
		InitHavokScriptImports(hksDll);
    }
}
