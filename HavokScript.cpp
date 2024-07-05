#include <iostream>
#include <Windows.h>
#include <string>
#include "HavokScript.h"

namespace hks {
	hks_pushnamedcclosureType pushnamedcclosure;
	luaL_checkintegerType checkinteger;
	luaL_checknumberType checknumber;
	hksi_lua_setfieldType setfield;
	GetTopType gettop;
	DoStringType dostring;
	hksi_lua_tobooleanType toboolean;
	hksi_lua_pushnumberType pushnumber;
	hksi_lua_pushintegerType pushinteger;
	hksi_luaL_errorType error;
	hksi_lua_pushfstringType pushfstring;
	CheckLStringType checklstring;
	hksi_lua_touserdataType touserdata;
	hksi_lua_getfieldType getfield;
	PopType pop;
	hksi_lua_createtableType createtable;
	lua_tocfunctionType tocfunction;
	hksL_checktableType checktable;
	hks_lua_objlenType objlen;
	hks_lua_gettableType gettable;

	namespace {
		static void InitHavokScriptImports(HMODULE hksDll) {
			pushnamedcclosure = (hks_pushnamedcclosureType)GetProcAddress(hksDll, "?hks_pushnamedcclosure@@YAXPEAUlua_State@@P6AH0@ZHPEBDH@Z");
			setfield = (hksi_lua_setfieldType)GetProcAddress(hksDll, "?hksi_lua_setfield@@YAXPEAUlua_State@@HPEBD@Z");
			checkinteger = (luaL_checkintegerType)GetProcAddress(hksDll, "?luaL_checkinteger@@YAHPEAUlua_State@@H@Z");
			gettop = (GetTopType)GetProcAddress(hksDll, "?GetTop@LuaState@LuaPlus@@QEBAHXZ");
			dostring = (DoStringType)GetProcAddress(hksDll, "?DoString@LuaState@LuaPlus@@QEAAHPEBD@Z");
			toboolean = (hksi_lua_tobooleanType)GetProcAddress(hksDll, "?hksi_lua_toboolean@@YAHPEAUlua_State@@H@Z");
			pushinteger = (hksi_lua_pushintegerType)GetProcAddress(hksDll, "?hksi_lua_pushinteger@@YAXPEAUlua_State@@H@Z");
			pushnumber = (hksi_lua_pushnumberType)GetProcAddress(hksDll, "?hksi_lua_pushnumber@@YAXPEAUlua_State@@N@Z");
			error = (hksi_luaL_errorType)GetProcAddress(hksDll, "?hksi_luaL_error@@YAHPEAUlua_State@@PEBDZZ");
			checknumber = (luaL_checknumberType)GetProcAddress(hksDll, "?luaL_checknumber@@YANPEAUlua_State@@H@Z");
			pushfstring = (hksi_lua_pushfstringType)GetProcAddress(hksDll, "?hksi_lua_pushfstring@@YAPEBDPEAUlua_State@@PEBDZZ");
			checklstring = (CheckLStringType)GetProcAddress(hksDll, "");
			pop = (PopType)GetProcAddress(hksDll, "?Pop@LuaState@LuaPlus@@QEAAXH@Z");
			touserdata = (hksi_lua_touserdataType)GetProcAddress(hksDll, "?hksi_lua_touserdata@@YAPEAXPEAUlua_State@@H@Z");
			getfield = (hksi_lua_getfieldType)GetProcAddress(hksDll, "?hksi_lua_getfield@@YAXPEAUlua_State@@HPEBD@Z");
			createtable = (hksi_lua_createtableType)GetProcAddress(hksDll, "?lua_createtable@@YAXPEAUlua_State@@HH@Z");
			tocfunction = (lua_tocfunctionType)GetProcAddress(hksDll, "?lua_tocfunction@@YAP6AHPEAUlua_State@@@Z0H@Z");
			checktable = (hksL_checktableType)GetProcAddress(hksDll, "?hksL_checktable@@YAXPEAUlua_State@@H@Z");
			objlen = (hks_lua_objlenType)GetProcAddress(hksDll, "?hksi_lua_objlen@@YA_KPEAUlua_State@@H@Z");
			gettable = (hks_lua_gettableType)GetProcAddress(hksDll, "?hksi_lua_gettable@@YAXPEAUlua_State@@H@Z");
		}
	}

	int checkplayerid(lua_State* L, int position) {
		int playerId = hks::checkinteger(L, position);
		if (0 <= playerId && playerId < 64) {
			return playerId;
		}

		std::string errorMessage = "Invalid playerId: ";
		hks::error(L, errorMessage.append(std::to_string(playerId)).c_str());
		return -1;
	}
	
	void pushboolean(lua_State* L, bool value) {
		// Access the Lua stack top pointer
		int* stackTop = *(int**)((uintptr_t)L + 0x48);

		// Push a Lua boolean value onto the stack
		*stackTop = 1;
		stackTop[2] = static_cast<unsigned int>(value);

		// Increment the stack top pointer by 4 bytes
		*(int**)((uintptr_t)L + 0x48) = stackTop + 0x4;
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
