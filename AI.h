#pragma once
#include "Player.h"

namespace AI {
	enum OutcomeType {
		// For
		A = 1,
		// Against
		B = 2
	};
	namespace Types {
		namespace CongressSupport {
			typedef class Class;
			// TODO: ModifierAnalysis
			typedef bool (__thiscall* District)(CongressSupport::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);
		}
	}

	constexpr uintptr_t DISTRICT_OFFSET = 0x44e4f0;

	namespace CongressSupport {
		extern Types::CongressSupport::District orig_District;
		extern Types::CongressSupport::District base_District;
		extern bool District(Types::CongressSupport::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);
		extern void Create();
	};
}
