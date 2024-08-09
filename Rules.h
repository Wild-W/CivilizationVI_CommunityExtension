#pragma once
#include "City.h"
#include "Runtime.h"

namespace Rules::Players {
	typedef class Instance;
	namespace Types {
		typedef bool(__thiscall* CanRaze)(Instance* rulesPlayers, City::Instance* city, int playerId);
	}

	constexpr uintptr_t CAN_RAZE_OFFSET = 0x37b5a0;
	extern bool CanRaze(Instance* rulesPlayers, City::Instance* city, int playerId);

	extern void Create();
}
