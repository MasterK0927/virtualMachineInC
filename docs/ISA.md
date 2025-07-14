# SimpleVM Instruction Set Architecture

## Overview

SimpleVM implements a 32-bit RISC-like instruction set with 19 opcodes, 8 general-purpose registers, and memory-mapped I/O.

## Registers

- **R0-R7**: 8 general-purpose 32-bit registers
- **PC**: Program counter
- **SP**: Stack pointer
- **FLAGS**: Status flags (bit 0 = zero flag)

## Memory Layout

```
0x00000000  +------------------+
            |   Program Code    |
            +------------------+
            |   Data/Stack      |
            +------------------+
0xFFFFFF00  |   Device Region   |  (256 bytes reserved)
            |   Console: 0xFF00  |
0xFFFFFFFF  +------------------+
```

## Instruction Format

### Type 1: No operands (1 byte)
```
HALT, RET
```

### Type 2: Register + Immediate (6 bytes)
```
LOADI Rd, imm32
```

### Type 3: Register + Register + Offset (5 bytes)
```
LOAD  Rd, [Rs + offset16]
STORE [Rd + offset16], Rs
```

### Type 4: Three registers (4 bytes)
```
ADD Rd, Ra, Rb
SUB Rd, Ra, Rb
AND Rd, Ra, Rb
OR  Rd, Ra, Rb
XOR Rd, Ra, Rb
```

### Type 5: Two registers (3 bytes)
```
CMP Ra, Rb
```

### Type 6: Single register (2 bytes)
```
PUSH Rn
POP  Rn
OUT  Rn
IN   Rn
```

### Type 7: Immediate address (5 bytes)
```
JMP  addr32
JZ   addr32
JNZ  addr32
CALL addr32
```

## Instruction Reference

| Opcode | Format | Description |
|--------|--------|-------------|
| HALT   | -      | Stop execution |
| LOADI  | Rd, imm | Load immediate value into register |
| LOAD   | Rd, [Rs+off] | Load from memory |
| STORE  | [Rd+off], Rs | Store to memory |
| ADD    | Rd, Ra, Rb | Rd = Ra + Rb |
| SUB    | Rd, Ra, Rb | Rd = Ra - Rb |
| AND    | Rd, Ra, Rb | Rd = Ra & Rb |
| OR     | Rd, Ra, Rb | Rd = Ra \| Rb |
| XOR    | Rd, Ra, Rb | Rd = Ra ^ Rb |
| CMP    | Ra, Rb | Compare Ra and Rb, set flags |
| PUSH   | Rn     | Push register to stack |
| POP    | Rn     | Pop from stack to register |
| JMP    | addr   | Unconditional jump |
| JZ     | addr   | Jump if zero flag set |
| JNZ    | addr   | Jump if zero flag clear |
| CALL   | addr   | Call subroutine |
| RET    | -      | Return from subroutine |
| OUT    | Rn     | Output register value |
| IN     | Rn     | Input value to register |

## Assembly Syntax

### Basic Instructions
```assembly
; Comments start with semicolon
LOADI R0, 42        ; Load immediate
ADD   R2, R0, R1    ; Three-register ALU
OUT   R0            ; Single register
HALT                ; No operands
```

### Labels and Jumps
```assembly
start:
    LOADI R0, 10
    CALL  subroutine
    HALT

subroutine:
    OUT   R0
    RET
```

### Memory Operations
```assembly
LOADI R0, 100       ; Value
LOADI R1, 0x1000    ; Address
STORE [R1 + 0], R0  ; Store value at address
LOAD  R2, [R1 + 0]  ; Load back
```

### Memory-Mapped I/O
```assembly
; Console device at 0xFF00
LOADI R0, 42
LOADI R1, 0xFF00
STORE [R1 + 0], R0  ; Print 42 via MMIO
```

## Program Headers

### Version 1 (Basic)
```
Magic: "VM01" (4 bytes)
Entry: 0x00000000 (4 bytes)
```

### Version 2 (With Integrity)
```
Magic: "VM02" (4 bytes)
Entry: entry_point (4 bytes)
Size:  payload_size (4 bytes)
Checksum: adler32 (4 bytes)
```
