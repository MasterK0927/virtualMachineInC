#include "vm/Instance.hpp"
#include "vm/ProgramLoader.hpp"

#include <fstream>
#include <stdexcept>

namespace vm {

VMInstance::VMInstance(const VMConfig& cfg, ILogger* logger)
    : m_cfg(cfg), m_logger(logger) {
    // Initialize memory and CPU on construction
    m_mem = std::make_unique<RamMemory>(m_cfg.memSize);
    m_cpu = std::make_unique<SimpleCPU>(*m_mem, m_logger);
}

void VMInstance::powerOn() {
    if (!m_mem || !m_cpu) {
        throw std::runtime_error("VMInstance not properly initialized");
    }
    m_cpu->reset();
}

void VMInstance::attachRamDisk(const std::string& /*path*/) {
    // Placeholder: In future, load a disk image into a mapped region
    if (m_logger) m_logger->info("attachRamDisk: not implemented, ignoring");
}

void VMInstance::loadProgramBytes(const std::vector<unsigned char>& bytes) {
    if (!m_mem) throw std::runtime_error("Memory not initialized");

    // Check for optional header
    std::vector<unsigned char> payload = bytes;
    u32 entry = 0;
    if (hasProgramHeader(bytes)) {
        auto hdr = readProgramHeader(bytes);
        payload = stripProgramHeader(bytes);
        entry = hdr.entry;
        if (hdr.version != 1) {
            throw std::runtime_error("Unsupported program version");
        }
    }

    if (payload.size() > m_mem->size()) throw std::runtime_error("Program too large for memory");

    // load at address 0
    auto& raw = m_mem->raw();
    std::fill(raw.begin(), raw.end(), 0);
    std::copy(payload.begin(), payload.end(), raw.begin());

    // reset CPU, then set entry if provided
    m_cpu->reset();
    if (entry != 0) {
        m_cpu->setPC(entry);
    }
}

void VMInstance::runUntilHalt() {
    if (m_breakpoints.empty()) {
        m_cpu->run(0);
    } else {
        // Run step-by-step to honor breakpoints
        // Note: no direct halted state; this will run until hitting a breakpoint.
        // For now, we execute a generous number of steps.
        const std::size_t maxSteps = 10'000'000; // safety cap
        for (std::size_t i = 0; i < maxSteps; ++i) {
            if (hitBreakpoint(m_cpu->getPC())) break;
            m_cpu->step();
        }
    }
}

void VMInstance::runSteps(std::size_t steps) {
    if (steps == 0) { runUntilHalt(); return; }
    for (std::size_t i = 0; i < steps; ++i) {
        if (hitBreakpoint(m_cpu->getPC())) break;
        m_cpu->step();
    }
}

void VMInstance::addBreakpoint(u32 addr) {
    m_breakpoints.insert(addr);
}

void VMInstance::removeBreakpoint(u32 addr) {
    m_breakpoints.erase(addr);
}

bool VMInstance::hitBreakpoint(u32 pc) const {
    return m_breakpoints.find(pc) != m_breakpoints.end();
}

std::vector<unsigned char> VMInstance::memRead(u32 addr, std::size_t len) const {
    if (!m_mem) throw std::runtime_error("Memory not initialized");
    if (addr + len > m_mem->size()) throw std::out_of_range("memRead out of range");
    std::vector<unsigned char> out;
    out.reserve(len);
    for (std::size_t i = 0; i < len; ++i) {
        out.push_back(m_mem->read8(addr + i));
    }
    return out;
}

void VMInstance::memWrite(u32 addr, const std::vector<unsigned char>& bytes) {
    if (!m_mem) throw std::runtime_error("Memory not initialized");
    if (addr + bytes.size() > m_mem->size()) throw std::out_of_range("memWrite out of range");
    for (std::size_t i = 0; i < bytes.size(); ++i) {
        m_mem->write8(addr + i, bytes[i]);
    }
}

void VMInstance::saveSnapshot(const std::string& path) const {
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) throw std::runtime_error("Failed to open snapshot for write: " + path);

    // Magic
    const char magic[4] = {'S','N','P','1'};
    ofs.write(magic, 4);

    // CPU state
    u32 pc = m_cpu->getPC();
    u32 sp = m_cpu->getSP();
    u32 flags = m_cpu->getFlags();
    ofs.write(reinterpret_cast<const char*>(&pc), sizeof(pc));
    ofs.write(reinterpret_cast<const char*>(&sp), sizeof(sp));
    ofs.write(reinterpret_cast<const char*>(&flags), sizeof(flags));

    // Registers
    std::size_t rcount = m_cpu->regCount();
    ofs.write(reinterpret_cast<const char*>(&rcount), sizeof(rcount));
    for (std::size_t i = 0; i < rcount; ++i) {
        u32 r = m_cpu->getReg(i);
        ofs.write(reinterpret_cast<const char*>(&r), sizeof(r));
    }

    // Memory
    std::size_t memSz = m_mem->size();
    ofs.write(reinterpret_cast<const char*>(&memSz), sizeof(memSz));
    ofs.write(reinterpret_cast<const char*>(m_mem->raw().data()), static_cast<std::streamsize>(memSz));
}

void VMInstance::loadSnapshot(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open snapshot for read: " + path);

    char magic[4];
    ifs.read(magic, 4);
    if (ifs.gcount() != 4 || magic[0] != 'S' || magic[1] != 'N' || magic[2] != 'P' || magic[3] != '1') {
        throw std::runtime_error("Invalid snapshot magic");
    }

    u32 pc = 0, sp = 0, flags = 0;
    ifs.read(reinterpret_cast<char*>(&pc), sizeof(pc));
    ifs.read(reinterpret_cast<char*>(&sp), sizeof(sp));
    ifs.read(reinterpret_cast<char*>(&flags), sizeof(flags));

    std::size_t rcount = 0;
    ifs.read(reinterpret_cast<char*>(&rcount), sizeof(rcount));

    if (rcount != m_cpu->regCount()) {
        throw std::runtime_error("Snapshot register count mismatch");
    }

    for (std::size_t i = 0; i < rcount; ++i) {
        u32 r = 0;
        ifs.read(reinterpret_cast<char*>(&r), sizeof(r));
        m_cpu->setReg(i, r);
    }

    std::size_t memSz = 0;
    ifs.read(reinterpret_cast<char*>(&memSz), sizeof(memSz));
    if (memSz != m_mem->size()) {
        throw std::runtime_error("Snapshot memory size mismatch");
    }
    ifs.read(reinterpret_cast<char*>(m_mem->raw().data()), static_cast<std::streamsize>(memSz));

    // Restore CPU pointers
    m_cpu->setPC(pc);
    m_cpu->setSP(sp);
    m_cpu->setFlags(flags);
}

} // namespace vm
