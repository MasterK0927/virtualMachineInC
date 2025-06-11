#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "vm/Types.hpp"
#include "vm/Logger.hpp"
#include "vm/Memory.hpp"
#include "vm/CPU.hpp"
#include "vm/Config.hpp"

namespace vm {

class VMInstance {
public:
    VMInstance(const VMConfig& cfg, ILogger* logger = nullptr);

    void powerOn();
    void attachRamDisk(const std::string& path); // placeholder (no-op for now)

    // Program loading
    void loadProgramBytes(const std::vector<unsigned char>& bytes);

    // Execution control
    void runUntilHalt();
    void runSteps(std::size_t steps);

    // Debug/inspection
    ICPU* cpu() { return m_cpu.get(); }
    const ICPU* cpu() const { return m_cpu.get(); }

    // Breakpoints
    void addBreakpoint(u32 addr);
    void removeBreakpoint(u32 addr);
    std::set<u32> breakpoints() const { return m_breakpoints; }

    // Memory access helpers for CLI
    std::vector<unsigned char> memRead(u32 addr, std::size_t len) const;
    void memWrite(u32 addr, const std::vector<unsigned char>& bytes);

    // Snapshots
    void saveSnapshot(const std::string& path) const;
    void loadSnapshot(const std::string& path);

private:
    bool hitBreakpoint(u32 pc) const;

private:
    VMConfig m_cfg;
    ILogger* m_logger{nullptr};
    std::unique_ptr<RamMemory> m_mem;
    std::unique_ptr<SimpleCPU> m_cpu;
    std::set<u32> m_breakpoints;
};

} // namespace vm
