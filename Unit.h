#pragma once
#include "HavokScript.h"

namespace Unit {
	typedef class Instance;
	namespace Types {
		typedef Instance* (__cdecl* GetInstance)(hks::lua_State* L, int index, bool);
		typedef void(__cdecl* Push)(hks::lua_State* L, Instance* unit);
	}

	constexpr uintptr_t GET_INSTANCE_OFFSET = 0x24da0;
	extern Types::GetInstance GetInstance;
	
	constexpr uintptr_t PUSH_OFFSET = 0x1eff0;
	extern Types::Push Push;

	extern void Create();
}