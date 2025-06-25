Virtual Machine in C++

This project implements a modular, stack-friendly register VM in modern C++, with an assembler and a command-line VM runner. It follows SOLID principles with clear interfaces for CPU, Memory, and Loader, plus a simple instance orchestration layer.

Project layout
- `include/vm/` – public interfaces and headers (`CPU.hpp`, `Memory.hpp`, `Decoder.hpp`, `Opcodes.hpp`, `ProgramLoader.hpp`, `Instance.hpp`, `Config.hpp`, ...)
- `src/` – core implementations (`CPU.cpp`, `Decoder.cpp`, `Instance.cpp`)
- `apps/asm/` – assembler CLI (`asm_app`)
- `apps/vm/` – VM runner CLI (`vm_app`)
- `examples/` – sample assembly programs
- `tests/` – small smoke tests (no framework)

Build
```bash
cmake -S . -B build
cmake --build build -j
```

Run the VM (demo or binary)
```bash
# Demo program bundled in vm_app
./build/vm_app --demo --dump

# Assemble an example and run it (raw bytecode)
./build/asm_app examples/print_number.asm -o build/print_number.bin
./build/vm_app build/print_number.bin --dump

# With header + entry point (recommended for labeled entry)
./build/asm_app examples/call_and_ret.asm -o build/call_and_ret.vmb --with-header --entry start
./build/vm_app build/call_and_ret.vmb --dump
```

Assembler usage
```bash
asm_app <input.asm> [-o output.bin] [--with-header] [--entry <label|addr>]
```
- `--with-header` emits a program header (see below) before bytecode.
- `--entry` sets the initial entry PC (label or numeric). Requires `--with-header` to take effect.

Program header (optional)
- Magic: `VMB1`
- Fields: `version` (u32, currently 1), `entry` (u32, initial PC)
- The VM loader validates the magic and version. If present, `entry` is used to set `PC`; otherwise `PC=0`.

Instruction set (summary)
- Data movement
  - `LOADI rD, imm32`
  - `LOAD rD, [rS + off16]`
  - `STORE [rD + off16], rS`
  - `PUSH rS`, `POP rD`
- ALU
  - `ADD rD, rA, rB`, `SUB rD, rA, rB`, `AND rD, rA, rB`, `OR rD, rA, rB`, `XOR rD, rA, rB`
  - `CMP rA, rB` (sets Z flag: 1 if equal)
- Control flow
  - `JMP addr32`, `JZ addr32`, `JNZ addr32`, `CALL addr32`, `RET`
- I/O
  - `OUT rS` (host stdout), `IN rD` (reads integer from stdin)
- System
  - `HALT`

CLI runner features (`vm_app`)
- Load from raw bytecode or headered program.
- Run until HALT or step N instructions (`--steps`).
- Interactive REPL (`--interactive`): breakpoints, memory read/write, register set, disassembly, snapshots.
- Dump CPU state (`--dump`).

Examples
- `examples/print_number.asm` – LOADI/OUT/HALT.
- `examples/call_and_ret.asm` – CALL/RET with labeled entry (use `--with-header --entry start`).
- `examples/branch_and_loop.asm` – CMP/JZ/JNZ with a simple loop.

Notes
- Endianness: little-endian encoding for immediates and memory I/O.
- Snapshot files are a simple binary format for development (subject to change).
- The code aims to be minimal yet extensible; contributions are welcome.
Learning to impl complex systems like VirtualMachine in C

This project involves building a virtual machine from scratch by completing the following tasks:  

1. **Defining a Custom Instruction Set Architecture**  
   - [ ] Design opcodes for arithmetic and logical operations.  
   - [ ] Add instructions for control flow (jumps, branches).  
   - [ ] Implement stack-related instructions (e.g., PUSH, POP).  
   - [ ] Enable basic input/output operations.  

2. **Developing Core VM Components**  
   - [ ] Create a CPU core with registers and a program counter.  
   - [ ] Implement the fetch-decode-execute cycle.  
   - [ ] Develop memory management for reading, writing, and allocation.  

3. **Creating a Program Execution Framework**  
   - [ ] Build functionality to load and execute binary programs.  
   - [ ] Add support for recursion and function calls.  
   - [ ] Integrate basic I/O functionality for interacting with the VM.  

4. **Adding Debugging and Testing Capabilities**  
   - [ ] Implement step-by-step execution for debugging.  
   - [ ] Add memory and register inspection tools.  
   - [ ] Provide logging for program behavior analysis.  

5. **Laying the Foundation for Future Features**  
   - [ ] Design a modular and extensible architecture.  
   - [ ] Threading, interrupts, or hardware emulation.  

