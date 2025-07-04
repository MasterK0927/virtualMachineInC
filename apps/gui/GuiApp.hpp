#pragma once

#include <memory>
#include <optional>
#include <string>

#include <SDL.h>

#include "vm/Instance.hpp"
#include "vm/Logger.hpp"
#include "vm/ConsoleCapture.hpp"

namespace gui {

class Panel {
public:
    virtual ~Panel() = default;
    virtual void draw(vm::VMInstance& inst) = 0;
};

class ControlsPanel;
class CPUPanel;
class MemoryPanel;
class ConsolePanel;
class BreakpointsPanel;
class DisassemblyPanel;

class GuiApp {
public:
    GuiApp(SDL_Window* window, SDL_Renderer* renderer,
           std::optional<std::string> programPath,
           bool verify, std::size_t memSize);

    // returns false if should quit
    bool handleEvent(const SDL_Event& e);
    void update();
    void draw();
    void shutdown();

private:
    void resetVM();

private:
    SDL_Window* m_window{nullptr};
    SDL_Renderer* m_renderer{nullptr};

    std::optional<std::string> m_programPath;
    bool m_verify{false};
    std::size_t m_memSize{64*1024};

    std::vector<unsigned char> m_program;

    vm::ConsoleLogger m_forwardLogger;
    vm::BufferedLogger m_logger; // buffers logs for GUI and forwards to console
    std::unique_ptr<vm::ConsoleCapture> m_consoleCapture; // captures stdout for OUT opcode
    vm::VMInstance m_inst;

    bool m_playing{false};
    int m_stepsPerFrame{1000};
    bool m_dirtyTitle{true};

    std::unique_ptr<Panel> m_controls;
    std::unique_ptr<Panel> m_cpu;
    std::unique_ptr<Panel> m_memory;
    std::unique_ptr<Panel> m_console;
    std::unique_ptr<Panel> m_breakpoints;
    std::unique_ptr<Panel> m_disasm;
};

} // namespace gui
