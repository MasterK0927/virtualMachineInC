#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "vm/Types.hpp"

namespace vm {

struct ILogger;
struct IMemory;

struct ICPU {
    virtual ~ICPU() = default;
    virtual void reset() = 0;
    virtual void run(std::size_t maxSteps = 0) = 0; // 0 => until HALT
    virtual void step() = 0;

    // Introspection
    virtual std::size_t regCount() const = 0;
    virtual u32 getReg(std::size_t idx) const = 0;
    virtual void setReg(std::size_t idx, u32 value) = 0;
    virtual u32 getPC() const = 0;
    virtual u32 getSP() const = 0;
    virtual u32 getFlags() const = 0;
    // Control for snapshot/restore
    virtual void setPC(u32 value) = 0;
    virtual void setSP(u32 value) = 0;
    virtual void setFlags(u32 value) = 0;
};

class SimpleCPU : public ICPU {
public:
    static constexpr std::size_t REG_COUNT = 8;

    SimpleCPU(IMemory& mem, ILogger* logger = nullptr);

    void reset() override;
    void run(std::size_t maxSteps = 0) override;
    void step() override;

    std::size_t regCount() const override { return REG_COUNT; }
    u32 getReg(std::size_t idx) const override { return idx < REG_COUNT ? m_regs[idx] : 0; }
    void setReg(std::size_t idx, u32 value) override { if (idx < REG_COUNT) m_regs[idx] = value; }

    u32 getPC() const override { return m_pc; }
    u32 getSP() const override { return m_sp; }
    u32 getFlags() const override { return m_flags; }
    void setPC(u32 value) override { m_pc = value; }
    void setSP(u32 value) override { m_sp = value; }
    void setFlags(u32 value) override { m_flags = value; }

private:
    void log(const char* level, const char* msg);

private:
    IMemory& m_mem;
    ILogger* m_logger;

    std::array<u32, REG_COUNT> m_regs{};
    u32 m_pc{0};
    u32 m_sp{0};
    u32 m_flags{0};
    bool m_halted{false};
};

} // namespace vm
