#include "HavokScript.h"
#include "MinHook.h"
#include "Runtime.h"

namespace MemoryManipulation {
    enum FieldType {
        FIELD_BYTE,
        FIELD_SHORT,
        FIELD_UNSIGNED_SHORT,
        FIELD_INT,
        FIELD_UNSIGNED_INT,
        FIELD_LONG_LONG_INT,
        FIELD_UNSIGNED_LONG_LONG_INT,
        FIELD_CHAR,
        FIELD_FLOAT,
        FIELD_DOUBLE,
        FIELD_C_STRING,
        FIELD_BOOL
    };

    namespace {
        static int PushCValue(hks::lua_State* L, FieldType fieldType, uintptr_t address) {
            switch (fieldType) {
            case FIELD_BYTE: hks::hksi_lua_pushinteger(L, *(byte*)address); break;
            case FIELD_SHORT: hks::hksi_lua_pushinteger(L, *(short*)address); break;
            case FIELD_INT: hks::hksi_lua_pushinteger(L, *(int*)address); break;
            case FIELD_LONG_LONG_INT: hks::hksi_lua_pushinteger(L, *(long long int*)address); break;
            case FIELD_UNSIGNED_LONG_LONG_INT: hks::hksi_lua_pushinteger(L, *(unsigned long long int*)address); break;
            case FIELD_CHAR: hks::hksi_lua_pushinteger(L, *(char*)address); break;
            case FIELD_FLOAT: hks::hksi_lua_pushnumber(L, *(float*)address); break;
            case FIELD_DOUBLE: hks::hksi_lua_pushnumber(L, *(double*)address); break;
            case FIELD_C_STRING: hks::hksi_lua_pushfstring(L, *(char**)address); break;
            default: hks::hksi_luaL_error(L, "Invalid FieldType parameter was passed!"); return 0;
            }
            return 1;
        }

        static void SetCValue(hks::lua_State* L, FieldType memoryType, uintptr_t address, int index) {
            switch (memoryType) {
            case FIELD_BYTE: *(byte*)address = static_cast<byte>(hks::checkinteger(L, index)); break;
            case FIELD_SHORT: *(short*)address = static_cast<short>(hks::checkinteger(L, index)); break;
            case FIELD_INT: *(int*)address = static_cast<int>(hks::checkinteger(L, index)); break;
            case FIELD_LONG_LONG_INT: *(long long int*)address = static_cast<long long int>(hks::checkinteger(L, index)); break;
            case FIELD_UNSIGNED_LONG_LONG_INT: *(unsigned long long int*)address =
                static_cast<unsigned long long int>(hks::checkinteger(L, index)); break;
            case FIELD_CHAR: *(char*)address = static_cast<char>(hks::checkinteger(L, index)); break;
            case FIELD_FLOAT: *(float*)address = static_cast<float>(hks::checknumber(L, index)); break;
            case FIELD_DOUBLE: *(double*)address = static_cast<double>(hks::checknumber(L, index)); break;
            case FIELD_C_STRING: {
                size_t length;
                const char* inputString = hks::CheckLString(L, index, &length);
                char* newString = (char*)malloc(length + 1);
                if (!newString) {
                    hks::hksi_luaL_error(L, "String memory allocation failed");
                    return;
                }
                strcpy_s(newString, length, inputString);
                *(char**)address = newString;
                break;
            }
            default: hks::hksi_luaL_error(L, "Invalid MemoryType parameter was passed!");
            }
        }
    }

    namespace LuaExport {
        int lMem(hks::lua_State* L) {
            // Check for number because int too small to store x64 pointer
            uintptr_t address = static_cast<uintptr_t>(hks::checknumber(L, 1));
            auto fieldType = static_cast<FieldType>(hks::checkinteger(L, 2));
            if (hks::GetTop(L) == 3) {
                SetCValue(L, fieldType, Runtime::GameCoreAddress + address, 3);
                return 0;
            }
            return PushCValue(L, fieldType, Runtime::GameCoreAddress + address);
        }

        int lObjMem(hks::lua_State* L) {
            hks::hksi_lua_getfield(L, 1, "__instance");
            auto objectAddress = reinterpret_cast<uintptr_t>(hks::hksi_lua_touserdata(L, -1));
            if (objectAddress == NULL) {
                hks::hksi_luaL_error(L, "Failed to retrieve __instance field as userdata. Is your object NULL?");
                return 0;
            }
            hks::Pop(L, 1);
            uintptr_t offsetAddress = static_cast<uintptr_t>(hks::checkinteger(L, 2));
            auto memoryType = static_cast<FieldType>(hks::checkinteger(L, 3));
            if (hks::GetTop(L) == 4) {
                SetCValue(L, memoryType, objectAddress + offsetAddress, 4);
                return 0;
            }
            return PushCValue(L, memoryType, objectAddress + offsetAddress);
        }
    }
}