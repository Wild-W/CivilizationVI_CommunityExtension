#include "Rules.h"
#include "HavokScript.h"
#include "EventSystems.h"
#include "Data.h"

namespace Rules::Players {
	Types::CanRaze orig_CanRaze;
	Types::CanRaze base_CanRaze;
	bool CanRaze(Instance* rulesPlayers, City::Instance* city, int playerId) {
		using namespace EventSystems;
		using namespace Data;

		if (DoesProcessorExist("CanRaze")) {
			auto variantMap = LuaVariantMap();
			variantMap.emplace("CityId", LuaVariant(*(int*)((uintptr_t)city + 0xa8)));
			variantMap.emplace("PlayerId", LuaVariant(playerId));

			variantMap.emplace("Result", LuaVariant(-1));

			if (CallCustomProcessor("CanRaze", variantMap, "Result")) {
				int result = std::get<int>(variantMap.at("Result"));

				if (result == 0) return false;
				else if (result == 1) return true;
			}
		}

		return base_CanRaze(rulesPlayers, city, playerId);
	}

	void Create() {
		using namespace Runtime;

		orig_CanRaze = GetGameCoreGlobalAt<Types::CanRaze>(CAN_RAZE_OFFSET);
		CreateHook(orig_CanRaze, &CanRaze, &base_CanRaze);
	}
}