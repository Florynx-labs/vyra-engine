# Rust editor native bridge

The GPUI editor talks to the C++ scene through the versioned C ABI in
`engine/editor_bridge`. Rust ownership, string conversion, ABI validation,
and native error handling live in `editor-rs/src/engine.rs`.

## Build the native shared library

From the repository root:

```sh
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DVYRA_BUILD_TESTS=ON \
  -DVYRA_BUILD_SAMPLES=OFF \
  -DVYRA_BUILD_GPU_TESTS=OFF
cmake --build build --target vyra_editor_bridge --parallel
ctest --test-dir build --output-on-failure
```

The shared library is produced in the CMake build tree (typically
`build/engine/editor_bridge/libvyra_editor_bridge.so` on Linux).

## Build the Rust editor against it

Set `VYRA_ENGINE_LIB_DIR` to the directory containing the shared library:

```sh
VYRA_ENGINE_LIB_DIR="$PWD/build/engine/editor_bridge" \
  cargo build --manifest-path editor-rs/Cargo.toml
```

On Linux and macOS the build script adds a local runtime search path for that
directory. On Windows, place the bridge DLL beside the editor executable or
add its directory to `PATH`.

A plain `cargo check` intentionally remains usable in CI without building or
linking the native engine. Running/linking the editor requires the shared
library and the environment variable above.

## Rust API

`vyra_editor::engine::EngineBridge` owns one native scene and exposes safe
methods for listing, creating, renaming, destroying, and clearing entities.
The wrapper validates ABI version 1, converts Rust strings to C strings,
translates native error messages, and destroys the opaque handle through
`Drop`. The native scene is not marked Send or Sync.

This step establishes the Rust/native boundary. The GPUI hierarchy and
Inspector still need to be switched from preview data to this API in the
next integration step; the viewport remains a separate renderer-interoperability task.
