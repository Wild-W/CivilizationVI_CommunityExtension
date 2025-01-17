#pragma once
#include <iostream>
#include <Windows.h>

#define PushLuaMethod(L, func, cclosurename, stackOffset, name) \
    hks::pushnamedcclosure(L, func, 0, cclosurename, 0); \
    hks::setfield(L, stackOffset, name);

namespace hks {
	constexpr int LUA_GLOBAL = -10002;
	constexpr int LUA_REGISTRYINDEX = -10000;

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
	extern GetTopType gettop;
	typedef int(__thiscall* DoStringType)(lua_State*, const char*);
	extern DoStringType dostring;
	typedef int(__cdecl* hksi_lua_tobooleanType)(lua_State*, int);
	extern hksi_lua_tobooleanType toboolean;
	typedef void(__cdecl* hksi_lua_pushnumberType)(lua_State*, double);
	extern hksi_lua_pushnumberType pushnumber;
	typedef void(__cdecl* hksi_lua_pushintegerType)(lua_State*, int);
	extern hksi_lua_pushintegerType pushinteger;
	typedef void(__cdecl* hksi_luaL_errorType)(lua_State*, char const*, ...);
	extern hksi_luaL_errorType error;
	typedef char const* (__cdecl* hksi_lua_pushfstringType)(lua_State*, char const*, ...);
	extern hksi_lua_pushfstringType pushfstring;
	typedef char const* (__cdecl* CheckLStringType)(lua_State*, int, unsigned __int64*);
	extern CheckLStringType checklstring;
	typedef void* (__cdecl* hksi_lua_touserdataType)(lua_State*, int);
	extern hksi_lua_touserdataType touserdata;
	typedef void(__cdecl* hksi_lua_getfieldType)(lua_State*, int, char const*);
	extern hksi_lua_getfieldType getfield;
	typedef void(__thiscall* PopType)(lua_State*, int);
	extern PopType pop;
	typedef void(__cdecl* hksi_lua_createtableType)(lua_State*, int, int);
	extern hksi_lua_createtableType createtable;
	typedef luaFunc(__cdecl* lua_tocfunctionType)(lua_State*, int);
	extern lua_tocfunctionType tocfunction;
	typedef void(__cdecl* hksL_checktableType)(lua_State*, int);
	extern hksL_checktableType checktable;
	typedef unsigned long long(__cdecl* hks_lua_objlenType)(lua_State*, int);
	extern hks_lua_objlenType objlen;
	typedef void(__cdecl* hks_lua_gettableType)(lua_State*, int);
	extern hks_lua_gettableType gettable;
	typedef void(__cdecl* hksi_lua_cpcallType)(lua_State*, int, luaFunc, int);
	extern hksi_lua_cpcallType cpcall;
	typedef int(__cdecl* RefType)(lua_State*, int);
	extern RefType ref;
	typedef int(__cdecl* hksi_lua_pcallType)(lua_State*, int, int, int);
	extern hksi_lua_pcallType pcall;
	typedef void(__cdecl* hski_lua_pushvalueType)(lua_State*, int);
	extern hski_lua_pushvalueType pushvalue;
	typedef void(__cdecl* hksi_lua_rawgetiType)(lua_State*, int, int);
	extern hksi_lua_rawgetiType rawgeti;
	typedef void(__cdecl* hksi_lua_unrefType)(hks::lua_State*, int, int);
	extern hksi_lua_unrefType unref;
	typedef void(__cdecl* hksi_lua_settableType)(hks::lua_State*, int);
	extern hksi_lua_settableType settable;
	typedef int(__cdecl* hksi_lua_isnumberType)(hks::lua_State*, int);
	extern hksi_lua_isnumberType isnumber;
	typedef int(__cdecl* hksi_lua_tonumberType)(hks::lua_State*, int);
	extern hksi_lua_tonumberType tonumber;
	typedef int(__cdecl* hksi_lua_isuserdataType)(hks::lua_State*, int);
	extern hksi_lua_isuserdataType isuserdata;

	extern int checkplayerid(lua_State*, int);
	extern void pushboolean(lua_State* L, bool value);
	extern void pushlightuserdata(lua_State* L, void* ptr);

	// Should only ever be called once
	extern void InitHavokScript();
}
