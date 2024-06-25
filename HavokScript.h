#pragma once
#include <iostream>
#include <Windows.h>

namespace hks {
	constexpr int LUA_GLOBAL = -10002;

	typedef struct lua_State lua_State;

	typedef int(__cdecl* luaFunc)(lua_State*);

	typedef void(__cdecl* hks_pushnamedcclosureType)(lua_State*, luaFunc, int, const char*, int);
	extern hks_pushnamedcclosureType pushnamedcclosure;
	typedef int(__cdecl* luaL_checkintegerType)(lua_State*, int);
	extern luaL_checkintegerType checkinteger;
	typedef double(__cdecl* luaL_checknumberType)(lua_State*, int);
	extern luaL_checknumberType checknumber;
	typedef void(__cdecl* hksi_lua_setfieldType)(lua_State*, int, const char*);
	extern hksi_lua_setfieldType setfield;
	typedef int(__thiscall* GetTopType)(lua_State*);
	extern GetTopType GetTop;
	typedef int(__thiscall* DoStringType)(lua_State*, const char*);
	extern DoStringType DoString;
	typedef int(__cdecl* hksi_lua_tobooleanType)(lua_State*, int);
	extern hksi_lua_tobooleanType hksi_lua_toboolean;
	typedef void(__cdecl* hksi_lua_pushnumberType)(lua_State*, double);
	extern hksi_lua_pushnumberType hksi_lua_pushnumber;
	typedef void(__cdecl* hksi_lua_pushintegerType)(lua_State*, int);
	extern hksi_lua_pushintegerType hksi_lua_pushinteger;
	typedef void(__cdecl* hksi_luaL_errorType)(lua_State*, char const*, ...);
	extern hksi_luaL_errorType hksi_luaL_error;
	typedef char const* (__cdecl* hksi_lua_pushfstringType)(lua_State*, char const*, ...);
	extern hksi_lua_pushfstringType hksi_lua_pushfstring;
	typedef char const* (__cdecl* CheckLStringType)(lua_State*, int, unsigned __int64*);
	extern CheckLStringType CheckLString;
	typedef void* (__cdecl* hksi_lua_touserdataType)(lua_State*, int);
	extern hksi_lua_touserdataType hksi_lua_touserdata;
	typedef void(__cdecl* hksi_lua_getfieldType)(lua_State*, int, char const*);
	extern hksi_lua_getfieldType hksi_lua_getfield;
	typedef void(__thiscall* PopType)(lua_State*, int);
	extern PopType Pop;
	typedef void(__cdecl* hksi_lua_createtableType)(lua_State*, int, int);
	extern hksi_lua_createtableType createtable;

	// Should only ever be called once
	extern void InitHavokScript();
}
