#pragma once
#include "HavokScript.h"

namespace Player {
	typedef class Cities;
	typedef class Diplomacy;
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

	extern void Create();
}
