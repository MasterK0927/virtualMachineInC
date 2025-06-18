#include "vm/Instance.hpp"
#include "vm/Logger.hpp"
#include "vm/Opcodes.hpp"
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
    // Program: LOADI R0, 7; OUT R0; HALT
    std::vector<unsigned char> prog;
    prog.push_back(static_cast<unsigned char>(Opcode::LOADI));
    prog.push_back(0x00);
    emit32(prog, 7);
    prog.push_back(static_cast<unsigned char>(Opcode::OUT));
    prog.push_back(0x00);
    prog.push_back(static_cast<unsigned char>(Opcode::HALT));

    ConsoleLogger logger;
    VMConfig cfg; cfg.memSize = 64 * 1024; cfg.name = "test";
    VMInstance instance(cfg, &logger);
    instance.powerOn();
    instance.loadProgramBytes(prog);
    instance.runUntilHalt();
    std::cout << "[TEST] Completed basic run" << std::endl;
    return 0;
}
