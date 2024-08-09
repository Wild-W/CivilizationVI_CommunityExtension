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

    static void assignVariant(hks::lua_State* L, LuaVariant& variant) {
        if (std::holds_alternative<std::string>(variant)) {
            size_t length = 0;
            auto value = hks::checklstring(L, -1, &length);
            std::get<std::string>(variant) = std::string(value, length);
        }
        else if (std::holds_alternative<double>(variant)) {
            auto value = hks::checknumber(L, -1);
            std::get<double>(variant) = value;
        }
        else if (std::holds_alternative<int>(variant)) {
            auto value = hks::checkinteger(L, -1);
            std::get<int>(variant) = value;
        }
    }

    void LuaVariantMap::rebuild(hks::lua_State* L) {
        for (auto& variantPair : *this) {
            hks::pushfstring(L, variantPair.first.c_str());
            hks::gettable(L, -2);

            assignVariant(L, variantPair.second);

            hks::pop(L, 1);
        }
    }

    void LuaVariantMap::reclaim(hks::lua_State* L, const std::string& propertyToGet) {
        hks::pushfstring(L, propertyToGet.c_str());
        hks::gettable(L, -2);

        auto& atValue = this->at(propertyToGet);
        assignVariant(L, atValue);

        hks::pop(L, 1);
    }
}