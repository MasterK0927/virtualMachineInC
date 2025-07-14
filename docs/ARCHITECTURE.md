# SimpleVM Architecture

## Overview

SimpleVM is a modular virtual machine implementation with a complete toolchain including assembler, runtime, and GUI debugger.

## Project Structure

```
virtualMachineInC/
├── include/vm/           # Public API headers
│   ├── CPU.hpp          # CPU interface and implementation
│   ├── Memory.hpp       # Memory abstractions
│   ├── Decoder.hpp      # Instruction decoder
│   ├── Instance.hpp     # VM instance management
│   ├── Bus.hpp          # Memory-mapped device bus
│   ├── Device.hpp       # Device interface
│   └── ...
├── src/                 # Core implementation
│   ├── CPU.cpp          # CPU execution engine
│   ├── Decoder.cpp      # Instruction decoding
│   ├── Instance.cpp     # VM lifecycle management
│   ├── Bus.cpp          # Device bus implementation
│   └── ...
├── apps/                # Applications
│   ├── vm/              # VM runner CLI
│   ├── asm/             # Assembler
│   └── gui/             # GUI debugger
├── tests/               # Unit and integration tests
├── examples/            # Example assembly programs
└── docs/                # Documentation
```

## Core Components

### VM Core (`include/vm/`, `src/`)
- **CPU**: Instruction execution engine with 19 opcodes
- **Memory**: RAM and memory-mapped device abstraction
- **Decoder**: Instruction parsing and disassembly
- **Instance**: High-level VM management
- **Bus**: Device interconnection system

### Applications (`apps/`)
- **vm**: Command-line VM runner with debugging features
- **asm**: Assembly language compiler
- **gui**: Professional debugging interface (optional)

### Testing (`tests/`)
- Unit tests for core functionality
- Integration tests for end-to-end validation
- Example programs for feature demonstration

## Design Principles

### Modularity
- Clean interfaces between components
- Pluggable device system
- Optional GUI with graceful fallback

### Extensibility
- Easy to add new instructions
- Simple device interface for I/O expansion
- Panel-based GUI architecture

### Reliability
- Comprehensive bounds checking
- Graceful error handling
- Memory safety throughout

## Build System

- CMake-based with optional components
- Cross-platform compatibility
- Automatic dependency management
- Install targets for distribution
