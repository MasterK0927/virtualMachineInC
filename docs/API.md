# SimpleVM API Reference

## Core Interfaces

### ICPU Interface
```cpp
class ICPU {
public:
    virtual void reset() = 0;
    virtual void run(std::size_t maxSteps = 0) = 0;
    virtual void step() = 0;
    
    // State inspection
    virtual u32 getReg(std::size_t idx) const = 0;
    virtual void setReg(std::size_t idx, u32 value) = 0;
    virtual u32 getPC() const = 0;
    virtual u32 getSP() const = 0;
    virtual u32 getFlags() const = 0;
};
```

### IMemory Interface
```cpp
class IMemory {
public:
    virtual std::size_t size() const = 0;
    virtual u8 read8(std::size_t addr) const = 0;
    virtual u16 read16(std::size_t addr) const = 0;
    virtual u32 read32(std::size_t addr) const = 0;
    virtual void write8(std::size_t addr, u8 v) = 0;
    virtual void write16(std::size_t addr, u16 v) = 0;
    virtual void write32(std::size_t addr, u32 v) = 0;
};
```

### IDevice Interface
```cpp
class IDevice {
public:
    virtual const char* name() const = 0;
    virtual std::size_t size() const = 0;
    
    virtual u8 read8(std::size_t offset) = 0;
    virtual u16 read16(std::size_t offset) = 0;
    virtual u32 read32(std::size_t offset) = 0;
    
    virtual void write8(std::size_t offset, u8 v) = 0;
    virtual void write16(std::size_t offset, u16 v) = 0;
    virtual void write32(std::size_t offset, u32 v) = 0;
};
```

## High-Level API

### VMInstance
```cpp
class VMInstance {
public:
    VMInstance(const VMConfig& cfg, ILogger* logger = nullptr);
    
    void powerOn();
    void loadProgramBytes(const std::vector<unsigned char>& bytes);
    void runUntilHalt();
    void runSteps(std::size_t steps);
    
    // Debugging
    ICPU* cpu();
    void addBreakpoint(u32 addr);
    void removeBreakpoint(u32 addr);
    
    // Memory access
    std::vector<unsigned char> memRead(u32 addr, std::size_t len) const;
    void memWrite(u32 addr, const std::vector<unsigned char>& bytes);
    
    // Snapshots
    void saveSnapshot(const std::string& path) const;
    void loadSnapshot(const std::string& path);
};
```

## Usage Examples

### Basic VM Usage
```cpp
#include "vm/Instance.hpp"
#include "vm/Logger.hpp"

// Create VM
ConsoleLogger logger;
VMConfig config;
config.memSize = 64 * 1024;
VMInstance vm(config, &logger);

// Load and run program
vm.powerOn();
auto program = loadBinaryFile("program.bin");
vm.loadProgramBytes(program);
vm.runUntilHalt();
```

### Custom Device
```cpp
class TimerDevice : public IDevice {
public:
    const char* name() const override { return "Timer"; }
    std::size_t size() const override { return 8; }
    
    u32 read32(std::size_t offset) override {
        if (offset == 0) return getCurrentTime();
        return 0;
    }
    
    void write32(std::size_t offset, u32 v) override {
        if (offset == 4) setAlarm(v);
    }
};
```

### GUI Panel
```cpp
class CustomPanel : public Panel {
public:
    void draw(VMInstance& inst) override {
        ImGui::Begin("Custom Panel");
        // Your GUI code here
        ImGui::End();
    }
};
```
