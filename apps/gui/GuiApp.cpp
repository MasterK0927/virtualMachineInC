#include "GuiApp.hpp"

#include <sstream>
#include <vector>
#include <functional>

// Dear ImGui
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"

#include "vm/Logger.hpp"
#include "vm/ProgramLoader.hpp"
#include "vm/Decoder.hpp"
#include "vm/Memory.hpp"

namespace gui {
using namespace vm;

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

    void draw(VMInstance& /*inst*/) override {
        ImGui::Begin("Controls");
        if (ImGui::Button(m_playing ? "Pause" : "Run")) m_playing = !m_playing;
        ImGui::SameLine();
        if (ImGui::Button("Step") && m_onStep) m_onStep();
        ImGui::SameLine();
        if (ImGui::Button("Reset") && m_onReset) m_onReset();
        ImGui::SliderInt("Steps/frame", &m_spf, 1, 20000);
        ImGui::Text("SPACE=step  p=run/pause  r=reset  ESC=quit");
        ImGui::End();
    }
private:
    bool& m_playing;
    int& m_spf;
    std::function<void()> m_onReset;
    std::function<void()> m_onStep;
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

    // Simplified panels
    auto controls = std::make_unique<ControlsPanel>(m_playing, m_stepsPerFrame);
    controls->setResetCallback([this]{ resetVM(); });
    controls->setStepCallback([this]{ m_inst.runSteps(1); m_dirtyTitle = true; });
    m_controls = std::move(controls);
    m_cpu = std::make_unique<CPUPanel>();
    m_console = std::make_unique<ConsolePanel>(m_logger);
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
    if (m_console) m_console->draw(m_inst);
}

void GuiApp::shutdown() {
    // placeholder for future resources
}

} // namespace gui
