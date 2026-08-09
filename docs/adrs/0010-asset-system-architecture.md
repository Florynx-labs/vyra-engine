# ADR 0010: Asset System Architecture

- **Status:** Accepted
- **Date:** 2026-08-09
- **Author:** VYRA Architecture Team

## Context

VYRA Engine requires a comprehensive asset management system to handle the loading, caching, and lifecycle management of game assets. The current implementation has no asset management infrastructure, which is a MEDIUM priority issue from the v0.1.1 architecture audit.

Assets include:
- Textures (2D images, cube maps, etc.)
- Meshes (3D models, vertex data)
- Materials (shader parameters, textures)
- Shaders (vertex, fragment, compute)
- Scenes (entity hierarchies, components)
- Audio (sounds, music)

## Decision

Implement a robust asset system with the following architecture:

### Core Abstractions

#### AssetHandle
- Lightweight reference to an asset
- Generation-based handle to prevent use-after-free
- Thread-safe handle validation
- Minimal overhead (64-bit value)

#### AssetID
- Unique identifier for asset types
- Hierarchical (type + name + version)
- Hash-based for fast lookup
- String representation for debugging

#### AssetMetadata
- Asset type information
- Source file path
- Dependencies (other assets)
- Version information
- Load parameters

#### AssetRegistry
- Central asset database
- Asset loading coordination
- Dependency resolution
- Lifecycle management
- Thread-safe operations

### Asset Loading Pipeline

```
Asset Request → AssetRegistry → AssetLoader → AssetCache → AssetHandle
```

1. **Asset Request**: Entity/system requests asset by AssetID
2. **AssetRegistry**: Checks cache, schedules load if needed
3. **AssetLoader**: Loads raw data from disk/network
4. **AssetCache**: Stores loaded assets with reference counting
5. **AssetHandle**: Returns handle to caller

### Asset Types

#### TextureAsset
- Format (PNG, JPEG, DDS, etc.)
- Mipmap generation
- Compression support
- Upload to GPU

#### MeshAsset
- Vertex/index data
- Material references
- LOD support
- AABB calculation

#### MaterialAsset
- Shader references
- Parameter values
- Texture bindings
- Render state

#### ShaderAsset
- Source code
- Compile options
- Variants
- Reflection data

#### SceneAsset
- Entity hierarchy
- Component data
- References to other assets
- Prefab support

#### AudioAsset
- Audio format (WAV, OGG, MP3)
- Sample rate
- Streaming support
- 3D audio properties

### Memory Management

#### Reference Counting
- Automatic cleanup when no references exist
- Shared ownership semantics
- Explicit release option for force unload

#### Asset Cache
- LRU eviction policy
- Size limits (configurable)
- Priority levels (critical, normal, background)
- Preloading support

#### Hot Reload
- File system monitoring
- Automatic reload on change
- Validation before reload
- Graceful degradation on errors

### Threading Model

#### Thread Safety
- AssetRegistry fully thread-safe
- Asset loaders execute on worker threads
- Asset cache uses atomic reference counting
- AssetHandle validation is lock-free

#### Load Priorities
- Critical assets load immediately
- Background assets load asynchronously
- Dependent assets load in dependency order
- Progress callbacks for long loads

### Error Handling

#### Load Failures
- Graceful degradation (fallback assets)
- Detailed error reporting
- Retry mechanisms (network assets)
- User notifications

#### Validation
- Asset format validation
- Integrity checking
- Version compatibility
- Dependency validation

## Implementation Strategy

### Phase 1: Core Infrastructure (v0.1.2)
- AssetHandle/AssetID implementation
- AssetRegistry foundation
- Basic asset loading pipeline
- Memory management basics

### Phase 2: Texture System (v0.1.3)
- TextureAsset implementation
- Image loading (stb_image)
- GPU upload integration
- Mipmap generation

### Phase 3: Mesh System (v0.1.4)
- MeshAsset implementation
- Model loading (assimp)
- Vertex buffer integration
- LOD support

### Phase 4: Material System (v0.1.5)
- MaterialAsset implementation
- Shader integration
- Parameter binding
- Material instances

### Phase 5: Scene System (v0.1.6)
- SceneAsset implementation
- Prefab support
- Asset references
- Deep copy optimization

### Phase 6: Audio System (v0.1.7)
- AudioAsset implementation
- Audio loading (OpenAL)
- Streaming support
- 3D audio integration

### Phase 7: Advanced Features (v0.1.8)
- Hot reload
- Asset bundles
- Asset compression
- Network loading

## Consequences

### Positive
- Comprehensive asset management
- Memory efficient with reference counting
- Thread-safe for modern multi-core systems
- Extensible for future asset types
- Hot reload support for development workflow

### Negative
- Additional system complexity
- More dependencies (stb_image, assimp, etc.)
- Larger memory footprint for cache
- Development time for implementation

### Mitigation
- Modular design allows incremental implementation
- Asset-specific loaders prevent monolithic system
- Configurable cache sizes prevent memory bloat
- Comprehensive testing for thread safety

## Future Evolution

- **v0.2.0**: GPU-driven asset streaming
- **v0.3.0**: Asset bundles for distribution
- **v0.4.0**: Asset market integration
- **v0.5.0**: AI-assisted asset generation
- **v1.0.0**: Complete asset pipeline with tooling

## Integration Points

### Editor Integration
- Asset browser UI
- Asset import pipeline
- Asset preview system
- Asset version control

### Runtime Integration
- Asset manager for game logic
- Asset preloading system
- Asset unloading for memory management
- Asset streaming for open worlds

### RHI Integration
- Texture upload to GPU
- Buffer creation for meshes
- Shader compilation integration
- Memory allocation tracking

## Success Criteria

### v0.1.2 Gate
- AssetHandle/AssetID implementation complete
- AssetRegistry foundation functional
- Basic loading pipeline operational
- Thread-safe operations verified
- Memory management functional
- No memory leaks in normal operation