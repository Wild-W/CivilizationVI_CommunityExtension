#pragma once
#include "HavokScript.h"

// TODO: Qualify names of structs to avoid using void pointers

namespace ProxyTypes {
	typedef void* (__cdecl* IMapPlot_GetInstance)(hks::lua_State*, int, bool);
	typedef void* (__cdecl* IPlayerCities_GetInstance)(hks::lua_State*, int, bool);
	typedef void(__cdecl* DiplomaticRelations_ChangeGrievanceScore)(void* diplomaticRelations, int player1Id, int player2Id, int amount);
	typedef void(__thiscall* SetAppeal)(void* plot, int appeal);
	typedef void(__cdecl* RegisterScriptData)(hks::lua_State*);
	typedef void* (__cdecl* DllCreateGameContext)(void);
	typedef void(__cdecl* PushMethods)(hks::lua_State*, int);
	typedef void* (__cdecl* DiplomaticRelations_Edit)(void);
	typedef bool(__cdecl* CCallWithErrorHandling)(hks::lua_State* L, hks::luaFunc, void* _);
	typedef void(__thiscall* Cities_AddGreatWork)(void* cities, unsigned int greatWorkIndex);
}
