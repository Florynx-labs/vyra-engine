#pragma once

#include "vyra/core/base.hpp"
#include "vyra/asset/asset_id.hpp"
#include "vyra/asset/asset_handle.hpp"
#include <string>
#include <vector>
#include <chrono>
#include <cstdint>

namespace vyra {
namespace asset {

/**
 * @brief Asset load status enumeration
 */
enum class AssetLoadStatus : uint8_t {
    Unloaded = 0,       // Asset not loaded
    Loading = 1,        // Asset currently being loaded
    Loaded = 2,         // Asset successfully loaded
    Failed = 3,         // Asset failed to load
    Unloading = 4,      // Asset currently being unloaded
    Reloading = 5       // Asset being reloaded (hot reload)
};

/**
 * @brief Convert AssetLoadStatus to string
 */
inline const char* AssetLoadStatusToString(AssetLoadStatus status) {
    switch (status) {
        case AssetLoadStatus::Unloaded: return "unloaded";
        case AssetLoadStatus::Loading: return "loading";
        case AssetLoadStatus::Loaded: return "loaded";
        case AssetLoadStatus::Failed: return "failed";
        case AssetLoadStatus::Unloading: return "unloading";
        case AssetLoadStatus::Reloading: return "reloading";
        default: return "unknown";
    }
}

/**
 * @brief Asset dependency information
 */
struct AssetDependency {
    AssetID assetId;           // ID of the dependency
    AssetHandle handle;         // Handle to the dependency
    bool required;              // Whether this dependency is required
    
    AssetDependency() : required(false) {}
    AssetDependency(AssetID id, AssetHandle h, bool req = true)
        : assetId(id), handle(h), required(req) {}
};

/**
 * @brief Comprehensive asset metadata storage
 * 
 * AssetMetadata stores all information about an asset needed for
 * lifecycle management, dependency tracking, and optimization.
 */
class VYRA_API AssetMetadata {
public:
    /**
     * @brief Default constructor
     */
    AssetMetadata()
        : status_(AssetLoadStatus::Unloaded)
        , memoryFootprint_(0)
        , refCount_(0)
        , loadTimeMs_(0)
        , lastAccessTime_(std::chrono::steady_clock::now())
        , persistent_(false) {}
    
    /**
     * @brief Construct with asset ID
     */
    explicit AssetMetadata(const AssetID& id)
        : assetId_(id)
        , status_(AssetLoadStatus::Unloaded)
        , memoryFootprint_(0)
        , refCount_(0)
        , loadTimeMs_(0)
        , lastAccessTime_(std::chrono::steady_clock::now())
        , persistent_(false) {}
    
    /**
     * @brief Get asset ID
     */
    const AssetID& GetAssetID() const { return assetId_; }
    
    /**
     * @brief Set asset ID
     */
    void SetAssetID(const AssetID& id) { assetId_ = id; }
    
    /**
     * @brief Get asset handle
     */
    const AssetHandle& GetHandle() const { return handle_; }
    
    /**
     * @brief Set asset handle
     */
    void SetHandle(const AssetHandle& handle) { handle_ = handle; }
    
    /**
     * @brief Get load status
     */
    AssetLoadStatus GetStatus() const { return status_; }
    
    /**
     * @brief Set load status
     */
    void SetStatus(AssetLoadStatus status) { status_ = status; }
    
    /**
     * @brief Get file path (if asset is file-based)
     */
    const std::string& GetFilePath() const { return filePath_; }
    
    /**
     * @brief Set file path
     */
    void SetFilePath(const std::string& path) { filePath_ = path; }
    
    /**
     * @brief Get memory footprint in bytes
     */
    size_t GetMemoryFootprint() const { return memoryFootprint_; }
    
    /**
     * @brief Set memory footprint
     */
    void SetMemoryFootprint(size_t bytes) { memoryFootprint_ = bytes; }
    
    /**
     * @brief Get reference count
     */
    uint32_t GetRefCount() const { return refCount_; }
    
    /**
     * @brief Increment reference count
     */
    void IncrementRefCount() { refCount_++; }
    
    /**
     * @brief Decrement reference count
     */
    void DecrementRefCount() {
        if (refCount_ > 0) refCount_--;
    }
    
    /**
     * @brief Get load time in milliseconds
     */
    uint32_t GetLoadTimeMs() const { return loadTimeMs_; }
    
    /**
     * @brief Set load time
     */
    void SetLoadTimeMs(uint32_t ms) { loadTimeMs_ = ms; }
    
    /**
     * @brief Get last access time
     */
    const std::chrono::steady_clock::time_point& GetLastAccessTime() const {
        return lastAccessTime_;
    }
    
    /**
     * @brief Update last access time to now
     */
    void UpdateAccessTime() {
        lastAccessTime_ = std::chrono::steady_clock::now();
    }
    
    /**
     * @brief Check if asset is persistent (never unload)
     */
    bool IsPersistent() const { return persistent_; }
    
    /**
     * @brief Set persistent flag
     */
    void SetPersistent(bool persistent) { persistent_ = persistent; }
    
    /**
     * @brief Get user data (custom metadata)
     */
    const std::string& GetUserData() const { return userData_; }
    
    /**
     * @brief Set user data
     */
    void SetUserData(const std::string& data) { userData_ = data; }
    
    /**
     * @brief Get dependencies
     */
    const std::vector<AssetDependency>& GetDependencies() const {
        return dependencies_;
    }
    
    /**
     * @brief Add dependency
     */
    void AddDependency(const AssetDependency& dep) {
        dependencies_.push_back(dep);
    }
    
    /**
     * @brief Remove dependency
     */
    void RemoveDependency(const AssetID& assetId) {
        dependencies_.erase(
            std::remove_if(dependencies_.begin(), dependencies_.end(),
                [&assetId](const AssetDependency& dep) {
                    return dep.assetId == assetId;
                }),
            dependencies_.end()
        );
    }
    
    /**
     * @brief Check if asset is loaded
     */
    bool IsLoaded() const {
        return status_ == AssetLoadStatus::Loaded;
    }
    
    /**
     * @brief Check if asset is loading
     */
    bool IsLoading() const {
        return status_ == AssetLoadStatus::Loading || 
               status_ == AssetLoadStatus::Reloading;
    }
    
    /**
     * @brief Check if asset failed to load
     */
    bool IsFailed() const {
        return status_ == AssetLoadStatus::Failed;
    }
    
    /**
     * @brief Get time since last access in milliseconds
     */
    uint64_t GetTimeSinceLastAccessMs() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            now - lastAccessTime_
        ).count();
    }
    
    /**
     * @brief Comparison operators
     */
    bool operator==(const AssetMetadata& other) const {
        return assetId_ == other.assetId_ && handle_ == other.handle_;
    }
    
    bool operator!=(const AssetMetadata& other) const {
        return !(*this == other);
    }
    
private:
    AssetID assetId_;
    AssetHandle handle_;
    AssetLoadStatus status_;
    std::string filePath_;
    size_t memoryFootprint_;
    uint32_t refCount_;
    uint32_t loadTimeMs_;
    std::chrono::steady_clock::time_point lastAccessTime_;
    bool persistent_;
    std::string userData_;
    std::vector<AssetDependency> dependencies_;
};

} // namespace asset
} // namespace vyra
