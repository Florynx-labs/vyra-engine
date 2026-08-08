# VYRA Engine

> **Create. Command. Build.**  
> *Where worlds are engineered.*

VYRA Engine is a modern, open-source, multiplatform C++23 game engine designed from its foundation for modularity, high performance, clean architecture, and future native agentic AI workflows.

---

## Features (v0.1 Foundation)

- **C++23 Modern Architecture**: Clean, modular layered targets (`vyra::core`, `vyra::platform`, `vyra::rhi`, `vyra::ecs`, `vyra::scene`, `vyra::runtime`, `vyra::editor`).
- **Cross-Platform HAL**: SDL3 windowing & input abstraction.
- **Vulkan 1.4 RHI**: Modern graphics API layer decoupled from gameplay logic.
- **Decoupled ECS**: EnTT backend wrapped under a zero-leak `vyra::ecs` API.
- **Deterministic Scene Serialization**: `.vyra` human-readable JSON project format.
- **Obsidian Editor Shell**: Modern Dear ImGui docking interface with dark engineering aesthetic.
- **Isolated Play Mode**: Execution sandbox cloning the `Editor World` into `Runtime World`.

---

## Quick Start & Building

### Prerequisites
- C++23 compliant compiler (MSVC 2022 v17.8+, Clang 17+, GCC 13+)
- CMake 3.28+
- Vulkan SDK 1.3+ / 1.4+
- Ninja / MSBuild

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/florynx-labs/vyra-engine.git
cd vyra-engine

# Configure CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

# Build targets
cmake --build build --config Debug

# Run tests
ctest --test-dir build --output-on-failure
```

---

## License

VYRA Engine is released under the [MIT License](LICENSE). Developed by Florynx Labs and open-source contributors.
