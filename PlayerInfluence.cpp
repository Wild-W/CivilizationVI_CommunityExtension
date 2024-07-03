#include "PlayerInfluence.h"
#include "Runtime.h"

namespace PlayerInfluence {
    ProxyTypes::PushMethods base_PushMethods;
    ProxyTypes::PushMethods orig_PushMethods;

    Types::GetInstance GetInstance;
    Types::SetTokensToGive SetTokensToGive;

    int lSetTokensToGive(hks::lua_State* L) {
        Influence* influence = GetInstance(L, 1, true);
        int tokens = hks::checkinteger(L, 2);

        SetTokensToGive(influence, tokens);
        return 0;
    }

    int lSetPoints(hks::lua_State* L) {
        Influence* influence = GetInstance(L, 1, true);
        double points = hks::checknumber(L, 2);

        *(unsigned int*)((uintptr_t)influence + 0xb8) = static_cast<unsigned int>(std::round(points * 256.0));
        return 0;
    }

    int lAdjustPoints(hks::lua_State* L) {
        void* influence = GetInstance(L, 1, true);
        double amountPoints = hks::checknumber(L, 2);

        *(unsigned int*)((uintptr_t)influence + 0xb8) += static_cast<unsigned int>(std::round(amountPoints * 256.0));
        return 0;
    }

    void __cdecl PushMethods(hks::lua_State* L, int stackOffset) {
        std::cout << "Hooked Influence::PushMethods!\n";

        PushLuaMethod(L, lSetTokensToGive, "lSetTokensToGive", stackOffset, "SetTokensToGive");
        PushLuaMethod(L, lSetPoints, "lSetPoints", stackOffset, "SetPoints");
        PushLuaMethod(L, lAdjustPoints, "lAdjustPoints", stackOffset, "AdjustPoints");

        base_PushMethods(L, stackOffset);
    }

    void Create() {
        using namespace Runtime;

        GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);
        SetTokensToGive = GetGameCoreGlobalAt<Types::SetTokensToGive>(SET_TOKENS_TO_GIVE_OFFSET);

        orig_PushMethods = GetGameCoreGlobalAt<ProxyTypes::PushMethods>(PUSH_METHODS_OFFSET);
        CreateHook(orig_PushMethods, &PushMethods, &base_PushMethods);
    }
}