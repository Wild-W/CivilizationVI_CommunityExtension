#include "EconomicManager.h"
#include "HavokScript.h"
#include "Runtime.h"
#include <algorithm>

namespace EconomicManager {
    Types::GetTourismFromMonopolies base_GetTourismFromMonopolies;
    Types::GetTourismFromMonopolies orig_GetTourismFromMonopolies;
    Types::Get Get;

    double globalMonopolyTourismMultiplier = 1.0;
    double playerMonopolyTourismMultipliers[64] = {};

    int __cdecl GetTourismFromMonopolies(Manager* economicManager, int playerId) {
        int result = base_GetTourismFromMonopolies(economicManager, playerId);
        return std::round(result * globalMonopolyTourismMultiplier * playerMonopolyTourismMultipliers[playerId]);
    }

    int lGetTourismFromMonopolies(hks::lua_State* L) {
        Manager* economicManager = Get();
        int playerId = hks::checkplayerid(L, 1);
        int tourism = GetTourismFromMonopolies(economicManager, playerId);

        hks::pushinteger(L, tourism);
        return 1;
    }

    int lGetMonopolyTourismMultiplier(hks::lua_State* L) {
        int args = hks::gettop(L);

        if (args == 0) {
            hks::pushnumber(L, globalMonopolyTourismMultiplier);
            return 1;
        }
        if (args == 1) {
            int playerId = hks::checkplayerid(L, 1);

            hks::pushnumber(L, playerMonopolyTourismMultipliers[playerId]);
            return 1;
        }
        hks::error(L, "Incorrect number of arguments. Expected 0 or 1.");
        return 0;
    }

    int lSetMonopolyTourismMultiplier(hks::lua_State* L) {
        int args = hks::gettop(L);

        if (args == 1) {
            globalMonopolyTourismMultiplier = hks::checknumber(L, 1);
        }
        else if (args == 2) {
            playerMonopolyTourismMultipliers[hks::checkplayerid(L, 1)] = hks::checknumber(L, 2);
        }
        else {
            hks::error(L, "Incorrect number of arguments. Expected 1 or 2.");
        }
        return 0;
    }

    int lChangeMonopolyTourismMultiplier(hks::lua_State* L) {
        int args = hks::gettop(L);

        if (args == 1) {
            globalMonopolyTourismMultiplier = hks::checknumber(L, 1);
        }
        else if (args == 2) {
            playerMonopolyTourismMultipliers[hks::checkplayerid(L, 1)] += hks::checknumber(L, 2);
        }
        else {
            hks::error(L, "Incorrect number of arguments. Expected 1 or 2.");
        }
        return 0;
    }

    int Register(hks::lua_State* L) {
        std::cout << "Registering EconomicManager!\n";

        hks::createtable(L, 0, 4);

        PushLuaMethod(L, lGetMonopolyTourismMultiplier, "lGetMonopolyTourismMultiplier", -2, "GetMonopolyTourismMultiplier");
        PushLuaMethod(L, lSetMonopolyTourismMultiplier, "lSetMonopolyTourismMultiplier", -2, "SetMonopolyTourismMultiplier");
        PushLuaMethod(L, lChangeMonopolyTourismMultiplier, "lChangeMonopolyTourismMultiplier", -2, "ChangeMonopolyTourismMultiplier");
        PushLuaMethod(L, lGetTourismFromMonopolies, "lGetTourismFromMonopolies", -2, "GetTourismFromMonopolies");

        hks::setfield(L, hks::LUA_GLOBAL, "EconomicManager");
        return 0;
    }

    void Create() {
        using namespace Runtime;

        Get = GetGameCoreGlobalAt<Types::Get>(GET_OFFSET);

        std::fill(std::begin(playerMonopolyTourismMultipliers), std::end(playerMonopolyTourismMultipliers), 1);
        orig_GetTourismFromMonopolies = GetGameCoreGlobalAt<Types::GetTourismFromMonopolies>(GET_TOURISM_FROM_MONOPOLIES_OFFSET);
        CreateHook(orig_GetTourismFromMonopolies, &GetTourismFromMonopolies, &base_GetTourismFromMonopolies);
    }
}