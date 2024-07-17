#include "AI.h"
#include "Runtime.h"
#include "EventSystems.h"
#include "Data.h"

namespace AI::CongressSupport {
	static bool HandleTargetChooser(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType,
		const char* name, const char* decisionKey, Types::TargetChooser baseChooser, void* modifierAnalysis, uintptr_t fieldOffset)
	{
		using namespace EventSystems;
		using namespace Data;

		if (DoesProcessorExist(name)) {
			auto variantMap = LuaVariantMap();
			variantMap.emplace("OutcomeType", LuaVariant(outcomeType));
			variantMap.emplace("PlayerId", LuaVariant(*(int*)((uintptr_t)player + 0xd8)));
			variantMap.emplace(decisionKey, LuaVariant(*(int*)((uintptr_t)modifierAnalysis + fieldOffset)));

			std::cout << "Calling Target Chooser: " << name << "!\n";

			if (CallCustomProcessor(name, variantMap)) {
				int decisionType = std::get<int>(variantMap.at(decisionKey));
				if (decisionType == -1) {
					return false;
				}

				*(int*)((uintptr_t)modifierAnalysis + fieldOffset) = decisionType;
				return true;
			}
		}

		return baseChooser(congressSupport, player, outcomeType, modifierAnalysis);
	}

	Types::TargetChooser orig_District;
	Types::TargetChooser base_District;
	bool District(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType, "DistrictTargetChooser", "DistrictIndex", base_District, modifierAnalysis, 0x1c);
	}

	Types::TargetChooser orig_UnitPromotionClass;
	Types::TargetChooser base_UnitPromotionClass;
	bool UnitPromotionClass(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"UnitPromotionClassTargetChooser", "UnitPromotionClassIndex", base_UnitPromotionClass, modifierAnalysis, 0x58);
	}

	Types::TargetChooser orig_UnitBuildYield;
	Types::TargetChooser base_UnitBuildYield;
	bool UnitBuildYield(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType, "UnitBuildYieldTargetChooser", "YieldIndex", base_UnitBuildYield, modifierAnalysis, 0x64);
	}

	int RegisterOutcomeTypes(hks::lua_State* L) {
		hks::createtable(L, 0, 2);

		hks::pushinteger(L, OutcomeType::A);
		hks::setfield(L, -2, "A");
		hks::pushinteger(L, OutcomeType::B);
		hks::setfield(L, -2, "B");

		hks::setfield(L, hks::LUA_GLOBAL, "OutcomeTypes");
	}

	void Create() {
		using namespace Runtime;

		orig_District = GetGameCoreGlobalAt<Types::TargetChooser>(DISTRICT_OFFSET);
		CreateHook(orig_District, &District, &base_District);

		orig_UnitPromotionClass = GetGameCoreGlobalAt<Types::TargetChooser>(UNIT_PROMOTION_CLASS_OFFSET);
		CreateHook(orig_UnitPromotionClass, &UnitPromotionClass, &base_UnitPromotionClass);
	}
}