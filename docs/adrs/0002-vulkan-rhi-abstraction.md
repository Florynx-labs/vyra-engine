# ADR 0002: Vulkan 1.4 Render Hardware Interface (RHI) Abstraction

- **Status**: Accepted
- **Date**: 2026-08-08
- **Author**: VYRA Architecture Team

## Context
Game engines need flexibility across different graphics APIs (Vulkan, DirectX 12, Metal). Exposing raw Vulkan headers (`vulkan.h`) throughout high-level gameplay code introduces tight coupling and compilation bloat.

## Decision
Implement a decoupled Render Hardware Interface (`vyra::rhi`) with pure virtual base classes (`RHIContext`, `RHIDevice`, `RHISwapChain`, `RHIBuffer`, `RHIShader`, `RHIPipeline`). The initial implementation uses Vulkan 1.4 loaded dynamically via Volk.

## Consequences
- High-level modules (`vyra::scene`, `vyra::renderer`) only interact with `vyra::rhi::*` types.
- Vulkan headers are completely private to `engine/rhi/src/vulkan/`.
- Future rendering backends (DirectX 12 / Metal) can be added cleanly without changing engine client code.
