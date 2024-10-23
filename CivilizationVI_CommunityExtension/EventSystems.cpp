#include "EventSystems.h"
#include "Runtime.h"
#include <vector>
#include "Game.h"
#include <mutex>
#include <functional>
#include <queue>
#include <condition_variable>
#include <unordered_map>
#include <iostream>
#include "Game.h"
#include "ForgeUI.h"

namespace EventSystems {
    struct ProcessorEvent {
        hks::lua_State* L;
        int callbackIndex;
    };

    struct QueuedEvent {
        ProcessorEvent processorEvent;
        Data::LuaVariantMap variantMap;
        std::string propertyToGet;
        std::promise<bool> promise;
    };

    std::mutex eventsMutex;
    std::unordered_map<std::string, std::vector<ProcessorEvent>> processorEvents;

    std::mutex eventQueueMutex;
    std::queue<QueuedEvent> eventQueue;
    std::condition_variable eventQueueCV;

    int lRegisterProcessor(hks::lua_State* L) {
        size_t length;
        const char* name = hks::checklstring(L, 1, &length);
        hks::pushvalue(L, 2); // Push the callback function
        int callbackIndex = hks::ref(L, hks::LUA_REGISTRYINDEX); // Store the callback in the registry

        std::lock_guard<std::mutex> lock(eventsMutex);
        processorEvents[name].push_back(ProcessorEvent{ L, callbackIndex });
        return 0;
    }

    bool DoesProcessorExist(const std::string& name) {
        std::lock_guard<std::mutex> lock(eventsMutex);
        auto eventIterator = processorEvents.find(name);
        return eventIterator != processorEvents.end();
    }

    static bool callProcessorInState(hks::lua_State* L, int callbackIndex, Data::LuaVariantMap& variantMap, const std::string& propertyToGet = "") {
        hks::rawgeti(L, hks::LUA_REGISTRYINDEX, callbackIndex);

        // Push the variant map as a Lua table onto the stack
        hks::createtable(L, 0, variantMap.size());
        for (const auto& pair : variantMap) {
            hks::pushfstring(L, pair.first.c_str());
            pair.second.push(L);
            hks::settable(L, -3);
        }

        if (hks::pcall(L, 1, 1, 0) != 0) {
            size_t length;
            std::cout << "Error calling processor: " << hks::checklstring(L, -1, &length) << "!\n";
            return false;
        }

        bool result = hks::toboolean(L, -1);
        hks::pop(L, 1); // Remove result from Lua stack

        if (!result) return false;

        if (!propertyToGet.empty()) {
            variantMap.reclaim(L, propertyToGet);
        }
        else {
            variantMap.rebuild(L);
        }

        return true;
    }

    std::future<bool> CallProcessorsAsync(const std::string& name, Data::LuaVariantMap& variantMap, const std::string& propertyToGet) {
        std::lock_guard<std::mutex> lock(eventsMutex);
        auto eventIterator = processorEvents.find(name);
        if (eventIterator == processorEvents.end()) {
            throw std::runtime_error("Processor not found");
        }

        std::promise<bool> promise;
        std::future<bool> future = promise.get_future();

        {
            std::lock_guard<std::mutex> queueLock(eventQueueMutex);
            for (const auto& processor : eventIterator->second) {
                eventQueue.push(QueuedEvent{ processor, variantMap, propertyToGet, std::move(promise) });
            }
        }
        eventQueueCV.notify_one();

        return future;
    }

    static void processQueuedProcessorEvents() {
        std::unique_lock<std::mutex> lock(eventQueueMutex);

        // Process each queued event
        while (!eventQueue.empty()) {
            auto& queuedEvent = eventQueue.front();
            
            {
                if (ForgeUI::manager != NULL) ForgeUI::GetLock(ForgeUI::manager);

                {
                    auto lpCritSec = (LPCRITICAL_SECTION)(Runtime::BaseAddress + 0x3552fa0);
                    EnterCriticalSection(lpCritSec);

                    bool result = callProcessorInState(
                        queuedEvent.processorEvent.L, queuedEvent.processorEvent.callbackIndex,
                        queuedEvent.variantMap, queuedEvent.propertyToGet);
                    queuedEvent.promise.set_value(result);

                    LeaveCriticalSection(lpCritSec);
                }

                if (ForgeUI::manager != NULL) ForgeUI::ReleaseLock(ForgeUI::manager);
            }

            eventQueue.pop();
        }
    }
    
    Types::PublishEvents base_PublishEvents;
    Types::PublishEvents orig_PublishEvents;
    void PublishEvents(void* _) {
        processQueuedProcessorEvents();
        base_PublishEvents(_);
    }

    void Create() {
        std::cout << "Establishing event hooks!\n";
        using namespace Runtime;

        orig_PublishEvents = GetGlobalAt<Types::PublishEvents>(PUBLISH_EVENTS_OFFSET);
        CreateHook(orig_PublishEvents, &PublishEvents, &base_PublishEvents);
    }
}
