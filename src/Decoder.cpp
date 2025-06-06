#include "vm/Decoder.hpp"
#include "vm/Memory.hpp"

namespace vm {

DecodedInst SimpleDecoder::decode(const IMemory& mem, u32 pc) const {
    DecodedInst di{};
    di.op = static_cast<Opcode>(mem.read8(pc));

    switch (di.op) {
        case Opcode::HALT: {
            di.size = 1;
            break;
        }
        case Opcode::LOADI: {
            // [OP][reg][imm32]
            di.a = mem.read8(pc + 1);
            di.imm = mem.read32(pc + 2);
            di.size = 1 + 1 + 4;
            break;
        }
        case Opcode::LOAD:
        case Opcode::STORE: {
            // [OP][rD][rS][imm16]  (for STORE, rD is base, rS is source)
            di.a = mem.read8(pc + 1);
            di.b = mem.read8(pc + 2);
            di.imm = mem.read16(pc + 3);
            di.size = 1 + 1 + 1 + 2;
            break;
        }
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR: {
            // [OP][rD][rA][rB]
            di.a = mem.read8(pc + 1);
            di.b = mem.read8(pc + 2); // rA in b for reuse
            di.imm = mem.read8(pc + 3); // use imm low byte to carry rB index
            di.size = 1 + 1 + 1 + 1;
            break;
        }
        case Opcode::CMP: {
            // [OP][rA][rB]
            di.a = mem.read8(pc + 1);
            di.b = mem.read8(pc + 2);
            di.size = 1 + 1 + 1;
            break;
        }
        case Opcode::JMP:
        case Opcode::JZ:
        case Opcode::JNZ:
        case Opcode::CALL: {
            // [OP][addr32]
            di.imm = mem.read32(pc + 1);
            di.size = 1 + 4;
            break;
        }
        case Opcode::RET: {
            di.size = 1;
            break;
        }
        case Opcode::PUSH:
        case Opcode::POP:
        case Opcode::OUT:
        case Opcode::IN: {
            // [OP][reg]
            di.a = mem.read8(pc + 1);
            di.size = 1 + 1;
            break;
        }
        default: {
            // For unimplemented instructions, treat as 1 byte to avoid infinite loops
            di.size = 1;
            break;
        }
    }

    return di;
}

} // namespace vm
