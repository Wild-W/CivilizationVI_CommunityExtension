#pragma once
#include "HavokScript.h"

namespace GameDiplomacy {
	typedef class DiplomaticRelations;
	typedef class IGameDiplomacy;
	namespace Types {
		typedef DiplomaticRelations* (__cdecl* Edit)(void);
		typedef DiplomaticRelations* (__cdecl* GetInstance)(hks::lua_State*, int, bool);
		typedef void(__cdecl* ChangeGrievanceScore)(DiplomaticRelations* diplomaticRelations, int player1Id, int player2Id, int amount);
		typedef void(__thiscall* PushMethods)(IGameDiplomacy* gameDiplomacy, hks::lua_State* L, int stackOffset);
	}

	constexpr uintptr_t GET_INSTANCE_OFFSET = 0x6d86e0;
	constexpr uintptr_t PUSH_METHODS_OFFSET = 0x745660;
	constexpr uintptr_t EDIT_OFFSET = 0x1d0220;
	constexpr uintptr_t CHANGE_GRIEVANCE_SCORE_OFFSET = 0x1cea40;

    extern Types::ChangeGrievanceScore ChangeGrievanceScore;
    extern Types::GetInstance GetInstance;

    extern Types::PushMethods base_GameDiplomacy_PushMethods;
    extern Types::PushMethods orig_GameDiplomacy_PushMethods;

	extern int lChangeGrievanceScore(hks::lua_State* L);

    extern void __cdecl PushMethods(IGameDiplomacy* gameDiplomacy, hks::lua_State* L, int stackOffset);

    extern void Create();
}
