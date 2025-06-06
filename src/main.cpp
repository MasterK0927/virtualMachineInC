#include "simplevm/vm.hpp"
#include "simplevm/opcodes.hpp"
#include "simplevm/types.hpp"

#include <cstdint>
#include <iostream>

using namespace simplevm;

static void emitPushI(Program& p, Word v) {
    p.push_back(static_cast<Byte>(OpCode::PUSHI));
    // little-endian encoding
    p.push_back(static_cast<Byte>(v & 0xFF));
    p.push_back(static_cast<Byte>((v >> 8) & 0xFF));
    p.push_back(static_cast<Byte>((v >> 16) & 0xFF));
    p.push_back(static_cast<Byte>((v >> 24) & 0xFF));
}

int main() {
    Program prog;

    emitPushI(prog, 2);
    emitPushI(prog, 3);
    prog.push_back(static_cast<Byte>(OpCode::ADD));
    prog.push_back(static_cast<Byte>(OpCode::PRINT));
    prog.push_back(static_cast<Byte>(OpCode::HALT));

    VM vm;
    vm.loadProgram(prog);
    try {
        vm.run(); // Expected output: 5
    } catch (const VMError& e) {
        std::cerr << "VMError: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
