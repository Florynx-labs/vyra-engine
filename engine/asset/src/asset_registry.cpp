#include "vyra/asset/asset_registry.hpp"
#include "vyra/core/log.hpp"
#include <algorithm>

namespace vyra {
namespace asset {

AssetRegistry::AssetRegistry() {
    VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Asset registry initialized");
}

AssetRegistry::~AssetRegistry() {
    Clear();
    VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Asset registry destroyed");
}

AssetHandle AssetRegistry::RegisterAsset(const AssetID& assetId, 
                                          const std::string& filePath,
                                          bool persistent) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if asset already registered
    auto it = idToHandle_.find(assetId);
    if (it != idToHandle_.end()) {
        AssetHandle existingHandle = it->second;
        if (IsValidHandle(existingHandle)) {
            VYRA_LOG_CHANNEL(LogChannel::Assets, warn, "Asset already registered: {}", assetId.ToString());
            return existingHandle;
        }
    }
    
    // Allocate new handle
    AssetHandle handle = AllocateHandle();
    if (!handle.IsValid()) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Failed to allocate handle for asset: {}", assetId.ToString());
        return AssetHandle::Invalid();
    }
    
    // Create asset entry
    AssetEntry& entry = assetEntries_[handle.GetIndex()];
    entry.metadata = AssetMetadata(assetId);
    entry.metadata.SetHandle(handle);
    entry.metadata.SetFilePath(filePath);
    entry.metadata.SetPersistent(persistent);
    entry.currentGeneration = handle.GetGeneration();
    
    // Update ID to handle mapping
    idToHandle_[assetId] = handle;
    
    VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Registered asset: {} (handle: {}, persistent: {})", 
                  assetId.ToString(), handle.ToUint64(), persistent);
    
    return handle;
}

bool AssetRegistry::UnregisterAsset(AssetHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(handle)) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, warn, "Attempted to unregister invalid handle: {}", handle.ToUint64());
        return false;
    }
    
    AssetEntry& entry = assetEntries_[handle.GetIndex()];
    const AssetID& assetId = entry.metadata.GetAssetID();
    
    // Unload if loaded
    if (entry.metadata.IsLoaded()) {
        CallUnloadFunction(&entry.metadata);
    }
    
    // Remove from ID mapping
    idToHandle_.erase(assetId);
    
    // Free handle
    FreeHandle(handle);
    
    VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Unregistered asset: {} (handle: {})", 
                  assetId.ToString(), handle.ToUint64());
    
    return true;
}

AssetMetadata* AssetRegistry::GetMetadata(AssetHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(handle)) {
        return nullptr;
    }
    
    return &assetEntries_[handle.GetIndex()].metadata;
}

const AssetMetadata* AssetRegistry::GetMetadata(AssetHandle handle) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(handle)) {
        return nullptr;
    }
    
    return &assetEntries_[handle.GetIndex()].metadata;
}

AssetMetadata* AssetRegistry::GetMetadata(const AssetID& assetId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = idToHandle_.find(assetId);
    if (it == idToHandle_.end()) {
        return nullptr;
    }
    
    return GetMetadata(it->second);
}

const AssetMetadata* AssetRegistry::GetMetadata(const AssetID& assetId) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = idToHandle_.find(assetId);
    if (it == idToHandle_.end()) {
        return nullptr;
    }
    
    return GetMetadata(it->second);
}

bool AssetRegistry::LoadAsset(AssetHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(handle)) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Attempted to load invalid handle: {}", handle.ToUint64());
        return false;
    }
    
    AssetEntry& entry = assetEntries_[handle.GetIndex()];
    AssetMetadata* metadata = &entry.metadata;
    
    if (metadata->IsLoaded()) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, debug, "Asset already loaded: {}", metadata->GetAssetID().ToString());
        return true;
    }
    
    if (metadata->IsLoading()) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, warn, "Asset already loading: {}", metadata->GetAssetID().ToString());
        return false;
    }
    
    // Set status to loading
    metadata->SetStatus(AssetLoadStatus::Loading);
    
    // Call load function
    bool success = CallLoadFunction(metadata);
    
    if (success) {
        metadata->SetStatus(AssetLoadStatus::Loaded);
        metadata->UpdateAccessTime();
        VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Asset loaded successfully: {}", metadata->GetAssetID().ToString());
    } else {
        metadata->SetStatus(AssetLoadStatus::Failed);
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Asset failed to load: {}", metadata->GetAssetID().ToString());
    }
    
    return success;
}

AssetHandle AssetRegistry::LoadAsset(const AssetID& assetId) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = idToHandle_.find(assetId);
    if (it == idToHandle_.end()) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Asset not registered: {}", assetId.ToString());
        return AssetHandle::Invalid();
    }
    
    AssetHandle handle = it->second;
    if (LoadAsset(handle)) {
        return handle;
    }
    
    return AssetHandle::Invalid();
}

bool AssetRegistry::UnloadAsset(AssetHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(handle)) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Attempted to unload invalid handle: {}", handle.ToUint64());
        return false;
    }
    
    AssetEntry& entry = assetEntries_[handle.GetIndex()];
    AssetMetadata* metadata = &entry.metadata;
    
    if (!metadata->IsLoaded()) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, debug, "Asset not loaded: {}", metadata->GetAssetID().ToString());
        return true;
    }
    
    // Check if asset is persistent
    if (metadata->IsPersistent()) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, warn, "Cannot unload persistent asset: {}", metadata->GetAssetID().ToString());
        return false;
    }
    
    // Check if asset has references
    if (metadata->GetRefCount() > 0) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, warn, "Cannot unload asset with references: {} (ref count: {})", 
                      metadata->GetAssetID().ToString(), metadata->GetRefCount());
        return false;
    }
    
    // Set status to unloading
    metadata->SetStatus(AssetLoadStatus::Unloading);
    
    // Call unload function
    CallUnloadFunction(metadata);
    
    metadata->SetStatus(AssetLoadStatus::Unloaded);
    VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Asset unloaded: {}", metadata->GetAssetID().ToString());
    
    return true;
}

bool AssetRegistry::AddReference(AssetHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(handle)) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Attempted to add reference to invalid handle: {}", handle.ToUint64());
        return false;
    }
    
    AssetEntry& entry = assetEntries_[handle.GetIndex()];
    entry.metadata.IncrementRefCount();
    entry.metadata.UpdateAccessTime();
    
    VYRA_LOG_CHANNEL(LogChannel::Assets, debug, "Added reference to asset: {} (ref count: {})", 
                   entry.metadata.GetAssetID().ToString(), entry.metadata.GetRefCount());
    
    return true;
}

bool AssetRegistry::RemoveReference(AssetHandle handle) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(handle)) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Attempted to remove reference from invalid handle: {}", handle.ToUint64());
        return false;
    }
    
    AssetEntry& entry = assetEntries_[handle.GetIndex()];
    
    if (entry.metadata.GetRefCount() == 0) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, warn, "Attempted to remove reference from asset with zero ref count: {}", 
                      entry.metadata.GetAssetID().ToString());
        return false;
    }
    
    entry.metadata.DecrementRefCount();
    entry.metadata.UpdateAccessTime();
    
    VYRA_LOG_CHANNEL(LogChannel::Assets, debug, "Removed reference from asset: {} (ref count: {})", 
                   entry.metadata.GetAssetID().ToString(), entry.metadata.GetRefCount());
    
    // Return true if asset can be unloaded (ref count is 0 and not persistent)
    return entry.metadata.GetRefCount() == 0 && !entry.metadata.IsPersistent();
}

bool AssetRegistry::AddDependency(AssetHandle dependent, AssetHandle dependency, bool required) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(dependent) || !IsValidHandle(dependency)) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Attempted to add dependency with invalid handles");
        return false;
    }
    
    AssetEntry& dependentEntry = assetEntries_[dependent.GetIndex()];
    AssetEntry& dependencyEntry = assetEntries_[dependency.GetIndex()];
    
    AssetDependency dep(
        dependencyEntry.metadata.GetAssetID(),
        dependency,
        required
    );
    
    dependentEntry.metadata.AddDependency(dep);
    
    VYRA_LOG_CHANNEL(LogChannel::Assets, debug, "Added dependency: {} -> {}", 
                   dependentEntry.metadata.GetAssetID().ToString(),
                   dependencyEntry.metadata.GetAssetID().ToString());
    
    return true;
}

bool AssetRegistry::RemoveDependency(AssetHandle dependent, AssetHandle dependency) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!IsValidHandle(dependent) || !IsValidHandle(dependency)) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Attempted to remove dependency with invalid handles");
        return false;
    }
    
    AssetEntry& dependentEntry = assetEntries_[dependent.GetIndex()];
    AssetEntry& dependencyEntry = assetEntries_[dependency.GetIndex()];
    
    dependentEntry.metadata.RemoveDependency(dependencyEntry.metadata.GetAssetID());
    
    VYRA_LOG_CHANNEL(LogChannel::Assets, debug, "Removed dependency: {} -> {}", 
                   dependentEntry.metadata.GetAssetID().ToString(),
                   dependencyEntry.metadata.GetAssetID().ToString());
    
    return true;
}

bool AssetRegistry::IsValidHandle(AssetHandle handle) const {
    if (!handle.IsValid()) {
        return false;
    }
    
    AssetHandle::IndexType index = handle.GetIndex();
    if (index >= assetEntries_.size()) {
        return false;
    }
    
    const AssetEntry& entry = assetEntries_[index];
    return handle.MatchesGeneration(entry.currentGeneration);
}

std::vector<AssetHandle> AssetRegistry::GetAssetsByType(AssetType type) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<AssetHandle> handles;
    
    for (const auto& pair : idToHandle_) {
        if (pair.first.GetType() == type && IsValidHandle(pair.second)) {
            handles.push_back(pair.second);
        }
    }
    
    return handles;
}

size_t AssetRegistry::GetAssetCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return idToHandle_.size();
}

size_t AssetRegistry::GetTotalMemoryFootprint() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t total = 0;
    for (const auto& entry : assetEntries_) {
        if (entry.metadata.IsLoaded()) {
            total += entry.metadata.GetMemoryFootprint();
        }
    }
    
    return total;
}

size_t AssetRegistry::GarbageCollect(uint64_t maxAgeMs) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    size_t unloadedCount = 0;
    
    for (auto& entry : assetEntries_) {
        if (!entry.metadata.IsLoaded()) {
            continue;
        }
        
        if (entry.metadata.IsPersistent()) {
            continue;
        }
        
        if (entry.metadata.GetRefCount() > 0) {
            continue;
        }
        
        // Check age if specified
        if (maxAgeMs > 0) {
            uint64_t age = entry.metadata.GetTimeSinceLastAccessMs();
            if (age < maxAgeMs) {
                continue;
            }
        }
        
        // Unload the asset
        entry.metadata.SetStatus(AssetLoadStatus::Unloading);
        CallUnloadFunction(&entry.metadata);
        entry.metadata.SetStatus(AssetLoadStatus::Unloaded);
        
        unloadedCount++;
        VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Garbage collected asset: {}", 
                      entry.metadata.GetAssetID().ToString());
    }
    
    if (unloadedCount > 0) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Garbage collection completed: {} assets unloaded", unloadedCount);
    }
    
    return unloadedCount;
}

void AssetRegistry::SetLoadFunction(AssetType type, AssetLoadFunction loadFunc) {
    std::lock_guard<std::mutex> lock(mutex_);
    loadFunctions_[type] = loadFunc;
    VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Set load function for asset type: {}", AssetTypeToString(type));
}

void AssetRegistry::SetUnloadFunction(AssetType type, AssetUnloadFunction unloadFunc) {
    std::lock_guard<std::mutex> lock(mutex_);
    unloadFunctions_[type] = unloadFunc;
    VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Set unload function for asset type: {}", AssetTypeToString(type));
}

void AssetRegistry::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Unload all loaded assets
    for (auto& entry : assetEntries_) {
        if (entry.metadata.IsLoaded()) {
            CallUnloadFunction(&entry.metadata);
        }
    }
    
    assetEntries_.clear();
    freeList_.clear();
    idToHandle_.clear();
    loadFunctions_.clear();
    unloadFunctions_.clear();
    
    VYRA_LOG_CHANNEL(LogChannel::Assets, info, "Asset registry cleared");
}

AssetRegistry::Statistics AssetRegistry::GetStatistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    Statistics stats = {};
    stats.totalAssets = idToHandle_.size();
    
    for (const auto& entry : assetEntries_) {
        if (entry.metadata.IsLoaded()) {
            stats.loadedAssets++;
            stats.totalMemoryBytes += entry.metadata.GetMemoryFootprint();
        }
        if (entry.metadata.IsFailed()) {
            stats.failedAssets++;
        }
        if (entry.metadata.IsPersistent()) {
            stats.persistentAssets++;
        }
        if (entry.metadata.GetRefCount() > 0) {
            stats.referencedAssets++;
        }
    }
    
    return stats;
}

AssetHandle AssetRegistry::AllocateHandle() {
    // Reuse free handle if available
    if (!freeList_.empty()) {
        AssetHandle::IndexType index = freeList_.back();
        freeList_.pop_back();
        
        AssetEntry& entry = assetEntries_[index];
        AssetHandle handle(index, entry.currentGeneration);
        
        return handle;
    }
    
    // Allocate new handle
    AssetHandle::IndexType index = static_cast<AssetHandle::IndexType>(assetEntries_.size());
    assetEntries_.emplace_back();
    
    AssetHandle handle(index, 0);
    return handle;
}

void AssetRegistry::FreeHandle(AssetHandle handle) {
    AssetHandle::IndexType index = handle.GetIndex();
    
    if (index >= assetEntries_.size()) {
        return;
    }
    
    // Increment generation to invalidate old handles
    assetEntries_[index].currentGeneration++;
    
    // Add to free list
    freeList_.push_back(index);
}

bool AssetRegistry::CallLoadFunction(AssetMetadata* metadata) {
    if (!metadata) {
        return false;
    }
    
    AssetType type = metadata->GetAssetID().GetType();
    auto it = loadFunctions_.find(type);
    
    if (it == loadFunctions_.end()) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "No load function registered for asset type: {}", 
                       AssetTypeToString(type));
        return false;
    }
    
    try {
        AssetHandle result = it->second(metadata->GetAssetID());
        return result.IsValid();
    } catch (const std::exception& e) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Exception in load function for asset {}: {}", 
                       metadata->GetAssetID().ToString(), e.what());
        return false;
    }
}

void AssetRegistry::CallUnloadFunction(AssetMetadata* metadata) {
    if (!metadata) {
        return;
    }
    
    AssetType type = metadata->GetAssetID().GetType();
    auto it = unloadFunctions_.find(type);
    
    if (it == unloadFunctions_.end()) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, warn, "No unload function registered for asset type: {}", 
                      AssetTypeToString(type));
        return;
    }
    
    try {
        it->second(metadata->GetHandle());
    } catch (const std::exception& e) {
        VYRA_LOG_CHANNEL(LogChannel::Assets, error, "Exception in unload function for asset {}: {}", 
                       metadata->GetAssetID().ToString(), e.what());
    }
}

} // namespace asset
} // namespace vyra
