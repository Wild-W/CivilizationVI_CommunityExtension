#pragma once
#include "HavokScript.h"

namespace Player {
	typedef class Cities;
	typedef class Diplomacy;
	typedef class Instance;
	typedef class Manager;
	namespace Cache {
		typedef class Instance;
		namespace Types {
			typedef Cache::Instance* (__cdecl* GetPlayer)(int playerId);
			typedef Cache::Instance* (__cdecl* GetPlayerInstance)(hks::lua_State* L);
		}

		constexpr uintptr_t GET_PLAYER_OFFSET = 0x7af30;
		extern Cache::Types::GetPlayer GetPlayer;

		constexpr uintptr_t GET_PLAYER_INSTANCE_OFFSET = 0x6a4090;
		extern Cache::Types::GetPlayerInstance GetPlayerInstance;
	}

	namespace Types {
		typedef Instance* (__cdecl* GetPlayerInstance)(hks::lua_State* L);
		typedef bool(__thiscall* AddAgenda)(Instance* player, int agendaIndex, int diplomaticVisibilityType);
		typedef void(__thiscall* PushMethods)(Instance*, hks::lua_State*, int);
	}

	constexpr uintptr_t GET_PLAYER_INSTANCE_OFFSET = 0x6e8120;
	extern Types::GetPlayerInstance GetPlayerInstance;

	constexpr uintptr_t ADD_AGENDA_OFFSET = 0x2f7400;
	extern Types::AddAgenda AddAgenda;

	constexpr uintptr_t PUSH_METHODS_OFFSET = 0x6ecbd0;
	extern void PushMethods(Instance* player, hks::lua_State* L, int stackOffset);

	extern void Create();
}
