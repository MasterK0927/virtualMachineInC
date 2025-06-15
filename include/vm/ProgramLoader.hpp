#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <cstring>

namespace vm {

struct ProgramHeader {
    char magic[4];    // 'V','M','B','1'
    std::uint32_t version; // 1 for now
    std::uint32_t entry;   // entry point PC
};

inline std::vector<unsigned char> loadBinaryFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open file: " + path);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

inline bool hasProgramHeader(const std::vector<unsigned char>& bytes) {
    if (bytes.size() < sizeof(ProgramHeader)) return false;
    ProgramHeader hdr{};
    std::memcpy(&hdr, bytes.data(), sizeof(ProgramHeader));
    return hdr.magic[0]=='V' && hdr.magic[1]=='M' && hdr.magic[2]=='B' && hdr.magic[3]=='1';
}

inline ProgramHeader readProgramHeader(const std::vector<unsigned char>& bytes) {
    if (!hasProgramHeader(bytes)) throw std::runtime_error("Program header missing or invalid");
    ProgramHeader hdr{};
    std::memcpy(&hdr, bytes.data(), sizeof(ProgramHeader));
    return hdr;
}

inline std::vector<unsigned char> stripProgramHeader(const std::vector<unsigned char>& bytes) {
    if (!hasProgramHeader(bytes)) return bytes;
    return std::vector<unsigned char>(bytes.begin() + sizeof(ProgramHeader), bytes.end());
}

} // namespace vm
