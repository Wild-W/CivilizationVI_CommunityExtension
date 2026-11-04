#include "Plot.h"
#include <set>
#include "Runtime.h"

namespace Plot {
    std::set<int> lockedPlots; // plot ids of plots with locked appeal

    ProxyTypes::PushMethods base_PushMethods;
    ProxyTypes::PushMethods orig_PushMethods;

    Types::SetAppeal base_SetAppeal;
    Types::SetAppeal orig_SetAppeal;
    
    Types::GetInstance GetInstance;
    Types::GetIndex GetIndex;

    void SetAppeal(Instance* plot, int appeal) {
        int id = GetIndex(plot);
        if (lockedPlots.find(id) == lockedPlots.end()) {
            base_SetAppeal(plot, appeal);
        }
    }

    int lSetAppeal(hks::lua_State* L) {
        Instance* plot = GetInstance(L, 1, true);
        int appeal = hks::checkinteger(L, 2);
        Runtime::MainLogger->Info("Plot with instance addr {}'s appeal is being set to {}",
            reinterpret_cast<uintptr_t>(plot), appeal);
        SetAppeal(plot, appeal);
        return 0;
    }

    int lLockAppeal(hks::lua_State* L) {
        Instance* plot = GetInstance(L, 1, true);
        bool setToLock = hks::toboolean(L, 2);

        int plotId = GetIndex(plot);

        if (setToLock) {
            lockedPlots.insert(plotId);
        }
        else {
            lockedPlots.erase(plotId);
        }
        return 0;
    }

    static void PushMethods(hks::lua_State* L, int stackOffset) {
        Runtime::MainLogger->Info("Hooked Plot::PushMethods!");

        PushLuaMethod(L, lSetAppeal, "lSetAppeal", stackOffset, "SetAppeal");
        PushLuaMethod(L, lLockAppeal, "lLockAppeal", stackOffset, "LockAppeal");

        base_PushMethods(L, stackOffset);
    }

    void Create() {
        using namespace Runtime;

        GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);
        GetIndex = GetGameCoreGlobalAt<Types::GetIndex>(GET_INDEX_OFFSET);

        lockedPlots = {};
        orig_SetAppeal = GetGameCoreGlobalAt<Types::SetAppeal>(SET_APPEAL_OFFSET);
        CreateHook(orig_SetAppeal, &SetAppeal, &base_SetAppeal);

        orig_PushMethods = GetGameCoreGlobalAt<ProxyTypes::PushMethods>(PUSH_METHODS_OFFSET);
        CreateHook(orig_PushMethods, &PushMethods, &base_PushMethods);
    }
}
