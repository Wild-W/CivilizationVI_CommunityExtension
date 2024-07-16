#include "UnitManager.h"
#include "Runtime.h"
#include "ProxyTypes.h"

namespace UnitManager {
	Types::ChangeOwner ChangeOwner;
	Types::Get Get;

	ProxyTypes::RegisterMembers base_RegisterMembers;
	ProxyTypes::RegisterMembers orig_RegisterMembers;

	int lChangeOwner(hks::lua_State* L) {
		Manager* manager = Get();

		Unit::Instance* unit = Unit::GetInstance(L, 1, true);
		Unit::Instance** unitRef = &unit;

		int playerId = hks::checkplayerid(L, 2);
		bool b1 = hks::toboolean(L, 3);
		bool b2 = hks::toboolean(L, 4);

		ChangeOwner(manager, unit, playerId, b1, b2, unitRef);

		Unit::Push(L, *unitRef);
		return 1;
	}

	int lGetInstance(hks::lua_State* L) {
		Unit::Instance* unit = (Unit::Instance*)static_cast<uintptr_t>(hks::checknumber(L, 1));

		Unit::Push(L, unit);
		return 1;
	}

	void RegisterMembers(hks::lua_State* L) {
		std::cout << "Hooked UnitManager::PushMethods!\n";

		PushLuaMethod(L, lChangeOwner, "lChangeOwner", -2, "ChangeOwner");
		PushLuaMethod(L, lGetInstance, "lGetInstance", -2, "GetInstance");

		base_RegisterMembers(L);
	}

	void Create() {
		using namespace Runtime;

		ChangeOwner = GetGameCoreGlobalAt<Types::ChangeOwner>(CHANGE_OWNER_OFFSET);
		Get = GetGameCoreGlobalAt<Types::Get>(GET_OFFSET);

		orig_RegisterMembers = GetGameCoreGlobalAt<ProxyTypes::RegisterMembers>(REGISTER_MEMBERS_OFFSET);
		CreateHook(orig_RegisterMembers, &RegisterMembers, &base_RegisterMembers);
	}
}