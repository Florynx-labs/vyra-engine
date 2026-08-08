# ADR 0001: Adoption of C++23 as Core Engine Language Standard

- **Status**: Accepted
- **Date**: 2026-08-08
- **Author**: VYRA Architecture Team

## Context
Game engine development requires strong type safety, explicit memory layout control, compile-time computations, and low overhead. Modern C++ standard revisions (C++20/C++23) introduce standard concepts, modules, enhanced constexpr, and ranges that reduce template boilerplate and improve code safety.

## Decision
Adopt **C++23** (`set(CMAKE_CXX_STANDARD 23)`) as the mandatory language standard for all VYRA Engine modules.

## Consequences
- Requires modern compilers (MSVC 2022 v17.8+, GCC 13+, Clang 17+).
- Provides modern features (`std::expected`, `<numbers>`, concepts, improved `constexpr`).
- Ensures zero legacy technical debt in engine codebase.
