#include "EmergencyManager.h"
#include "Runtime.h"

namespace EmergencyManager {
    Types::ChangePlayerScore ChangePlayerScore;
    Types::ChangePlayerScore2 ChangePlayerScore2;
    Types::Get Get;

    int lChangePlayerScore(hks::lua_State* L) {
        int argCount = hks::gettop(L);

        Manager* manager = Get();
        int playerId = hks::checkinteger(L, 1);

        if (argCount == 3) {
            int emergencyHash = hks::checkinteger(L, 2);
            int amount = hks::checkinteger(L, 3);
            ChangePlayerScore(manager, playerId, emergencyHash, amount);
        }
        else if (argCount == 4) {
            int otherPlayerId = hks::checkinteger(L, 2);
            int emergencyIndex = hks::checkinteger(L, 3);
            int amount = hks::checkinteger(L, 4);
            ChangePlayerScore2(manager, playerId, otherPlayerId, emergencyIndex, amount);
        }
        else {
            hks::error(L, "Incorrect number of arguments. Expected 3 or 4.");
        }

        return 0;
    }

    int Register(hks::lua_State* L) {
        std::cout << "Registering EmergencyManager!\n";

        hks::createtable(L, 0, 1);

        PushLuaMethod(L, lChangePlayerScore, "lChangePlayerScore", -2, "ChangePlayerScore");

        hks::setfield(L, hks::LUA_GLOBAL, "EmergencyManager");
        return 0;
    }

	void Create() {
        using namespace Runtime;

        ChangePlayerScore = GetGameCoreGlobalAt<Types::ChangePlayerScore>(CHANGE_PLAYER_SCORE_OFFSET);
        ChangePlayerScore2 = GetGameCoreGlobalAt<Types::ChangePlayerScore2>(CHANGE_PLAYER_SCORE_2_OFFSET);
        Get = GetGameCoreGlobalAt<Types::Get>(GET_OFFSET);
	}
}