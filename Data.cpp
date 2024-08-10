#include "Data.h"

namespace Data {
    void LuaVariant::push(hks::lua_State* L) const {
        auto& variant = *this;

        if (std::holds_alternative<std::string>(variant)) {
            hks::pushfstring(L, std::get<std::string>(variant).c_str());
        }
        else if (std::holds_alternative<double>(variant)) {
            hks::pushnumber(L, std::get<double>(variant));
        }
        else if (std::holds_alternative<int>(variant)) {
            hks::pushinteger(L, std::get<int>(variant));
        }
    }

    static void assignVariant(hks::lua_State* L, LuaVariant& variant, const char* str) {
        hks::pushfstring(L, str);
        hks::gettable(L, -2);

        if (std::holds_alternative<int>(variant)) {
            auto value = hks::checkinteger(L, -1);
            std::get<int>(variant) = value;
        }
        else if (std::holds_alternative<std::string>(variant)) {
            size_t length = 0;
            auto value = hks::checklstring(L, -1, &length);
            std::get<std::string>(variant) = std::string(value, length);
        }
        else if (std::holds_alternative<double>(variant)) {
            auto value = hks::checknumber(L, -1);
            std::get<double>(variant) = value;
        }

        hks::pop(L, 1);
    }

    void LuaVariantMap::rebuild(hks::lua_State* L) {
        for (auto& variantPair : *this) {
            assignVariant(L, variantPair.second, variantPair.first.c_str());
        }
    }

    void LuaVariantMap::reclaim(hks::lua_State* L, const std::string& propertyToGet) {
        auto& atValue = this->at(propertyToGet);
        assignVariant(L, atValue, propertyToGet.c_str());
    }
}