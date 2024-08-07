#include "Player.h"
#include "Runtime.h"

namespace Player {
	namespace Cache {
		Cache::Types::GetPlayer GetPlayer;
		Cache::Types::GetPlayerInstance GetPlayerInstance;
	}

	Types::GetPlayerInstance GetPlayerInstance;
	Types::AddAgenda AddAgenda;

	static int lAddAgenda(hks::lua_State* L) {
		Instance* player = GetPlayerInstance(L);
		int agendaIndex = hks::checkinteger(L, 2);
		int diplomaticVisibilityType = hks::checkinteger(L, 3);

		AddAgenda(player, agendaIndex, diplomaticVisibilityType);
		return 0;
	}

	Types::PushMethods base_PushMethods;
	Types::PushMethods orig_PushMethods;
	void PushMethods(Instance* player, hks::lua_State* L, int stackOffset) {
		std::cout << "Hooked Player::PushMethods!\n";

		PushLuaMethod(L, lAddAgenda, "lAddAgenda", stackOffset, "AddAgenda");

		base_PushMethods(player, L, stackOffset);
	}

	void Create() {
		using namespace Runtime;

		Cache::GetPlayer = GetGameCoreGlobalAt<Cache::Types::GetPlayer>(Cache::GET_PLAYER_OFFSET);
		Cache::GetPlayerInstance = GetGameCoreGlobalAt<Cache::Types::GetPlayerInstance>(Cache::GET_PLAYER_INSTANCE_OFFSET);

		GetPlayerInstance = GetGameCoreGlobalAt<Types::GetPlayerInstance>(GET_PLAYER_INSTANCE_OFFSET);
		AddAgenda = GetGameCoreGlobalAt<Types::AddAgenda>(ADD_AGENDA_OFFSET);

		orig_PushMethods = GetGameCoreGlobalAt<Types::PushMethods>(PUSH_METHODS_OFFSET);
		CreateHook(orig_PushMethods, &PushMethods, &base_PushMethods);
	}
}
