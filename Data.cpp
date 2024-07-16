#include "Data.h"

namespace Data {
    void LuaVariant::push(hks::lua_State* L) const {
        std::visit([L](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::string>) {
                hks::pushfstring(L, arg.c_str());
            }
            else if constexpr (std::is_same_v<T, double>) {
                hks::pushnumber(L, arg);
            }
            else if constexpr (std::is_same_v<T, int>) {
                hks::pushinteger(L, arg);
            }
            }, *this);
    }

    void LuaVariantMap::rebuild(hks::lua_State* L) {
        for (auto& variantPair : *this) {
            hks::pushfstring(L, variantPair.first.c_str());
            hks::gettable(L, -2);

            std::visit([L, &variantPair](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, std::string>) {
                    size_t length;
                    variantPair.second = LuaVariant(std::string(hks::checklstring(L, -1, &length)));
                }
                else if constexpr (std::is_same_v<T, double>) {
                    variantPair.second = LuaVariant(hks::checknumber(L, -1));
                }
                else if constexpr (std::is_same_v<T, int>) {
                    variantPair.second = LuaVariant(hks::checkinteger(L, -1));
                }
                }, variantPair.second);

            hks::pop(L, 1);
        }
    }
}