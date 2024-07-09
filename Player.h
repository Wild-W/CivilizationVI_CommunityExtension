#pragma once
#include "PlayerGovernors.h"

namespace Player {
	typedef class Cities;
	typedef class Diplomacy;
	namespace Cache {
		typedef class Instance;
		namespace Types {
			typedef PlayerGovernors::Cache::Governors* (__thiscall* EditGovernors)(Cache::Instance* cachePlayer);
			typedef Cache::Instance* (__cdecl* GetPlayer)(int playerId);
		}

		constexpr uintptr_t EDIT_GOVERNORS_OFFSET = 0xbe290;
		extern Cache::Types::EditGovernors EditGovernors;

		constexpr uintptr_t GET_PLAYER_OFFSET = 0x7af30;
		extern Cache::Types::GetPlayer GetPlayer;
	}

	extern void Create();
}
