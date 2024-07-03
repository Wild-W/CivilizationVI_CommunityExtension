#pragma once
#include "HavokScript.h"

namespace PlayerInfluence {
    typedef class Influence;
	namespace Types {
        typedef void(__thiscall* SetTokensToGive)(Influence* influence, int tokens);
        typedef Influence* (__cdecl* GetInstance)(hks::lua_State*, int, bool);
	}
	constexpr uintptr_t PUSH_METHODS_OFFSET = 0x6f3650;
	constexpr uintptr_t GET_INSTANCE_OFFSET = 0x6f34f0;
	constexpr uintptr_t SET_TOKENS_TO_GIVE_OFFSET = 0x2edaf0;

    extern ProxyTypes::PushMethods base_PushMethods;
    extern ProxyTypes::PushMethods orig_PushMethods;
    
    extern Types::GetInstance GetInstance;
    extern Types::SetTokensToGive SetTokensToGive;

    extern int lSetTokensToGive(hks::lua_State* L);
    extern int lSetPoints(hks::lua_State* L);
    extern int lAdjustPoints(hks::lua_State* L);

    extern void __cdecl PushMethods(hks::lua_State* L, int stackOffset);

    extern void Create();
}
