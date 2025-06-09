#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <stdexcept>

#include "vm/Types.hpp"

namespace vm {

struct IMemory {
    virtual ~IMemory() = default;
    virtual std::size_t size() const = 0;
    virtual u8  read8(std::size_t addr) const = 0;
    virtual u16 read16(std::size_t addr) const = 0;
    virtual u32 read32(std::size_t addr) const = 0;
    virtual void write8(std::size_t addr, u8 v) = 0;
    virtual void write16(std::size_t addr, u16 v) = 0;
    virtual void write32(std::size_t addr, u32 v) = 0;
};

class RamMemory : public IMemory {
public:
    explicit RamMemory(std::size_t size)
        : m_data(size, 0) {}

    std::size_t size() const override { return m_data.size(); }

    u8 read8(std::size_t addr) const override {
        bounds(addr, 1);
        return m_data[addr];
    }

    u16 read16(std::size_t addr) const override {
        bounds(addr, 2);
        return static_cast<u16>(m_data[addr]) |
               static_cast<u16>(m_data[addr + 1] << 8);
    }

    u32 read32(std::size_t addr) const override {
        bounds(addr, 4);
        return static_cast<u32>(m_data[addr]) |
               (static_cast<u32>(m_data[addr + 1]) << 8) |
               (static_cast<u32>(m_data[addr + 2]) << 16) |
               (static_cast<u32>(m_data[addr + 3]) << 24);
    }

    void write8(std::size_t addr, u8 v) override {
        bounds(addr, 1);
        m_data[addr] = v;
    }

    void write16(std::size_t addr, u16 v) override {
        bounds(addr, 2);
        m_data[addr] = static_cast<u8>(v & 0xFF);
        m_data[addr + 1] = static_cast<u8>((v >> 8) & 0xFF);
    }

    void write32(std::size_t addr, u32 v) override {
        bounds(addr, 4);
        m_data[addr] = static_cast<u8>(v & 0xFF);
        m_data[addr + 1] = static_cast<u8>((v >> 8) & 0xFF);
        m_data[addr + 2] = static_cast<u8>((v >> 16) & 0xFF);
        m_data[addr + 3] = static_cast<u8>((v >> 24) & 0xFF);
    }

    const std::vector<u8>& raw() const { return m_data; }
    std::vector<u8>& raw() { return m_data; }

private:
    void bounds(std::size_t addr, std::size_t count) const {
        if (addr + count > m_data.size()) {
            throw std::out_of_range("memory access out of range");
        }
    }

    std::vector<u8> m_data;
};

} // namespace vm
