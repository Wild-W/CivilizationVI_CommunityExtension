#include "PlayerGovernors.h"
#include "Runtime.h"
#include "Game.h"
#include <unordered_map>

namespace PlayerGovernors {
    Types::PushMethods base_PushMethods;
    Types::PushMethods orig_PushMethods;

    Types::GetTurnsToEstablish base_GetTurnsToEstablish;
    Types::GetTurnsToEstablish orig_GetTurnsToEstablish;

    Types::PromoteGovernor PromoteGovernor;
    Types::GetInstance GetInstance;
    Types::GetGovernor GetGovernor;
    Types::NeutralizeGovernor NeutralizeGovernor;
    Types::UnassignGovernor UnassignGovernor;
    Types::ChangeNeutralizedTurns ChangeNeutralizedTurns;
    Types::ChangeNeutralizedIndefinitely ChangeNeutralizedIndefinitely;

    std::unordered_map<Governors*, std::unordered_map<int, int>> governorsTurnsToEstablishDelay = {};
    
    namespace Cache {
        Cache::Types::GetTurnsToEstablish base_GetTurnsToEstablish;
        Cache::Types::GetTurnsToEstablish orig_GetTurnsToEstablish;

        int GetTurnsToEstablish(Cache::Governors* governors, int governorIndex) {
            std::cout << "Hooked CacheGovernors::GetTurnsToEstablish!\n" << governorIndex << ' ' << GetTurnsToEstablishDelay(governors, governorIndex) << '\n';
            return Cache::base_GetTurnsToEstablish(governors, governorIndex) + GetTurnsToEstablishDelay(governors, governorIndex);
        }
    }

    int GetTurnsToEstablishDelay(Governors* governors, int governorHash) {
        int indexChangeResult = 0;
        auto governorsIterator = governorsTurnsToEstablishDelay.find(governors);
        if (governorsIterator != governorsTurnsToEstablishDelay.end()) {
            auto indexIterator = governorsIterator->second.find(governorHash);
            if (indexIterator != governorsIterator->second.end()) {
                indexChangeResult = indexIterator->second;
            }
        }

        return indexChangeResult;
    }

    // Hook
    int GetTurnsToEstablish(Governors* governors, int governorIndex) {
        std::cout << "Hooked Governors::GetTurnsToEstablish!\n" << governorIndex << ' ' << GetTurnsToEstablishDelay(governors, governorIndex) << '\n';
        return base_GetTurnsToEstablish(governors, governorIndex) + GetTurnsToEstablishDelay(governors, governorIndex);
    }

    void SetTurnsToEstablishDelay(Governors* governors, int governorHash, int amount) {
        governorsTurnsToEstablishDelay[governors][governorHash] = amount;
    }

    void ChangeTurnsToEstablishDelay(Governors* governors, int governorHash, int amount) {
        governorsTurnsToEstablishDelay[governors][governorHash] += amount;
    }

    int GetNeutralizedIndefinitely(Governors* governors) {
        int* neutralizedCount = (int*)Game::FAutoVariable_edit((void*)((uintptr_t)governors + 0x120));
        return *neutralizedCount;
    }


    int lChangeTurnsToEstablishDelay(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int governorHash = hks::checkinteger(L, 2);
        int amount = hks::checkinteger(L, 3);

        ChangeTurnsToEstablishDelay(governors, governorHash, amount);
        return 0;
    }

    int lSetTurnsToEstablishDelay(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int governorHash = hks::checkinteger(L, 2);
        int amount = hks::checkinteger(L, 3);

        SetTurnsToEstablishDelay(governors, governorHash, amount);
        return 0;
    }

    int lGetTurnsToEstablishDelay(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int governorHash = hks::checkinteger(L, 2);

        hks::pushinteger(L, GetTurnsToEstablishDelay(governors, governorHash));
        return 1;
    }

    int lGetNeutralizedIndefinitely(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int governorIndex = hks::checkinteger(L, 2);

        hks::pushinteger(L, GetNeutralizedIndefinitely(governors));
        return 1;
    }

    int lChangeNeutralizedIndefinitely(hks::lua_State* L) {
        Governors* governors = GetInstance(L, 1, true);
        int amount = hks::checkinteger(L, 2);

        ChangeNeutralizedIndefinitely(governors, amount);
        return 0;
    }

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
        PushLuaMethod(L, lChangeNeutralizedIndefinitely, "lChangeNeutralizedIndefinitely", stackOffset, "ChangeNeutralizedIndefinitely");
        PushLuaMethod(L, lGetNeutralizedIndefinitely, "lGetNeutralizedIndefinitely", stackOffset, "GetNeutralizedIndefinitely");
        PushLuaMethod(L, lChangeTurnsToEstablishDelay, "lChangeTurnsToEstablishDelay", stackOffset, "ChangeTurnsToEstablishDelay");
        PushLuaMethod(L, lGetTurnsToEstablishDelay, "lGetTurnsToEstablishDelay", stackOffset, "GetTurnsToEstablishDelay");
        PushLuaMethod(L, lSetTurnsToEstablishDelay, "lSetTurnsToEstablishDelay", stackOffset, "SetTurnsToEstablishDelay");

        base_PushMethods(playerGovernors, L, stackOffset);
    }

    void Create() {
        using namespace Runtime;

        PromoteGovernor = GetGameCoreGlobalAt<Types::PromoteGovernor>(PROMOTE_GOVERNOR_OFFSET);
        GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);
        GetGovernor = GetGameCoreGlobalAt<Types::GetGovernor>(GET_GOVERNOR_OFFSET);
        NeutralizeGovernor = GetGameCoreGlobalAt<Types::NeutralizeGovernor>(NEUTRALIZE_GOVERNOR_OFFSET);
        UnassignGovernor = GetGameCoreGlobalAt<Types::UnassignGovernor>(UNASSIGN_GOVERNOR_OFFSET);
        ChangeNeutralizedIndefinitely = GetGameCoreGlobalAt<Types::ChangeNeutralizedIndefinitely>(CHANGE_NEUTRALIZED_INDEFINITELY_OFFSET);

        orig_PushMethods = GetGameCoreGlobalAt<Types::PushMethods>(PUSH_METHODS_OFFSET);
        CreateHook(orig_PushMethods, &PushMethods, &base_PushMethods);

        orig_GetTurnsToEstablish = GetGameCoreGlobalAt<Types::GetTurnsToEstablish>(GET_TURNS_TO_ESTABLISH_OFFSET);
        CreateHook(orig_GetTurnsToEstablish, &GetTurnsToEstablish, &base_GetTurnsToEstablish);

        // Cache::orig_GetTurnsToEstablish = GetGameCoreGlobalAt<Cache::Types::GetTurnsToEstablish>(CACHE_GET_TURNS_TO_ESTABLISH_OFFSET);
        // CreateHook(Cache::orig_GetTurnsToEstablish, &Cache::GetTurnsToEstablish, &Cache::base_GetTurnsToEstablish);
    }
}
