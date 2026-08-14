#pragma once

#include "vyra/core/base.hpp"
#include <cstdint>
#include <stdexcept>

namespace vyra {
namespace asset {

/**
 * @brief Generation-based handle for safe asset references
 * 
 * AssetHandle provides use-after-free detection and stale handle elimination
 * through generation numbers. Each handle contains an index and generation.
 * When an asset is freed, the generation is incremented, invalidating all
 * old handles to that asset.
 */
class VYRA_API AssetHandle {
public:
    using IndexType = uint32_t;
    using GenerationType = uint32_t;
    
    static constexpr IndexType INVALID_INDEX = static_cast<IndexType>(-1);
    static constexpr GenerationType INVALID_GENERATION = static_cast<GenerationType>(-1);
    
    /**
     * @brief Default constructor - creates invalid handle
     */
    constexpr AssetHandle() 
        : index_(INVALID_INDEX), generation_(INVALID_GENERATION) {}
    
    /**
     * @brief Construct handle from index and generation
     */
    constexpr AssetHandle(IndexType index, GenerationType generation)
        : index_(index), generation_(generation) {}
    
    /**
     * @brief Construct handle from index only (generation = 0)
     */
    constexpr explicit AssetHandle(IndexType index)
        : index_(index), generation_(0) {}
    
    /**
     * @brief Check if handle is valid
     */
    constexpr bool IsValid() const {
        return index_ != INVALID_INDEX && generation_ != INVALID_GENERATION;
    }
    
    /**
     * @brief Get handle index
     */
    constexpr IndexType GetIndex() const { return index_; }
    
    /**
     * @brief Get handle generation
     */
    constexpr GenerationType GetGeneration() const { return generation_; }
    
    /**
     * @brief Check if handle matches expected generation (use-after-free detection)
     */
    constexpr bool MatchesGeneration(GenerationType expected) const {
        return generation_ == expected;
    }
    
    /**
     * @brief Create invalid handle
     */
    static constexpr AssetHandle Invalid() {
        return AssetHandle(INVALID_INDEX, INVALID_GENERATION);
    }
    
    /**
     * @brief Comparison operators
     */
    constexpr bool operator==(const AssetHandle& other) const {
        return index_ == other.index_ && generation_ == other.generation_;
    }
    
    constexpr bool operator!=(const AssetHandle& other) const {
        return !(*this == other);
    }
    
    constexpr bool operator<(const AssetHandle& other) const {
        if (index_ != other.index_) return index_ < other.index_;
        return generation_ < other.generation_;
    }
    
    /**
     * @brief Convert to uint64_t for storage
     */
    constexpr uint64_t ToUint64() const {
        return (static_cast<uint64_t>(generation_) << 32) | static_cast<uint64_t>(index_);
    }
    
    /**
     * @brief Create from uint64_t
     */
    static constexpr AssetHandle FromUint64(uint64_t value) {
        return AssetHandle(
            static_cast<IndexType>(value & 0xFFFFFFFF),
            static_cast<GenerationType>(value >> 32)
        );
    }
    
private:
    IndexType index_;
    GenerationType generation_;
};

/**
 * @brief Hash function for AssetHandle (for use in unordered_map, etc.)
 */
struct AssetHandleHash {
    std::size_t operator()(const AssetHandle& handle) const {
        return std::hash<uint64_t>()(handle.ToUint64());
    }
};

} // namespace asset
} // namespace vyra
