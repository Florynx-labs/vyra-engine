# VYRA Engine — Architecture Overview

VYRA Engine is engineered with a strict layered architecture designed for long-term scalability, low-latency performance, and clean isolation between system modules.

```
                  +-----------------------------------+
                  |            vyra_editor            |
                  |  (Dear ImGui Docking, Editor UI)  |
                  +-----------------+-----------------+
                                    |
                  +-----------------v-----------------+
                  |             vyra_scene            |
                  | (Scene, EditorCamera, Serializer) |
                  +-----------------+-----------------+
                                    |
                  +-----------------v-----------------+
                  |            vyra_renderer          |
                  |   (Mesh, Vertex3D, MeshRenderer)  |
                  +--------+----------------+---------+
                           |                |
         +-----------------v---+        +---v-----------------+
         |       vyra_ecs      |        |       vyra_rhi      |
         | (EnTT Engine Wrap)  |        | (Vulkan 1.4 Context)|
         +-----------------+---+        +---+-----------------+
                           |                |
         +-----------------v----------------v---------+
         |                vyra_platform               |
         |         (SDL3 Windowing & Events)          |
         +-------------------------+------------------+
                                   |
         +-------------------------v------------------+
         |                 vyra_core                  |
         |       (Log, UUID, Time, Assert, Types)     |
         +--------------------------------------------+
```

---

## Layer Responsibilities

### 1. `vyra::core`
- Primitive types, `Ref<T>` / `Scope<T>` smart pointers.
- Structured logging system based on `spdlog` (`VYRA_LOG_*`).
- Deterministic 128-bit UUID generation (`vyra::UUID`).
- High-precision time and delta step calculation (`Timestep`).

### 2. `vyra::platform`
- OS window lifecycle management via SDL3.
- Event dispatch system for keyboard, mouse, window events.
- Platform input polling abstractions.

### 3. `vyra::rhi` (Render Hardware Interface)
- Abstract graphics hardware layer (`RHIContext`, `RHIDevice`, `RHISwapChain`).
- Vulkan 1.4 implementation via `volk` dynamic loader.
- Graphics resources abstraction (`RHIBuffer`, `RHIShader`, `RHIPipeline`).

### 4. `vyra::renderer`
- Geometry data structures (`Mesh`, `Vertex3D`).
- Procedural primitive mesh factory (Cube, Grid floor, UV Sphere).
- High-level mesh rendering system with GLSL shader support (`mesh.vert`, `mesh.frag`).

### 5. `vyra::ecs`
- Strict wrapper around EnTT engine registry.
- `Entity` handle abstraction isolating external components from EnTT details.
- Component queries, emplace/replace semantics, tag & transform components.

### 6. `vyra::scene`
- World containers (`Scene`).
- `EditorCamera` orbit/pan/zoom flight controller.
- JSON scene serialization & deserialization (`SceneSerializer`).
- Isolated scene cloning for Play Mode state sandboxing (`Scene::Copy`).

### 7. `vyra::editor`
- Professional Obsidian Dark UI theme styled for engineering workflows.
- Dockable interface panels (`ViewportPanel`, `HierarchyPanel`, `InspectorPanel`, `ConsolePanel`).
- State machine toolbar for Edit / Play / Pause simulation control.
