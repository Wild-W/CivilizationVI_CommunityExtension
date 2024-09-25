#pragma once
#include "HavokScript.h"
#include <vector>

namespace ProxyTypes {
	typedef void(__thiscall* SetMaxTurns)(void* game, int turnCount);
	typedef void(__cdecl* RegisterScriptDataForUI)(hks::lua_State* _, hks::lua_State* L);
	typedef void(__cdecl* RegisterScriptData)(hks::lua_State*);
	typedef void* (__cdecl* DllCreateGameContext)(void);
	typedef void(__cdecl* PushMethods)(hks::lua_State*, int);
	typedef bool(__cdecl* CCallWithErrorHandling)(hks::lua_State* L, hks::luaFunc, void*);
	typedef void(__thiscall* Buildings_AddGreatWorkSlots)(void* buildings, unsigned int buildingType);
	typedef void(__thiscall* GlobalParameters_Initialize)(void* globalParameters, void* databaseConnection);
	typedef bool(__thiscall* GlobalParameters_Get)(void* globalParameters, void* databaseQuery, const char* name, float* value, float defaultValue);
	typedef void(__thiscall* ApplyTourism)(void* playerCulture);
	typedef void(__cdecl* GetPlayersToProcess)(std::vector<int>*);
	typedef void(__cdecl* RegisterMembers)(hks::lua_State*);
}
