#pragma once

#include <cstdint>

namespace simplevm {

enum class OpCode : std::uint8_t {
    NOP   = 0x00,
    PUSHI = 0x01, // push 32-bit signed immediate (little-endian)
    ADD   = 0x02, // pop a, pop b, push (a + b)
    PRINT = 0x03, // pop a, print a
    HALT  = 0xFF  // stop execution
};

} // namespace simplevm
