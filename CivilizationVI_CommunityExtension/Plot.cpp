#include "Plot.h"
#include <set>
#include "Runtime.h"

namespace Plot {
    std::set<short*> lockedAppeals;

    ProxyTypes::PushMethods base_PushMethods;
    ProxyTypes::PushMethods orig_PushMethods;

    Types::SetAppeal base_SetAppeal;
    Types::SetAppeal orig_SetAppeal;
    
    Types::GetInstance GetInstance;

    void SetAppeal(Instance* plot, int appeal) {
        std::cout << "Hooked SetAppeal!\n";
        if (lockedAppeals.find((short*)((uintptr_t)plot + 0x4a)) == lockedAppeals.end()) {
            base_SetAppeal(plot, appeal);
        }
    }

    int lSetAppeal(hks::lua_State* L) {
        Instance* plot = GetInstance(L, 1, true);
        int appeal = hks::checkinteger(L, 2);
        std::cout << plot << ' ' << appeal << '\n';
        SetAppeal(plot, appeal);
        return 0;
    }

    int lLockAppeal(hks::lua_State* L) {
        Instance* plot = GetInstance(L, 1, true);
        bool setToLock = hks::toboolean(L, 2);
        short* appealAddress = (short*)((uintptr_t)plot + 0x4a);
        if (setToLock) {
            lockedAppeals.insert(appealAddress);
        }
        else {
            lockedAppeals.erase(appealAddress);
        }
        return 0;
    }

    static void PushMethods(hks::lua_State* L, int stackOffset) {
        std::cout << "Hooked Plot::PushMethods!\n";

        PushLuaMethod(L, lSetAppeal, "lSetAppeal", stackOffset, "SetAppeal");
        PushLuaMethod(L, lLockAppeal, "lLockAppeal", stackOffset, "LockAppeal");

        base_PushMethods(L, stackOffset);
    }

    void Create() {
        using namespace Runtime;

        GetInstance = GetGameCoreGlobalAt<Types::GetInstance>(GET_INSTANCE_OFFSET);

        lockedAppeals = {};
        orig_SetAppeal = GetGameCoreGlobalAt<Types::SetAppeal>(SET_APPEAL_OFFSET);
        CreateHook(orig_SetAppeal, &SetAppeal, &base_SetAppeal);

        orig_PushMethods = GetGameCoreGlobalAt<ProxyTypes::PushMethods>(PUSH_METHODS_OFFSET);
        CreateHook(orig_PushMethods, &PushMethods, &base_PushMethods);
    }
}
