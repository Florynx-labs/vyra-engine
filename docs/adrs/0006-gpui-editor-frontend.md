# ADR 0006: GPUI Editor Frontend

- **Status:** Accepted
- **Date:** 2026-11-24

## Context

VYRA's current editor is a C++ Dear ImGui application. The project is built around a C++23 engine core and a Vulkan 1.4 rendering backend. The editor needs a richer desktop UI without coupling the engine runtime to a UI framework.

## Decision

Adopt GPUI as the intended frontend for the user-facing editor, hosted in a standalone Rust application under `editor-rs/`. Keep the C++ engine and renderer as the authoritative runtime.

The first implementation is intentionally a visual shell. It does not embed Vulkan, load scenes through the engine, or mutate C++ objects.

## Integration boundary

Future communication must use a versioned, narrow C ABI. Rust must not own C++ objects or retain unvalidated raw pointers. Initial bridge operations should expose:
- engine startup/shutdown and structured error reporting;
- immutable scene/entity snapshots with stable IDs;
- explicit commands for entity/component edits;
- renderer surface lifecycle only after a dedicated platform interop prototype.

## Migration

Dear ImGui remains in the existing C++ editor during the transition and may remain useful for internal diagnostics. It is not removed by this decision.

## Risks

- GPUI is pre-1.0 and can introduce breaking changes. Pin exact versions and isolate GPUI-specific code.
- Vulkan surface ownership and native window handles differ by platform. No cross-platform viewport embedding is assumed until proven.
- Rust/C++ ABI ownership and thread boundaries must be documented and tested.
