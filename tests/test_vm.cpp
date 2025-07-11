#include "vm/Instance.hpp"
#include "vm/Logger.hpp"
#include "vm/Opcodes.hpp"
#include "vm/Decoder.hpp"
#include "vm/Memory.hpp"
#include <vector>
#include <iostream>

using namespace vm;

static void emit32(std::vector<unsigned char>& out, unsigned v) {
    out.push_back(static_cast<unsigned char>(v & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 8) & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 16) & 0xFF));
    out.push_back(static_cast<unsigned char>((v >> 24) & 0xFF));
}

int main() {
    std::cout << "[TEST] Starting VM tests..." << std::endl;
    
    // Test 1: Basic program execution
    {
        std::cout << "[TEST] Test 1: Basic LOADI/OUT/HALT" << std::endl;
        std::vector<unsigned char> prog;
        prog.push_back(static_cast<unsigned char>(Opcode::LOADI));
        prog.push_back(0x00);
        emit32(prog, 7);
        prog.push_back(static_cast<unsigned char>(Opcode::OUT));
        prog.push_back(0x00);
        prog.push_back(static_cast<unsigned char>(Opcode::HALT));

        ConsoleLogger logger;
        VMConfig cfg; cfg.memSize = 64 * 1024; cfg.name = "test1";
        VMInstance instance(cfg, &logger);
        instance.powerOn();
        instance.loadProgramBytes(prog);
        instance.runUntilHalt();
        
        // Verify final state
        const ICPU* cpu = instance.cpu();
        if (cpu->getReg(0) == 7) {
            std::cout << "[TEST] ✓ Test 1 passed" << std::endl;
        } else {
            std::cout << "[TEST] ✗ Test 1 failed: R0=" << cpu->getReg(0) << std::endl;
        }
    }
    
    // Test 2: ALU operations
    {
        std::cout << "[TEST] Test 2: ALU operations" << std::endl;
        std::vector<unsigned char> prog;
        // LOADI R0, 10
        prog.push_back(static_cast<unsigned char>(Opcode::LOADI));
        prog.push_back(0x00);
        emit32(prog, 10);
        // LOADI R1, 5  
        prog.push_back(static_cast<unsigned char>(Opcode::LOADI));
        prog.push_back(0x01);
        emit32(prog, 5);
        // ADD R2, R0, R1
        prog.push_back(static_cast<unsigned char>(Opcode::ADD));
        prog.push_back(0x02); // rD
        prog.push_back(0x00); // rA
        prog.push_back(0x01); // rB
        // HALT
        prog.push_back(static_cast<unsigned char>(Opcode::HALT));

        ConsoleLogger logger;
        VMConfig cfg; cfg.memSize = 64 * 1024; cfg.name = "test2";
        VMInstance instance(cfg, &logger);
        instance.powerOn();
        instance.loadProgramBytes(prog);
        instance.runUntilHalt();
        
        const ICPU* cpu = instance.cpu();
        if (cpu->getReg(2) == 15) {
            std::cout << "[TEST] ✓ Test 2 passed" << std::endl;
        } else {
            std::cout << "[TEST] ✗ Test 2 failed: R2=" << cpu->getReg(2) << std::endl;
        }
    }
    
    // Test 3: Disassembly
    {
        std::cout << "[TEST] Test 3: Disassembly" << std::endl;
        SimpleDecoder decoder;
        RamMemory mem(16);
        
        // LOADI R0, 42
        mem.write8(0, static_cast<u8>(Opcode::LOADI));
        mem.write8(1, 0);
        mem.write32(2, 42);
        
        auto decoded = decoder.decode(mem, 0);
        std::string disasm = disassemble(decoded);
        
        if (disasm == "LOADI R0, 42") {
            std::cout << "[TEST] ✓ Test 3 passed: " << disasm << std::endl;
        } else {
            std::cout << "[TEST] ✗ Test 3 failed: " << disasm << std::endl;
        }
    }
    
    std::cout << "[TEST] All tests completed!" << std::endl;
    return 0;
}
