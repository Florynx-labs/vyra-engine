# ADR 0004: Human-Readable JSON Scene Format (.vyra)

- **Status**: Accepted
- **Date**: 2026-08-08
- **Author**: VYRA Architecture Team

## Context
Engine project scenes must be stored on disk in a format suitable for version control (git diffs), manual inspection, and robust serialization across engine updates.

## Decision
Adopt human-readable JSON (`.vyra` extension) for scene project files powered by `nlohmann_json`. Every entity is identified by a 128-bit `UUID` to maintain entity references across sessions.

## Consequences
- Clean Git diffs when changing entity properties in the Editor.
- Deterministic roundtrip serialization (100% data integrity verified in Catch2 unit tests).
- Binary scene caching can be added in future versions as an optimization layer.
