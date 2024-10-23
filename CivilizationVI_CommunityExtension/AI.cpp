#include "AI.h"
#include "Runtime.h"
#include "EventSystems.h"
#include "Data.h"
#include <functional>
#include <string>

namespace AI::CongressSupport {
	// TODO: Find a way to optimize target choosers. Currently they are ran for every player (even minor civs), every turn, and for both types of outcomes.
	// This is a problem with the game's base code, and could prove problematic for performance if we hook these functions by calling lua code.

	static bool HandleTargetChooser(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType,
		const std::string& name, const std::string& decisionKey, Types::TargetChooser baseChooser, void* modifierAnalysis, unsigned int fieldOffset)
	{
		using namespace EventSystems;
		using namespace Data;

		if (DoesProcessorExist(name)) {
			auto variantMap = LuaVariantMap();
			variantMap.emplace("OutcomeType", LuaVariant(outcomeType));
			variantMap.emplace("PlayerId", LuaVariant(*(int*)((uintptr_t)player + 0xd8)));
			variantMap.emplace(decisionKey, LuaVariant(*(int*)((uintptr_t)modifierAnalysis + fieldOffset)));

			std::cout << "Calling Target Chooser: " << name << "!\n";

			if (CallProcessorsAsync(name, variantMap, decisionKey).get()) {
				int decisionType = std::get<int>(variantMap.at(decisionKey));
				std::cout << "Decision type: " << decisionType << '\n';
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
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"DistrictTargetChooser", "DistrictIndex", base_District, modifierAnalysis, 0x1c);
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
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"UnitBuildYieldTargetChooser", "YieldIndex", base_UnitBuildYield, modifierAnalysis, 0x64);
	}

	Types::TargetChooser orig_TradingPartners;
	Types::TargetChooser base_TradingPartners;
	bool TradingPartners(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"TradingPartnersTargetChooser", "TargetPlayerId", base_TradingPartners, modifierAnalysis, 0x40);
	}

	Types::TargetChooser orig_PlayerOrDiploLeader;
	Types::TargetChooser base_PlayerOrDiploLeader;
	bool PlayerOrDiploLeader(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"PlayerOrDiploLeaderTargetChooser", "TargetPlayerId", base_PlayerOrDiploLeader, modifierAnalysis, 0x40);
	}

	// Unused normally
	Types::TargetChooser orig_GreatPersonClass;
	Types::TargetChooser base_GreatPersonClass;
	bool GreatPersonClass(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"GreatPersonClassTargetChooser", "GreatPersonClassIndex", base_GreatPersonClass, modifierAnalysis, 0x6c);
	}

	Types::TargetChooser orig_GreatPersonPatronage;
	Types::TargetChooser base_GreatPersonPatronage;
	bool GreatPersonPatronage(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"GreatPersonPatronageTargetChooser", "GreatPersonClassIndex", base_GreatPersonClass, modifierAnalysis, 0x6c);
	}

	Types::TargetChooser orig_SpyOperation;
	Types::TargetChooser base_SpyOperation;
	bool SpyOperation(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"SpyOperationTargetChooser", "UnitOperationIndex", base_SpyOperation, modifierAnalysis, 0x54);
	}

	Types::TargetChooser orig_MostCommonLuxury;
	Types::TargetChooser base_MostCommonLuxury;
	bool MostCommonLuxury(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"MostCommonLuxuryTargetChooser", "ResourceIndex", base_MostCommonLuxury, modifierAnalysis, 0x4c);
	}

	Types::TargetChooser orig_MinorCivBonus;
	Types::TargetChooser base_MinorCivBonus;
	bool MinorCivBonus(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"MinorCivBonusTargetChooser", "MinorCivBonusIndex", base_MinorCivBonus, modifierAnalysis, 0x3c);
	}

	Types::TargetChooser orig_GrievancesType;
	Types::TargetChooser base_GrievancesType;
	bool GrievancesType(Types::Class* congressSupport, Player::Instance* player, OutcomeType outcomeType, void* modifierAnalysis) {
		return HandleTargetChooser(congressSupport, player, outcomeType,
			"GrievancesTypeTargetChooser", "TargetPlayerId", base_GrievancesType, modifierAnalysis, 0x40);
	}

	int RegisterOutcomeTypes(hks::lua_State* L) {
		hks::createtable(L, 0, 2);

		hks::pushinteger(L, OutcomeType::A);
		hks::setfield(L, -2, "A");
		hks::pushinteger(L, OutcomeType::B);
		hks::setfield(L, -2, "B");

		hks::setfield(L, hks::LUA_GLOBAL, "OutcomeTypes");
		return 0;
	}

	void Create() {
		using namespace Runtime;

		orig_District = GetGameCoreGlobalAt<Types::TargetChooser>(DISTRICT_OFFSET);
		CreateHook(orig_District, &District, &base_District);

		orig_UnitPromotionClass = GetGameCoreGlobalAt<Types::TargetChooser>(UNIT_PROMOTION_CLASS_OFFSET);
		CreateHook(orig_UnitPromotionClass, &UnitPromotionClass, &base_UnitPromotionClass);

		orig_UnitBuildYield = GetGameCoreGlobalAt<Types::TargetChooser>(UNIT_BUILD_YIELD_OFFSET);
		CreateHook(orig_UnitBuildYield, &UnitBuildYield, &base_UnitBuildYield);

		orig_TradingPartners = GetGameCoreGlobalAt<Types::TargetChooser>(TRADING_PARTNERS_OFFSET);
		CreateHook(orig_TradingPartners, &TradingPartners, &base_TradingPartners);

		orig_PlayerOrDiploLeader = GetGameCoreGlobalAt<Types::TargetChooser>(PLAYER_OR_DIPLO_LEADER_OFFSET);
		CreateHook(orig_PlayerOrDiploLeader, &PlayerOrDiploLeader, &base_PlayerOrDiploLeader);

		orig_GreatPersonClass = GetGameCoreGlobalAt<Types::TargetChooser>(GREAT_PERSON_CLASS_OFFSET);
		CreateHook(orig_GreatPersonClass, &GreatPersonClass, &base_GreatPersonClass);

		orig_GreatPersonPatronage = GetGameCoreGlobalAt<Types::TargetChooser>(GREAT_PERSON_PATRONAGE_OFFSET);
		CreateHook(orig_GreatPersonPatronage, &GreatPersonPatronage, &base_GreatPersonPatronage);

		orig_SpyOperation = GetGameCoreGlobalAt<Types::TargetChooser>(SPY_OPERATION_OFFSET);
		CreateHook(orig_SpyOperation, &SpyOperation, &base_SpyOperation);

		orig_MostCommonLuxury = GetGameCoreGlobalAt<Types::TargetChooser>(MOST_COMMON_LUXURY_OFFSET);
		CreateHook(orig_MostCommonLuxury, &MostCommonLuxury, &base_MostCommonLuxury);

		orig_MinorCivBonus = GetGameCoreGlobalAt<Types::TargetChooser>(MINOR_CIV_BONUS_OFFSET);
		CreateHook(orig_MinorCivBonus, &MinorCivBonus, &base_MinorCivBonus);

		orig_GrievancesType = GetGameCoreGlobalAt<Types::TargetChooser>(GRIEVANCES_TYPE_OFFSET);
		CreateHook(orig_GrievancesType, &GrievancesType, &base_GrievancesType);
	}
}

namespace AI::Espionage {
	Types::GetAIEspionage GetAIEspionage;
	Types::GetMostUsedSpyMission GetMostUsedSpyMission;

	static int lGetMostUsedSpyMission(hks::lua_State* L) {
		Player::Instance* player = Player::GetPlayerInstance(L);

		hks::pushinteger(L, GetMostUsedSpyMission(GetAIEspionage(player)));
		return 1;
	}

	int RegisterAIEspionageManager(hks::lua_State* L) {
		std::cout << "Registering AIEspionageManager!\n";

		hks::createtable(L, 0, 1);

		PushLuaMethod(L, lGetMostUsedSpyMission, "lGetMostUsedSpyMission", -2, "GetMostUsedSpyMission");

		hks::setfield(L, hks::LUA_GLOBAL, "AIEspionageManager");
		return 0;
	}

	void Create() {
		using namespace Runtime;

		GetAIEspionage = GetGameCoreGlobalAt<Types::GetAIEspionage>(GET_AI_ESPIONAGE_OFFSET);
		GetMostUsedSpyMission = GetGameCoreGlobalAt<Types::GetMostUsedSpyMission>(GET_MOST_USED_SPY_MISSION_OFFSET);
	}
}