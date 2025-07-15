#include <SDL.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <optional>
#include <sstream>

// Dear ImGui
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"

#include "vm/Instance.hpp"
#include "vm/ProgramLoader.hpp"
#include "vm/Logger.hpp"
#include "vm/Opcodes.hpp"
#include "GuiApp.hpp"

using namespace vm;

static std::vector<unsigned char> loadProgramMaybe(const std::optional<std::string>& path) {
    if (path.has_value()) return loadBinaryFile(*path);
    // Fallback to small demo: LOADI R0,123; OUT R0; HALT
    std::vector<unsigned char> prog;
    prog.push_back(static_cast<unsigned char>(Opcode::LOADI)); prog.push_back(0);
    prog.push_back(123); prog.push_back(0); prog.push_back(0); prog.push_back(0);
    prog.push_back(static_cast<unsigned char>(Opcode::OUT)); prog.push_back(0);
    prog.push_back(static_cast<unsigned char>(Opcode::HALT));
    return prog;
}

int main(int argc, char** argv) {
    std::optional<std::string> programPath;
    bool verify = false;
    std::size_t memSize = 64 * 1024;

    for (int i=1;i<argc;++i) {
        std::string a = argv[i];
        if (a == "--verify") verify = true;
        else if (a == "--mem" && i+1 < argc) { memSize = static_cast<std::size_t>(std::stoul(argv[++i])); }
        else if (!a.empty() && a[0] != '-') programPath = a;
    }

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("SimpleVM GUI", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 700, SDL_WINDOW_SHOWN);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        std::fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    gui::GuiApp app(window, renderer, programPath, verify, memSize);

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if (!app.handleEvent(e)) running = false;
        }
        app.update();
        // ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        app.draw();

        // Rendering
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 20, 20, 22, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // ImGui shutdown
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
