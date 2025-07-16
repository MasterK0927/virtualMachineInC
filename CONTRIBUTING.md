# Contributing to SimpleVM

Thank you for your interest in contributing to SimpleVM! This document provides guidelines for contributing to the project.

## Development Setup

### Prerequisites
- C++17 compatible compiler (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.15+
- Git
- SDL2 (optional, for GUI)

### Building
```bash
git clone <repository-url>
cd virtualMachineInC
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build -j
```

### Running Tests
```bash
ctest --test-dir build --output-on-failure
./build/vm_tests
```

## Code Style

### C++ Guidelines
- Follow C++17 standard
- Use RAII for resource management
- Prefer `std::unique_ptr` and `std::shared_ptr` over raw pointers
- Use `const` wherever possible
- Avoid global variables

### Naming Conventions
- Classes: `PascalCase` (e.g., `SimpleCPU`)
- Functions/methods: `camelCase` (e.g., `runSteps`)
- Variables: `camelCase` (e.g., `memSize`)
- Constants: `UPPER_CASE` (e.g., `REG_COUNT`)
- Private members: `m_` prefix (e.g., `m_registers`)

### File Organization
- Headers in `include/vm/`
- Implementation in `src/`
- Applications in `apps/`
- Tests in `tests/`
- Documentation in `docs/`

## Architecture Guidelines

### Interfaces
- Use pure virtual interfaces for major components
- Keep interfaces minimal and focused
- Document interface contracts clearly

### Error Handling
- Use exceptions for error conditions
- Provide clear error messages
- Validate inputs at public API boundaries

### Memory Safety
- Always check array bounds
- Use RAII for resource cleanup
- Avoid memory leaks

## Contribution Process

### 1. Issue Discussion
- Check existing issues before creating new ones
- Discuss major changes in issues before implementation
- Use issue templates when available

### 2. Development
- Create a feature branch from `master`
- Make focused, atomic commits
- Write clear commit messages
- Add tests for new functionality

### 3. Testing
- Ensure all existing tests pass
- Add unit tests for new features
- Test on multiple platforms if possible
- Update documentation as needed

### 4. Pull Request
- Create PR with clear description
- Reference related issues
- Ensure CI passes
- Respond to review feedback

## Commit Message Format

```
type(scope): brief description

Longer description if needed

Fixes #123
```

Types:
- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes
- `refactor`: Code refactoring
- `test`: Test additions/changes
- `build`: Build system changes

## Adding New Features

### New Instructions
1. Add opcode to `include/vm/Opcodes.hpp`
2. Update decoder in `src/Decoder.cpp`
3. Implement execution in `src/CPU.cpp`
4. Add assembler support in `apps/asm/main.cpp`
5. Add tests and examples

### New Devices
1. Implement `IDevice` interface
2. Add device to `src/` directory
3. Update CMakeLists.txt
4. Add device mapping in `Instance.cpp`
5. Document device interface

### GUI Panels
1. Inherit from `Panel` base class
2. Implement `draw()` method
3. Add to `GuiApp.cpp`
4. Follow ImGui best practices

## Testing Guidelines

### Unit Tests
- Test individual components in isolation
- Use descriptive test names
- Cover edge cases and error conditions
- Keep tests fast and reliable

### Integration Tests
- Test component interactions
- Validate end-to-end workflows
- Use realistic test data

### Example Programs
- Demonstrate specific features
- Include clear comments
- Test with both CLI and GUI tools

## Documentation

### Code Documentation
- Document public APIs with clear comments
- Explain complex algorithms
- Include usage examples
- Keep documentation up to date

### User Documentation
- Update README for user-facing changes
- Add examples for new features
- Document breaking changes
- Maintain architecture documentation

## Release Process

### Version Numbering
- Follow semantic versioning (MAJOR.MINOR.PATCH)
- MAJOR: Breaking changes
- MINOR: New features (backward compatible)
- PATCH: Bug fixes

### Release Checklist
- [ ] All tests pass
- [ ] Documentation updated
- [ ] Version numbers updated
- [ ] Changelog updated
- [ ] Release notes prepared

## Getting Help

- Check existing documentation
- Search existing issues
- Ask questions in discussions
- Contact maintainers for guidance

## Code of Conduct

- Be respectful and inclusive
- Focus on constructive feedback
- Help others learn and grow
- Maintain a welcoming environment
