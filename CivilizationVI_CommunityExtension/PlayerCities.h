#pragma once
#include "HavokScript.h"
#include "ProxyTypes.h"
#include "Player.h"

namespace PlayerCities {
	typedef class IPlayerCities;
	namespace Types {
		typedef Player::Cities* (__cdecl* GetInstance)(hks::lua_State*, int, bool);
		typedef void(__thiscall* AddGreatWork)(Player::Cities* cities, unsigned int greatWorkIndex);
	}
	
	extern ProxyTypes::PushMethods base_PushMethods;
	extern ProxyTypes::PushMethods orig_PushMethods;

	extern Types::AddGreatWork AddGreatWork;
	extern Types::GetInstance GetInstance;

	extern int lAddGreatWork(hks::lua_State* L);

	extern void __cdecl PushMethods(hks::lua_State* L, int stackOffset);

	constexpr uintptr_t GET_INSTANCE_OFFSET = 0x6ee9b0;
	constexpr uintptr_t ADD_GREAT_WORK_OFFSET = 0x2643b0;
	constexpr uintptr_t PUSH_METHODS_OFFSET = 0x6eeb10;

	extern void Create();
}