#pragma once
#include "HavokScript.h"

namespace ProxyTypes {
	typedef void* (__cdecl* IMapPlot_GetInstance)(hks::lua_State*, int, bool);
	typedef void(__cdecl* DiplomaticRelations_ChangeGrievanceScore)(void* diplomaticRelations, int player1Id, int player2Id, int amount);
	typedef void(__thiscall* SetAppeal)(void* __ptr64 plot, int appeal);
	typedef void(__cdecl* RegisterScriptData)(hks::lua_State*);
	typedef void* (__cdecl* DllCreateGameContext)(void);
	typedef void(__cdecl* PushMethods)(hks::lua_State*, int);
}
