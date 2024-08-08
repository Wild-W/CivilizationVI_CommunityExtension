#include "HavokScript.h"
#include "MinHook.h"
#include "Runtime.h"
#include <vector>
#include <sstream>
#include "asmjit/x86.h"
#include "capstone/capstone.h"
#include <mutex>

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

    namespace {
        const asmjit::x86::Reg& GetRegister(FieldType fieldType, int index) {
            using namespace asmjit;

            switch (fieldType) {
            // Integer registers
            case FIELD_BYTE:
            case FIELD_SHORT:
            case FIELD_UNSIGNED_SHORT:
            case FIELD_INT:
            case FIELD_UNSIGNED_INT:
            case FIELD_LONG_LONG:
            case FIELD_POINTER:
            case FIELD_UNSIGNED_LONG_LONG:
            case FIELD_CHAR:
            case FIELD_C_STRING:
            case FIELD_BOOL: {
                switch (index) {
                case 0: return x86::rcx;
                case 1: return x86::rdx;
                case 2: return x86::r8;
                case 3: return x86::r9;
                }
                break;
            }

            // Floating point registers
            case FIELD_FLOAT:
            case FIELD_DOUBLE:
                switch (index) {
                case 0: return x86::xmm0;
                case 1: return x86::xmm1;
                case 2: return x86::xmm2;
                case 3: return x86::xmm3;
                }
            }
        }

        bool isExecutable(void* address) {
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

        void* CreateTrampoline(const byte* source, size_t size) {
            std::cout << "source and size: " << source << ' ' << size << '\n';
            // Allocate executable memory
            void* trampolineAddress = VirtualAlloc(NULL, size + 5, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (!trampolineAddress) {
                std::cout << "Failed to allocate memory\n";
                return nullptr;
            }

            // Copy the code to the newly allocated memory
            memcpy(trampolineAddress, source, size);

            // Calculate the address to jump back to
            byte* jumpSrc = (byte*)trampolineAddress + size;
            intptr_t jumpTarget = (intptr_t)(source + size);
            intptr_t relativeOffset = jumpTarget - (intptr_t)(jumpSrc + 5);

            // Add a jump back to the original function after the copied bytes
            *jumpSrc = 0xE9; // JMP opcode
            *(DWORD*)(jumpSrc + 1) = (DWORD)relativeOffset;

            // Change memory protection to execute/read
            DWORD oldProtect;
            if (!VirtualProtect(trampolineAddress, size + 5, PAGE_EXECUTE_READ, &oldProtect)) {
                std::cout << "Failed to change memory protection\n";
                VirtualFree(trampolineAddress, 0, MEM_RELEASE);
                return nullptr;
            }

            return trampolineAddress;
        }

        void PushDynamicValue(asmjit::x86::Assembler* a, std::pair<asmjit::x86::Reg, FieldType> pair) {
            using namespace asmjit;

            if (pair.second == FIELD_FLOAT || pair.second == FIELD_DOUBLE) {
                // Allocate space on the stack
                a->sub(x86::rsp, 8);
                // Move value onto the stack
                a->movaps(x86::ptr(x86::rsp), pair.first.as<x86::Xmm>());

                return;
            }

            a->push(pair.first.as<x86::Gp>());
        }

        void PopDynamicValue(asmjit::x86::Assembler* a, std::pair<asmjit::x86::Reg, FieldType> pair) {
            using namespace asmjit;

            if (pair.second == FIELD_FLOAT || pair.second == FIELD_DOUBLE) {
                // Move value from the stack into Xmm register
                a->movaps(pair.first.as<x86::Xmm>(), x86::ptr(x86::rsp));
                // Reset stack pointer
                a->add(x86::rsp, 8);

                return;
            }

            a->pop(pair.first.as<x86::Gp>());
        }

        void* CreateLuaCallback(hks::lua_State* L, std::vector<std::pair<asmjit::x86::Reg, FieldType>> regs, void* trampoline, int luaCallbackIndex) {
            using namespace asmjit;

            CodeHolder code;
            code.init(Runtime::Jit.environment(), Runtime::Jit.cpuFeatures());

            x86::Assembler a(&code);

            FileLogger logger(stdout);
            code.setLogger(&logger);

            // Begin constructing the function
            // Function prologue
            a.push(x86::rbp);
            a.mov(x86::rbp, x86::rsp);

            // Backup the registers in prologue in ascending order
            int i;
            for (i = 0; i < regs.size(); i++) {
                PushDynamicValue(&a, regs[i]);
            }

            // Backup the registers to prepare for pushing arguments onto the lua stack in descending order
            for (i = regs.size() - 1; i >= 0; i--) {
                PushDynamicValue(&a, regs[i]);
            }

            // Put lua_State* in parameter 1
            a.mov(x86::rcx, L);

            // Get the callback function
            a.mov(x86::rdx, hks::LUA_REGISTRYINDEX);
            a.mov(x86::r8, luaCallbackIndex);
            a.call(hks::rawgeti);

            for (const auto& pair : regs) {
                switch (pair.second) {
                case FIELD_FLOAT:
                case FIELD_DOUBLE:
                    // Put floating point number in parameter 2
                    a.movaps(x86::xmm1, pair.first.as<x86::Xmm>());
                    a.call(hks::pushnumber);
                    break;

                case FIELD_BOOL:
                    a.pop(x86::rdx);
                    a.call(hks::pushboolean);
                    break;

                case FIELD_BYTE:
                case FIELD_SHORT:
                case FIELD_INT:
                    a.pop(x86::rdx);
                    a.call(hks::pushinteger);
                    break;

                case FIELD_LONG_LONG:
                case FIELD_UNSIGNED_LONG_LONG:
                case FIELD_POINTER:
                case FIELD_C_STRING:
                    // Put 64-bit integer into rax
                    a.pop(x86::rax);
                    // Move the 64-bit integer from rax into the second parameter (xmm1)
                    a.movq(x86::xmm1, x86::rax);
                    a.call(hks::pushnumber);
                    break;

                default: hks::error(L, "Unimplemented!!!!"); std::cout << pair.second << '\n';
                }
            }

            // Call lua function
            a.mov(x86::rdx, regs.size());
            a.xor_(x86::r8, x86::r8);
            a.xor_(x86::r9, x86::r9);
            a.call(hks::pcall);

            // Function epilogue
            // Restore prologue backups in opposite order
            for (i = regs.size() - 1; i >= 0; i--) {
                PopDynamicValue(&a, regs[i]);
            }

            a.mov(x86::rsp, x86::rbp);
            a.pop(x86::rbp);
            a.jmp(trampoline);

            void* func;
            Error err = Runtime::Jit.add(&func, &code);
            if (err) {
                hks::error(L, "Failed to create the hook function!");
                return nullptr;
            }

            return func;
        }

        /// Breakdown of what the hell this does:
        /// 1: Disassembles target function
        /// 2: Copies the shortest number of bytes that can hold a 5 byte jump without splitting instructions
        /// 3: Creates a trampoline function with the copied bytes
        /// 4: Appends the trampoline function with a jump to [target function + the number of bytes that were copied]
        /// 5: Assembles a hook function that captures the values of the target function's first 4 or less parameters and passes them to a function in the lua state
        /// 6: Appends the hook function with a jump to the trampoline function
        /// 7: Overwrites the first 5 bytes of the target function with a jump to the hook function
        /// Voila!
        void RegisterCallEvent(hks::lua_State* L, void* targetFunction, std::vector<std::pair<asmjit::x86::Reg, FieldType>> regs, int luaCallbackIndex) {
            csh handle;
            cs_insn* insn;
            size_t count;

            // Initialize disassembler
            cs_err errCode = cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
            if (errCode != CS_ERR_OK) {
                std::cout << "Failed to initialize disassembler! Error code: " << errCode << std::endl;
                return;
            }

            // Disassemble the target function
            count = cs_disasm(handle, (byte*)targetFunction, 64, (uintptr_t)targetFunction, 0, &insn);
            if (count == 0) {
                std::cout << "ERROR: Failed to disassemble given function.\n";
                cs_close(&handle);
                return;
            }

            std::cout << "count: " << count << '\n';
            size_t size = 0;
            int i = 0;

            while (size < 5 && i < count) {
                size += insn[i].size;
                i++;
            }

            void* trampoline = CreateTrampoline((byte*)targetFunction, size);
            if (!trampoline) {
                std::cout << "Trampoline function failed to create!\n";
                cs_free(insn, count);
                cs_close(&handle);
                return;
            }

            void* hookFunction = CreateLuaCallback(L, regs, trampoline, luaCallbackIndex);
            if (!hookFunction) {
                std::cout << "Hook function failed to create!\n";
                cs_free(insn, count);
                cs_close(&handle);
                return;
            }

            DWORD relativeAddress = (DWORD)((uintptr_t)hookFunction - (uintptr_t)targetFunction - 5);
            byte jumpToHook[5] = {
                0xe9,
                (byte)((relativeAddress >> 0) & 0xFF),
                (byte)((relativeAddress >> 8) & 0xFF),
                (byte)((relativeAddress >> 16) & 0xFF),
                (byte)((relativeAddress >> 24) & 0xFF)
            };

            Runtime::WriteCodeToProcess((uintptr_t)targetFunction, jumpToHook, sizeof(jumpToHook));

            std::cout << "Function addresses: \n"
                << "Target: " << targetFunction
                << "\nTrampoline: " << trampoline
                << "\nHook: " << hookFunction << ' ' << *(byte*)hookFunction << '\n';

            std::cout << "Hook installed successfully.\n";

            cs_free(insn, count);
            cs_close(&handle);
        }
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
            std::vector<std::pair<asmjit::x86::Reg, FieldType>> parameters;
            for (int i = 1; i <= parametersLength; i++) {
                hks::pushinteger(L, i);
                hks::gettable(L, 3);

                int fieldType = hks::checkinteger(L, -1);
                std::cout << fieldType << '\n';
                if (fieldType < FIELD_BYTE || fieldType >= _FIELD_TYPE_COUNT_) {
                    hks::error(L, "Invalid FieldType parameter was passed!");
                    return 0;
                }

                parameters.push_back(std::make_pair(GetRegister((FieldType)fieldType, i - 1), (FieldType)fieldType));
                hks::pop(L, 1);
            }

            RegisterCallEvent(L, (void*)(Runtime::GameCoreAddress + address), parameters, callbackIndex);
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