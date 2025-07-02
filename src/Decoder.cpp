#include "vm/Decoder.hpp"
#include "vm/Memory.hpp"

#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace vm {

DecodedInst SimpleDecoder::decode(const IMemory& mem, u32 pc) const {
    DecodedInst inst;
    if (pc >= mem.size()) {
        throw std::runtime_error("PC out of bounds");
    }
    
    inst.op = static_cast<Opcode>(mem.read8(pc));
    inst.size = 1;
    
    switch (inst.op) {
        case Opcode::HALT:
            break;
        case Opcode::LOADI:
            if (pc + 5 >= mem.size()) throw std::runtime_error("LOADI: insufficient bytes");
            inst.a = mem.read8(pc + 1);
            inst.imm = mem.read32(pc + 2);
            inst.size = 6;
            break;
        case Opcode::LOAD:
        case Opcode::STORE:
            if (pc + 3 >= mem.size()) throw std::runtime_error("LOAD/STORE: insufficient bytes");
            inst.a = mem.read8(pc + 1);
            inst.imm = mem.read16(pc + 2);
            inst.size = 4;
            break;
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::CMP:
            if (pc + 2 >= mem.size()) throw std::runtime_error("ALU: insufficient bytes");
            inst.a = mem.read8(pc + 1);
            inst.b = mem.read8(pc + 2);
            inst.size = 3;
            break;
        case Opcode::PUSH:
        case Opcode::POP:
        case Opcode::OUT:
        case Opcode::IN:
            if (pc >= mem.size()) throw std::runtime_error("Single-reg: insufficient bytes");
            inst.a = mem.read8(pc + 1);
            inst.size = 2;
            break;
        case Opcode::JMP:
        case Opcode::JZ:
        case Opcode::JNZ:
        case Opcode::CALL:
            if (pc + 4 >= mem.size()) throw std::runtime_error("Jump/Call: insufficient bytes");
            inst.imm = mem.read32(pc + 1);
            inst.size = 5;
            break;
        case Opcode::RET:
            break;
        default:
            throw std::runtime_error("Unknown opcode");
    }
    
    return inst;
}

std::string opcodeToString(Opcode op) {
    switch (op) {
        case Opcode::HALT: return "HALT";
        case Opcode::LOADI: return "LOADI";
        case Opcode::LOAD: return "LOAD";
        case Opcode::STORE: return "STORE";
        case Opcode::ADD: return "ADD";
        case Opcode::SUB: return "SUB";
        case Opcode::AND: return "AND";
        case Opcode::OR: return "OR";
        case Opcode::XOR: return "XOR";
        case Opcode::CMP: return "CMP";
        case Opcode::PUSH: return "PUSH";
        case Opcode::POP: return "POP";
        case Opcode::JMP: return "JMP";
        case Opcode::JZ: return "JZ";
        case Opcode::JNZ: return "JNZ";
        case Opcode::CALL: return "CALL";
        case Opcode::RET: return "RET";
        case Opcode::OUT: return "OUT";
        case Opcode::IN: return "IN";
        default: return "UNKNOWN";
    }
}

std::string disassemble(const DecodedInst& inst) {
    std::ostringstream os;
    os << opcodeToString(inst.op);
    
    switch (inst.op) {
        case Opcode::HALT:
        case Opcode::RET:
            break;
        case Opcode::LOADI:
            os << " R" << static_cast<int>(inst.a) << ", " << inst.imm;
            break;
        case Opcode::LOAD:
            os << " R" << static_cast<int>(inst.a) << ", [R" << static_cast<int>(inst.b) << " + " << inst.imm << "]";
            break;
        case Opcode::STORE:
            os << " [R" << static_cast<int>(inst.a) << " + " << inst.imm << "], R" << static_cast<int>(inst.b);
            break;
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR:
        case Opcode::CMP:
            os << " R" << static_cast<int>(inst.a) << ", R" << static_cast<int>(inst.b);
            break;
        case Opcode::PUSH:
        case Opcode::POP:
        case Opcode::OUT:
        case Opcode::IN:
            os << " R" << static_cast<int>(inst.a);
            break;
        case Opcode::JMP:
        case Opcode::JZ:
        case Opcode::JNZ:
        case Opcode::CALL:
            os << " 0x" << std::hex << inst.imm;
            break;
        default:
            os << " ???";
            break;
    }
    
    return os.str();
}

} // namespace vm
