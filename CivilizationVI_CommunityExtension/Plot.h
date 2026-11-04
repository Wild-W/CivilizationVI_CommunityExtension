#pragma once
#include "HavokScript.h"
#include "ProxyTypes.h"
#include <set>

namespace Plot {
    typedef class Instance;

    namespace Types {
        typedef void(__thiscall* SetAppeal)(Instance* plot, int appeal);
        typedef Instance* (__cdecl* GetInstance)(hks::lua_State*, int, bool);
        typedef int(__thiscall* GetIndex)(Instance* plot);
    }

    extern ProxyTypes::PushMethods base_PushMethods;
    extern ProxyTypes::PushMethods orig_PushMethods;

    extern Types::SetAppeal base_SetAppeal;
    extern Types::SetAppeal orig_SetAppeal;
    
    extern std::set<int> lockedPlots;

    constexpr uintptr_t SET_APPEAL_OFFSET = 0x61270;
    constexpr uintptr_t PUSH_METHODS_OFFSET = 0x1b2e0;
    constexpr uintptr_t GET_INSTANCE_OFFSET = 0x15d60;
    constexpr uintptr_t GET_INDEX_OFFSET = 0x5f2d0;

    extern Types::GetInstance GetInstance;
    extern Types::GetIndex GetIndex;

    extern void __cdecl SetAppeal(Instance* plot, int appeal);

    extern int lSetAppeal(hks::lua_State* L);
    extern int lLockAppeal(hks::lua_State* L);

    extern void Create();
}

