#pragma once

#include <string>
#include "vm/Types.hpp"
#include "vm/Opcodes.hpp"

namespace vm {

struct IMemory;

struct DecodedInst {
    Opcode op{Opcode::HALT};
    u8 a{0};
    u8 b{0};
    u32 imm{0};
    u8 size{1};
};

struct IDecoder {
    virtual ~IDecoder() = default;
    virtual DecodedInst decode(const IMemory& mem, u32 pc) const = 0;
};

class SimpleDecoder : public IDecoder {
public:
    DecodedInst decode(const IMemory& mem, u32 pc) const override;
};

// Disassembly utilities
std::string disassemble(const DecodedInst& inst);
std::string opcodeToString(Opcode op);

} // namespace vm
