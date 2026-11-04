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
        std::shared_ptr<Data::LuaVariantMap> variantMapPtr;
        Data::LuaVariantMap& variantMap;
        std::string propertyToGet;
        std::promise<bool> promise;

        QueuedEvent(const ProcessorEvent& pe, Data::LuaVariantMap& vm, const std::string& prop, std::promise<bool>&& prom)
            : processorEvent(pe)
            , variantMapPtr(std::make_shared<Data::LuaVariantMap>(vm))
            , variantMap(*variantMapPtr)
            , propertyToGet(prop)
            , promise(std::move(prom))
        {}
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

        if (hks::pcall(L, 1, 1, 0) != hks::LUA_OK) {
            size_t length;
            Runtime::MainLogger->Error("Error calling processor: {}", hks::checklstring(L, -1, &length));
			hks::pop(L, 1); // Remove error message from Lua stack
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
        Runtime::MainLogger->Info("Sending event signal from thread id {}", GetCurrentThreadId());
        std::lock_guard<std::mutex> lock(eventsMutex);

        auto resultPromise = std::make_shared<std::promise<bool>>();
        std::future<bool> resultFuture = resultPromise->get_future();

        auto eventIterator = processorEvents.find(name);
        if (eventIterator == processorEvents.end()) {
            Runtime::MainLogger->Error("Could not find processor with name \"{}\"", name);
			resultPromise->set_value(false);
            return resultFuture;
        }

        std::vector<std::future<bool>> futures;
        auto sharedMap = std::make_shared<Data::LuaVariantMap>(variantMap);

        {
            std::lock_guard<std::mutex> queueLock(eventQueueMutex);
            for (const auto& processor : eventIterator->second) {
                std::promise<bool> eventPromise;
                futures.push_back(eventPromise.get_future());

                eventQueue.push(QueuedEvent(
                    processor,
                    *sharedMap,
                    propertyToGet,
                    std::move(eventPromise)
                ));
            }
        }
        eventQueueCV.notify_one();

        // Launch a thread to wait for all futures and aggregate the results
        std::thread([futures = std::move(futures), resultPromise, sharedMap]() mutable {
            bool allSucceeded = true;
            for (auto& future : futures) {
                if (!future.get()) {
                    allSucceeded = false;
                    break;
                }
            }
            resultPromise->set_value(allSucceeded);
            }).detach();

        return resultFuture;
    }

    static void processQueuedProcessorEvents() {
        std::unique_lock<std::mutex> lock(eventQueueMutex);

        while (!eventQueue.empty()) {
            auto& queuedEvent = eventQueue.front();
            lock.unlock();

            {
                auto lpCritSec = Runtime::GetGlobalAt<LPCRITICAL_SECTION>(0x3552fa0);
                EnterCriticalSection(lpCritSec);

                bool result = callProcessorInState(
                    queuedEvent.processorEvent.L,
                    queuedEvent.processorEvent.callbackIndex,
                    queuedEvent.variantMap,
                    queuedEvent.propertyToGet
                );
                queuedEvent.promise.set_value(result);

                LeaveCriticalSection(lpCritSec);
            }

            lock.lock();
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
        Runtime::MainLogger->Info("Establishing event hooks!");
        using namespace Runtime;

        orig_PublishEvents = GetGlobalAt<Types::PublishEvents>(PUBLISH_EVENTS_OFFSET);
        CreateHook(orig_PublishEvents, &PublishEvents, &base_PublishEvents);
    }
}
