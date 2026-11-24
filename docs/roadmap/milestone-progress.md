# VYRA Engine Roadmap Milestone Progress

**Last audited:** 2026-11-24  
**Current version:** v0.1.4 (hardening in progress)  
**Target:** v0.1.9 VYRA Hub  
**Milestone accounting:** 4 of 10 milestones have implementation marked complete (v0.1.0–v0.1.3). This is 40% of the v0.1.0–v0.1.9 sequence. Completion does not imply every platform or GPU validation gate has passed.

## Milestone status

| Version | Milestone | Status | Remaining gate |
|---|---|---|---|
| v0.1.0 | Foundation | Implementation complete | Historical baseline |
| v0.1.1 | Foundation hardening | Implementation complete | Historical baseline |
| v0.1.2 | Stabilization | Implementation marked complete | Re-run full suite on clean CI |
| v0.1.3 | Asset system & reliability | Implementation marked complete | Validate import pipeline scope; full GPU validation |
| v0.1.4 | Performance foundation | **Hardening in progress** | Clean Debug/Release builds, tests, GPU validation |
| v0.1.5 | Custom VYRA ECS | Planned | ECS design, migration strategy and benchmarks |
| v0.1.6 | Runtime systems | Planned | Input, event and lifecycle APIs |
| v0.1.7 | Rendering & asset maturity | Planned | Materials, lighting, texture/mesh import |
| v0.1.8 | Build & developer experience | Planned | Project format, build pipeline and tools |
| v0.1.9 | VYRA Hub | Planned | Launcher, version management and project workflow |

## v0.1.4 — Performance foundation

### Implemented in the baseline
- CPU/frame/memory profiling foundation and profiling macros.
- Worker-thread job queue, handles and scheduler foundation.

### Hardening in this branch
- Shared completion state and repeatable job waits.
- Scheduler lifecycle synchronization and graceful queue draining.
- Condition-variable-based `WaitForAll` instead of busy polling.
- Exception-safe worker execution.
- Thread-safe profiler snapshots and monotonic timing.
- Saturating memory free accounting.
- Fixed-size object pool with stable addresses and deterministic lifetime.
- ECS entity-storage reservation API.
- Regression tests for the above paths.
- Headless unit-test configuration via `VYRA_BUILD_GPU_TESTS=OFF`.

### Release gate
v0.1.4 is not complete until clean Debug and Release builds, the complete unit suite, Windows compilation and a Vulkan-capable GPU validation run are green. No benchmark improvement is claimed until repeatable before/after measurements exist.

## v0.1.5 — Custom VYRA ECS

- Specify entity/component/storage invariants and public API.
- Benchmark EnTT baseline against the proposed implementation.
- Implement archetype or sparse-set strategy only after workload measurements.
- Provide migration and compatibility tests.
- Do not remove EnTT until parity and performance gates pass.

## v0.1.6 — Runtime systems

- Input abstraction and action mapping.
- Event dispatch and runtime lifecycle.
- Headless runtime target independent of editor UI.
- Deterministic update/timestep tests.

## v0.1.7 — Rendering and assets

- Material and shader resource model.
- Texture and mesh import pipeline.
- Lighting foundation and render graph direction.
- RHI-only renderer calls; Vulkan-specific code stays in backend modules.

## v0.1.8 — Developer experience

- Project manifest and versioned project format.
- Reproducible build pipeline.
- Asset inspection and import tooling.
- API docs and sample projects.

## v0.1.9 — VYRA Hub

- Project launcher and recent-project workflow.
- Engine version installation/selection.
- Project templates and settings.
- Safe update and rollback workflow.

## Open technical debt

- Renderer bypasses RHI abstractions: [#7](https://github.com/Florynx-labs/vyra-engine/issues/7), [#14](https://github.com/Florynx-labs/vyra-engine/issues/14).
- ECS entity raw registry pointer lifetime: [#13](https://github.com/Florynx-labs/vyra-engine/issues/13).
- Resource use-after-free, reference counting and generation handles: [#4](https://github.com/Florynx-labs/vyra-engine/issues/4), [#5](https://github.com/Florynx-labs/vyra-engine/issues/5), [#6](https://github.com/Florynx-labs/vyra-engine/issues/6).
- Scene serialization schema validation and migration: [#8](https://github.com/Florynx-labs/vyra-engine/issues/8).
- Vulkan validation and platform build coverage: [#9](https://github.com/Florynx-labs/vyra-engine/issues/9), [#15](https://github.com/Florynx-labs/vyra-engine/issues/15), [#10](https://github.com/Florynx-labs/vyra-engine/issues/10), [#16](https://github.com/Florynx-labs/vyra-engine/issues/16).

See [P0/P1 audit](../audits/2026-11-p0-p1-audit.md) for detailed findings and validation commands.
