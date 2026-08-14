#pragma once

#include "vyra/core/base.hpp"
#include "vyra/asset/asset_id.hpp"
#include "vyra/asset/asset_handle.hpp"
#include "vyra/asset/asset_metadata.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

namespace vyra {
namespace asset {

/**
 * @brief Asset loading function signature
 * 
 * AssetLoadFunction is called by the registry to load an asset.
 * It should return the asset handle or an invalid handle on failure.
 */
using AssetLoadFunction = std::function<AssetHandle(const AssetID&)>;

/**
 * @brief Asset unloading function signature
 * 
 * AssetUnloadFunction is called by the registry to unload an asset.
 */
using AssetUnloadFunction = std::function<void(AssetHandle)>;

/**
 * @brief Central asset database with thread-safe operations
 * 
 * AssetRegistry provides centralized asset management with:
 * - Thread-safe asset registration and lookup
 * - Dependency tracking and management
 * - Reference counting for automatic cleanup
 * - Generation-based handle validation
 * - Garbage collection for unused assets
 */
class VYRA_API AssetRegistry {
public:
    /**
     * @brief Constructor
     */
    AssetRegistry();
    
    /**
     * @brief Destructor
     */
    ~AssetRegistry();
    
    /**
     * @brief Register an asset with the registry
     * 
     * @param assetId The asset ID to register
     * @param filePath Optional file path for file-based assets
     * @param persistent Whether the asset should never be unloaded
     * @return AssetHandle for the registered asset (invalid if registration fails)
     */
    AssetHandle RegisterAsset(const AssetID& assetId, 
                              const std::string& filePath = "",
                              bool persistent = false);
    
    /**
     * @brief Unregister an asset from the registry
     * 
     * @param handle Handle to the asset to unregister
     * @return true if unregistration succeeded
     */
    bool UnregisterAsset(AssetHandle handle);
    
    /**
     * @brief Get asset metadata by handle
     * 
     * @param handle Handle to the asset
     * @return Pointer to metadata, or nullptr if not found
     */
    AssetMetadata* GetMetadata(AssetHandle handle);
    
    /**
     * @brief Get asset metadata by handle (const version)
     */
    const AssetMetadata* GetMetadata(AssetHandle handle) const;
    
    /**
     * @brief Get asset metadata by asset ID
     * 
     * @param assetId The asset ID
     * @return Pointer to metadata, or nullptr if not found
     */
    AssetMetadata* GetMetadata(const AssetID& assetId);
    
    /**
     * @brief Get asset metadata by asset ID (const version)
     */
    const AssetMetadata* GetMetadata(const AssetID& assetId) const;
    
    /**
     * @brief Load an asset
     * 
     * @param handle Handle to the asset to load
     * @return true if loading succeeded
     */
    bool LoadAsset(AssetHandle handle);
    
    /**
     * @brief Load an asset by ID
     * 
     * @param assetId The asset ID to load
     * @return Handle to the loaded asset (invalid if loading failed)
     */
    AssetHandle LoadAsset(const AssetID& assetId);
    
    /**
     * @brief Unload an asset
     * 
     * @param handle Handle to the asset to unload
     * @return true if unloading succeeded
     */
    bool UnloadAsset(AssetHandle handle);
    
    /**
     * @brief Increment reference count for an asset
     * 
     * @param handle Handle to the asset
     * @return true if increment succeeded
     */
    bool AddReference(AssetHandle handle);
    
    /**
     * @brief Decrement reference count for an asset
     * 
     * @param handle Handle to the asset
     * @return true if decrement succeeded and asset may be eligible for unloading
     */
    bool RemoveReference(AssetHandle handle);
    
    /**
     * @brief Add a dependency between assets
     * 
     * @param dependent Handle to the dependent asset
     * @param dependency Handle to the dependency asset
     * @param required Whether the dependency is required
     * @return true if dependency was added
     */
    bool AddDependency(AssetHandle dependent, AssetHandle dependency, bool required = true);
    
    /**
     * @brief Remove a dependency between assets
     * 
     * @param dependent Handle to the dependent asset
     * @param dependency Handle to the dependency asset
     * @return true if dependency was removed
     */
    bool RemoveDependency(AssetHandle dependent, AssetHandle dependency);
    
    /**
     * @brief Check if an asset handle is valid
     * 
     * @param handle Handle to validate
     * @return true if handle is valid and refers to a live asset
     */
    bool IsValidHandle(AssetHandle handle) const;
    
    /**
     * @brief Get all assets of a specific type
     * 
     * @param type The asset type to query
     * @return Vector of handles to assets of the specified type
     */
    std::vector<AssetHandle> GetAssetsByType(AssetType type) const;
    
    /**
     * @brief Get total number of registered assets
     */
    size_t GetAssetCount() const;
    
    /**
     * @brief Get total memory footprint of all loaded assets
     */
    size_t GetTotalMemoryFootprint() const;
    
    /**
     * @brief Perform garbage collection
     * 
     * Unloads assets with zero reference count that are not persistent.
     * 
     * @param maxAgeMs Maximum age in milliseconds (0 for no age limit)
     * @return Number of assets unloaded
     */
    size_t GarbageCollect(uint64_t maxAgeMs = 0);
    
    /**
     * @brief Set asset load function for a specific asset type
     * 
     * @param type The asset type
     * @param loadFunc The load function
     */
    void SetLoadFunction(AssetType type, AssetLoadFunction loadFunc);
    
    /**
     * @brief Set asset unload function for a specific asset type
     * 
     * @param type The asset type
     * @param unloadFunc The unload function
     */
    void SetUnloadFunction(AssetType type, AssetUnloadFunction unloadFunc);
    
    /**
     * @brief Clear all assets from the registry
     */
    void Clear();
    
    /**
     * @brief Get registry statistics
     */
    struct Statistics {
        size_t totalAssets;
        size_t loadedAssets;
        size_t failedAssets;
        size_t totalMemoryBytes;
        size_t persistentAssets;
        size_t referencedAssets;
    };
    
    Statistics GetStatistics() const;

private:
    /**
     * @brief Internal asset storage
     */
    struct AssetEntry {
        AssetMetadata metadata;
        AssetHandle::GenerationType currentGeneration;
        
        AssetEntry() : currentGeneration(0) {}
    };
    
    /**
     * @brief Allocate a new handle
     */
    AssetHandle AllocateHandle();
    
    /**
     * @brief Free a handle (increment generation)
     */
    void FreeHandle(AssetHandle handle);
    
    /**
     * @brief Call the appropriate load function for an asset
     */
    bool CallLoadFunction(AssetMetadata* metadata);
    
    /**
     * @brief Call the appropriate unload function for an asset
     */
    void CallUnloadFunction(AssetMetadata* metadata);
    
    // Thread safety
    mutable std::mutex mutex_;
    
    // Asset storage indexed by handle index
    std::vector<AssetEntry> assetEntries_;
    
    // Free list for handle allocation
    std::vector<AssetHandle::IndexType> freeList_;
    
    // Asset ID to handle mapping
    std::unordered_map<AssetID, AssetHandle, AssetIDHash> idToHandle_;
    
    // Load functions per asset type
    std::unordered_map<AssetType, AssetLoadFunction> loadFunctions_;
    
    // Unload functions per asset type
    std::unordered_map<AssetType, AssetUnloadFunction> unloadFunctions_;
};

} // namespace asset
} // namespace vyra
