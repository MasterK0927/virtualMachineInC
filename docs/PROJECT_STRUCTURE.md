# Project Structure

This document describes the organization of the SimpleVM codebase following modern C++ best practices.

## Directory Layout

```
virtualMachineInC/
├── .github/                    # GitHub workflows and templates
│   └── workflows/
│       └── ci.yml             # Continuous integration
├── .gitignore                 # Git ignore patterns
├── CMakeLists.txt             # Main build configuration
├── LICENSE                    # MIT license
├── README.md                  # Project overview and quick start
├── CHANGELOG.md               # Version history
├── CONTRIBUTING.md            # Development guidelines
│
├── docs/                      # Documentation
│   ├── ARCHITECTURE.md        # System design overview
│   ├── API.md                 # Programming interface reference
│   ├── ISA.md                 # Instruction set manual
│   └── PROJECT_STRUCTURE.md   # This file
│
├── include/vm/                # Public API headers
│   ├── Bus.hpp                # Memory-mapped device bus
│   ├── CPU.hpp                # CPU interface and implementation
│   ├── Config.hpp             # Configuration structures
│   ├── ConsoleCapture.hpp     # Stdout capture for GUI
│   ├── ConsoleDevice.hpp      # Console device implementation
│   ├── Decoder.hpp            # Instruction decoder
│   ├── Device.hpp             # Device interface
│   ├── Instance.hpp           # VM instance management
│   ├── Logger.hpp             # Logging interfaces
│   ├── Memory.hpp             # Memory abstractions
│   ├── Opcodes.hpp            # Instruction opcodes
│   ├── ProgramLoader.hpp      # Program loading utilities
│   ├── Types.hpp              # Common type definitions
│   └── VM.hpp                 # Main VM header
│
├── src/                       # Core implementation
│   ├── Bus.cpp                # Device bus implementation
│   ├── CPU.cpp                # CPU execution engine
│   ├── ConsoleDevice.cpp      # Console device
│   ├── Decoder.cpp            # Instruction decoding
│   └── Instance.cpp           # VM lifecycle management
│
├── apps/                      # Applications
│   ├── asm/                   # Assembler
│   │   └── main.cpp           # Assembly compiler
│   ├── gui/                   # GUI debugger (optional)
│   │   ├── GuiApp.hpp         # GUI application header
│   │   ├── GuiApp.cpp         # GUI implementation
│   │   └── main.cpp           # GUI main entry point
│   └── vm/                    # VM runner
│       └── main.cpp           # CLI VM runner
│
├── tests/                     # Testing
│   └── test_vm.cpp            # Unit and integration tests
│
└── examples/                  # Example programs
    ├── branch_and_loop.asm    # Control flow demonstration
    ├── call_and_ret.asm       # Function calls
    ├── comprehensive_test.asm # Full feature test
    ├── mmio_print.asm         # Memory-mapped I/O
    └── print_number.asm       # Basic I/O
```

## Design Principles

### Modularity
- **Clear separation**: Core VM, applications, and documentation
- **Interface-based**: Clean abstractions between components
- **Optional components**: GUI can be disabled without affecting core

### Maintainability
- **Consistent naming**: Following C++ conventions
- **Documentation**: Each major component documented
- **Testing**: Unit tests and integration examples

### Extensibility
- **Plugin architecture**: Easy to add new devices
- **Panel system**: GUI panels are pluggable
- **Instruction set**: New opcodes can be added systematically

## Build Targets

### Core Targets
- **vmcore**: Static library with VM implementation
- **vm_app**: Command-line VM runner
- **asm_app**: Assembly compiler
- **vm_tests**: Test suite

### Optional Targets
- **vm_gui**: GUI debugger (requires SDL2)

### Utility Targets
- **install**: System-wide installation
- **package**: Distribution archive creation

## Dependencies

### Required
- C++17 compatible compiler
- CMake 3.15+

### Optional
- SDL2 (for GUI)
- Dear ImGui (auto-fetched)

## Configuration Options

- `BUILD_GUI`: Enable GUI debugger (default: OFF)
- `BUILD_TESTING`: Enable test suite (default: ON)
- `CMAKE_BUILD_TYPE`: Debug/Release/RelWithDebInfo

## Best Practices Followed

### Code Organization
- Headers in `include/` for public API
- Implementation in `src/` for core logic
- Applications separated in `apps/`
- Documentation centralized in `docs/`

### Interface Design
- Pure virtual interfaces for major components
- RAII for resource management
- Exception-based error handling
- Const-correctness throughout

### Build System
- Modern CMake practices
- Optional component builds
- Cross-platform compatibility
- Proper install targets

### Documentation
- README for quick start
- Detailed architecture documentation
- API reference for developers
- Contributing guidelines

This structure supports both educational use and professional development while maintaining clean separation of concerns and extensibility.
