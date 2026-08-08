# Changelog

All notable changes to VYRA Engine will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2026-08-08

### Added
- Initial project architecture setup for VYRA Engine v0.1 Foundation.
- Modular C++23 CMake target configuration (`vyra::core`, `vyra::platform`, `vyra::rhi`, `vyra::ecs`, `vyra::scene`, `vyra::runtime`, `vyra::editor`).
- Asynchronous multi-channel logging system (`spdlog` wrapper) with `VYRA_LOG_*` and `VYRA_ASSERT` macros.
- Core math, UUID, timestep, and high-precision timer utilities.
- Catch2 unit test suite integration.
