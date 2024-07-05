#include "PlayerGovernors.h"
#include "Runtime.h"

namespace PlayerGovernors {
    Types::PushMethods base_PushMethods;
    Types::PushMethods orig_PushMethods;

    Types::PromoteGovernor PromoteGovernor;
    Types::GetInstance GetInstance;
    Types::GetGovernor GetGovernor;
    Types::NeutralizeGovernor NeutralizeGovernor;
    Types::UnassignGovernor UnassignGovernor;

    int lPromoteGovernor(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int governorIndex = hks::checkinteger(L, 2);
        int governorPromotionIndex = hks::checkinteger(L, 3);

        hks::pushboolean(L, PromoteGovernor(governors, governorIndex, governorPromotionIndex));
        return 1;
    }

    int lNeutralizeGovernor(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int governorIndex = hks::checkinteger(L, 2);
        int neutralizedTurns = hks::checkinteger(L, 3);

        Governor::Instance* governor = GetGovernor(governors, governorIndex);

        NeutralizeGovernor(governors, governor, neutralizedTurns);
        return 0;
    }

    int lChangeNeutralizedTurns(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int governorIndex = hks::checkinteger(L, 2);
        int neutralizedTurns = hks::checkinteger(L, 3);

        Governor::Instance* governor = GetGovernor(governors, governorIndex);

        ChangeNeutralizedTurns(governor, neutralizedTurns);
        return 0;
    }

    int lUnassignGovernor(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int governorIndex = hks::checkinteger(L, 2);
        bool unknown1 = hks::toboolean(L, 3);
        bool unknown2 = hks::toboolean(L, 4);

        Governor::Instance* governor = GetGovernor(governors, governorIndex);

        UnassignGovernor(governors, governor, unknown1, unknown2);
        return 0;
    }

    void __cdecl PushMethods(IPlayerGovernors* playerGovernors, hks::lua_State* L, int stackOffset) {
        std::cout << "Hooked PlayerGovernors::PushMethods!\n";

        PushLuaMethod(L, lPromoteGovernor, "lPromoteGovernor", stackOffset, "PromoteGovernor");
        PushLuaMethod(L, lNeutralizeGovernor, "lNeutralizeGovernor", stackOffset, "NeutralizeGovernor");
        PushLuaMethod(L, lChangeNeutralizedTurns, "lChangeNeutralizedTurns", stackOffset, "ChangeNeutralizedTurns");
        PushLuaMethod(L, lUnassignGovernor, "lUnassignGovernor", stackOffset, "UnassignGovernor");

        base_PushMethods(playerGovernors, L, stackOffset);
    }

    void Create() {
        using namespace Runtime;

        PromoteGovernor = GetGameCoreGlobalAt<Types::PromoteGovernor>(PROMOTE_GOVERNOR_OFFSET);
        GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);
        GetGovernor = GetGameCoreGlobalAt<Types::GetGovernor>(GET_GOVERNOR_OFFSET);
        NeutralizeGovernor = GetGameCoreGlobalAt<Types::NeutralizeGovernor>(NEUTRALIZE_GOVERNOR_OFFSET);
        UnassignGovernor = GetGameCoreGlobalAt<Types::UnassignGovernor>(UNASSIGN_GOVERNOR_OFFSET);

        orig_PushMethods = GetGameCoreGlobalAt<Types::PushMethods>(PUSH_METHODS_OFFSET);
        CreateHook(orig_PushMethods, &PushMethods, &base_PushMethods);
    }
}
