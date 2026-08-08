# ADR 0005: Transient World Isolation for Play Mode

- **Status**: Accepted
- **Date**: 2026-08-08
- **Author**: VYRA Architecture Team

## Context
When running Play Mode inside a game engine editor, runtime scripts, physics, and gameplay mechanics mutate entity transforms, spawn temporary objects, or destroy entities. Mutating the authoring scene directly causes permanent loss of editor state.

## Decision
Implement transient world sandboxing via `Scene::Copy`. Entering Play Mode clones `m_EditorScene` to `m_ActiveScene`. Stopping Play Mode discards `m_ActiveScene` and restores `m_ActiveScene = m_EditorScene`.

## Consequences
- Guaranteed zero side-effects on authoring scenes during gameplay testing.
- Simple, rock-solid state machine (`Edit`, `Play`, `Pause`).
- Verified with automated unit test suite.
