#include "vm/CPU.hpp"
#include "vm/Memory.hpp"
#include "vm/Logger.hpp"
#include "vm/Decoder.hpp"
#include "vm/Opcodes.hpp"
#include <iostream>
#include <sstream>

namespace vm {

void SimpleCPU::log(const char* level, const char* msg) {
    if (!m_logger) return;
    std::ostringstream os;
    os << "PC=" << m_pc << " SP=" << m_sp << " | " << msg;
    if (std::string(level) == "info") m_logger->info(os.str());
    else if (std::string(level) == "warn") m_logger->warn(os.str());
    else m_logger->error(os.str());
}

SimpleCPU::SimpleCPU(IMemory& mem, ILogger* logger)
    : m_mem(mem), m_logger(logger) {
    reset();
}

void SimpleCPU::reset() {
    m_regs.fill(0);
    m_pc = 0;
    m_sp = static_cast<u32>(m_mem.size() - 4);
    m_flags = 0;
    m_halted = false;
}

void SimpleCPU::run(std::size_t maxSteps) {
    std::size_t steps = 0;
    while (!m_halted) {
        step();
        if (maxSteps && ++steps >= maxSteps) break;
    }
}

void SimpleCPU::step() {
    SimpleDecoder decoder;
    auto di = decoder.decode(m_mem, m_pc);

    auto setZ = [&](u32 val){
        if (val == 0) m_flags |= 0x1; else m_flags &= ~0x1u;
    };

    switch (di.op) {
        case Opcode::LOAD: {
            // LOAD rD, [rS + imm16]
            const u8 rD = di.a;
            const u8 rS = di.b;
            if (rD < REG_COUNT && rS < REG_COUNT) {
                u32 addr = m_regs[rS] + static_cast<u32>(di.imm & 0xFFFF);
                // Little-endian 32-bit load
                u32 val = m_mem.read32(addr);
                m_regs[rD] = val;
                setZ(val);
                m_pc += di.size;
                log("info", "LOAD");
            } else {
                log("error", "Invalid register in LOAD");
                m_halted = true;
            }
            break;
        }
        case Opcode::STORE: {
            // STORE [rD + imm16], rS   (rD is base, rS is source)
            const u8 rD = di.a;
            const u8 rS = di.b;
            if (rD < REG_COUNT && rS < REG_COUNT) {
                u32 addr = m_regs[rD] + static_cast<u32>(di.imm & 0xFFFF);
                u32 val = m_regs[rS];
                m_mem.write32(addr, val);
                m_pc += di.size;
                log("info", "STORE");
            } else {
                log("error", "Invalid register in STORE");
                m_halted = true;
            }
            break;
        }
        case Opcode::HALT: {
            m_halted = true;
            m_pc += di.size;
            log("info", "HALT");
            break;
        }
        case Opcode::LOADI: {
            if (di.a < REG_COUNT) {
                m_regs[di.a] = di.imm;
                setZ(m_regs[di.a]);
                m_pc += di.size;
                log("info", "LOADI");
            } else {
                log("error", "Invalid register in LOADI");
                m_halted = true;
            }
            break;
        }
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::XOR: {
            const u8 rD = di.a;
            const u8 rA = di.b;
            const u8 rB = di.c;
            if (rD < REG_COUNT && rA < REG_COUNT && rB < REG_COUNT) {
                u32 a = m_regs[rA];
                u32 b = m_regs[rB];
                u32 res = 0;
                if (di.op == Opcode::ADD) res = a + b;
                else if (di.op == Opcode::SUB) res = a - b;
                else if (di.op == Opcode::AND) res = a & b;
                else if (di.op == Opcode::OR)  res = a | b;
                else if (di.op == Opcode::XOR) res = a ^ b;
                m_regs[rD] = res;
                setZ(res);
                m_pc += di.size;
                log("info", "ALU");
            } else {
                log("error", "Invalid register in ALU op");
                m_halted = true;
            }
            break;
        }
        case Opcode::CMP: {
            const u8 rA = di.a;
            const u8 rB = di.b;
            if (rA < REG_COUNT && rB < REG_COUNT) {
                setZ(m_regs[rA] == m_regs[rB] ? 0 : 1); // Z=1 if equal
                m_pc += di.size;
                log("info", "CMP");
            } else {
                log("error", "Invalid register in CMP");
                m_halted = true;
            }
            break;
        }
        case Opcode::JMP: {
            m_pc = di.imm;
            log("info", "JMP");
            break;
        }
        case Opcode::JZ: {
            if (m_flags & 0x1) {
                m_pc = di.imm;
            } else {
                m_pc += di.size;
            }
            log("info", "JZ");
            break;
        }
        case Opcode::JNZ: {
            if (!(m_flags & 0x1)) {
                m_pc = di.imm;
            } else {
                m_pc += di.size;
            }
            log("info", "JNZ");
            break;
        }
        case Opcode::PUSH: {
            const u8 rS = di.a;
            if (rS < REG_COUNT && m_sp >= 4) {
                m_sp -= 4;
                m_mem.write32(m_sp, m_regs[rS]);
                m_pc += di.size;
                log("info", "PUSH");
            } else {
                log("error", "Stack overflow in PUSH");
                m_halted = true;
            }
            break;
        }
        case Opcode::POP: {
            const u8 rD = di.a;
            if (rD < REG_COUNT && m_sp + 4 <= m_mem.size()) {
                m_regs[rD] = m_mem.read32(m_sp);
                m_sp += 4;
                setZ(m_regs[rD]);
                m_pc += di.size;
                log("info", "POP");
            } else {
                log("error", "Stack underflow in POP");
                m_halted = true;
            }
            break;
        }
        case Opcode::CALL: {
            // push return address, jump to imm
            if (m_sp >= 4) {
                u32 ret = m_pc + di.size;
                m_sp -= 4;
                m_mem.write32(m_sp, ret);
                m_pc = di.imm;
                log("info", "CALL");
            } else {
                log("error", "Stack overflow in CALL");
                m_halted = true;
            }
            break;
        }
        case Opcode::RET: {
            if (m_sp + 4 <= m_mem.size()) {
                u32 ret = m_mem.read32(m_sp);
                m_sp += 4;
                m_pc = ret;
                log("info", "RET");
            } else {
                log("error", "Stack underflow in RET");
                m_halted = true;
            }
            break;
        }
        case Opcode::OUT: {
            if (di.a < REG_COUNT) {
                // OUT to host stdout
                std::cout << m_regs[di.a] << std::endl;
                m_pc += di.size;
                log("info", "OUT");
            } else {
                log("error", "Invalid register in OUT");
                m_halted = true;
            }
            break;
        }
        case Opcode::IN: {
            if (di.a < REG_COUNT) {
                std::int64_t input = 0;
                if (!(std::cin >> input)) {
                    log("error", "IN failed to read from stdin");
                    m_halted = true;
                    break;
                }
                m_regs[di.a] = static_cast<u32>(input);
                setZ(m_regs[di.a]);
                m_pc += di.size;
                log("info", "IN");
            } else {
                log("error", "Invalid register in IN");
                m_halted = true;
            }
            break;
        }
        default: {
            log("error", "Unimplemented opcode encountered");
            m_halted = true;
            break;
        }
    }
}

} // namespace vm
