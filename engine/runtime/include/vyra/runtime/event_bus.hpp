#pragma once

#include "vyra/core/base.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

namespace vyra::runtime {

// A small typed event bus. Callbacks execute outside the internal lock, so a
// listener may safely publish another event or unsubscribe itself.
class VYRA_API EventBus {
public:
    using Subscription = uint64_t;

    EventBus() = default;
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    template <typename Event, typename Callback>
    Subscription Subscribe(Callback&& callback) {
        std::lock_guard lock(m_Mutex);
        const Subscription id = ++m_NextSubscription;
        auto& listeners = m_Listeners[std::type_index(typeid(Event))];
        listeners.push_back(Listener{
            id,
            [fn = std::function<void(const Event&)>(std::forward<Callback>(callback))]
            (const void* event) { fn(*static_cast<const Event*>(event)); }
        });
        return id;
    }

    bool Unsubscribe(Subscription subscription) {
        if (subscription == 0) return false;
        std::lock_guard lock(m_Mutex);
        for (auto mapIt = m_Listeners.begin(); mapIt != m_Listeners.end(); ++mapIt) {
            auto& listeners = mapIt->second;
            const auto oldSize = listeners.size();
            std::erase_if(listeners, [subscription](const Listener& listener) {
                return listener.id == subscription;
            });
            if (listeners.size() != oldSize) {
                if (listeners.empty()) m_Listeners.erase(mapIt);
                return true;
            }
        }
        return false;
    }

    template <typename Event>
    size_t Publish(const Event& event) const {
        std::vector<Callback> callbacks;
        {
            std::lock_guard lock(m_Mutex);
            const auto it = m_Listeners.find(std::type_index(typeid(Event)));
            if (it == m_Listeners.end()) return 0;
            callbacks.reserve(it->second.size());
            for (const auto& listener : it->second) callbacks.push_back(listener.callback);
        }
        for (const auto& callback : callbacks) callback(&event);
        return callbacks.size();
    }

    void Clear() {
        std::lock_guard lock(m_Mutex);
        m_Listeners.clear();
    }

    size_t GetListenerCount() const {
        std::lock_guard lock(m_Mutex);
        size_t count = 0;
        for (const auto& [_, listeners] : m_Listeners) count += listeners.size();
        return count;
    }

private:
    using Callback = std::function<void(const void*)>;
    struct Listener { Subscription id; Callback callback; };

    mutable std::mutex m_Mutex;
    std::unordered_map<std::type_index, std::vector<Listener>> m_Listeners;
    Subscription m_NextSubscription{0};
};

} // namespace vyra::runtime
