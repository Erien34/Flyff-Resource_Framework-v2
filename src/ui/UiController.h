#pragma once
#include <string>
#include <vector>
#include <SDL.h>
#include <SDL_opengl.h>
#include "DataState.h"
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

namespace core { class DataController; }
namespace ui   { class DialogFirstRun; }

namespace ui
{
class UiController
{
public:
    UiController();
    ~UiController();

    bool initialize(const std::string& title, int width, int height);
    void run();
    void shutdown();

    // WICHTIG → DataController in UI übertragen
    void injectDataController(core::DataController* dc);

    // Log sichtbar im Debugfenster
    void pushLog(const std::string& msg);

private:
    // Rendering + UI Loop
    void beginFrame();
    void endFrame();
    void drawDockspace();
    void drawDebugConsole();

private:
    SDL_Window*   m_window = nullptr;
    SDL_GLContext m_glCtx  = nullptr;
    bool m_running = false;

    std::vector<std::string> m_logBuffer;

    // NEW → First Run Dialog
    DialogFirstRun* m_firstDialog = nullptr;
    core::DataController* m_dataController = nullptr;
    core::DataState m_lastDataState = core::DataState::NotInitialized;
};
}
