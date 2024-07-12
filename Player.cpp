#include "Player.h"
#include "Runtime.h"

namespace Player {
	namespace Cache {
		Cache::Types::GetPlayer GetPlayer;
		Cache::Types::GetPlayerInstance GetPlayerInstance;
	}

	void Create() {
		using namespace Runtime;

		Cache::GetPlayer = GetGameCoreGlobalAt<Cache::Types::GetPlayer>(Cache::GET_PLAYER_OFFSET);
		Cache::GetPlayerInstance = GetGameCoreGlobalAt<Cache::Types::GetPlayerInstance>(Cache::GET_PLAYER_INSTANCE_OFFSET);
	}
}