#include "GovernorManager.h"
#include "PlayerGovernors.h"
#include "Player.h"
#include "Runtime.h"

namespace GovernorManager {
    int lChangeTurnsToEstablishDelay(hks::lua_State* L) {
        int playerId = hks::checkplayerid(L, 1);
        int governorHash = hks::checkinteger(L, 2);
        int amount = hks::checkinteger(L, 3);

        PlayerGovernors::Cache::ChangeTurnsToEstablishDelay(Player::Cache::EditGovernors(Player::Cache::GetPlayer(playerId)), governorHash, amount);
        PlayerGovernors::ChangeTurnsToEstablishDelay(PlayerGovernors::Edit(playerId), governorHash, amount);
        return 0;
    }

    int lSetTurnsToEstablishDelay(hks::lua_State* L) {
        int playerId = hks::checkplayerid(L, 1);
        int governorHash = hks::checkinteger(L, 2);
        int amount = hks::checkinteger(L, 3);

        PlayerGovernors::Cache::SetTurnsToEstablishDelay(Player::Cache::EditGovernors(Player::Cache::GetPlayer(playerId)), governorHash, amount);
        PlayerGovernors::SetTurnsToEstablishDelay(PlayerGovernors::Edit(playerId), governorHash, amount);
        return 0;
    }

    int lGetTurnsToEstablishDelay(hks::lua_State* L) {
        int playerId = hks::checkplayerid(L, 1);
        int governorHash = hks::checkinteger(L, 2);

        hks::pushinteger(L, PlayerGovernors::GetTurnsToEstablishDelay(PlayerGovernors::Edit(playerId), governorHash));
        return 1;
    }

    int Register(hks::lua_State* L) {
        std::cout << "Registering GovernorManager!\n";

        hks::createtable(L, 0, 3);

        PushLuaMethod(L, lChangeTurnsToEstablishDelay, "lChangeTurnsToEstablishDelay", -2, "ChangeTurnsToEstablishDelay");
        PushLuaMethod(L, lGetTurnsToEstablishDelay, "lGetTurnsToEstablishDelay", -2, "GetTurnsToEstablishDelay");
        PushLuaMethod(L, lSetTurnsToEstablishDelay, "lSetTurnsToEstablishDelay", -2, "SetTurnsToEstablishDelay");

        hks::setfield(L, hks::LUA_GLOBAL, "GovernorManager");

        return 0;
    }
}