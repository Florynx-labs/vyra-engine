<p align="center">
  <img src="vyra-logo.png" alt="VYRA Engine Logo" width="360"/>
</p>

<h1 align="center">VYRA ENGINE</h1>

<p align="center">
  <strong>Create. Command. Build.</strong><br/>
  <em>Where worlds are engineered.</em>
</p>

<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-blue.svg" alt="License"/></a>
  <a href="#"><img src="https://img.shields.io/badge/Version-v0.1.4-informational.svg" alt="Version 0.1.4"/></a>
  <a href="#"><img src="https://img.shields.io/badge/C%2B%2B-23-purple.svg" alt="C++23"/></a>
  <a href="#"><img src="https://img.shields.io/badge/Vulkan-1.4-red.svg" alt="Vulkan 1.4"/></a>
  <a href="#"><img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux%20%7C%20macOS-lightgrey.svg" alt="Platform"/></a>
  <a href="#"><img src="https://img.shields.io/badge/Tests-Catch2%20suite-informational.svg" alt="Catch2 test suite"/></a>
</p>

---

## Overview

**VYRA Engine** is a modern, open-source, multiplatform C++23 game engine designed around performance, strict layer decoupling, modern Vulkan 1.4 rendering, and extensible tooling.

Developed under **Florynx Labs**, VYRA provides a high-performance alternative engine built for engineers who value architectural elegance, low-latency execution, and a roadmap focused on measurable performance and correctness.

---

## Key Features (v0.1.4 · hardening in progress)

- **C++23 Native Core**: Modern C++ architecture (`vyra::core`, `vyra::platform`, `vyra::rhi`, `vyra::renderer`, `vyra::ecs`, `vyra::scene`, `vyra::editor`).
- **Vulkan 1.4 RHI foundation**: Vulkan backend accessed through the engine graphics abstraction; remaining renderer bypasses are tracked as technical debt.
- **Decoupled ECS**: EnTT backend encapsulated inside `vyra::ecs::Registry` and `vyra::ecs::Entity`.
- **Procedural Geometry & Mesh Renderer**: Built-in 3D mesh primitives (Cube, Grid floor, Sphere) and GLSL hemisphere lighting shader pipeline.
- **Deterministic Scene Serialization**: Human-readable `.vyra` JSON scene format with 128-bit UUID entity tracking.
- **Legacy C++ Editor**: Dear ImGui docking interface retained during the transition.
- **GPUI Editor Preview**: Standalone Rust editor shell with hierarchy, viewport workspace, inspector and console (experimental; see `editor-rs/`).
- **Transient World Isolation**: Sandbox Play Mode cloning authoring scenes to isolated runtime worlds (`Edit`, `Play`, `Pause`).
- **Catch2 test suite**: Unit and optional GPU integration tests covering core, platform, RHI, ECS, scene, assets, profiling and jobs. CI validation is tracked separately from test source coverage.

---

## System Architecture

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

For detailed component documentation, see [Architecture Overview](docs/architecture.md).

---

## Architectural Decision Records (ADRs)

- [ADR 0001: C++23 Standard Adoption](docs/adrs/0001-cpp23-standard.md)
- [ADR 0002: Vulkan 1.4 RHI Abstraction](docs/adrs/0002-vulkan-rhi-abstraction.md)
- [ADR 0003: Decoupled ECS Architecture](docs/adrs/0003-entt-ecs-wrapper.md)
- [ADR 0004: JSON Scene Serialization](docs/adrs/0004-json-scene-serialization.md)
- [ADR 0005: Transient World Isolation](docs/adrs/0005-transient-world-isolation.md)
- [ADR 0006: GPUI Editor Frontend](docs/adrs/0006-gpui-editor-frontend.md)

---

## Quick Start & Building

### Prerequisites
- **Compiler**: MSVC 2022 v17.8+ (Windows), GCC 13+ or Clang 17+ (Linux/macOS)
- **Build Tools**: CMake 3.28+, Ninja or MSBuild
- **SDK**: Vulkan SDK 1.3 / 1.4

### Build Commands

```bash
# Clone the repository
git clone https://github.com/florynx-labs/vyra-engine.git
cd vyra-engine

# Configure CMake build tree
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build engine, editor & tests
cmake --build build --config Debug

# Execute full automated test suite
ctest --test-dir build --output-on-failure -C Debug
```

---

### GPUI Editor Preview

The experimental GPUI shell can be launched independently of the C++ editor:

```bash
cargo run --manifest-path editor-rs/Cargo.toml
```

It currently provides the editor workspace UI only. The live engine bridge and Vulkan viewport integration are planned follow-up milestones. See [editor-rs/README.md](editor-rs/README.md).

## License

VYRA Engine is open-source software licensed under the [MIT License](LICENSE).  
Copyright (c) 2026 **Florynx Labs**.
