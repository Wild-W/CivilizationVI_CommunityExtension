#include "Rules.h"
#include "HavokScript.h"
#include "EventSystems.h"
#include "Data.h"

namespace Rules::Players {
	// Temporary workaround until processor hooking system is stable
	static bool canRaze = true;

	Types::CanRaze orig_CanRaze;
	Types::CanRaze base_CanRaze;
	bool CanRaze(Instance* rulesPlayers, City::Instance* city, int playerId) {
		using namespace EventSystems;
		using namespace Data;

		if (!canRaze) return false;

		if (DoesProcessorExist("CanRaze")) {
			auto variantMap = LuaVariantMap();
			variantMap.emplace("CityId", LuaVariant(*(int*)((uintptr_t)city + 0xa8)));
			variantMap.emplace("PlayerId", LuaVariant(playerId));

			variantMap.emplace("Result", LuaVariant(-1));

			if (CallProcessorsAsync("CanRaze", variantMap, "Result").get()) {
				int result = std::get<int>(variantMap.at("Result"));

				if (result == 0) return false;
				else if (result == 1) return true;
			}
		}

		return base_CanRaze(rulesPlayers, city, playerId);
	}

	static int lSetCanRaze(hks::lua_State* L) {
		canRaze = hks::toboolean(L, 1);
		return 0;
	}

	static int lGetCanRaze(hks::lua_State* L) {
		hks::pushboolean(L, canRaze);
		return 1;
	}

	int Register(hks::lua_State* L) {
		std::cout << "Registering Rules!\n";

		hks::createtable(L, 0, 2);

		PushLuaMethod(L, lSetCanRaze, "lSetCanRaze", -2, "SetCanRaze");
		PushLuaMethod(L, lGetCanRaze, "lGetCanRaze", -2, "GetCanRaze");

		hks::setfield(L, hks::LUA_GLOBAL, "Rules");
		return 0;
	}

	void Create() {
		using namespace Runtime;

		orig_CanRaze = GetGameCoreGlobalAt<Types::CanRaze>(CAN_RAZE_OFFSET);
		CreateHook(orig_CanRaze, &CanRaze, &base_CanRaze);
	}
}