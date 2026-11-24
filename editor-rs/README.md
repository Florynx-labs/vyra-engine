# VYRA Editor (GPUI)

This crate is the first standalone GPUI-based editor shell for VYRA Engine. It tracks upstream Zed GPUI at the exact revision recorded in `Cargo.toml`.

## Run

Requirements:
- Rust stable (edition 2024)
- A supported desktop environment
- Native platform dependencies required by GPUI

From the repository root:

```bash
cargo run --manifest-path editor-rs/Cargo.toml
```

The first build downloads and compiles GPUI and its platform dependencies.

## Current scope

- Native GPUI application window
- VYRA dark editor shell
- Scene hierarchy, viewport placeholder, inspector and console
- Basic toolbar and editor status bar

The viewport is deliberately a UI placeholder in this first milestone. It is **not** yet connected to the Vulkan renderer or the live C++ scene. The current C++/Dear ImGui editor remains available while the GPUI shell is brought up.

## Planned integration

1. Stabilize the GPUI shell and pin the GPUI dependency.
2. Introduce a narrow C ABI for engine lifecycle, scene snapshots and entity commands.
3. Add a Vulkan viewport interop proof of concept.
4. Move editor panels onto live engine data.
5. Retire Dear ImGui from user-facing editor UI only after feature parity.
