# ADR 0009: Versioning and Release Strategy

- **Status:** Accepted
- **Date:** 2026-08-09
- **Author:** VYRA Architecture Team

## Context

VYRA Engine requires a systematic approach to versioning and releases to support:

1. **Multiple engine versions** running simultaneously (VYRA Hub requirement)
2. **Project compatibility** with specific engine versions
3. **Clear migration paths** between versions
4. **Automated release engineering**
5. **Semantic versioning** for API stability

The current implementation has minimal versioning infrastructure.

## Decision

Implement a comprehensive versioning and release strategy:

### Versioning Scheme

#### Semantic Versioning (SemVer)
- **Format**: MAJOR.MINOR.PATCH (e.g., 0.1.1)
- **MAJOR**: Breaking changes, API redesigns
- **MINOR**: New features, backward-compatible additions
- **PATCH**: Bug fixes, backward-compatible changes

#### Pre-release Identifiers
- **Format**: MAJOR.MINOR.PATCH-PRERELEASE (e.g., 0.1.1-alpha.1)
- **Types**: alpha, beta, rc (release candidate)
- **Numbering**: Sequential (alpha.1, alpha.2, etc.)

#### Build Metadata
- **Format**: MAJOR.MINOR.PATCH+BUILDINFO (e.g., 0.1.1+20231201.123456)
- **Content**: Date, commit hash, build number

### Version Storage

#### CMake Integration
```cmake
project(VYRA
    VERSION 0.1.1
    DESCRIPTION "Modern Open-Source Multiplatform C++23 Game Engine"
)
```

#### Runtime Version Access
- **Header**: `vyra/core/version.hpp`
- **API**: `vyra::Version::GetMajor()`, `vyra::Version::GetMinor()`, etc.
- **String**: `vyra::Version::ToString()`

#### Project File Version
- **Format**: Project.vyra contains `required_engine_version`
- **Validation**: Hub checks compatibility before launch

### Release Categories

#### Foundation Releases (0.1.x)
- **Scope**: Core architecture, basic rendering, editor foundations
- **Stability**: Evolving API, breaking changes expected
- **Support**: Short-term support, rapid iteration

#### Creator Releases (0.2.x)
- **Scope**: Full creator features, stable editor, production-ready
- **Stability**: Stable API, backward compatibility prioritized
- **Support**: Medium-term support, LTS candidates

#### VS Releases (0.5.x)
- **Scope**: VYRA VS integration, AI agent workflows
- **Stability**: Stable API, backward compatibility required
- **Support**: Long-term support, LTS

#### Lang Releases (1.x)
- **Scope**: VYRA Language, complete ecosystem
- **Stability**: Production-ready, API stability guarantees
- **Support**: Long-term support, LTS

### Release Process

#### Phase 1: Feature Freeze
- No new features for current milestone
- Focus on bug fixes and stabilization
- Update documentation

#### Phase 2: Release Candidate
- Create release branch (e.g., release/v0.1.1)
- Perform comprehensive testing
- Generate release notes

#### Phase 3: Release
- Tag release (git tag v0.1.1)
- Build release artifacts
- Publish documentation
- Update CHANGELOG.md

#### Phase 4: Post-Release
- Create next version branch
- Begin feature development
- Monitor for critical issues

### Changelog Format

#### Keep a Changelog Format
```markdown
## [0.1.1] - 2026-08-09

### Added
- New feature descriptions

### Changed
- Backward-compatible changes

### Deprecated
- Features to be removed

### Removed
- Removed features

### Fixed
- Bug fixes

### Security
- Security fixes
```

### Release Engineering

#### Build Configurations
- **Debug**: Full debugging, validation enabled
- **Development**: Optimized debugging, partial validation
- **Release**: Full optimization, no validation
- **RelWithDebInfo**: Release with debug symbols

#### Platform Targets
- **Windows**: MSVC 2022, Vulkan 1.4
- **Linux**: GCC 13+, Vulkan 1.4
- **macOS**: Clang 17+, Vulkan 1.4 (MoltenVK)

#### Package Types
- **Source**: Complete source code
- **SDK**: Pre-built libraries + headers
- **Editor**: Pre-built editor executable
- **Runtime**: Minimal runtime for packaged games

### Compatibility Matrix

#### Engine → Project Compatibility
- **Patch versions**: Fully compatible (0.1.0 → 0.1.1)
- **Minor versions**: May require migration (0.1.x → 0.2.x)
- **Major versions**: Breaking changes (0.x → 1.x)

#### API Stability Guarantees
- **Patch**: No breaking changes
- **Minor**: Breaking changes deprecated first
- **Major**: Breaking changes documented in migration guide

## Consequences

### Positive
- Clear versioning for developers and users
- Compatibility validation through VYRA Hub
- Automated release engineering
- Comprehensive changelog
- Multiple build configurations for different needs

### Negative
- Additional release process overhead
- Compatibility matrix maintenance
- Version checking runtime overhead
- Multiple build configuration complexity

### Mitigation
- Automated version checking tools
- Clear migration documentation
- Compatibility testing automation
- Build system integration

## Implementation Status

- ✅ Versioning strategy defined
- ✅ CMake version integration planned
- ✅ Runtime version API designed
- 🔄 Project version validation pending
- 🔄 Automated release engineering pending
- 🔄 Changelog automation pending

## Release Schedule

#### v0.1.1 Foundation Hardening
- **Target**: 2026-08-15
- **Focus**: Critical fixes, 2D foundation, shader pipeline

#### v0.1.2 Stabilization
- **Target**: 2026-08-30
- **Focus**: Renderer reliability, scene robustness

#### v0.1.3 Creator Editor
- **Target**: 2026-09-15
- **Focus**: Complete editor UI, editor functionality

#### v0.1.4 Performance Foundation
- **Target**: 2026-09-30
- **Focus**: Profiling, ECS optimization, job system

#### v0.1.5 Custom VYRA ECS
- **Target**: 2026-10-15
- **Focus**: Custom ECS development, Rust evaluation

#### v0.1.6 Runtime Systems Foundation
- **Target**: 2026-10-30
- **Focus**: Input, events, runtime lifecycle

#### v0.1.7 Rendering + Asset Maturity
- **Target**: 2026-11-15
- **Focus**: Materials, lighting, asset import

#### v0.1.8 Build + Developer Experience
- **Target**: 2026-11-30
- **Focus**: Project system, build pipeline, documentation

#### v0.1.9 VYRA Hub
- **Target**: 2026-12-15
- **Focus**: Hub application, version management

## Future Evolution

- **v0.2.0**: First LTS release candidate
- **v0.5.0**: VYRA VS integration LTS
- **v1.0.0**: Production-ready VYRA Engine LTS
- **Ongoing**: Security patches for LTS versions