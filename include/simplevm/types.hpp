#pragma once

#include <cstdint>
#include <vector>

namespace simplevm {

using Byte = std::uint8_t;
using Word = std::uint32_t; // 32-bit word for immediates and stack values
using Program = std::vector<Byte>;

} // namespace simplevm
