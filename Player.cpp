#include "Player.h"
#include "Runtime.h"

namespace Player {
	namespace Cache {
		Cache::Types::EditGovernors EditGovernors;
		Cache::Types::GetPlayer GetPlayer;
	}

	void Create() {
		using namespace Runtime;

		Cache::EditGovernors = GetGameCoreGlobalAt<Cache::Types::EditGovernors>(Cache::EDIT_GOVERNORS_OFFSET);
		Cache::GetPlayer = GetGameCoreGlobalAt<Cache::Types::GetPlayer>(Cache::GET_PLAYER_OFFSET);
	}
}