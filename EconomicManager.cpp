#include "EconomicManager.h"
#include "HavokScript.h"
#include "Runtime.h"

namespace EconomicManager {
    Types::GetTourismFromMonopolies base_GetTourismFromMonopolies;
    Types::GetTourismFromMonopolies orig_GetTourismFromMonopolies;
    Types::Get Get;

    double monopolyTourismMultiplier = 1.0;

    int __cdecl GetTourismFromMonopolies(Manager* economicManager, int playerId) {
        int result = base_GetTourismFromMonopolies(economicManager, playerId);
        return std::round(result * monopolyTourismMultiplier);
    }

    int lGetTourismFromMonopolies(hks::lua_State* L) {
        Manager* economicManager = Get();
        int playerId = hks::checkinteger(L, 1);

        int tourism = GetTourismFromMonopolies(economicManager, playerId);
        return 1;
    }

    int lGetMonopolyTourismMultiplier(hks::lua_State* L) {
        hks::pushnumber(L, monopolyTourismMultiplier);
        return 1;
    }

    int lSetMonopolyTourismMultiplier(hks::lua_State* L) {
        monopolyTourismMultiplier = hks::checknumber(L, 1);
        return 0;
    }

    int lChangeMonopolyTourismMultiplier(hks::lua_State* L) {
        monopolyTourismMultiplier += hks::checknumber(L, 1);
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

        orig_GetTourismFromMonopolies = GetGameCoreGlobalAt<Types::GetTourismFromMonopolies>(GET_TOURISM_FROM_MONOPOLIES_OFFSET);
        CreateHook(orig_GetTourismFromMonopolies, &GetTourismFromMonopolies, &base_GetTourismFromMonopolies);
    }
}