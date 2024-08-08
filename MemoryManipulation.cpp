#include "HavokScript.h"
#include "MinHook.h"
#include "Runtime.h"
#include <vector>
#include <sstream>
#include <unordered_map>
#include <stack>
#include "plh/plh_Hook.h"
#include "trampoline_common.h"

namespace MemoryManipulation {
    enum FieldType {
        FIELD_BYTE,
        FIELD_SHORT,
        FIELD_UNSIGNED_SHORT,
        FIELD_INT,
        FIELD_UNSIGNED_INT,
        FIELD_LONG_LONG,
        FIELD_UNSIGNED_LONG_LONG,
        FIELD_CHAR,
        FIELD_FLOAT,
        FIELD_DOUBLE,
        FIELD_C_STRING,
        FIELD_BOOL,
        FIELD_POINTER,
        _FIELD_TYPE_COUNT_
    };

    struct LuaCallback {
        hks::lua_State* L;
        int registryIndex;
        const std::vector<FieldType> parameters;
    };

    std::unordered_map<uintptr_t, std::vector<LuaCallback>> callbackMap;

    static bool isExecutable(void* address) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery(address, &mbi, sizeof(mbi))) {
            return (mbi.Protect & PAGE_EXECUTE) ||
                (mbi.Protect & PAGE_EXECUTE_READ) ||
                (mbi.Protect & PAGE_EXECUTE_READWRITE) ||
                (mbi.Protect & PAGE_EXECUTE_WRITECOPY);
        }
        return false;
    }

    static int PushCValue(hks::lua_State* L, FieldType fieldType, uintptr_t address) {
        switch (fieldType) {
        case FIELD_BYTE: hks::pushinteger(L, *(byte*)address); break;
        case FIELD_SHORT: hks::pushinteger(L, *(short*)address); break;
        case FIELD_UNSIGNED_SHORT: hks::pushinteger(L, *(unsigned short*)address); break;
        case FIELD_INT: hks::pushinteger(L, *(int*)address); break;
        case FIELD_UNSIGNED_INT: hks::pushnumber(L, static_cast<double>(*(unsigned int*)address)); break;
        case FIELD_LONG_LONG: hks::pushnumber(L, static_cast<double>(*(long long int*)address)); break;
        case FIELD_POINTER: hks::pushlightuserdata(L, *(void**)address); break;
        case FIELD_UNSIGNED_LONG_LONG: hks::pushnumber(L, static_cast<double>(*(unsigned long long int*)address)); break;
        case FIELD_CHAR: hks::pushinteger(L, *(char*)address); break;
        case FIELD_FLOAT: hks::pushnumber(L, *(float*)address); break;
        case FIELD_DOUBLE: hks::pushnumber(L, *(double*)address); break;
        case FIELD_C_STRING: hks::pushfstring(L, *(char**)address); break;
        case FIELD_BOOL: hks::pushboolean(L, *(bool*)address); break;
        default: hks::error(L, "Invalid FieldType parameter was passed!"); return 0;
        }
        return 1;
    }

    static void SetCValue(hks::lua_State* L, FieldType memoryType, uintptr_t address, int index) {
        if (isExecutable((void*)address)) {
            // User is trying to overwrite executable data! No good!
            std::stringstream errorStream;
            errorStream << "Could not write to executable address 0x" << std::hex << address << '!';
            hks::error(L, errorStream.str().c_str());
            return;
        }

        switch (memoryType) {
        case FIELD_BYTE: *(byte*)address = static_cast<byte>(hks::checkinteger(L, index)); break;
        case FIELD_SHORT: *(short*)address = static_cast<short>(hks::checkinteger(L, index)); break;
        case FIELD_UNSIGNED_SHORT: *(unsigned short*)address = static_cast<unsigned short>(hks::checkinteger(L, index)); break;
        case FIELD_INT: *(int*)address = hks::checkinteger(L, index); break;
        case FIELD_UNSIGNED_INT: *(unsigned int*)address = static_cast<unsigned int>(hks::checkinteger(L, index)); break;
        case FIELD_LONG_LONG: *(long long int*)address = static_cast<long long int>(hks::checkinteger(L, index)); break;
        case FIELD_POINTER: {
            if (!hks::isuserdata(L, index)) {
                hks::error(L, "Type mismatch: value is not userdata!");
                return;
            }
            *(uintptr_t*)address = reinterpret_cast<uintptr_t>(hks::touserdata(L, index));
            break;
        }
        case FIELD_UNSIGNED_LONG_LONG: *(unsigned long long int*)address = static_cast<unsigned long long int>(hks::checknumber(L, index)); break;
        case FIELD_CHAR: *(char*)address = static_cast<char>(hks::checkinteger(L, index)); break;
        case FIELD_FLOAT: *(float*)address = static_cast<float>(hks::checknumber(L, index)); break;
        case FIELD_DOUBLE: *(double*)address = hks::checknumber(L, index); break;
        case FIELD_C_STRING: {
            size_t length;
            const char* inputString = hks::checklstring(L, index, &length);
            char* newString = (char*)malloc(length + 1);
            if (!newString) {
                hks::error(L, "String memory allocation failed!");
                return;
            }
            strcpy_s(newString, length + 1, inputString);
            newString[length] = '\0';

            *(char**)address = newString;
            break;
        }
        case FIELD_BOOL: *(bool*)address = hks::toboolean(L, index); break;
        default: hks::error(L, "Invalid FieldType parameter was passed!");
        }
    }

    static plh::HookAction callEventEnter(void* targetFunc, void* callbackParam, size_t frameBase) {
        printf("callEventEnter(func: %p, param: %p, frame: %p)\n", targetFunc, callbackParam, (void*)frameBase);
        std::cout << "here!!!!!\n";

        //plh::VaList va;
        //plh::vaStart(va, frameBase);
        //
        //const auto& callbacks = callbackMap[(uintptr_t)targetFunc];
        //for (const auto& callback : callbacks) {
        //    for (int i = 0; i < frameBase; i++) {
        //
        //    }
        //}

        return plh::HookAction_JumpTarget;
    }

    thread_local std::stack<uint64_t> hookJumpAddresses;

    static void PushAddress(uint64_t addr) //push the address of the jump target
    {
        hookJumpAddresses.push(addr);
    }

    static void InstallLuaHook(void* func2hook) {
        SetOtherThreadsSuspended(true);

        DWORD oldProtect;
        VirtualProtect(func2hook, 1024, PAGE_EXECUTE_READWRITE, &oldProtect);

        //102 is the size of the "pre-payload" instructions that are written below
        //the trampoline will be located after these instructions in memory
        void* hookMemory = AllocatePageNearAddress(func2hook);

        uint32_t trampolineSize = BuildTrampoline(func2hook, (void*)((char*)hookMemory + 102));

        uint8_t* memoryIter = (uint8_t*)hookMemory;
        uint64_t trampolineAddress = (uint64_t)(memoryIter)+102;

        plh::HookArena arena;
        plh::Hook* hook = arena.allocate(func2hook, (void*)0xabcdef, callEventEnter, NULL);
        plh::enableHooks();

        memoryIter += WriteSaveArgumentRegisters(memoryIter);
        memoryIter += WriteMovToRCX(memoryIter, trampolineAddress);
        memoryIter += WriteSubRSP32(memoryIter); //allocate home space for function call
        memoryIter += WriteAbsoluteCall64(memoryIter, &PushAddress);
        memoryIter += WriteAddRSP32(memoryIter);
        memoryIter += WriteRestoreArgumentRegisters(memoryIter);
        memoryIter += WriteAbsoluteJump64(memoryIter, hook);

        //create the relay function
        void* relayFuncMemory = memoryIter + trampolineSize;
        WriteAbsoluteJump64(relayFuncMemory, hookMemory); //write relay func instructions

        //install the hook
        uint8_t jmpInstruction[5] = { 0xE9, 0x0, 0x0, 0x0, 0x0 };
        const int32_t relAddr = int32_t((int64_t)relayFuncMemory - ((int64_t)func2hook + sizeof(jmpInstruction)));
        memcpy(jmpInstruction + 1, &relAddr, 4);
        memcpy(func2hook, jmpInstruction, sizeof(jmpInstruction));

        std::cout << "hook: " << hook << "\nrelay: " << relayFuncMemory << "\n";

        SetOtherThreadsSuspended(false);
    }

    static void RegisterCallEvent(hks::lua_State* L, uintptr_t address, const std::vector<FieldType>& parameters, int luaCallbackIndex) {
        // If hook doesn't already exist
        if (!callbackMap.count(address)) {
            InstallLuaHook((void*)address);
        }
        callbackMap[address].push_back(LuaCallback{ L, luaCallbackIndex, parameters });
    }

    namespace LuaExport {
        int lMem(hks::lua_State* L) {
            // Check for number because int too small to store x64 pointer
            uintptr_t address = static_cast<uintptr_t>(hks::checknumber(L, 1));
            auto fieldType = static_cast<FieldType>(hks::checkinteger(L, 2));
            if (hks::gettop(L) == 3) {
                SetCValue(L, fieldType, Runtime::GameCoreAddress + address, 3);
                return 0;
            }
            return PushCValue(L, fieldType, Runtime::GameCoreAddress + address);
        }

        int lObjMem(hks::lua_State* L) {
            uintptr_t objectAddress;
            // Unsafe
            // if (hks::isnumber(L, 1)) {
            //     objectAddress = static_cast<uintptr_t>(hks::tonumber(L, 1));
            // }
            if (hks::isuserdata(L, 1)) {
                objectAddress = reinterpret_cast<uintptr_t>(hks::touserdata(L, 1));
            }
            else {
                hks::getfield(L, 1, "__instance");
                objectAddress = reinterpret_cast<uintptr_t>(hks::touserdata(L, -1));
                if (objectAddress == NULL) {
                    hks::error(L, "Failed to retrieve __instance field as userdata. Is your object NULL?");
                    return 0;
                }
                hks::pop(L, 1);
            }
            uintptr_t offsetAddress = static_cast<uintptr_t>(hks::checkinteger(L, 2));
            auto memoryType = static_cast<FieldType>(hks::checkinteger(L, 3));
            if (hks::gettop(L) == 4) {
                SetCValue(L, memoryType, objectAddress + offsetAddress, 4);
                return 0;
            }
            return PushCValue(L, memoryType, objectAddress + offsetAddress);
        }

        int lRegisterCallEvent(hks::lua_State* L) {
            hks::pushvalue(L, 1);
            int callbackIndex = hks::ref(L, hks::LUA_REGISTRYINDEX);
            std::cout << "Function from lua: " << callbackIndex << '\n';
            uintptr_t address = static_cast<uintptr_t>(hks::checknumber(L, 2));

            int parametersLength = hks::objlen(L, 3);
            std::cout << "parameter count: " << parametersLength << '\n';
            std::vector<FieldType> parameters;
            for (int i = 1; i <= parametersLength; i++) {
                hks::pushinteger(L, i);
                hks::gettable(L, 3);

                int fieldType = hks::checkinteger(L, -1);
                std::cout << fieldType << '\n';
                if (fieldType < FIELD_BYTE || fieldType >= _FIELD_TYPE_COUNT_) {
                    hks::error(L, "Invalid FieldType parameter was passed!");
                    return 0;
                }

                parameters.push_back((FieldType)fieldType);
                hks::pop(L, 1);
            }

            RegisterCallEvent(L, Runtime::GameCoreAddress + address, parameters, callbackIndex);
            return 0;
        }

        void PushFieldTypes(hks::lua_State* L) {
            std::cout << "Pushing Field Types!\n";

            hks::pushinteger(L, FIELD_BYTE);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_BYTE");

            hks::pushinteger(L, FIELD_SHORT);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_SHORT");

            hks::pushinteger(L, FIELD_UNSIGNED_SHORT);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_UNSIGNED_SHORT");

            hks::pushinteger(L, FIELD_INT);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_INT");

            hks::pushinteger(L, FIELD_UNSIGNED_INT);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_UNSIGNED_INT");

            hks::pushinteger(L, FIELD_LONG_LONG);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_LONG_LONG");

            hks::pushinteger(L, FIELD_UNSIGNED_LONG_LONG);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_UNSIGNED_LONG_LONG");

            hks::pushinteger(L, FIELD_CHAR);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_CHAR");

            hks::pushinteger(L, FIELD_FLOAT);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_FLOAT");

            hks::pushinteger(L, FIELD_DOUBLE);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_DOUBLE");

            hks::pushinteger(L, FIELD_C_STRING);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_C_STRING");

            hks::pushinteger(L, FIELD_BOOL);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_BOOL");

            hks::pushinteger(L, FIELD_POINTER);
            hks::setfield(L, hks::LUA_GLOBAL, "FIELD_POINTER");
        }
    }
}