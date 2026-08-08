# Contributing to VYRA Engine

Thank you for your interest in contributing to VYRA Engine!

## Principles & Guidelines

1. **Layered Architecture**: Respect module boundaries. Core cannot depend on Platform, Platform cannot depend on RHI, Renderer cannot depend on Editor.
2. **Code Style**: Format code using `.clang-format` (LLVM base, 4 spaces indent, 120 column limit).
3. **C++23 Standard**: Use modern C++23 features where appropriate (concepts, ranges, `std::expected`, `std::format`, RAII).
4. **No Naked Allocation**: Prefer smart pointers or engine custom allocators. Ownership must be explicit.
5. **Testing**: Every new feature or bug fix in core/ecs/scene must include Catch2 unit tests under `tests/unit/`.

## Submitting Pull Requests

1. Fork the repository and create a branch from `main`.
2. Ensure your changes compile cleanly without warnings (`-Werror` / `/WX`).
3. Run test suite: `ctest --test-dir build --output-on-failure`.
4. Open a pull request describing the changes and referencing any related issues.
