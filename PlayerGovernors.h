#pragma once
#include "HavokScript.h"
#include "Governor.h"

namespace PlayerGovernors {
    typedef class Governors;
    typedef class IPlayerGovernors;
    namespace Types {
        typedef void(__thiscall* NeutralizeGovernor)(Governors* governors, Governor::Instance* governor, int neutralizedTurns);
        typedef bool(__thiscall* PromoteGovernor)(Governors* governors, int governorId, int governorPromotionIndex);
        typedef Governors* (__cdecl* GetInstance)(hks::lua_State*, int, bool);
        typedef void(__thiscall* PushMethods)(IPlayerGovernors*, hks::lua_State*, int);
        typedef Governor::Instance* (__thiscall* GetGovernor)(Governors* governors, int governorIndex);
        typedef void(__thiscall* ChangeNeutralizedTurns)(Governor::Instance* governor, int neutralizedTurns);
        typedef void(__thiscall* UnassignGovernor)(Governors* governors, Governor::Instance* governor, bool unknown1, bool unknown2);
    }

    extern Types::PushMethods base_PushMethods;
    extern Types::PushMethods orig_PushMethods;

    extern Types::PromoteGovernor PromoteGovernor;
    extern Types::GetInstance GetInstance;
    extern Types::GetGovernor GetGovernor;
    extern Types::NeutralizeGovernor NeutralizeGovernor;
    extern Types::ChangeNeutralizedTurns ChangeNeutralizedTurns;
    extern Types::UnassignGovernor UnassignGovernor;

    constexpr uintptr_t PROMOTE_GOVERNOR_OFFSET = 0x2df340;
    constexpr uintptr_t PUSH_METHODS_OFFSET = 0x713b20;
    constexpr uintptr_t GET_INSTANCE_OFFSET = 0x7139c0;
    constexpr uintptr_t NEUTRALIZE_GOVERNOR_OFFSET = 0x2df270;
    constexpr uintptr_t GET_GOVERNOR_OFFSET = 0x2de960;
    constexpr uintptr_t CHANGE_NEUTRALIZED_TURNS = 0x259140;
    constexpr uintptr_t UNASSIGN_GOVERNOR_OFFSET = 0x2df800;

    extern int lPromoteGovernor(hks::lua_State* L);
    extern int lNeutralizeGovernor(hks::lua_State* L);
    extern int lChangeNeutralizedTurns(hks::lua_State* L);
    extern int lUnassignGovernor(hks::lua_State* L);

    extern void __cdecl PushMethods(IPlayerGovernors* playerGovernors, hks::lua_State* L, int stackOffset);

    extern void Create();
}