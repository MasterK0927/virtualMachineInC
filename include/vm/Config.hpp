#pragma once

#include <cstddef>
#include <optional>
#include <string>

namespace vm {

struct VMConfig {
    std::string name{"vm0"};
    std::size_t memSize{64 * 1024};
    std::optional<std::string> programPath{};
    std::optional<std::string> diskPath{};
    bool interactive{false};
    bool dumpAfter{false};
    std::size_t steps{0};
};

} // namespace vm
