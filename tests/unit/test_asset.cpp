#include <catch2/catch_test_macros.hpp>
#include <vyra/asset/asset.hpp>
#include <vyra/core/log.hpp>

TEST_CASE("VYRA Asset System - AssetHandle", "[asset][unit]") {
    vyra::Log::Init();
    
    SECTION("Default Construction") {
        vyra::asset::AssetHandle handle;
        REQUIRE_FALSE(handle.IsValid());
        REQUIRE(handle.GetIndex() == vyra::asset::AssetHandle::INVALID_INDEX);
    }
    
    SECTION("Valid Construction") {
        vyra::asset::AssetHandle handle(5, 2);
        REQUIRE(handle.IsValid());
        REQUIRE(handle.GetIndex() == 5);
        REQUIRE(handle.GetGeneration() == 2);
    }
    
    SECTION("Generation Matching") {
        vyra::asset::AssetHandle handle(5, 2);
        REQUIRE(handle.MatchesGeneration(2));
        REQUIRE_FALSE(handle.MatchesGeneration(3));
    }
    
    SECTION("Invalid Handle Creation") {
        vyra::asset::AssetHandle handle = vyra::asset::AssetHandle::Invalid();
        REQUIRE_FALSE(handle.IsValid());
    }
    
    SECTION("Comparison Operators") {
        vyra::asset::AssetHandle handle1(5, 2);
        vyra::asset::AssetHandle handle2(5, 2);
        vyra::asset::AssetHandle handle3(5, 3);
        
        REQUIRE(handle1 == handle2);
        REQUIRE(handle1 != handle3);
        REQUIRE(handle1 < handle3); // Same index, different generation
    }
    
    SECTION("Uint64 Conversion") {
        vyra::asset::AssetHandle handle(5, 2);
        uint64_t value = handle.ToUint64();
        
        vyra::asset::AssetHandle reconstructed = vyra::asset::AssetHandle::FromUint64(value);
        REQUIRE(reconstructed == handle);
    }
    
    vyra::Log::Shutdown();
}

TEST_CASE("VYRA Asset System - AssetID", "[asset][unit]") {
    vyra::Log::Init();
    
    SECTION("Default Construction") {
        vyra::asset::AssetID assetId;
        REQUIRE_FALSE(assetId.IsValid());
    }
    
    SECTION("Valid Construction") {
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/characters/player");
        REQUIRE(assetId.IsValid());
        REQUIRE(assetId.GetType() == vyra::asset::AssetType::Texture);
        REQUIRE(assetId.GetPath() == "textures/characters/player");
    }
    
    SECTION("String Construction") {
        vyra::asset::AssetID assetId("texture:textures/characters/player");
        REQUIRE(assetId.IsValid());
        REQUIRE(assetId.GetType() == vyra::asset::AssetType::Texture);
        REQUIRE(assetId.GetPath() == "textures/characters/player");
    }
    
    SECTION("String Conversion") {
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Mesh, "meshes/characters/human");
        std::string str = assetId.ToString();
        REQUIRE(str == "mesh:meshes/characters/human");
    }
    
    SECTION("Path Normalization") {
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures\\characters\\player");
        REQUIRE(assetId.GetPath() == "textures/characters/player"); // Should normalize to forward slashes
    }
    
    SECTION("Parent Path") {
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/characters/player/default");
        REQUIRE(assetId.GetParentPath() == "textures/characters/player");
    }
    
    SECTION("Asset Name") {
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/characters/player/default");
        REQUIRE(assetId.GetName() == "default");
    }
    
    SECTION("Child Creation") {
        vyra::asset::AssetID parent(vyra::asset::AssetType::Texture, "textures/characters");
        vyra::asset::AssetID child = parent.CreateChild("player");
        REQUIRE(child.GetPath() == "textures/characters/player");
        REQUIRE(child.GetType() == vyra::asset::AssetType::Texture);
    }
    
    SECTION("Comparison Operators") {
        vyra::asset::AssetID id1(vyra::asset::AssetType::Texture, "textures/player");
        vyra::asset::AssetID id2(vyra::asset::AssetType::Texture, "textures/player");
        vyra::asset::AssetID id3(vyra::asset::AssetType::Mesh, "meshes/player");
        
        REQUIRE(id1 == id2);
        REQUIRE(id1 != id3);
    }
    
    vyra::Log::Shutdown();
}

TEST_CASE("VYRA Asset System - AssetMetadata", "[asset][unit]") {
    vyra::Log::Init();
    
    SECTION("Default Construction") {
        vyra::asset::AssetMetadata metadata;
        REQUIRE(metadata.GetStatus() == vyra::asset::AssetLoadStatus::Unloaded);
        REQUIRE(metadata.GetRefCount() == 0);
        REQUIRE(metadata.GetMemoryFootprint() == 0);
    }
    
    SECTION("Construction with AssetID") {
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/player");
        vyra::asset::AssetMetadata metadata(assetId);
        REQUIRE(metadata.GetAssetID() == assetId);
    }
    
    SECTION("Status Management") {
        vyra::asset::AssetMetadata metadata;
        metadata.SetStatus(vyra::asset::AssetLoadStatus::Loading);
        REQUIRE(metadata.GetStatus() == vyra::asset::AssetLoadStatus::Loading);
        REQUIRE(metadata.IsLoading());
        
        metadata.SetStatus(vyra::asset::AssetLoadStatus::Loaded);
        REQUIRE(metadata.IsLoaded());
        REQUIRE_FALSE(metadata.IsLoading());
    }
    
    SECTION("Reference Counting") {
        vyra::asset::AssetMetadata metadata;
        REQUIRE(metadata.GetRefCount() == 0);
        
        metadata.IncrementRefCount();
        REQUIRE(metadata.GetRefCount() == 1);
        
        metadata.IncrementRefCount();
        REQUIRE(metadata.GetRefCount() == 2);
        
        metadata.DecrementRefCount();
        REQUIRE(metadata.GetRefCount() == 1);
    }
    
    SECTION("Memory Footprint") {
        vyra::asset::AssetMetadata metadata;
        metadata.SetMemoryFootprint(1024 * 1024); // 1MB
        REQUIRE(metadata.GetMemoryFootprint() == 1024 * 1024);
    }
    
    SECTION("Persistent Flag") {
        vyra::asset::AssetMetadata metadata;
        REQUIRE_FALSE(metadata.IsPersistent());
        
        metadata.SetPersistent(true);
        REQUIRE(metadata.IsPersistent());
    }
    
    SECTION("Dependency Management") {
        vyra::asset::AssetMetadata metadata;
        vyra::asset::AssetID depId(vyra::asset::AssetType::Texture, "textures/dependency");
        vyra::asset::AssetDependency dep(depId, vyra::asset::AssetHandle(1, 1), true);
        
        metadata.AddDependency(dep);
        REQUIRE(metadata.GetDependencies().size() == 1);
        
        metadata.RemoveDependency(depId);
        REQUIRE(metadata.GetDependencies().size() == 0);
    }
    
    SECTION("Access Time Tracking") {
        vyra::asset::AssetMetadata metadata;
        metadata.UpdateAccessTime();
        
        // Time since last access should be very small
        uint64_t timeSinceAccess = metadata.GetTimeSinceLastAccessMs();
        REQUIRE(timeSinceAccess < 1000); // Less than 1 second
    }
    
    vyra::Log::Shutdown();
}

TEST_CASE("VYRA Asset System - AssetRegistry", "[asset][unit]") {
    vyra::Log::Init();
    
    SECTION("Registry Creation") {
        vyra::asset::AssetRegistry registry;
        REQUIRE(registry.GetAssetCount() == 0);
    }
    
    SECTION("Asset Registration") {
        vyra::asset::AssetRegistry registry;
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/player");
        
        vyra::asset::AssetHandle handle = registry.RegisterAsset(assetId, "textures/player.png", false);
        REQUIRE(handle.IsValid());
        REQUIRE(registry.GetAssetCount() == 1);
    }
    
    SECTION("Metadata Retrieval") {
        vyra::asset::AssetRegistry registry;
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/player");
        
        vyra::asset::AssetHandle handle = registry.RegisterAsset(assetId);
        vyra::asset::AssetMetadata* metadata = registry.GetMetadata(handle);
        
        REQUIRE(metadata != nullptr);
        REQUIRE(metadata->GetAssetID() == assetId);
    }
    
    SECTION("Reference Counting") {
        vyra::asset::AssetRegistry registry;
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/player");
        
        vyra::asset::AssetHandle handle = registry.RegisterAsset(assetId);
        
        REQUIRE(registry.AddReference(handle));
        REQUIRE(registry.GetMetadata(handle)->GetRefCount() == 1);
        
        REQUIRE(registry.RemoveReference(handle));
        REQUIRE(registry.GetMetadata(handle)->GetRefCount() == 0);
    }
    
    SECTION("Handle Validation") {
        vyra::asset::AssetRegistry registry;
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/player");
        
        vyra::asset::AssetHandle handle = registry.RegisterAsset(assetId);
        REQUIRE(registry.IsValidHandle(handle));
        
        vyra::asset::AssetHandle invalidHandle = vyra::asset::AssetHandle::Invalid();
        REQUIRE_FALSE(registry.IsValidHandle(invalidHandle));
    }
    
    SECTION("Statistics") {
        vyra::asset::AssetRegistry registry;
        vyra::asset::AssetID assetId1(vyra::asset::AssetType::Texture, "textures/player");
        vyra::asset::AssetID assetId2(vyra::asset::AssetType::Mesh, "meshes/player");
        
        registry.RegisterAsset(assetId1);
        registry.RegisterAsset(assetId2);
        
        vyra::asset::AssetRegistry::Statistics stats = registry.GetStatistics();
        REQUIRE(stats.totalAssets == 2);
    }
    
    SECTION("Clear Registry") {
        vyra::asset::AssetRegistry registry;
        vyra::asset::AssetID assetId(vyra::asset::AssetType::Texture, "textures/player");
        
        registry.RegisterAsset(assetId);
        REQUIRE(registry.GetAssetCount() == 1);
        
        registry.Clear();
        REQUIRE(registry.GetAssetCount() == 0);
    }
    
    vyra::Log::Shutdown();
}
