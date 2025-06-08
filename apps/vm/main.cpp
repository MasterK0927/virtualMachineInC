#include <iostream>
#include <fstream>
#include <vector>
#include <memory>
#include <string>
#include <optional>
#include <sstream>
#include <algorithm>

#include "vm/Types.hpp"
#include "vm/Logger.hpp"
#include "vm/Memory.hpp"
#include "vm/Bus.hpp"
#include "vm/Device.hpp"
#include "vm/CPU.hpp"
#include "vm/ProgramLoader.hpp"
#include "vm/VM.hpp"
#include "vm/Config.hpp"
#include "vm/Instance.hpp"
#include "vm/Opcodes.hpp"

using namespace vm;

static std::vector<unsigned char> load_file_bytes(const std::string& path) {
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) throw std::runtime_error("Failed to open file: " + path);
    return std::vector<unsigned char>((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
}

static void disassemble(const std::vector<unsigned char>& bytes) {
    auto rd8 = [&](std::size_t addr) -> unsigned int {
        return addr < bytes.size() ? static_cast<unsigned int>(bytes[addr]) : 0;
    };
    auto rd16 = [&](std::size_t addr) -> unsigned int {
        return rd8(addr) | (rd8(addr + 1) << 8);
    };
    auto rd32 = [&](std::size_t addr) -> unsigned int {
        return rd8(addr) | (rd8(addr + 1) << 8) | (rd8(addr + 2) << 16) | (rd8(addr + 3) << 24);
    };

    std::size_t pc = 0;
    while (pc < bytes.size()) {
        auto op = static_cast<Opcode>(rd8(pc));
        std::cout << std::hex << pc << ": ";
        switch (op) {
            case Opcode::HALT:
                std::cout << "HALT\n";
                pc += 1; break;
            case Opcode::LOADI: {
                unsigned r = rd8(pc + 1);
                unsigned imm = rd32(pc + 2);
                std::cout << "LOADI R" << std::dec << r << ", " << imm << "\n";
                pc += 6; break;
            }
            case Opcode::LOAD: {
                unsigned rD = rd8(pc + 1);
                unsigned rS = rd8(pc + 2);
                unsigned off = rd16(pc + 3);
                std::cout << "LOAD R" << std::dec << rD << ", [R" << rS << "+" << off << "]\n";
                pc += 5; break;
            }
            case Opcode::STORE: {
                unsigned rD = rd8(pc + 1);
                unsigned rS = rd8(pc + 2);
                unsigned off = rd16(pc + 3);
                std::cout << "STORE [R" << std::dec << rD << "+" << off << "], R" << rS << "\n";
                pc += 5; break;
            }
            case Opcode::ADD:
            case Opcode::SUB:
            case Opcode::AND:
            case Opcode::OR:
            case Opcode::XOR: {
                unsigned rD = rd8(pc + 1);
                unsigned rA = rd8(pc + 2);
                unsigned rB = rd8(pc + 3);
                const char* mnem = (op == Opcode::ADD ? "ADD" : op == Opcode::SUB ? "SUB" : op == Opcode::AND ? "AND" : op == Opcode::OR ? "OR" : "XOR");
                std::cout << mnem << " R" << std::dec << rD << ", R" << rA << ", R" << rB << "\n";
                pc += 4; break;
            }
            case Opcode::CMP: {
                unsigned rA = rd8(pc + 1);
                unsigned rB = rd8(pc + 2);
                std::cout << "CMP R" << std::dec << rA << ", R" << rB << "\n";
                pc += 3; break;
            }
            case Opcode::JMP:
            case Opcode::JZ:
            case Opcode::JNZ:
            case Opcode::CALL: {
                unsigned addr = rd32(pc + 1);
                const char* mnem = (op == Opcode::JMP ? "JMP" : op == Opcode::JZ ? "JZ" : op == Opcode::JNZ ? "JNZ" : "CALL");
                std::cout << mnem << " 0x" << std::hex << addr << "\n";
                pc += 5; break;
            }
            case Opcode::RET:
                std::cout << "RET\n";
                pc += 1; break;
            case Opcode::PUSH: {
                unsigned r = rd8(pc + 1);
                std::cout << std::dec << "PUSH R" << r << "\n";
                pc += 2; break;
            }
            case Opcode::POP: {
                unsigned r = rd8(pc + 1);
                std::cout << std::dec << "POP R" << r << "\n";
                pc += 2; break;
            }
            case Opcode::OUT: {
                unsigned r = rd8(pc + 1);
                std::cout << std::dec << "OUT R" << r << "\n";
                pc += 2; break;
            }
            case Opcode::IN: {
                unsigned r = rd8(pc + 1);
                std::cout << std::dec << "IN R" << r << "\n";
                pc += 2; break;
            }
            default:
                std::cout << std::dec << "DB 0x" << std::hex << rd8(pc) << "\n";
                pc += 1; break;
        }
    }
}

static std::vector<unsigned char> build_demo_program() {
    // Program: LOADI R0, 12345; OUT R0; HALT
    std::vector<unsigned char> p;
    p.reserve(1 + 1 + 4 + 1 + 1 + 1);

    // LOADI
    p.push_back(static_cast<unsigned char>(Opcode::LOADI)); // opcode
    p.push_back(0x00); // R0
    vm::u32 imm = 12345;
    p.push_back(static_cast<unsigned char>(imm & 0xFF));
    p.push_back(static_cast<unsigned char>((imm >> 8) & 0xFF));
    p.push_back(static_cast<unsigned char>((imm >> 16) & 0xFF));
    p.push_back(static_cast<unsigned char>((imm >> 24) & 0xFF));

    // OUT R0
    p.push_back(static_cast<unsigned char>(Opcode::OUT));
    p.push_back(0x00);

    // HALT
    p.push_back(static_cast<unsigned char>(Opcode::HALT));

    return p;
}

static void dump_cpu_state(const ICPU* cpu) {
    if (!cpu) return;
    std::cout << "=== CPU STATE ===\n";
    std::cout << "PC=" << cpu->getPC() << " SP=" << cpu->getSP() << " FLAGS=" << cpu->getFlags() << "\n";
    for (std::size_t i = 0; i < cpu->regCount(); ++i) {
        std::cout << "R" << i << "=" << cpu->getReg(i) << (i + 1 == cpu->regCount() ? "\n" : " ");
    }
}

int main(int argc, char** argv) {
    try {
        ConsoleLogger logger;

        // Simple CLI parsing
        std::optional<std::string> binaryPath;
        std::size_t steps = 0; // 0 means run until HALT
        bool dumpAfter = false;
        bool disasmOnly = false;
        std::string instanceName = "vm0";
        std::size_t memSize = 64 * 1024; // default 64 KiB
        std::optional<std::string> diskPath; // RAM disk image
        bool interactive = false;
        std::optional<std::string> configPath;

        auto parseMem = [](const std::string& s) -> std::size_t {
            if (s.empty()) return 0;
            std::size_t mult = 1;
            std::string num = s;
            char last = static_cast<char>(std::tolower(s.back()));
            if (last == 'k') { mult = 1024; num = s.substr(0, s.size()-1); }
            else if (last == 'm') { mult = 1024*1024; num = s.substr(0, s.size()-1); }
            return static_cast<std::size_t>(std::stoull(num)) * mult;
        };

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--steps" && i + 1 < argc) {
                steps = static_cast<std::size_t>(std::stoul(argv[++i]));
            } else if (arg == "--dump") {
                dumpAfter = true;
            } else if (arg == "--demo") {
                binaryPath.reset();
            } else if (arg == "--disasm") {
                disasmOnly = true;
            } else if (arg == "--name" && i + 1 < argc) {
                instanceName = argv[++i];
            } else if (arg == "--mem" && i + 1 < argc) {
                memSize = parseMem(argv[++i]);
            } else if (arg == "--disk" && i + 1 < argc) {
                diskPath = argv[++i];
            } else if (arg == "--interactive") {
                interactive = true;
            } else if (arg == "--config" && i + 1 < argc) {
                configPath = argv[++i];
            } else if (!arg.empty() && arg[0] != '-') {
                binaryPath = arg;
            }
        }

        // Apply config file overrides if provided (simple key=value per line)
        if (configPath.has_value()) {
            std::ifstream cfs(*configPath);
            if (!cfs) throw std::runtime_error("Failed to open config: " + *configPath);
            std::string line;
            while (std::getline(cfs, line)) {
                auto posHash = line.find('#');
                if (posHash != std::string::npos) line = line.substr(0, posHash);
                auto pos = line.find('=');
                if (pos == std::string::npos) continue;
                std::string key = line.substr(0, pos);
                std::string val = line.substr(pos + 1);
                auto trimf = [](std::string& s){
                    auto notspace = [](int ch){ return !std::isspace(ch); };
                    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notspace));
                    s.erase(std::find_if(s.rbegin(), s.rend(), notspace).base(), s.end());
                };
                trimf(key); trimf(val);
                if (key == "name") instanceName = val;
                else if (key == "mem") memSize = parseMem(val);
                else if (key == "disk") diskPath = val;
                else if (key == "program") binaryPath = val;
                else if (key == "steps") steps = static_cast<std::size_t>(std::stoull(val));
                else if (key == "dump") dumpAfter = (val == "1" || val == "true" || val == "yes");
            }
        }

        // Build VM config and instance
        VMConfig cfg;
        cfg.name = instanceName;
        cfg.memSize = memSize;
        cfg.programPath = binaryPath;
        cfg.diskPath = diskPath;
        cfg.interactive = interactive;
        cfg.dumpAfter = dumpAfter;
        cfg.steps = steps;

        std::cout << "Launching VM instance '" << cfg.name << "' with memory " << cfg.memSize << " bytes" << std::endl;
        VMInstance instance(cfg, &logger);
        instance.powerOn();
        if (diskPath.has_value()) {
            instance.attachRamDisk(*diskPath);
        }

        auto loadProgram = [&](const std::optional<std::string>& path) -> std::vector<unsigned char> {
            if (path.has_value()) return load_file_bytes(*path);
            return build_demo_program();
        };

        if (interactive) {
            std::cout << "Entering interactive mode. Commands: load <file>, start [steps], step [n], reset, dump, disasm [file],\n"
                         "  break add <addr>|del <addr>|list, mem read <addr> <len>|write <addr> <b0> [b1...],\n"
                         "  regs set Rn <val>, save <file>, loadsnap <file>, quit" << std::endl;
            std::vector<unsigned char> program = loadProgram(binaryPath);
            instance.loadProgramBytes(program);
            std::string line;
            while (true) {
                std::cout << instanceName << "> ";
                if (!std::getline(std::cin, line)) break;
                std::istringstream iss(line);
                std::string cmd; iss >> cmd;
                if (cmd == "quit" || cmd == "exit") break;
                else if (cmd == "reset") { instance.loadProgramBytes(program); std::cout << "OK" << std::endl; }
                else if (cmd == "dump") { dump_cpu_state(instance.cpu()); }
                else if (cmd == "start") {
                    std::size_t s = 0; iss >> s; if (s == 0) { instance.loadProgramBytes(program); instance.runUntilHalt(); } else { instance.loadProgramBytes(program); instance.runSteps(s); }
                    std::cout << "DONE" << std::endl;
                } else if (cmd == "step") {
                    std::size_t n = 1; iss >> n; if (n == 0) n = 1; instance.runSteps(n); std::cout << "STEPPED " << n << std::endl;
                } else if (cmd == "load") {
                    std::string p; iss >> p; if (p.empty()) { std::cout << "No file" << std::endl; continue; }
                    program = load_file_bytes(p); instance.loadProgramBytes(program); std::cout << "LOADED" << std::endl;
                } else if (cmd == "disasm") {
                    std::string p; iss >> p; if (!p.empty()) { auto b = load_file_bytes(p); disassemble(b); } else { disassemble(program); }
                } else if (cmd == "break") {
                    std::string sub; iss >> sub;
                    auto parseNum = [](const std::string& s)->u32{ if (s.rfind("0x",0)==0||s.rfind("0X",0)==0) return static_cast<u32>(std::stoul(s,nullptr,16)); return static_cast<u32>(std::stoul(s,nullptr,10)); };
                    if (sub == "add") { std::string a; iss >> a; if (a.empty()) { std::cout << "Usage: break add <addr>" << std::endl; } else { instance.addBreakpoint(parseNum(a)); std::cout << "BP added" << std::endl; } }
                    else if (sub == "del") { std::string a; iss >> a; if (a.empty()) { std::cout << "Usage: break del <addr>" << std::endl; } else { instance.removeBreakpoint(parseNum(a)); std::cout << "BP removed" << std::endl; } }
                    else if (sub == "list") { for (auto a : instance.breakpoints()) std::cout << std::hex << "0x" << a << std::dec << "\n"; if (instance.breakpoints().empty()) std::cout << "(none)" << std::endl; }
                    else { std::cout << "Usage: break add|del|list" << std::endl; }
                } else if (cmd == "mem") {
                    std::string sub; iss >> sub;
                    auto parseNum = [](const std::string& s)->u32{ if (s.rfind("0x",0)==0||s.rfind("0X",0)==0) return static_cast<u32>(std::stoul(s,nullptr,16)); return static_cast<u32>(std::stoul(s,nullptr,10)); };
                    if (sub == "read") { std::string a; std::size_t len; iss >> a >> len; if (a.empty()||len==0) { std::cout << "Usage: mem read <addr> <len>" << std::endl; } else { auto v = instance.memRead(parseNum(a), len); for (size_t i=0;i<v.size();++i){ if(i%16==0) std::cout<<"\n"; std::cout<<std::hex<<" "<<(int)v[i]<<std::dec; } std::cout<<"\n"; } }
                    else if (sub == "write") { std::string a; iss >> a; std::vector<unsigned char> bytes; std::string bx; while (iss >> bx) { bytes.push_back((unsigned char)parseNum(bx)); } if (a.empty()||bytes.empty()) { std::cout << "Usage: mem write <addr> <b0> [b1...]" << std::endl; } else { instance.memWrite(parseNum(a), bytes); std::cout << "OK" << std::endl; } }
                    else { std::cout << "Usage: mem read|write ..." << std::endl; }
                } else if (cmd == "regs") {
                    std::string sub; iss >> sub; if (sub == "set") { std::string r; unsigned long val; iss >> r >> val; if (r.size()<2 || (r[0]!='R' && r[0]!='r')) { std::cout << "Usage: regs set Rn <val>" << std::endl; } else { size_t idx = std::stoul(r.substr(1)); instance.cpu()->setReg(idx, (u32)val); std::cout << "OK" << std::endl; } } else { std::cout << "Usage: regs set Rn <val>" << std::endl; }
                } else if (cmd == "save") {
                    std::string p; iss >> p; if (p.empty()) { std::cout << "Usage: save <file>" << std::endl; } else { instance.saveSnapshot(p); std::cout << "SAVED" << std::endl; }
                } else if (cmd == "loadsnap") {
                    std::string p; iss >> p; if (p.empty()) { std::cout << "Usage: loadsnap <file>" << std::endl; } else { instance.loadSnapshot(p); std::cout << "LOADED" << std::endl; }
                } else {
                    std::cout << "Unknown command" << std::endl;
                }
            }
        } else {
            std::vector<unsigned char> program = loadProgram(binaryPath);
            if (disasmOnly) {
                disassemble(program);
            } else {
                if (steps == 0) { instance.loadProgramBytes(program); instance.runUntilHalt(); }
                else { instance.loadProgramBytes(program); instance.runSteps(steps); }
                if (dumpAfter) dump_cpu_state(instance.cpu());
            }
        }
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
