# ADR 0008: Vulkan Testing Strategy

- **Status:** Accepted
- **Date:** 2026-08-09
- **Author:** VYRA Architecture Team

## Context

Vulkan is a complex, low-level graphics API that requires comprehensive testing to ensure:

1. **Correctness**: Proper GPU resource management and synchronization
2. **Performance**: Efficient rendering without unnecessary overhead
3. **Compatibility**: Works across different GPUs and drivers
4. **Validation**: Catches errors using Vulkan validation layers
5. **CI Integration**: Enables automated testing without blocking headless CI

The current test suite has minimal RHI coverage and no GPU-specific tests.

## Decision

Implement a multi-tiered Vulkan testing strategy:

### Test Categories

#### 1. Unit Tests (CPU-only)
- **Scope**: RHI interface testing without GPU
- **Examples**: Command pool allocation, handle validation, error code conversion
- **Environment**: Any CI environment, no GPU required
- **Framework**: Catch2, tagged with `[vulkan][unit]`

#### 2. Smoke Tests (GPU-required)
- **Scope**: Basic Vulkan initialization and object creation
- **Examples**: Context creation, device selection, surface queries
- **Environment**: Requires GPU with Vulkan support
- **Framework**: Catch2, tagged with `[vulkan][gpu][smoke]`
- **CI**: Optional GPU runners, skip gracefully if unavailable

#### 3. Integration Tests (GPU-required)
- **Scope**: Cross-subsystem Vulkan integration
- **Examples**: Full render pipeline, swapchain rendering, resource lifecycle
- **Environment**: Requires GPU with Vulkan support
- **Framework**: Catch2, tagged with `[vulkan][gpu][integration]`
- **CI**: Dedicated GPU test runners

#### 4. Validation Tests (Debug builds)
- **Scope**: Vulkan validation layer error detection
- **Examples**: Resource synchronization, memory safety, API usage
- **Environment**: Debug builds with validation layers enabled
- **Framework**: Catch2, tagged with `[vulkan][validation]`
- **CI**: Debug configuration GPU runners

### Test Organization

```
tests/
├── unit/           # CPU-only unit tests
│   └── test_rhi.cpp
├── gpu/            # GPU-specific tests
│   ├── test_vulkan_smoke.cpp
│   ├── test_vulkan_integration.cpp
│   └── test_vulkan_validation.cpp
└── benchmark/      # Performance benchmarks
    └── benchmark_vulkan.cpp
```

### CI Strategy

#### Headless CI (GPU unavailable)
- Run unit tests only
- Skip GPU tests with graceful degradation
- Report test coverage for CPU components

#### GPU CI (GPU available)
- Run all test categories
- Enable validation layers in debug builds
- Generate validation reports
- Performance benchmarking

### Validation Layer Strategy

#### Debug Builds
- Always enable validation layers
- Use VK_EXT_debug_utils for debug messaging
- Enable all validation features:
  - Best practices
  - Synchronization
  - GPU-assisted validation

#### Release Builds
- Disable validation layers for performance
- Use VK_KHR_shader_subgroup_extended_types for optimization
- Minimal validation for critical paths only

### Resource Lifecycle Testing

#### Object Creation
- Test successful creation with valid parameters
- Test failure handling with invalid parameters
- Verify handle uniqueness and validity

#### Object Destruction
- Test proper destruction order
- Test double-destruction detection
- Test resource cleanup on error paths

#### Synchronization
- Test fence timeout handling
- Test semaphore signaling
- Test command buffer completion

### Memory Testing

#### Allocation
- Test different memory types (device-local, host-visible)
- Test allocation failure handling
- Test memory alignment requirements

#### Mapping
- Test memory mapping/unmapping
- Test flush/invalidate operations
- Test mapped memory access patterns

## Consequences

### Positive
- Comprehensive Vulkan validation
- CI-compatible testing strategy
- Clear separation of test concerns
- Performance regression detection
- Better error detection through validation layers

### Negative
- Increased test maintenance overhead
- GPU test infrastructure requirements
- Longer CI pipeline times for GPU tests
- Potential CI flakiness from GPU/driver variations

### Mitigation
- Clear test categorization and tagging
- GPU test runners with consistent hardware
- Test timeout and retry logic
- Comprehensive test documentation

## Implementation Status

- ✅ Test categorization strategy defined
- ✅ GPU smoke test structure implemented
- 🔄 Integration test suite pending
- 🔄 Validation test suite pending
- 🔄 CI pipeline integration pending
- 🔄 Performance benchmarking pending

## Test Coverage Goals

- **Unit Tests**: 80%+ coverage of RHI interfaces
- **Smoke Tests**: 100% of basic Vulkan operations
- **Integration Tests**: Major rendering pipelines
- **Validation Tests**: All critical error paths
- **Performance**: Key rendering operations benchmarked

## Future Evolution

- **v0.1.2**: Complete integration test suite
- **v0.1.3**: Validation test suite with error injection
- **v0.1.4**: Performance regression testing
- **v0.1.5**: Multi-GPU compatibility testing
- **v0.2.0**: Automated GPU fuzzing