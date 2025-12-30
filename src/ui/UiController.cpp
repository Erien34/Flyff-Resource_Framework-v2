#include "UiController.h"
#include "DataState.h"
#include "DialogFirstRun.h"
#include "controller/DataController.h"
#include "Log.h"

using namespace ui;

// =============================================================
// Constructor + Destructor
// =============================================================
UiController::UiController() {}
UiController::~UiController() { shutdown(); }


// =============================================================
// INIT (SDL + IMGUI + OPENGL)
// =============================================================
bool UiController::initialize(const std::string& title, int width, int height)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
        return false;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    m_window = SDL_CreateWindow(
        title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
        );

    m_glCtx = SDL_GL_CreateContext(m_window);
    SDL_GL_MakeCurrent(m_window, m_glCtx);

    // ImGui Setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // Docking bleibt aktiv

    // 🚫 Multi-Viewport komplett deaktivieren
    io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
    io.ConfigViewportsNoDecoration = true;
    io.ConfigViewportsNoTaskBarIcon = true;

    io.BackendFlags &= ~ImGuiBackendFlags_PlatformHasViewports;
    io.BackendFlags &= ~ImGuiBackendFlags_RendererHasViewports;

    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glCtx);
    ImGui_ImplOpenGL3_Init("#version 330");

    m_running = true;
    pushLog("UI Initialized.");
    return true;
}


// =============================================================
// MAIN LOOP
// =============================================================

void UiController::run()
{
    m_running = true;
    SDL_Event e;

    while (m_running)
    {
        // --------------------------------------------------
        // 1) Tick DataController
        // --------------------------------------------------
        if (m_dataController)
            m_dataController->update();

        // --------------------------------------------------
        // 2) Detect DataState transitions (ENTSCHEIDEND)
        // --------------------------------------------------
        if (m_dataController)
        {
            auto currentState = m_dataController->dataState();

            if (currentState != m_lastDataState)
            {
                //core::Log::info("UI detected DataState transition");

                if (currentState == core::DataState::FirstRunRequired)
                {
                    if (!m_firstDialog)
                    {
                        core::Log::info("Opening FirstRun dialog");
                        m_firstDialog = new DialogFirstRun(m_dataController);
                    }
                }

                m_lastDataState = currentState;
            }
        }

        // --------------------------------------------------
        // 3) SDL Events
        // --------------------------------------------------
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
                m_running = false;

            // Wichtig: ImGui muss Events bekommen
            ImGui_ImplSDL2_ProcessEvent(&e);
        }

        // --------------------------------------------------
        // 4) Begin UI frame
        // --------------------------------------------------
        beginFrame();

        drawDockspace();

        // --------------------------------------------------
        // 5) First Run Dialog
        // --------------------------------------------------
        if (m_firstDialog)
        {
            m_firstDialog->draw();

            if (m_firstDialog->shouldClose)
            {
                delete m_firstDialog;
                m_firstDialog = nullptr;
            }
        }

        // --------------------------------------------------
        // 6) Future editors here
        // --------------------------------------------------

        // --------------------------------------------------
        // 7) End UI frame
        // --------------------------------------------------
        endFrame();
    }
}

// =============================================================
// FRAME CONTROL
// =============================================================
void UiController::beginFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void UiController::endFrame()
{
    ImGui::Render();
    glViewport(0, 0, 1280, 720);
    glClearColor(0.13f, 0.13f, 0.13f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(m_window);
}


// =============================================================
// DOCKSPACE (Deine gewünschte Version)
// =============================================================
void UiController::drawDockspace()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGuiID dockspace_id    = ImGui::GetID("MainDockspace");

    ImGui::DockSpaceOverViewport(
        dockspace_id,
        viewport,
        ImGuiDockNodeFlags_None,
        nullptr
        );
}

// =============================================================
// DEBUG LOG WINDOW
// =============================================================
void UiController::drawDebugConsole()
{
    ImGui::Begin("Debug Log");
    for (auto& msg : m_logBuffer)
        ImGui::TextUnformatted(msg.c_str());
    ImGui::End();
}

void UiController::pushLog(const std::string& msg)
{
    m_logBuffer.push_back(msg);
}


// =============================================================
// CONNECT TO DATA CONTROLLER
// =============================================================
void UiController::injectDataController(core::DataController* dc)
{
    m_dataController = dc;
}

// =============================================================
// SHUTDOWN
// =============================================================
void UiController::shutdown()
{
    // Nur wenn ImGui existiert
    if (ImGui::GetCurrentContext())
    {
        ImGui_ImplOpenGL3_Shutdown();     // <-- Erst Renderer Backend
        ImGui_ImplSDL2_Shutdown();         // <-- Dann SDL Backend

        ImGui::DestroyPlatformWindows();   // <-- Viewport Ressourcen freigeben (keine Nebenfenster)
        ImGui::DestroyContext();           // <-- Kontext zerstören (ganz zum Schluss!)
    }

    // SDL Fenster danach beenden
    if (m_window)
        SDL_DestroyWindow(m_window);

    SDL_Quit();
}
