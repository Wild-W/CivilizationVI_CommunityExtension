#include "Plot.h"
#include <set>

namespace Plot {
    std::set<short*> lockedAppeals;

    void __cdecl Hook_SetAppeal(Instance* plot, int appeal) {
        std::cout << "Hooked SetAppeal!\n";
        if (lockedAppeals.find((short*)((uintptr_t)plot + 0x4a)) == lockedAppeals.end()) {
            base_SetAppeal(plot, appeal);
        }
    }

    int lSetAppeal(hks::lua_State* L) {
        Instance* plot = GetInstance(L, 1, true);
        int appeal = hks::checkinteger(L, 2);
        std::cout << plot << ' ' << appeal << '\n';
        Hook_SetAppeal(plot, appeal);
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
}
