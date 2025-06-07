#pragma once

namespace vm {

enum class Opcode : unsigned char {
    HALT  = 0x00,
    LOADI = 0x10,
    LOAD  = 0x11,
    STORE = 0x12,
    ADD   = 0x20,
    SUB   = 0x21,
    AND   = 0x22,
    OR    = 0x23,
    XOR   = 0x24,
    CMP   = 0x25,
    PUSH  = 0x30,
    POP   = 0x31,
    JMP   = 0x40,
    JZ    = 0x41,
    JNZ   = 0x42,
    CALL  = 0x43,
    RET   = 0x44,
    OUT   = 0x50,
    IN    = 0x51
};

} // namespace vm
