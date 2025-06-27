#include "vm/Bus.hpp"

#include <stdexcept>

namespace vm {

const DeviceMapping* BusMemory::find(std::size_t addr) const {
    for (const auto& m : m_maps) {
        if (addr >= m.base && addr < m.base + m.size) return &m;
    }
    return nullptr;
}

DeviceMapping* BusMemory::find(std::size_t addr) {
    for (auto& m : m_maps) {
        if (addr >= m.base && addr < m.base + m.size) return &m;
    }
    return nullptr;
}

void BusMemory::mapDevice(std::size_t base, std::shared_ptr<IDevice> dev) {
    if (!dev) throw std::invalid_argument("mapDevice: null device");
    DeviceMapping m;
    m.base = base;
    m.size = dev->size();
    m.device = std::move(dev);
    // naive overlap check
    for (const auto& ex : m_maps) {
        std::size_t endA = m.base + m.size, endB = ex.base + ex.size;
        if (!(endA <= ex.base || endB <= m.base)) {
            throw std::runtime_error("Device mapping overlaps existing device");
        }
    }
    m_maps.push_back(std::move(m));
}

u8 BusMemory::read8(std::size_t addr) const {
    if (auto m = find(addr)) {
        return m->device->read8(addr - m->base);
    }
    return m_ram.read8(addr);
}

u16 BusMemory::read16(std::size_t addr) const {
    if (auto m = find(addr)) {
        return m->device->read16(addr - m->base);
    }
    return m_ram.read16(addr);
}

u32 BusMemory::read32(std::size_t addr) const {
    if (auto m = find(addr)) {
        return m->device->read32(addr - m->base);
    }
    return m_ram.read32(addr);
}

void BusMemory::write8(std::size_t addr, u8 v) {
    if (auto m = find(addr)) {
        m->device->write8(addr - m->base, v);
        return;
    }
    m_ram.write8(addr, v);
}

void BusMemory::write16(std::size_t addr, u16 v) {
    if (auto m = find(addr)) {
        m->device->write16(addr - m->base, v);
        return;
    }
    m_ram.write16(addr, v);
}

void BusMemory::write32(std::size_t addr, u32 v) {
    if (auto m = find(addr)) {
        m->device->write32(addr - m->base, v);
        return;
    }
    m_ram.write32(addr, v);
}

} // namespace vm
