#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>

namespace vyra::core {

/**
 * @brief Thread-safe fixed-size object pool with stable addresses.
 *
 * Slots are allocated in blocks and never moved. Create() constructs objects
 * in-place; Destroy() runs the destructor and returns the slot to the free list.
 * The pool owns storage, not object lifetime beyond its own destruction.
 */
template <typename T, std::size_t BlockSize = 256>
class ObjectPool {
    static_assert(BlockSize > 0, "ObjectPool block size must be greater than zero");
    static_assert(std::is_nothrow_destructible_v<T>,
                  "Pooled objects must have noexcept destructors");

    struct Slot {
        alignas(T) std::byte storage[sizeof(T)];
        Slot* next{nullptr};
        bool occupied{false};
    };

public:
    ObjectPool() = default;
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;

    ~ObjectPool() { Clear(); }

    template <typename... Args>
    T* Create(Args&&... args) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_freeList) AddBlock();

        Slot* slot = m_freeList;
        T* object = reinterpret_cast<T*>(slot->storage);
        // Keep the slot on the free list if construction throws.
        std::construct_at(object, std::forward<Args>(args)...);
        m_freeList = slot->next;
        slot->next = nullptr;
        slot->occupied = true;
        ++m_size;
        return object;
    }

    bool Destroy(T* object) noexcept {
        if (!object) return false;
        std::lock_guard<std::mutex> lock(m_mutex);
        Slot* slot = FindSlot(object);
        if (!slot || !slot->occupied) return false;

        std::destroy_at(object);
        slot->occupied = false;
        slot->next = m_freeList;
        m_freeList = slot;
        --m_size;
        return true;
    }

    void Clear() noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto& block : m_blocks) {
            for (std::size_t index = 0; index < BlockSize; ++index) {
                Slot& slot = block[index];
                if (slot.occupied) {
                    std::destroy_at(reinterpret_cast<T*>(slot.storage));
                    slot.occupied = false;
                }
            }
        }
        m_blocks.clear();
        m_freeList = nullptr;
        m_size = 0;
    }

    std::size_t Size() const noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_size;
    }

    std::size_t Capacity() const noexcept {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_blocks.size() * BlockSize;
    }

private:
    Slot* FindSlot(T* object) const noexcept {
        const auto address = reinterpret_cast<std::uintptr_t>(object);
        for (const auto& block : m_blocks) {
            const auto begin = reinterpret_cast<std::uintptr_t>(block.get());
            const auto bytes = sizeof(Slot) * BlockSize;
            if (address >= begin && address < begin + bytes) {
                const auto offset = address - begin;
                if (offset % sizeof(Slot) == 0) {
                    return &block[offset / sizeof(Slot)];
                }
                return nullptr;
            }
        }
        return nullptr;
    }

    void AddBlock() {
        auto block = std::make_unique<Slot[]>(BlockSize);
        Slot* slots = block.get();
        m_blocks.push_back(std::move(block));
        for (std::size_t index = 0; index < BlockSize; ++index) {
            slots[index].next = m_freeList;
            m_freeList = &slots[index];
        }
    }

    mutable std::mutex m_mutex;
    std::vector<std::unique_ptr<Slot[]>> m_blocks;
    Slot* m_freeList{nullptr};
    std::size_t m_size{0};
};

} // namespace vyra::core
