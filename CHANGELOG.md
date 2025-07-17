# Changelog

All notable changes to SimpleVM will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2025-07-16

### Added
- Complete virtual machine implementation with 19-instruction ISA
- 8 general-purpose registers (R0-R7) plus PC/SP/FLAGS
- Memory-mapped I/O system with device bus architecture
- Console device for output at memory address 0xFF00
- Full assembler with label support and program headers
- Command-line VM runner with debugging capabilities
- Professional GUI debugger with real-time inspection
- Comprehensive test suite with unit and integration tests
- Cross-platform build system with CMake
- Complete documentation (Architecture, API, ISA)

### Core VM Features
- **CPU**: Fetch-decode-execute cycle with proper instruction handling
- **Memory**: 64KB RAM with bounds checking and device mapping
- **Decoder**: Robust instruction parsing with disassembly support
- **Instance**: High-level VM management with lifecycle control

### Instruction Set
- **Data Movement**: LOADI, LOAD, STORE, PUSH, POP
- **Arithmetic**: ADD, SUB, AND, OR, XOR
- **Comparison**: CMP with zero flag
- **Control Flow**: JMP, JZ, JNZ, CALL, RET
- **I/O**: OUT, IN
- **System**: HALT

### Tools
- **asm_app**: Assembly language compiler with header support
- **vm_app**: CLI runner with interactive debugging, breakpoints, memory inspection
- **vm_gui**: Professional debugger with panels for CPU, memory, console, breakpoints

### Development Features
- Modular architecture with clean interfaces
- Comprehensive error handling and bounds checking
- RAII resource management throughout
- Optional GUI build with SDL2 + Dear ImGui
- Install targets and packaging support
- CI/CD pipeline with automated testing

### Documentation
- Complete README with quick start guide
- Architecture documentation explaining system design
- API reference for all public interfaces
- ISA manual with instruction format details
- Contributing guidelines for developers

## [Unreleased]

### Planned
- Enhanced memory panel with scrollable hex view
- Disassembly panel with instruction highlighting
- Settings persistence (window layout, preferences)
- Additional devices (timer, keyboard, display)
- Performance optimizations and JIT compilation
- Network I/O capabilities
- Floating-point instruction extensions
