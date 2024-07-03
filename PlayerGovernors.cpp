#include "PlayerGovernors.h"
#include "Runtime.h"

namespace PlayerGovernors {
    Types::PushMethods base_PushMethods;
    Types::PushMethods orig_PushMethods;

    Types::PromoteGovernor PromoteGovernor;
    Types::GetInstance GetInstance;

    int lPromoteGovernor(hks::lua_State* L) {
        void* governors = GetInstance(L, 1, true);
        int governorId = hks::checkinteger(L, 2);
        int governorPromotionIndex = hks::checkinteger(L, 3);

        hks::pushinteger(L, PromoteGovernor(governors, governorId, governorPromotionIndex));
        return 1;
    }

    void __cdecl PushMethods(IPlayerGovernors* playerGovernors, hks::lua_State* L, int stackOffset) {
        std::cout << "Hooked PlayerGovernors::PushMethods!\n";

        PushLuaMethod(L, lPromoteGovernor, "lPromoteGovernor", stackOffset, "PromoteGovernor");

        base_PushMethods(playerGovernors, L, stackOffset);
    }

    void Create() {
        using namespace Runtime;

        PromoteGovernor = GetGameCoreGlobalAt<Types::PromoteGovernor>(PROMOTE_GOVERNOR_OFFSET);
        GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);

        orig_PushMethods = GetGameCoreGlobalAt<Types::PushMethods>(PUSH_METHODS_OFFSET);
        CreateHook(orig_PushMethods, &PushMethods, &base_PushMethods);
    }
}
