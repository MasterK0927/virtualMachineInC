#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>
#include <cstdint>
#include <cstring>

namespace vm {

// Versioned program headers. Magic is always 'V','M','B','1'.
struct ProgramHeaderV1 {
    char magic[4];        // 'V','M','B','1'
    std::uint32_t version; // 1
    std::uint32_t entry;   // entry point PC
};

struct ProgramHeaderV2 {
    char magic[4];        // 'V','M','B','1'
    std::uint32_t version; // 2
    std::uint32_t entry;   // entry point PC
    std::uint32_t payloadSize; // number of payload bytes following the header
    std::uint32_t checksum;    // Adler-32 of payload
};

inline std::vector<unsigned char> loadBinaryFile(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open file: " + path);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

inline bool hasProgramHeader(const std::vector<unsigned char>& bytes) {
    if (bytes.size() < sizeof(ProgramHeaderV1)) return false;
    return bytes[0]=='V' && bytes[1]=='M' && bytes[2]=='B' && bytes[3]=='1';
}

inline std::uint32_t adler32(const unsigned char* data, std::size_t len) {
    // Simple Adler-32 implementation
    const std::uint32_t MOD_ADLER = 65521;
    std::uint32_t a = 1, b = 0;
    for (std::size_t i = 0; i < len; ++i) {
        a = (a + data[i]) % MOD_ADLER;
        b = (b + a) % MOD_ADLER;
    }
    return (b << 16) | a;
}

inline bool readAnyHeader(const std::vector<unsigned char>& bytes, ProgramHeaderV1& outV1, ProgramHeaderV2& outV2, bool& isV2) {
    if (!hasProgramHeader(bytes)) return false;
    // Peek version (assumes at least V1 size)
    std::memcpy(&outV1, bytes.data(), sizeof(ProgramHeaderV1));
    if (outV1.version == 1) { isV2 = false; return true; }
    if (bytes.size() < sizeof(ProgramHeaderV2)) throw std::runtime_error("Truncated v2 program header");
    std::memcpy(&outV2, bytes.data(), sizeof(ProgramHeaderV2));
    if (outV2.version == 2) { isV2 = true; return true; }
    throw std::runtime_error("Unsupported program header version");
}

inline std::vector<unsigned char> stripProgramHeader(const std::vector<unsigned char>& bytes) {
    if (!hasProgramHeader(bytes)) return bytes;
    // Determine version to know header size
    ProgramHeaderV1 v1{}; ProgramHeaderV2 v2{}; bool isV2 = false;
    readAnyHeader(bytes, v1, v2, isV2);
    const std::size_t hdrSize = isV2 ? sizeof(ProgramHeaderV2) : sizeof(ProgramHeaderV1);
    return std::vector<unsigned char>(bytes.begin() + hdrSize, bytes.end());
}

inline void verifyHeaderAndPayloadIfRequested(const std::vector<unsigned char>& bytes, bool verify) {
    if (!verify) return;
    if (!hasProgramHeader(bytes)) throw std::runtime_error("Verification requested but header missing");
    ProgramHeaderV1 v1{}; ProgramHeaderV2 v2{}; bool isV2 = false;
    readAnyHeader(bytes, v1, v2, isV2);
    if (!isV2) {
        // v1 has no checksum; accept but warn by throwing only if strictly needed. We accept for backward compat.
        return;
    }
    // Validate payload size and checksum
    const std::size_t hdrSize = sizeof(ProgramHeaderV2);
    if (bytes.size() < hdrSize) throw std::runtime_error("Invalid v2 header size");
    const std::size_t payloadAvail = bytes.size() - hdrSize;
    if (payloadAvail != v2.payloadSize) throw std::runtime_error("Payload size mismatch in header");
    std::uint32_t csum = adler32(bytes.data() + hdrSize, payloadAvail);
    if (csum != v2.checksum) throw std::runtime_error("Checksum mismatch in program payload");
}

} // namespace vm
