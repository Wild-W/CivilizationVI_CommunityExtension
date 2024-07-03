#include "CultureManager.h"
#include "Runtime.h"

namespace CultureManager {
    Types::Get Get;
    Types::FindOrAddGreatWork FindOrAddGreatWork;
    Types::SetGreatWorkPlayer SetGreatWorkPlayer;

    int lFindOrAddGreatWork(hks::lua_State* L) {
        Culture* culture = Get();
        unsigned int greatWorkIndex = hks::checkinteger(L, 1);

        int greatWorkListIndex = FindOrAddGreatWork(culture, greatWorkIndex);
        hks::pushinteger(L, greatWorkListIndex);
        return 1;
    }

    int lSetGreatWorkPlayer(hks::lua_State* L) {
        Culture* culture = Get();
        int greatWorkListIndex = hks::checkinteger(L, 1);
        int playerId = hks::checkinteger(L, 2);

        SetGreatWorkPlayer(culture, greatWorkListIndex, playerId);
        return 0;
    }

    int Register(hks::lua_State* L) {
        std::cout << "Registering CultureManager!\n";

        hks::createtable(L, 0, 2);

        PushLuaMethod(L, lFindOrAddGreatWork, "lFindOrAddGreatWork", -2, "FindOrAddGreatWork");
        PushLuaMethod(L, lSetGreatWorkPlayer, "lSetGreatWorkPlayer", -2, "SetGreatWorkPlayer");

        hks::setfield(L, hks::LUA_GLOBAL, "CultureManager");
        return 0;
    }

    void Create() {
        using namespace Runtime;

        Get = GetGameCoreGlobalAt<Types::Get>(GET_OFFSET);
        FindOrAddGreatWork = GetGameCoreGlobalAt<Types::FindOrAddGreatWork>(FIND_OR_ADD_GREAT_WORK_OFFSET);
        SetGreatWorkPlayer = GetGameCoreGlobalAt<Types::SetGreatWorkPlayer>(SET_GREAT_WORK_PLAYER_OFFSET);
    }
}