# ADR 0003: Decoupled ECS Architecture via EnTT Wrapper

- **Status**: Accepted
- **Date**: 2026-08-08
- **Author**: VYRA Architecture Team

## Context
An Entity Component System (ECS) is required for high-performance cache-friendly entity management. Direct usage of third-party libraries (like raw EnTT templates) across the entire codebase leaks external dependency symbols and risks breaking API changes when upgrading dependencies.

## Decision
Wrap EnTT inside `vyra::ecs::Registry` and `vyra::ecs::Entity`. High-level components and systems interact only through `vyra::ecs::Entity` handle wrappers.

## Consequences
- Clean separation between engine API and third-party ECS storage.
- Safe component queries (`Emplace`, `GetComponent`, `HasComponent`, `Each`).
- Future migration or customization of entity storage internals remains isolated to `engine/ecs/`.
