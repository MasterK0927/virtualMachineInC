#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

namespace vm {

inline std::vector<unsigned char> loadBinaryFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open file: " + path);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

} // namespace vm
