# SimpleVM

A complete virtual machine implementation in C++ with assembler, runtime, and GUI debugger.

## Features

- **Complete VM**: 19-instruction RISC-like ISA with 8 registers
- **Memory Management**: 64KB RAM with memory-mapped device support  
- **Assembler**: Full assembly language with labels and program headers
- **CLI Tools**: VM runner with debugging, disassembly, and interactive mode
- **GUI Debugger**: Professional debugging interface with real-time inspection
- **Device System**: Extensible memory-mapped I/O with console device

## Quick Start

### Build
```bash
# Core VM and CLI tools
cmake -S . -B build && cmake --build build -j

# With GUI debugger (requires SDL2)
cmake -S . -B build -DBUILD_GUI=ON && cmake --build build -j
```

### Usage
```bash
# Assemble and run a program
./build/asm_app examples/print_number.asm -o program.bin
./build/vm_app program.bin --dump

# GUI debugger (if built)
./build/vm_gui program.bin
```

## Example Program

```assembly
; Print a number and halt
start:
    LOADI R0, 42        ; Load value
    OUT   R0            ; Print to console  
    HALT                ; Stop execution
```

## Documentation

- **[Architecture](docs/ARCHITECTURE.md)**: System design and components
- **[API Reference](docs/API.md)**: Programming interfaces
- **[ISA Manual](docs/ISA.md)**: Complete instruction set documentation

## Project Structure

```
├── include/vm/          # Public API headers
├── src/                 # Core implementation  
├── apps/                # Applications (vm, asm, gui)
├── tests/               # Unit and integration tests
├── examples/            # Example assembly programs
└── docs/                # Documentation
```

## Tools

- **`asm_app`**: Assembly language compiler
- **`vm_app`**: Command-line VM runner with debugging
- **`vm_gui`**: Professional GUI debugger (optional)

## Key Features

### Instruction Set
19 opcodes including data movement, ALU operations, control flow, stack operations, and I/O.

### Memory-Mapped I/O
Extensible device system with console device at 0xFF00.

### Program Headers
Versioned format with integrity checking and entry point specification.

### Professional Debugging
- Real-time CPU state inspection
- Memory hex viewer with editing
- Interactive breakpoint management
- Console output capture

## Installation

```bash
# Install system-wide
sudo cmake --install build

# Create distribution package
cmake --build build --target package
```

## Testing

```bash
./build/vm_tests                    # Unit tests
ctest --test-dir build             # All tests
```

## License

MIT License - see LICENSE file for details.
