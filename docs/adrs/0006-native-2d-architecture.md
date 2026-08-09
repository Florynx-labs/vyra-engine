# ADR 0006: Native 2D Architecture

- **Status:** Accepted
- **Date:** 2026-08-09
- **Author:** VYRA Architecture Team

## Context

VYRA Engine is designed to support 2D, 3D, and Hybrid rendering. The initial v0.1.0 implementation focused on 3D rendering foundations. To fulfill the engine's multi-format promise, we need to implement a native 2D rendering architecture that:

1. Integrates with existing ECS and RHI systems
2. Provides efficient 2D transform handling
3. Supports 2D camera systems with orthographic projection
4. Enables sprite rendering with future sprite atlas support
5. Maintains consistency with 3D systems where appropriate

## Decision

Implement a native 2D architecture with the following components:

### 2D Transform System
- **Transform2DComponent**: 2D transform with position (vec2), rotation (float), scale (vec2), and Z-order for depth sorting
- **Matrix representation**: 3x3 transformation matrices for 2D operations
- **Integration**: Reuses existing ECS component pattern from TransformComponent

### 2D Camera System
- **Camera2D**: Orthographic camera with 2D-specific features
- **Features**: Position, rotation, zoom, aspect ratio handling
- **Projection**: Orthographic projection matrix generation
- **Viewport**: Dynamic viewport resize support
- **Integration**: Follows same pattern as existing CameraComponent

### Sprite Rendering System
- **SpriteComponent**: Color, texture path, tiling factor, and UV coordinates
- **SpriteRenderer**: Batch sprite rendering with quad geometry
- **Vertex Format**: 2D-specific vertex layout (position, UV, color)
- **Future Support**: Sprite atlas integration, 2D animation preparation

### Architecture Principles
1. **Reuse Common Infrastructure**: ECS, RHI, resource management
2. **Separate 2D/3D Rendering**: Distinct pipelines but shared GPU resources
3. **Future-Ready**: Architecture supports sprite atlases, tilemaps, 2D particles
4. **Consistent API**: Similar patterns to 3D systems for developer familiarity

## Consequences

### Positive
- Complete 2D rendering foundation aligned with engine goals
- Efficient 2D-specific transform math (3x3 matrices vs 4x4)
- Orthographic camera optimized for 2D workflows
- Sprite system ready for atlas integration
- Consistent with existing ECS patterns

### Negative
- Additional component types increase system complexity
- 2D/3D separation requires careful rendering pipeline management
- Additional shader requirements for 2D rendering

### Mitigation
- Clear naming conventions (Transform2DComponent vs TransformComponent)
- Shared resource management where possible
- Unified material system for 2D/3D convergence

## Implementation Status

- ✅ Transform2DComponent implemented
- ✅ Camera2D implemented with orthographic projection
- ✅ SpriteComponent implemented with UV coordinate support
- ✅ SpriteRenderer foundation implemented
- 🔄 Integration with existing scene system pending
- 🔄 2D shader pipeline pending
- 🔄 Sprite atlas support planned for future versions

## Future Evolution

- **v0.1.2**: Sprite atlas integration
- **v0.1.3**: 2D animation system
- **v0.1.4**: 2D particle system
- **v0.1.5**: 2D lighting integration
- **v0.1.7**: Tilemap system
- **v0.2.0**: Hybrid 2D/3D scene composition