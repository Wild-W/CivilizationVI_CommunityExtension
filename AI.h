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

		namespace Types {
			typedef class Class;
			typedef bool(__thiscall* TargetChooser)(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);
		}

		constexpr uintptr_t DISTRICT_OFFSET = 0x44e4f0;
		extern bool District(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);

		constexpr uintptr_t UNIT_PROMOTION_CLASS_OFFSET = 0x453ca0;
		extern bool UnitPromotionClass(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);

		constexpr uintptr_t UNIT_BUILD_YIELD_OFFSET = 0x4539f0;
		extern bool UnitBuildYield(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);

		constexpr uintptr_t TRADING_PARTNERS_OFFSET = 0x453850;
		extern bool TradingPartners(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);

		constexpr uintptr_t PLAYER_OR_DIPLO_LEADER_OFFSET = 0x452ce0;
		extern bool PlayerOrDiploLeader(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);

		constexpr uintptr_t GREAT_PERSON_CLASS_OFFSET = 0x44f4c0;
		extern bool GreatPersonClass(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);

		constexpr uintptr_t GREAT_PERSON_PATRONAGE_OFFSET = 0x44f570;
		extern bool GreatPersonPatronage(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);

		constexpr uintptr_t SPY_OPERATION_OFFSET = 0x453820;
		extern bool SpyOperation(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);

		constexpr uintptr_t MOST_COMMON_LUXURY_OFFSET = 0x4500a0;
		extern bool MostCommonLuxury(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis);
		
		extern int RegisterOutcomeTypes(hks::lua_State* L);

		extern void Create();
	};
}
