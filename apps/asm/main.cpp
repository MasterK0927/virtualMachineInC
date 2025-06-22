#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <cctype>
#include <algorithm>

#include "vm/Opcodes.hpp"
#include "vm/ProgramLoader.hpp" // ProgramHeader
#include <optional>

using Byte = unsigned char;

static std::string trim(const std::string& s) {
    std::string r = s;
    // remove comments starting with ';' or '#'
    auto posSemi = r.find(';');
    if (posSemi != std::string::npos) r = r.substr(0, posSemi);
    auto posHash = r.find('#');
    if (posHash != std::string::npos) r = r.substr(0, posHash);
    auto posSlash = r.find("//");
    if (posSlash != std::string::npos) r = r.substr(0, posSlash);
    // trim
    auto notspace = [](int ch){ return !std::isspace(ch); };
    r.erase(r.begin(), std::find_if(r.begin(), r.end(), notspace));
    r.erase(std::find_if(r.rbegin(), r.rend(), notspace).base(), r.end());
    return r;
}

static bool ieq(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    for (size_t i=0;i<a.size();++i) if (std::tolower(a[i]) != std::tolower(b[i])) return false;
    return true;
}

static int parseReg(const std::string& tok) {
    if (tok.size() < 2 || (tok[0] != 'R' && tok[0] != 'r')) return -1;
    int n = std::stoi(tok.substr(1));
    if (n < 0 || n > 7) return -1;
    return n;
}

static uint32_t parseImm(const std::string& tok) {
    std::string t = tok;
    // remove + signs
    t.erase(std::remove(t.begin(), t.end(), '+'), t.end());
    if (t.size() > 2 && t[0]=='0' && (t[1]=='x' || t[1]=='X')) {
        return static_cast<uint32_t>(std::stoul(t, nullptr, 16));
    }
    return static_cast<uint32_t>(std::stoul(t, nullptr, 10));
}

static std::vector<std::string> splitTokens(const std::string& line) {
    std::vector<std::string> toks;
    std::string tok;
    for (size_t i=0;i<line.size();++i) {
        char c = line[i];
        if (c==',' || c=='[' || c==']') {
            if (!tok.empty()) { toks.push_back(trim(tok)); tok.clear(); }
        } else if (std::isspace(static_cast<unsigned char>(c))) {
            if (!tok.empty()) { toks.push_back(trim(tok)); tok.clear(); }
        } else {
            tok.push_back(c);
        }
    }
    if (!tok.empty()) toks.push_back(trim(tok));
    return toks;
}

struct Line {
    std::string raw;
    std::string label; // optional
    std::vector<std::string> toks; // opcode + operands tokens
    size_t address = 0; // filled in pass1
    size_t size = 0; // filled in pass1
};

static size_t instrSize(const std::string& op) {
    if (ieq(op, "HALT") || ieq(op, "RET")) return 1;
    if (ieq(op, "LOADI")) return 1+1+4;
    if (ieq(op, "LOAD") || ieq(op, "STORE")) return 1+1+1+2;
    if (ieq(op, "ADD") || ieq(op, "SUB") || ieq(op, "AND") || ieq(op, "OR") || ieq(op, "XOR")) return 1+1+1+1;
    if (ieq(op, "CMP")) return 1+1+1;
    if (ieq(op, "PUSH") || ieq(op, "POP") || ieq(op, "OUT") || ieq(op, "IN")) return 1+1;
    if (ieq(op, "JMP") || ieq(op, "JZ") || ieq(op, "JNZ") || ieq(op, "CALL")) return 1+4;
    throw std::runtime_error("Unknown opcode in size: " + op);
}

static Byte opcodeOf(const std::string& op) {
    using namespace vm;
    if (ieq(op, "HALT")) return static_cast<Byte>(vm::Opcode::HALT);
    if (ieq(op, "LOADI")) return static_cast<Byte>(vm::Opcode::LOADI);
    if (ieq(op, "LOAD")) return static_cast<Byte>(vm::Opcode::LOAD);
    if (ieq(op, "STORE")) return static_cast<Byte>(vm::Opcode::STORE);
    if (ieq(op, "ADD")) return static_cast<Byte>(vm::Opcode::ADD);
    if (ieq(op, "SUB")) return static_cast<Byte>(vm::Opcode::SUB);
    if (ieq(op, "AND")) return static_cast<Byte>(vm::Opcode::AND);
    if (ieq(op, "OR")) return static_cast<Byte>(vm::Opcode::OR);
    if (ieq(op, "XOR")) return static_cast<Byte>(vm::Opcode::XOR);
    if (ieq(op, "CMP")) return static_cast<Byte>(vm::Opcode::CMP);
    if (ieq(op, "PUSH")) return static_cast<Byte>(vm::Opcode::PUSH);
    if (ieq(op, "POP")) return static_cast<Byte>(vm::Opcode::POP);
    if (ieq(op, "JMP")) return static_cast<Byte>(vm::Opcode::JMP);
    if (ieq(op, "JZ")) return static_cast<Byte>(vm::Opcode::JZ);
    if (ieq(op, "JNZ")) return static_cast<Byte>(vm::Opcode::JNZ);
    if (ieq(op, "CALL")) return static_cast<Byte>(vm::Opcode::CALL);
    if (ieq(op, "RET")) return static_cast<Byte>(vm::Opcode::RET);
    if (ieq(op, "OUT")) return static_cast<Byte>(vm::Opcode::OUT);
    if (ieq(op, "IN")) return static_cast<Byte>(vm::Opcode::IN);
    throw std::runtime_error("Unknown opcode: " + op);
}

static void emit32(std::vector<Byte>& out, uint32_t v) {
    out.push_back(static_cast<Byte>(v & 0xFF));
    out.push_back(static_cast<Byte>((v >> 8) & 0xFF));
    out.push_back(static_cast<Byte>((v >> 16) & 0xFF));
    out.push_back(static_cast<Byte>((v >> 24) & 0xFF));
}

static void emit16(std::vector<Byte>& out, uint32_t v) {
    out.push_back(static_cast<Byte>(v & 0xFF));
    out.push_back(static_cast<Byte>((v >> 8) & 0xFF));
}

int main(int argc, char** argv) {
    try {
        std::string inputPath;
        std::string outputPath = "a.bin";
        bool withHeader = false;
        std::optional<std::string> entryOpt; // label or numeric
        for (int i=1;i<argc;++i) {
            std::string arg = argv[i];
            if (arg == "-o" && i+1 < argc) { outputPath = argv[++i]; }
            else if (arg == "--with-header") { withHeader = true; }
            else if (arg == "--entry" && i+1 < argc) { entryOpt = argv[++i]; }
            else if (inputPath.empty()) { inputPath = arg; }
            else { throw std::runtime_error("Unexpected arg: " + arg); }
        }
        if (inputPath.empty()) {
            std::cerr << "Usage: asm <input.asm> [-o output.bin] [--with-header] [--entry <label|addr>]\n";
            return 2;
        }
        std::ifstream ifs(inputPath);
        if (!ifs) throw std::runtime_error("Failed to open input: " + inputPath);

        std::vector<Line> lines;
        std::string raw;
        while (std::getline(ifs, raw)) {
            std::string t = trim(raw);
            if (t.empty()) continue;
            Line ln; ln.raw = raw;
            // label?
            auto colon = t.find(':');
            if (colon != std::string::npos) {
                ln.label = trim(t.substr(0, colon));
                t = trim(t.substr(colon+1));
            }
            if (!t.empty()) {
                ln.toks = splitTokens(t);
            }
            lines.push_back(std::move(ln));
        }

        // Pass 1: addresses and labels
        std::unordered_map<std::string,size_t> labels;
        size_t addr = 0;
        for (auto& ln : lines) {
            ln.address = addr;
            if (!ln.label.empty()) {
                if (labels.count(ln.label)) throw std::runtime_error("Duplicate label: " + ln.label);
                labels[ln.label] = addr;
            }
            if (!ln.toks.empty()) {
                ln.size = instrSize(ln.toks[0]);
                addr += ln.size;
            } else {
                ln.size = 0;
            }
        }

        // Pass 2: encode
        std::vector<Byte> out;
        out.reserve(addr);
        for (const auto& ln : lines) {
            if (ln.toks.empty()) continue;
            const std::string op = ln.toks[0];
            out.push_back(opcodeOf(op));
            if (ieq(op, "HALT") || ieq(op, "RET")) {
                // nothing
            } else if (ieq(op, "LOADI")) {
                if (ln.toks.size() != 3) throw std::runtime_error("LOADI expects: LOADI Rn, imm");
                int rD = parseReg(ln.toks[1]); if (rD < 0) throw std::runtime_error("Invalid reg in LOADI");
                uint32_t imm = 0;
                if (labels.count(ln.toks[2])) imm = static_cast<uint32_t>(labels.at(ln.toks[2]));
                else imm = parseImm(ln.toks[2]);
                out.push_back(static_cast<Byte>(rD));
                emit32(out, imm);
            } else if (ieq(op, "LOAD")) {
                if (ln.toks.size() != 4) throw std::runtime_error("LOAD expects: LOAD Rd, [Rs + imm]");
                int rD = parseReg(ln.toks[1]);
                int rS = parseReg(ln.toks[2]);
                if (rD < 0 || rS < 0) throw std::runtime_error("Invalid reg in LOAD");
                uint32_t off = 0;
                if (labels.count(ln.toks[3])) off = static_cast<uint32_t>(labels.at(ln.toks[3]));
                else off = parseImm(ln.toks[3]);
                out.push_back(static_cast<Byte>(rD));
                out.push_back(static_cast<Byte>(rS));
                emit16(out, off);
            } else if (ieq(op, "STORE")) {
                if (ln.toks.size() != 4) throw std::runtime_error("STORE expects: STORE [Rd + imm], Rs");
                int rD = parseReg(ln.toks[1]);
                int rS = parseReg(ln.toks[3]);
                if (rD < 0 || rS < 0) throw std::runtime_error("Invalid reg in STORE");
                uint32_t off = 0;
                if (labels.count(ln.toks[2])) off = static_cast<uint32_t>(labels.at(ln.toks[2]));
                else off = parseImm(ln.toks[2]);
                out.push_back(static_cast<Byte>(rD));
                out.push_back(static_cast<Byte>(rS));
                emit16(out, off);
            } else if (ieq(op, "ADD") || ieq(op, "SUB") || ieq(op, "AND") || ieq(op, "OR") || ieq(op, "XOR")) {
                if (ln.toks.size() != 4) throw std::runtime_error(op + " expects: " + op + " Rd, Ra, Rb");
                int rD = parseReg(ln.toks[1]);
                int rA = parseReg(ln.toks[2]);
                int rB = parseReg(ln.toks[3]);
                if (rD < 0 || rA < 0 || rB < 0) throw std::runtime_error("Invalid reg in ALU op");
                out.push_back(static_cast<Byte>(rD));
                out.push_back(static_cast<Byte>(rA));
                out.push_back(static_cast<Byte>(rB));
            } else if (ieq(op, "CMP")) {
                if (ln.toks.size() != 3) throw std::runtime_error("CMP expects: CMP Ra, Rb");
                int rA = parseReg(ln.toks[1]);
                int rB = parseReg(ln.toks[2]);
                if (rA < 0 || rB < 0) throw std::runtime_error("Invalid reg in CMP");
                out.push_back(static_cast<Byte>(rA));
                out.push_back(static_cast<Byte>(rB));
            } else if (ieq(op, "PUSH") || ieq(op, "POP") || ieq(op, "OUT") || ieq(op, "IN")) {
                if (ln.toks.size() != 2) throw std::runtime_error(op + " expects: " + op + " Rn");
                int r = parseReg(ln.toks[1]);
                if (r < 0) throw std::runtime_error("Invalid reg in single-reg op");
                out.push_back(static_cast<Byte>(r));
            } else if (ieq(op, "JMP") || ieq(op, "JZ") || ieq(op, "JNZ") || ieq(op, "CALL")) {
                if (ln.toks.size() != 2) throw std::runtime_error(op + " expects: " + op + " label|addr");
                uint32_t addr32 = 0;
                if (labels.count(ln.toks[1])) addr32 = static_cast<uint32_t>(labels.at(ln.toks[1]));
                else addr32 = parseImm(ln.toks[1]);
                emit32(out, addr32);
            } else {
                throw std::runtime_error("Unknown op in pass2: " + op);
            }
        }

        std::ofstream ofs(outputPath, std::ios::binary);
        if (!ofs) throw std::runtime_error("Failed to open output: " + outputPath);
        if (withHeader) {
            vm::ProgramHeaderV2 hdr{};
            hdr.magic[0]='V'; hdr.magic[1]='M'; hdr.magic[2]='B'; hdr.magic[3]='1';
            hdr.version = 2;
            // Determine entry: label or numeric, default 0
            unsigned entry = 0;
            if (entryOpt.has_value()) {
                if (labels.count(*entryOpt)) entry = static_cast<unsigned>(labels.at(*entryOpt));
                else entry = parseImm(*entryOpt);
            }
            hdr.entry = entry;
            hdr.payloadSize = static_cast<std::uint32_t>(out.size());
            hdr.checksum = vm::adler32(out.data(), out.size());
            ofs.write(reinterpret_cast<const char*>(&hdr), static_cast<std::streamsize>(sizeof(hdr)));
        }
        ofs.write(reinterpret_cast<const char*>(out.data()), static_cast<std::streamsize>(out.size()));
        std::size_t total = out.size() + (withHeader ? sizeof(vm::ProgramHeaderV2) : 0);
        std::cout << "Wrote " << total << " bytes to " << outputPath << "\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "asm error: " << ex.what() << "\n";
        return 1;
    }
}
