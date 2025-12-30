#pragma once

#include <string>

namespace core
{
class DataController;
}

namespace ui
{
class DialogFirstRun
{
public:
    explicit DialogFirstRun(core::DataController* dc);

    void draw();

    bool shouldClose = false;

private:
    // ---------------- Wizard ----------------
    enum class WizardStep
    {
        Welcome = 0,
        Paths,
        Review
    };

    WizardStep currentStep = WizardStep::Welcome;

    // ---------------- State ----------------
    core::DataController* dc = nullptr;
    bool popupOpened = false;

    // ---------------- Paths ----------------
    std::string clientPath;
    std::string resourcePath;
    std::string sourcePath;

    bool openClient   = false;
    bool openResource = false;
    bool openSource   = false;

    // ---------------- Helpers ----------------
    void drawWelcome();
    void drawPaths();
    void drawReview();
    void drawNavigation();

    void handleFileDialogs();

    bool valid() const;
    bool canGoNext() const;
};
}
