#pragma once
#include "HavokScript.h"

namespace GovernorManager {
    extern int lGetTurnsToEstablishDelay(hks::lua_State* L);
    extern int lChangeTurnsToEstablishDelay(hks::lua_State* L);
    extern int lSetTurnsToEstablishDelay(hks::lua_State* L);

    extern int Register(hks::lua_State* L);
}