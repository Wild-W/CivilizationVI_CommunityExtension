#include "CityTradeManager.h"
#include "Runtime.h"
#include "Game.h"

namespace CityTradeManager {
    Types::SetHasConstructedTradingPost SetHasConstructedTradingPost;

    int lSetHasConstructedTradingPost(hks::lua_State* L) {
        hks::getfield(L, 1, "__instance");
        auto city = (City::Instance*)hks::touserdata(L, -1);
        if (city == NULL) {
            hks::error(L, "Failed to retrieve __instance field as userdata. Is your object NULL?");
            return 0;
        }
        hks::pop(L, 1);

        void* cityTrade = Game::FAutoVariable_edit((void*)((uintptr_t)city + 0xe78));
        int playerId = hks::checkinteger(L, 2);
        bool didConstruct = hks::toboolean(L, 3);
        std::cout << cityTrade << ' ' << playerId << ' ' << didConstruct << '\n';

        SetHasConstructedTradingPost((City::Trade*)cityTrade, playerId, didConstruct);
        return 0;
    }

    int Register(hks::lua_State* L) {
        std::cout << "Registering CityTradeManager!\n";

        hks::createtable(L, 0, 1);

        PushLuaMethod(L, lSetHasConstructedTradingPost, "lSetHasConstructedTradingPost", -2, "SetHasConstructedTradingPost");

        hks::setfield(L, hks::LUA_GLOBAL, "CityTradeManager");
        return 0;
    }

    void Create() {
        using namespace Runtime;

        SetHasConstructedTradingPost = GetGameCoreGlobalAt<Types::SetHasConstructedTradingPost>(SET_HAS_CONSTRUCTED_TRADING_POST_OFFSET);
    }
}
