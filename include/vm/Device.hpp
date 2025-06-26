#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "vm/Types.hpp"

namespace vm {

// Simple memory-mapped device interface. Implementations should be side-effect free on reads
// unless the device semantics require otherwise.
struct IDevice {
    virtual ~IDevice() = default;

    virtual const char* name() const = 0;
    virtual std::size_t size() const = 0; // size in bytes of the mapped region

    virtual u8  read8(std::size_t offset) = 0;
    virtual u16 read16(std::size_t offset) = 0;
    virtual u32 read32(std::size_t offset) = 0;

    virtual void write8(std::size_t offset, u8 v) = 0;
    virtual void write16(std::size_t offset, u16 v) = 0;
    virtual void write32(std::size_t offset, u32 v) = 0;
};

} // namespace vm
