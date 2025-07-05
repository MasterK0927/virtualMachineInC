#include "GuiApp.hpp"

#include <sstream>
#include <vector>

// Dear ImGui
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "vm/Logger.hpp"
#include "vm/ProgramLoader.hpp"
#include "vm/Decoder.hpp"

namespace gui {
using namespace vm;

namespace {
static std::vector<unsigned char> loadProgramMaybe(const std::optional<std::string>& path) {
    if (path.has_value()) return loadBinaryFile(*path);
    // Default tiny demo: LOADI R0,123; OUT R0; HALT
    std::vector<unsigned char> prog;
    prog.push_back(static_cast<unsigned char>(Opcode::LOADI)); prog.push_back(0);
    prog.push_back(123); prog.push_back(0); prog.push_back(0); prog.push_back(0);
    prog.push_back(static_cast<unsigned char>(Opcode::OUT)); prog.push_back(0);
    prog.push_back(static_cast<unsigned char>(Opcode::HALT));
    return prog;
}

class ControlsPanel : public Panel {
public:
    ControlsPanel(bool& playing, int& stepsPerFrame) : m_playing(playing), m_spf(stepsPerFrame) {}
    void setResetCallback(std::function<void()> cb) { m_onReset = std::move(cb); }
    void setStepCallback(std::function<void()> cb) { m_onStep = std::move(cb); }
    void setLoadProgramCallback(std::function<void(const std::string&)> cb) { m_onLoadProgram = std::move(cb); }
    void setSaveSnapshotCallback(std::function<void(const std::string&)> cb) { m_onSaveSnap = std::move(cb); }
    void setLoadSnapshotCallback(std::function<void(const std::string&)> cb) { m_onLoadSnap = std::move(cb); }

    void draw(VMInstance& /*inst*/) override {
        ImGui::Begin("Controls");
        if (ImGui::Button(m_playing ? "Pause" : "Run")) m_playing = !m_playing;
        ImGui::SameLine();
        if (ImGui::Button("Step") && m_onStep) m_onStep();
        ImGui::SameLine();
        if (ImGui::Button("Reset") && m_onReset) m_onReset();
        ImGui::SliderInt("Steps/frame", &m_spf, 1, 20000);
        ImGui::Text("SPACE=step  p=run/pause  r=reset  1/2/3 spf presets  ESC=quit");

        ImGui::Separator();
        ImGui::InputText("Program Path", &m_progPath);
        if (ImGui::Button("Load Program") && !m_progPath.empty() && m_onLoadProgram) m_onLoadProgram(m_progPath);

        ImGui::Separator();
        ImGui::InputText("Snapshot Path", &m_snapPath);
        if (ImGui::Button("Save Snapshot") && !m_snapPath.empty() && m_onSaveSnap) m_onSaveSnap(m_snapPath);
        ImGui::SameLine();
        if (ImGui::Button("Load Snapshot") && !m_snapPath.empty() && m_onLoadSnap) m_onLoadSnap(m_snapPath);

        ImGui::End();
    }
private:
    bool& m_playing;
    int& m_spf;
    std::function<void()> m_onReset;
    std::function<void()> m_onStep;
    std::function<void(const std::string&)> m_onLoadProgram;
    std::function<void(const std::string&)> m_onSaveSnap;
    std::function<void(const std::string&)> m_onLoadSnap;
    std::string m_progPath;
    std::string m_snapPath;
};

class CPUPanel : public Panel {
public:
    void draw(VMInstance& inst) override {
        ImGui::Begin("CPU");
        ICPU* cpu = inst.cpu();
        if (cpu) {
            int pc = static_cast<int>(cpu->getPC());
            int sp = static_cast<int>(cpu->getSP());
            int flags = static_cast<int>(cpu->getFlags());
            if (ImGui::InputInt("PC", &pc)) cpu->setPC(static_cast<u32>(pc));
            if (ImGui::InputInt("SP", &sp)) cpu->setSP(static_cast<u32>(sp));
            if (ImGui::InputInt("FLAGS", &flags)) cpu->setFlags(static_cast<u32>(flags));
            for (std::size_t i = 0; i < cpu->regCount(); ++i) {
                int rv = static_cast<int>(cpu->getReg(i));
                std::string lbl = "R" + std::to_string(i);
                if (ImGui::InputInt(lbl.c_str(), &rv)) cpu->setReg(i, static_cast<u32>(rv));
            }
        }
        ImGui::End();
    }
};

class MemoryPanel : public Panel {
public:
    void draw(VMInstance& inst) override {
        ImGui::Begin("Memory");
        static unsigned addr = 0;
        static int length = 128;
        ImGui::InputScalar("Addr", ImGuiDataType_U32, &addr);
        ImGui::InputInt("Length", &length);
        if (length < 0) length = 0;
        if (ImGui::Button("Read")) {
            try {
                m_buf = inst.memRead(addr, static_cast<std::size_t>(length));
            } catch (const std::exception& ex) {
                m_error = ex.what();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) { m_buf.clear(); }
        if (!m_error.empty()) { ImGui::TextColored(ImVec4(1,0.2f,0.2f,1), "%s", m_error.c_str()); m_error.clear(); }

        // Hex view (read-only)
        for (std::size_t i = 0; i < m_buf.size(); i += 16) {
            ImGui::Text("%08X: ", static_cast<unsigned>(addr + i)); ImGui::SameLine();
            std::string line;
            for (std::size_t j = 0; j < 16 && i + j < m_buf.size(); ++j) {
                char b[4]; std::snprintf(b, sizeof(b), "%02X ", m_buf[i+j]);
                line += b;
            }
            ImGui::TextUnformatted(line.c_str());
        }

        // Simple write: addr + value
        static unsigned waddr = 0; static unsigned wval = 0; static int wlen = 1;
        ImGui::Separator();
        ImGui::InputScalar("WAddr", ImGuiDataType_U32, &waddr);
        ImGui::InputInt("WLen(1/2/4)", &wlen);
        ImGui::InputScalar("WVal", ImGuiDataType_U32, &wval);
        if (ImGui::Button("Write")) {
            try {
                std::vector<unsigned char> data;
                if (wlen == 1) { data.push_back(static_cast<unsigned char>(wval & 0xFF)); }
                else if (wlen == 2) {
                    data.push_back(static_cast<unsigned char>(wval & 0xFF));
                    data.push_back(static_cast<unsigned char>((wval >> 8) & 0xFF));
                } else {
                    data.push_back(static_cast<unsigned char>(wval & 0xFF));
                    data.push_back(static_cast<unsigned char>((wval >> 8) & 0xFF));
                    data.push_back(static_cast<unsigned char>((wval >> 16) & 0xFF));
                    data.push_back(static_cast<unsigned char>((wval >> 24) & 0xFF));
                }
                inst.memWrite(waddr, data);
            } catch (const std::exception& ex) {
                m_error = ex.what();
            }
        }

        ImGui::End();
    }
private:
    std::vector<unsigned char> m_buf;
    std::string m_error;
};

class ConsolePanel : public Panel {
public:
    explicit ConsolePanel(BufferedLogger& log) : m_log(log) {}
    void draw(VMInstance&) override {
        ImGui::Begin("Console");
        if (ImGui::Button("Clear")) m_log.clear();
        ImGui::Separator();
        for (const auto& line : m_log.lines()) {
            ImGui::TextUnformatted(line.c_str());
        }
        ImGui::End();
    }
private:
    BufferedLogger& m_log;
};

class BreakpointsPanel : public Panel {
public:
    void draw(VMInstance& inst) override {
        ImGui::Begin("Breakpoints");
        // List existing
        auto bps = inst.breakpoints();
        for (auto addr : bps) {
            ImGui::Text("%08X", addr);
            ImGui::SameLine();
            if (ImGui::SmallButton((std::string("Remove ")+std::to_string(addr)).c_str())) {
                inst.removeBreakpoint(addr);
            }
        }
        // Add new
        static unsigned newbp = 0;
        ImGui::InputScalar("Address", ImGuiDataType_U32, &newbp);
        ImGui::SameLine();
        if (ImGui::Button("Add")) inst.addBreakpoint(newbp);
        ImGui::End();
    }
};

class DisassemblyPanel : public Panel {
public:
    void draw(VMInstance& inst) override {
        ImGui::Begin("Disassembly");
        
        ICPU* cpu = inst.cpu();
        if (!cpu) {
            ImGui::Text("No CPU available");
            ImGui::End();
            return;
        }
        
        u32 pc = cpu->getPC();
        ImGui::Text("Current PC: 0x%08X", pc);
        
        // Decode and display current instruction + lookahead
        SimpleDecoder decoder;
        try {
            // Get memory access (we'll use memRead helper)
            auto memBytes = inst.memRead(pc, 32); // read 32 bytes for lookahead
            
            // Create temporary memory wrapper for decoder
            RamMemory tempMem(memBytes.size());
            for (size_t i = 0; i < memBytes.size(); ++i) {
                tempMem.write8(i, memBytes[i]);
            }
            
            u32 addr = pc;
            for (int i = 0; i < 8 && addr < pc + memBytes.size(); ++i) {
                try {
                    auto decoded = decoder.decode(tempMem, addr - pc);
                    std::string disasm = disassemble(decoded);
                    
                    bool isCurrent = (addr == pc);
                    if (isCurrent) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "-> %08X: %s", addr, disasm.c_str());
                    } else {
                        ImGui::Text("   %08X: %s", addr, disasm.c_str());
                    }
                    
                    // Context menu for setting breakpoints
                    if (ImGui::BeginPopupContextItem()) {
                        if (ImGui::MenuItem("Set PC here")) {
                            cpu->setPC(addr);
                        }
                        if (ImGui::MenuItem("Add breakpoint")) {
                            inst.addBreakpoint(addr);
                        }
                        ImGui::EndPopup();
                    }
                    
                    addr += decoded.size;
                } catch (const std::exception&) {
                    ImGui::Text("   %08X: <decode error>", addr);
                    addr += 1; // skip one byte
                }
            }
        } catch (const std::exception& ex) {
            ImGui::Text("Memory read error: %s", ex.what());
        }
        
        ImGui::Separator();
        
        // Jump to address
        static unsigned jumpAddr = 0;
        ImGui::InputScalar("Jump to", ImGuiDataType_U32, &jumpAddr);
        ImGui::SameLine();
        if (ImGui::Button("Set PC")) {
            cpu->setPC(jumpAddr);
        }
        
        ImGui::End();
    }
};

} // namespace

GuiApp::GuiApp(SDL_Window* window, SDL_Renderer* renderer,
               std::optional<std::string> programPath,
               bool verify, std::size_t memSize)
    : m_window(window), m_renderer(renderer), m_programPath(std::move(programPath)), m_verify(verify), m_memSize(memSize),
      m_inst([&](){ VMConfig c; c.name="vm_gui"; c.memSize = m_memSize; return c; }(), &m_logger)
{
    // Forward buffered logs to console as well
    m_logger.setForward(&m_forwardLogger);
    
    // Capture stdout for OUT opcode display in GUI
    m_consoleCapture = std::make_unique<ConsoleCapture>(&m_logger);
    
    m_inst.powerOn();
    m_program = loadProgramMaybe(m_programPath);
    if (m_verify) verifyHeaderAndPayloadIfRequested(m_program, true);
    m_inst.loadProgramBytes(m_program);

    // Panels
    auto controls = std::make_unique<ControlsPanel>(m_playing, m_stepsPerFrame);
    controls->setResetCallback([this]{ resetVM(); });
    controls->setStepCallback([this]{ m_inst.runSteps(1); m_dirtyTitle = true; });
    controls->setLoadProgramCallback([this](const std::string& p){
        try {
            m_program = loadBinaryFile(p);
            if (m_verify) verifyHeaderAndPayloadIfRequested(m_program, true);
            m_inst.powerOn();
            m_inst.loadProgramBytes(m_program);
            m_dirtyTitle = true;
        } catch (const std::exception& ex) {
            m_logger.error(std::string("Load program failed: ")+ex.what());
        }
    });
    controls->setSaveSnapshotCallback([this](const std::string& p){
        try { m_inst.saveSnapshot(p); m_logger.info(std::string("Saved snapshot to ")+p); }
        catch (const std::exception& ex) { m_logger.error(std::string("Save snapshot failed: ")+ex.what()); }
    });
    controls->setLoadSnapshotCallback([this](const std::string& p){
        try { m_inst.loadSnapshot(p); m_logger.info(std::string("Loaded snapshot from ")+p); m_dirtyTitle = true; }
        catch (const std::exception& ex) { m_logger.error(std::string("Load snapshot failed: ")+ex.what()); }
    });
    m_controls = std::move(controls);
    m_cpu = std::make_unique<CPUPanel>();
    m_memory = std::make_unique<MemoryPanel>();
    m_console = std::make_unique<ConsolePanel>(m_logger);
    m_breakpoints = std::make_unique<BreakpointsPanel>();
    m_disasm = std::make_unique<DisassemblyPanel>();
}

void GuiApp::resetVM() {
    m_inst.powerOn();
    m_inst.loadProgramBytes(m_program);
    m_dirtyTitle = true;
}

bool GuiApp::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_QUIT) return false;
    if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
            case SDLK_ESCAPE: return false;
            case SDLK_SPACE: m_inst.runSteps(1); m_dirtyTitle = true; break;
            case SDLK_r: resetVM(); break;
            case SDLK_p: m_playing = !m_playing; break;
            case SDLK_d: {
                const ICPU* cpu = m_inst.cpu();
                std::printf("=== CPU ===\nPC=%u SP=%u\n", cpu->getPC(), cpu->getSP());
            } break;
            case SDLK_1: m_stepsPerFrame = 1; break;
            case SDLK_2: m_stepsPerFrame = 100; break;
            case SDLK_3: m_stepsPerFrame = 1000; break;
            default: break;
        }
    }
    return true;
}

void GuiApp::update() {
    if (m_playing) {
        m_inst.runSteps(static_cast<std::size_t>(m_stepsPerFrame));
        m_dirtyTitle = true;
    }
    
    // Process any captured console output
    if (m_consoleCapture) {
        m_consoleCapture->getAndClear(); // This forwards to logger automatically
    }
    
    if (m_dirtyTitle) {
        const ICPU* cpu = m_inst.cpu();
        std::ostringstream os;
        os << "SimpleVM GUI | PC=" << cpu->getPC() << " SP=" << cpu->getSP();
        SDL_SetWindowTitle(m_window, os.str().c_str());
        m_dirtyTitle = false;
    }
}

void GuiApp::draw() {
    if (m_controls) m_controls->draw(m_inst);
    if (m_cpu) m_cpu->draw(m_inst);
    if (m_memory) m_memory->draw(m_inst);
    if (m_console) m_console->draw(m_inst);
    if (m_breakpoints) m_breakpoints->draw(m_inst);
    if (m_disasm) m_disasm->draw(m_inst);
}

void GuiApp::shutdown() {
    // placeholder for future resources
}

} // namespace gui
