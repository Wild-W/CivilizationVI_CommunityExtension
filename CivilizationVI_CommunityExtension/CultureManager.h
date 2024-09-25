#pragma once
#include "HavokScript.h"

namespace CultureManager {
	typedef class Culture;
	namespace Types {
		typedef Culture* (__cdecl* Get)(void);
		typedef int(__thiscall* FindOrAddGreatWork)(Culture* culture, unsigned int greatWorkIndex);
		typedef void(__thiscall* SetGreatWorkPlayer)(Culture* culture, unsigned int, int playerId);
	}

	extern Types::Get Get;
	extern Types::FindOrAddGreatWork FindOrAddGreatWork;
	extern Types::SetGreatWorkPlayer SetGreatWorkPlayer;

	constexpr uintptr_t GET_OFFSET = 0x1c8250;
	constexpr uintptr_t FIND_OR_ADD_GREAT_WORK_OFFSET = 0x1c80e0;
	constexpr uintptr_t SET_GREAT_WORK_PLAYER_OFFSET = 0x1c8c80;

	extern int lFindOrAddGreatWork(hks::lua_State* L);
	extern int lSetGreatWorkPlayer(hks::lua_State* L);

	extern int Register(hks::lua_State* L);

	extern void Create();
}
