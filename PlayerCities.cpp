#include "PlayerCities.h"
#include "Runtime.h"

namespace PlayerCities {
	ProxyTypes::PushMethods base_PushMethods;
	ProxyTypes::PushMethods orig_PushMethods;

	Types::AddGreatWork AddGreatWork;
	Types::GetInstance GetInstance;

	int lAddGreatWork(hks::lua_State* L) {
		Cities* cities = GetInstance(L, 1, true);
		int greatWorkListIndex = hks::checkinteger(L, 2);
		std::cout << cities << ' ' << greatWorkListIndex << '\n';

		AddGreatWork(cities, greatWorkListIndex);
		return 0;
	}

	void __cdecl PushMethods(hks::lua_State* L, int stackOffset) {
		std::cout << "Hooked Cities::PushMethods!\n";

		PushLuaMethod(L, lAddGreatWork, "lAddGreatWork", stackOffset, "AddGreatWork");

		base_PushMethods(L, stackOffset);
	}

	void Create() {
		using namespace Runtime;

		GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);
		AddGreatWork = GetGameCoreGlobalAt<Types::AddGreatWork>(ADD_GREAT_WORK_OFFSET);

		orig_PushMethods = GetGameCoreGlobalAt<ProxyTypes::PushMethods>(PUSH_METHODS_OFFSET);
		CreateHook(orig_PushMethods, &PushMethods, &base_PushMethods);
	}
}
