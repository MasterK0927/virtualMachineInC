#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <memory>

#include "vm/Types.hpp"
#include "vm/Memory.hpp"
#include "vm/Device.hpp"

namespace vm {

struct DeviceMapping {
    std::size_t base{0};
    std::size_t size{0};
    std::shared_ptr<IDevice> device; // shared so multiple components can hold refs
};

// BusMemory composes a backing RAM and a set of memory-mapped devices.
class BusMemory : public IMemory {
public:
    explicit BusMemory(RamMemory& ram) : m_ram(ram) {}

    // IMemory
    std::size_t size() const override { return m_ram.size(); }
    u8  read8(std::size_t addr) const override;
    u16 read16(std::size_t addr) const override;
    u32 read32(std::size_t addr) const override;
    void write8(std::size_t addr, u8 v) override;
    void write16(std::size_t addr, u16 v) override;
    void write32(std::size_t addr, u32 v) override;

    // Bus API
    void mapDevice(std::size_t base, std::shared_ptr<IDevice> dev);
    const std::vector<DeviceMapping>& mappings() const { return m_maps; }

private:
    const DeviceMapping* find(std::size_t addr) const;
    DeviceMapping* find(std::size_t addr);

private:
    RamMemory& m_ram;
    std::vector<DeviceMapping> m_maps;
};

} // namespace vm
