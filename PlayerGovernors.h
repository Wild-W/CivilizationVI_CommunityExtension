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
        typedef void(__thiscall* ChangeNeutralizedIndefinitely)(Governors* governors, int count);
        typedef int(__thiscall* GetTurnsToEstablish)(Governors* governors, int governorHash);
    }

    namespace Cache {
        typedef class Governors;
        namespace Types {
            typedef int(__thiscall* GetTurnsToEstablish)(Governors* governors, int governorHash);
        }
        extern Cache::Types::GetTurnsToEstablish orig_GetTurnsToEstablish;
        extern Cache::Types::GetTurnsToEstablish base_GetTurnsToEstablish;
        extern Cache::Types::GetTurnsToEstablish GetTurnsToEstablish;
    }

    extern Types::PushMethods base_PushMethods;
    extern Types::PushMethods orig_PushMethods;

    extern Types::GetTurnsToEstablish base_GetTurnsToEstablish;
    extern Types::GetTurnsToEstablish orig_GetTurnsToEstablish;

    extern Types::PromoteGovernor PromoteGovernor;
    extern Types::GetInstance GetInstance;
    extern Types::GetGovernor GetGovernor;
    extern Types::NeutralizeGovernor NeutralizeGovernor;
    extern Types::ChangeNeutralizedTurns ChangeNeutralizedTurns;
    extern Types::UnassignGovernor UnassignGovernor;
    extern Types::ChangeNeutralizedIndefinitely ChangeNeutralizedIndefinitely;
    extern int GetNeutralizedIndefinitely(Governors* governors);
    extern int GetTurnsToEstablishDelay(Governors* governors, int governorIndex);
    extern int GetTurnsToEstablish(Governors* governors, int governorIndex);
    extern void SetTurnsToEstablishDelay(Governors* governors, int governorIndex, int amount);
    extern void ChangeTurnsToEstablishDelay(Governors* governors, int governorIndex, int amount);

    constexpr uintptr_t PROMOTE_GOVERNOR_OFFSET = 0x2df340;
    constexpr uintptr_t PUSH_METHODS_OFFSET = 0x713b20;
    constexpr uintptr_t GET_INSTANCE_OFFSET = 0x7139c0;
    constexpr uintptr_t NEUTRALIZE_GOVERNOR_OFFSET = 0x2df270;
    constexpr uintptr_t GET_GOVERNOR_OFFSET = 0x2de960;
    constexpr uintptr_t CHANGE_NEUTRALIZED_TURNS = 0x259140;
    constexpr uintptr_t UNASSIGN_GOVERNOR_OFFSET = 0x2df800;
    constexpr uintptr_t CHANGE_NEUTRALIZED_INDEFINITELY_OFFSET = 0x2de310;
    constexpr uintptr_t GET_TURNS_TO_ESTABLISH_OFFSET = 0x2ded40;
    constexpr uintptr_t CACHE_GET_TURNS_TO_ESTABLISH_OFFSET = 0xbab10;

    extern int lGetTurnsToEstablishDelay(hks::lua_State* L);
    extern int lChangeTurnsToEstablishDelay(hks::lua_State* L);
    extern int lSetTurnsToEstablishDelay(hks::lua_State* L);
    extern int lGetNeutralizedIndefinitely(hks::lua_State* L);
    extern int lChangeNeutralizedIndefinitely(hks::lua_State* L);
    extern int lPromoteGovernor(hks::lua_State* L);
    extern int lNeutralizeGovernor(hks::lua_State* L);
    extern int lChangeNeutralizedTurns(hks::lua_State* L);
    extern int lUnassignGovernor(hks::lua_State* L);

    extern void __cdecl PushMethods(IPlayerGovernors* playerGovernors, hks::lua_State* L, int stackOffset);

    extern void Create();
}