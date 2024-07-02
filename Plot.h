#pragma once
#include "HavokScript.h"
#include "ProxyTypes.h"

namespace Plot {
    typedef class Instance;

    ProxyTypes::PushMethods base_Plot_PushMethods;
    ProxyTypes::PushMethods orig_Plot_PushMethods;

    ProxyTypes::SetAppeal base_SetAppeal;
    ProxyTypes::SetAppeal orig_SetAppeal;

    typedef Instance* (__cdecl* IMapPlot_GetInstance)(hks::lua_State*, int, bool);
    IMapPlot_GetInstance GetInstance;

    extern void __cdecl Hook_SetAppeal(Instance* plot, int appeal);

    extern int lSetAppeal(hks::lua_State* L);

    extern int lLockAppeal(hks::lua_State* L);
};

