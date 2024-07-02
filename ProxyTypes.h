#pragma once
#include "HavokScript.h"
#include <vector>

// TODO: Qualify names of structs to avoid using void pointers

namespace ProxyTypes {
	typedef void(__thiscall* SetMaxTurns)(void* game, int turnCount);
	typedef void* (__cdecl* IMapPlot_GetInstance)(hks::lua_State*, int, bool);
	typedef void* (__cdecl* IPlayerCities_GetInstance)(hks::lua_State*, int, bool);
	typedef void* (__cdecl* IPlayerInfluence_GetInstance)(hks::lua_State*, int, bool);
	typedef void* (__cdecl* ICityTrade_GetInstance)(hks::lua_State*, int, bool);
	typedef void* (__cdecl* Governors_GetInstance)(hks::lua_State*, int, bool);
	typedef void(__cdecl* RegisterScriptDataForUI)(hks::lua_State* _, hks::lua_State* L);
	typedef void(__cdecl* DiplomaticRelations_ChangeGrievanceScore)(void* diplomaticRelations, int player1Id, int player2Id, int amount);
	typedef void(__thiscall* SetAppeal)(void* plot, int appeal);
	typedef void(__cdecl* RegisterScriptData)(hks::lua_State*);
	typedef void* (__cdecl* DllCreateGameContext)(void);
	typedef void(__cdecl* PushMethods)(hks::lua_State*, int);
	typedef void* (__cdecl* DiplomaticRelations_Edit)(void);
	typedef bool(__cdecl* CCallWithErrorHandling)(hks::lua_State* L, hks::luaFunc, void*);
	typedef void(__thiscall* Cities_AddGreatWork)(void* cities, unsigned int greatWorkIndex);
	typedef void(__thiscall* Buildings_AddGreatWorkSlots)(void* buildings, unsigned int buildingType);
	typedef void(__thiscall* Influence_SetTokensToGive)(void* influence, int tokens);
	typedef void(__thiscall* SetHasConstructedTradingPost)(void* trade, int playerId, bool didConstruct);
	typedef void(__thiscall* InstancedPushMethods)(void*, hks::lua_State*, int);
	typedef void* (__thiscall* FAutoVariable_edit)(void* object);
	typedef void* (__cdecl* Culture_Get)(void);
	typedef int (__thiscall* FindOrAddGreatWork)(void* culture, unsigned int greatWorkIndex);
	typedef void(__thiscall* SetGreatWorkPlayer)(void* culture, unsigned int, int playerId);
	typedef void* (__cdecl* DiplomaticRelations_GetInstance)(hks::lua_State*, int, bool);
	typedef bool(__thiscall* PromoteGovernor)(void* governors, int governorId, int governorPromotionIndex);
	typedef void(__thiscall* NeutralizeGovernor)(void* governors, void* governor, int neutralizedTurns);
	typedef void(__thiscall* EmergencyManager_ChangePlayerScore)(void* manager, int playerId, int emergencyIndex, int amount);
	typedef void(__thiscall* EmergencyManager_ChangePlayerScore2)(void* manager, int player1Id, int player2Id, int emergencyIndex, int amount);
	typedef void* (__cdecl* EmergencyManager_Get)(void);
	typedef void(__thiscall* GlobalParameters_Initialize)(void* globalParameters, void* databaseConnection);
	typedef bool(__thiscall* GlobalParameters_Get)(void* globalParameters, void* databaseQuery, const char* name, float* value, float defaultValue);
	typedef int(__thiscall* GetTourismFromMonopolies)(void* economicManager, int playerId);
	typedef void(__thiscall* ApplyTourism)(void* playerCulture);
	typedef void(__cdecl* GetPlayersToProcess)(std::vector<int>*);
	typedef void* (__cdecl* EconomicManager_Get)(void);
}
