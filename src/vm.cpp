#include "simplevm/vm.hpp"

#include <cassert>
#include <cstring>
#include <iostream>

namespace simplevm {

VM::VM(const VMConfig& cfg)
    : m_cfg(cfg) {
    m_stack.reserve(m_cfg.stackCapacity);
}

void VM::loadProgram(const Program& programBytes) {
    m_program = programBytes;
    reset();
}

void VM::reset() {
    m_stack.clear();
    m_pc = 0;
    m_halted = false;
}

void VM::run() {
    while (!m_halted) {
        if (m_pc >= m_program.size()) {
            throw VMError("PC out of bounds: no HALT encountered");
        }
        auto opcode = static_cast<OpCode>(m_program[m_pc++]);
        switch (opcode) {
            case OpCode::NOP: {
                break;
            }
            case OpCode::PUSHI: {
                Word imm = fetchWordLE();
                push(imm);
                break;
            }
            case OpCode::ADD: {
                if (m_stack.size() < 2) throw VMError("ADD requires two operands");
                Word a = pop();
                Word b = pop();
                push(b + a);
                break;
            }
            case OpCode::PRINT: {
                if (m_stack.empty()) throw VMError("PRINT requires one operand");
                Word v = pop();
                std::cout << v << std::endl;
                break;
            }
            case OpCode::HALT: {
                m_halted = true;
                break;
            }
            default:
                throw VMError("Unknown opcode encountered");
        }
    }
}

void VM::push(Word v) {
    if (m_stack.size() >= m_cfg.stackCapacity) {
        throw VMError("Stack overflow");
    }
    m_stack.push_back(v);
}

Word VM::pop() {
    if (m_stack.empty()) {
        throw VMError("Stack underflow");
    }
    Word v = m_stack.back();
    m_stack.pop_back();
    return v;
}

Word VM::fetchWordLE() {
    if (m_pc + 4 > m_program.size()) {
        throw VMError("Unexpected EOF while reading immediate");
    }
    // Little-endian decode
    Word v = static_cast<Word>(m_program[m_pc]) |
             (static_cast<Word>(m_program[m_pc + 1]) << 8) |
             (static_cast<Word>(m_program[m_pc + 2]) << 16) |
             (static_cast<Word>(m_program[m_pc + 3]) << 24);
    m_pc += 4;
    return v;
}

} // namespace simplevm
