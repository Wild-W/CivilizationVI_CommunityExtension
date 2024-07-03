#include "GameDiplomacy.h"
#include "Runtime.h"

namespace GameDiplomacy {
    Types::ChangeGrievanceScore ChangeGrievanceScore;
    Types::GetInstance GetInstance;

    Types::PushMethods base_GameDiplomacy_PushMethods;
    Types::PushMethods orig_GameDiplomacy_PushMethods;

    int lChangeGrievanceScore(hks::lua_State* L) {
        DiplomaticRelations* diplomaticRelations = GetInstance(L, 1, true);
        int player1Id = hks::checkinteger(L, 2);
        int player2Id = hks::checkinteger(L, 3);
        int amount = hks::checkinteger(L, 4);

        ChangeGrievanceScore(diplomaticRelations, player1Id, player2Id, amount);
        return 0;
    }

    void __cdecl PushMethods(IGameDiplomacy* gameDiplomacy, hks::lua_State* L, int stackOffset) {
        std::cout << "Hooked GameDiplomacy::PushMethods!\n";

        PushLuaMethod(L, lChangeGrievanceScore, "lChangeGrievanceScore", stackOffset, "ChangeGrievanceScore");

        base_GameDiplomacy_PushMethods(gameDiplomacy, L, stackOffset);
    }

    void Create() {
        using namespace Runtime;

        GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);
        ChangeGrievanceScore = GetGameCoreGlobalAt<Types::ChangeGrievanceScore>(CHANGE_GRIEVANCE_SCORE_OFFSET);

        orig_GameDiplomacy_PushMethods = GetGameCoreGlobalAt<Types::PushMethods>(PUSH_METHODS_OFFSET);
        CreateHook(orig_GameDiplomacy_PushMethods, &PushMethods, &base_GameDiplomacy_PushMethods);
    }
}
