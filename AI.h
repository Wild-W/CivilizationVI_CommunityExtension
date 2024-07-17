#pragma once
#include "Player.h"
#include "HavokScript.h"

namespace AI {
	namespace CongressSupport {
		enum OutcomeType {
			// For
			A = 1,
			// Against
			B = 2
		};

		constexpr uintptr_t DISTRICT_OFFSET = 0x44e4f0;
		constexpr uintptr_t UNIT_PROMOTION_CLASS_OFFSET = 0x453ca0;
		constexpr uintptr_t UNIT_BUILD_YIELD_OFFSET = 0x4539f0;

		namespace Types {
			typedef class Class;
			typedef bool(__thiscall* TargetChooser)(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);
		}
		extern bool District(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);
		extern bool UnitPromotionClass(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);
		extern bool UnitBuildYield(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);
		
		extern int RegisterOutcomeTypes(hks::lua_State* L);

		extern void Create();
	};
}
