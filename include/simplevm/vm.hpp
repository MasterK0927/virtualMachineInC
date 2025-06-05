#pragma once

#include "simplevm/types.hpp"
#include "simplevm/opcodes.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace simplevm {

struct VMConfig {
    std::size_t stackCapacity = 1024; // number of 32-bit values
};

class VMError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class VM {
public:
    explicit VM(const VMConfig& cfg = {});

    void loadProgram(const Program& programBytes);
    void reset();
    void run();

    // For debugging/introspection
    std::size_t stackSize() const { return m_stack.size(); }

private:
    void push(Word v);
    Word pop();
    Word fetchWordLE();

private:
    VMConfig m_cfg{};
    Program m_program{};
    std::vector<Word> m_stack{};
    std::size_t m_pc{0};
    bool m_halted{false};
};

} // namespace simplevm
