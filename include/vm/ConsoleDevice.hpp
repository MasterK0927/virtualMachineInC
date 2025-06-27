#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>

#include "vm/Device.hpp"
#include "vm/Logger.hpp"

namespace vm {

// A simple memory-mapped console output device.
// Writing to offset 0 prints the value to stdout.
// Size: 4 bytes (writes of 8/16/32 bits accepted at offset 0)
class ConsoleOutDevice : public IDevice {
public:
    explicit ConsoleOutDevice(ILogger* logger = nullptr) : m_logger(logger) {}

    const char* name() const override { return "ConsoleOut"; }
    std::size_t size() const override { return 4; }

    u8  read8(std::size_t) override { return 0; }
    u16 read16(std::size_t) override { return 0; }
    u32 read32(std::size_t) override { return 0; }

    void write8(std::size_t offset, u8 v) override {
        if (offset == 0) {
            std::cout << static_cast<unsigned>(v) << std::endl;
            if (m_logger) m_logger->info("ConsoleOutDevice: write8");
        }
    }
    void write16(std::size_t offset, u16 v) override {
        if (offset == 0) {
            std::cout << v << std::endl;
            if (m_logger) m_logger->info("ConsoleOutDevice: write16");
        }
    }
    void write32(std::size_t offset, u32 v) override {
        if (offset == 0) {
            std::cout << v << std::endl;
            if (m_logger) m_logger->info("ConsoleOutDevice: write32");
        }
    }

private:
    ILogger* m_logger{nullptr};
};

} // namespace vm
