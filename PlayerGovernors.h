#pragma once
#include "HavokScript.h"

namespace PlayerGovernors {
    typedef class Governors;
    typedef class IPlayerGovernors;
    namespace Types {
        typedef void(__thiscall* NeutralizeGovernor)(Governors* governors, void* governor, int neutralizedTurns);
        typedef bool(__thiscall* PromoteGovernor)(Governors* governors, int governorId, int governorPromotionIndex);
        typedef Governors* (__cdecl* GetInstance)(hks::lua_State*, int, bool);
        typedef void(__thiscall* PushMethods)(IPlayerGovernors*, hks::lua_State*, int);
    }

    extern Types::PushMethods base_PushMethods;
    extern Types::PushMethods orig_PushMethods;

    extern Types::PromoteGovernor PromoteGovernor;
    extern Types::GetInstance GetInstance;

    constexpr uintptr_t PROMOTE_GOVERNOR_OFFSET = 0x2df340;
    constexpr uintptr_t PUSH_METHODS_OFFSET = 0x713b20;
    constexpr uintptr_t GET_INSTANCE_OFFSET = 0x7139c0;
    constexpr uintptr_t NEUTRALIZE_GOVERNOR_OFFSET = 0x2df270;

    extern int lPromoteGovernor(hks::lua_State* L);

    extern void __cdecl PushMethods(IPlayerGovernors* playerGovernors, hks::lua_State* L, int stackOffset);

    extern void Create();
}