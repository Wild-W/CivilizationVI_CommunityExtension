#pragma once
#include "ProxyTypes.h"
#include "HavokScript.h"
#include "Runtime.h"

namespace EconomicManager {
    extern ProxyTypes::GetTourismFromMonopolies base_GetTourismFromMonopolies;
    extern ProxyTypes::GetTourismFromMonopolies orig_GetTourismFromMonopolies;
    extern ProxyTypes::EconomicManager_Get Get;

    extern double monopolyTourismMultiplier;

    extern int __cdecl GetTourismFromMonopolies(void* economicManager, int playerId);

    extern int lGetTourismFromMonopolies(hks::lua_State* L);
    extern int lGetMonopolyTourismMultiplier(hks::lua_State* L);
    extern int lSetMonopolyTourismMultiplier(hks::lua_State* L);
    extern int lChangeMonopolyTourismMultiplier(hks::lua_State* L);

    extern int Register(hks::lua_State* L);

    constexpr uintptr_t ECONOMIC_MANAGER_GET_OFFSET = 0x62cdf0;
    constexpr uintptr_t GET_TOURISM_FROM_MONOPOLIES_OFFSET = 0x62dd20;

    extern void Create();
}
