#pragma once
#include "HavokScript.h"

namespace EmergencyManager {
	typedef class Manager;
	namespace Types {
		typedef void(__thiscall* ChangePlayerScore)(Manager* manager, int playerId, int emergencyIndex, int amount);
		typedef void(__thiscall* ChangePlayerScore2)(Manager* manager, int player1Id, int player2Id, int emergencyIndex, int amount);
		typedef Manager* (__cdecl* Get)(void);
	}
	constexpr uintptr_t GET_OFFSET = 0x19a7b0;
	constexpr uintptr_t CHANGE_PLAYER_SCORE_OFFSET = 0x1991f0;
	constexpr uintptr_t CHANGE_PLAYER_SCORE_2_OFFSET = 0x199140;

    extern int lChangePlayerScore(hks::lua_State* L);

	extern int Register(hks::lua_State* L);

	extern void Create();
}