# ADR 0007: Slang Shader Pipeline

- **Status:** Accepted
- **Date:** 2026-08-09
- **Author:** VYRA Architecture Team

## Context

VYRA Engine currently uses GLSL shaders directly in the renderer layer. This approach has several limitations:

1. **No shader abstraction**: Renderer tightly coupled to GLSL
2. **Limited cross-platform**: GLSL requires different targets for different platforms
3. **No reflection**: No automatic shader reflection for resource binding
4. **No caching**: No shader compilation caching
5. **No future-proofing**: Blocks transition to more modern shader languages

Slang is a modern shading language that offers:
- Cross-platform compilation (SPIR-V, HLSL, GLSL, etc.)
- Built-in reflection capabilities
- Modern language features
- Better tooling and IDE support
- Future AI-native workflow compatibility

## Decision

Implement a comprehensive shader pipeline with Slang as the primary shader language:

### Shader Abstraction Layer
- **ShaderSource**: Language-agnostic shader source representation
- **ShaderCompiler**: Pluggable compiler interface for different languages
- **ShaderArtifact**: Compiled shader bytecode (SPIR-V) with metadata
- **ShaderReflector**: Automatic shader reflection for resource binding
- **ShaderCache**: Caching system for compiled shaders
- **ShaderManager**: High-level shader management interface

### Language Support
- **Primary**: Slang (target language)
- **Transitional**: GLSL (marked as deprecated, will be phased out)
- **Future**: HLSL (for DirectX compatibility)
- **Intermediate**: SPIR-V (Vulkan target)

### Pipeline Architecture
```
Shader Source (Slang/GLSL)
    ↓
Shader Compiler (Slang compiler / glslang)
    ↓
Shader Artifact (SPIR-V bytecode)
    ↓
Shader Reflection (Resource binding, input layout)
    ↓
Shader Cache (File-based, versioned)
    ↓
RHI Integration (RHIShader abstraction)
```

### Implementation Strategy
1. **Phase 1**: Implement shader abstraction interfaces
2. **Phase 2**: Integrate Slang compiler
3. **Phase 3**: Implement shader reflection
4. **Phase 4**: Implement shader caching
5. **Phase 5**: Migrate existing GLSL shaders to Slang
6. **Phase 6**: Deprecate direct GLSL usage

## Consequences

### Positive
- Modern shader language with better tooling
- Cross-platform shader compilation
- Automatic shader reflection reduces boilerplate
- Shader caching improves load times
- Future-proof for AI-native workflows
- Clean separation between shader source and RHI

### Negative
- Additional build complexity (Slang compiler integration)
- Migration effort for existing GLSL shaders
- Larger dependency footprint
- Learning curve for developers unfamiliar with Slang

### Mitigation
- Gradual migration path with GLSL compatibility
- Comprehensive documentation and examples
- Build system integration for automatic compilation
- Shader validation and error reporting

## Implementation Status

- ✅ Shader abstraction interfaces designed
- ✅ ShaderSource, ShaderArtifact, ShaderReflector interfaces defined
- ✅ File-based shader cache interface designed
- 🔄 Slang compiler integration pending
- 🔄 GLSL to Slang migration pending
- 🔄 Existing GLSL shaders marked as transitional

## GLSL Transition Plan

1. **v0.1.1**: Mark existing GLSL as transitional, document migration path
2. **v0.1.2**: Implement Slang compiler integration
3. **v0.1.3**: Implement shader reflection system
4. **v0.1.4**: Migrate core shaders to Slang
5. **v0.1.5**: Deprecate GLSL usage, add warnings
6. **v0.1.6**: Remove GLSL dependencies, Slang-only pipeline

## Future Evolution

- **v0.1.7**: Hot-reload support for Slang shaders
- **v0.1.8**: Shader variant system (quality levels, platforms)
- **v0.2.0**: AI-assisted shader generation with Slang
- **v0.3.0**: Compute shader integration with Slang