#pragma once
#include "ProxyTypes.h"
#include "HavokScript.h"
#include "Runtime.h"

namespace EconomicManager {
    typedef class Manager;
    namespace Types {
        typedef Manager* (__cdecl* Get)(void);
        typedef int(__thiscall* GetTourismFromMonopolies)(Manager* economicManager, int playerId);
    }
    
    extern Types::GetTourismFromMonopolies base_GetTourismFromMonopolies;
    extern Types::GetTourismFromMonopolies orig_GetTourismFromMonopolies;
    extern Types::Get Get;

    extern double globalMonopolyTourismMultiplier;
    extern double playerMonopolyTourismMultipliers[64];

    extern int __cdecl GetTourismFromMonopolies(Manager* economicManager, int playerId);

    extern int lGetTourismFromMonopolies(hks::lua_State* L);
    extern int lGetMonopolyTourismMultiplier(hks::lua_State* L);
    extern int lSetMonopolyTourismMultiplier(hks::lua_State* L);
    extern int lChangeMonopolyTourismMultiplier(hks::lua_State* L);

    extern int Register(hks::lua_State* L);

    constexpr uintptr_t GET_OFFSET = 0x62cdf0;
    constexpr uintptr_t GET_TOURISM_FROM_MONOPOLIES_OFFSET = 0x62dd20;

    extern void Create();
}
