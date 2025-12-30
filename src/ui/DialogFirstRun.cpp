#include "ui/DialogFirstRun.h"

#include "core/Controller/DataController.h"

#include <imgui.h>
#include <ImGuiFileDialog.h>

namespace ui
{

DialogFirstRun::DialogFirstRun(core::DataController* controller)
    : dc(controller)
{
}

// ======================================================
// Main draw
// ======================================================

void DialogFirstRun::draw()
{
    ImVec2 vp = ImGui::GetMainViewport()->Size;

    ImGui::SetNextWindowSize(
        ImVec2(vp.x * 0.75f, vp.y * 0.6f),
        ImGuiCond_Appearing
    );

    ImGui::SetNextWindowPos(
        ImGui::GetMainViewport()->GetCenter(),
        ImGuiCond_Appearing,
        ImVec2(0.5f, 0.5f)
    );

    if (!popupOpened)
    {
        ImGui::OpenPopup("First Run Setup");
        ImGui::SetNextWindowFocus();
        popupOpened = true;
    }

    if (ImGui::BeginPopup(
            "First Run Setup",
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse))
    {
        // ---------------- CONTENT ----------------
        switch (currentStep)
        {
        case WizardStep::Welcome:
            drawWelcome();
            break;

        case WizardStep::Paths:
            drawPaths();
            break;

        case WizardStep::Review:
            drawReview();
            break;
        }

        ImGui::Dummy(ImVec2(0, 20));
        drawNavigation();

        ImGui::EndPopup();
    }
}

// ======================================================
// Steps
// ======================================================

void DialogFirstRun::drawWelcome()
{
    ImGui::TextWrapped(
        "Welcome to FlyFF Resource Framework.\n\n"
        "This setup wizard will guide you through the initial configuration.\n\n"
        "You will be asked to select the Client, Resource and Source folders."
    );
}

void DialogFirstRun::drawPaths()
{
    ImGui::TextWrapped("Please select the required folders.");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10));

    // ---------------- CLIENT ----------------
    ImGui::Text("Client Folder:");
    ImGui::SameLine(200);
    ImGui::SetNextItemWidth(500);
    char cbuf[256]; strcpy_s(cbuf, clientPath.c_str());
    ImGui::InputText("##client", cbuf, IM_ARRAYSIZE(cbuf));
    clientPath = cbuf;
    ImGui::SameLine();
    if (ImGui::Button("Browse##client")) openClient = true;

    // ---------------- RESOURCE ----------------
    ImGui::Text("Resource Folder:");
    ImGui::SameLine(200);
    ImGui::SetNextItemWidth(500);
    char rbuf[256]; strcpy_s(rbuf, resourcePath.c_str());
    ImGui::InputText("##resource", rbuf, IM_ARRAYSIZE(rbuf));
    resourcePath = rbuf;
    ImGui::SameLine();
    if (ImGui::Button("Browse##res")) openResource = true;

    // ---------------- SOURCE ----------------
    ImGui::Text("Source Folder:");
    ImGui::SameLine(200);
    ImGui::SetNextItemWidth(500);
    char sbuf[256]; strcpy_s(sbuf, sourcePath.c_str());
    ImGui::InputText("##source", sbuf, IM_ARRAYSIZE(sbuf));
    sourcePath = sbuf;
    ImGui::SameLine();
    if (ImGui::Button("Browse##src")) openSource = true;

    handleFileDialogs();
}

void DialogFirstRun::drawReview()
{
    ImGui::Text("Review your configuration:");
    ImGui::Separator();
    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Text("Client Folder:");
    ImGui::TextWrapped("%s", clientPath.c_str());
    ImGui::Dummy(ImVec2(0, 5));

    ImGui::Text("Resource Folder:");
    ImGui::TextWrapped("%s", resourcePath.c_str());
    ImGui::Dummy(ImVec2(0, 5));

    ImGui::Text("Source Folder:");
    ImGui::TextWrapped("%s", sourcePath.c_str());
}

// ======================================================
// Navigation
// ======================================================

void DialogFirstRun::drawNavigation()
{
    ImGui::Separator();

    if (currentStep != WizardStep::Welcome)
    {
        if (ImGui::Button("Back"))
            currentStep = (WizardStep)((int)currentStep - 1);
        ImGui::SameLine();
    }

    ImGui::BeginDisabled(!canGoNext());

    if (ImGui::Button(currentStep == WizardStep::Review ? "Finish" : "Next"))
    {
        if (currentStep == WizardStep::Review)
        {
            dc->applyFirstRunConfig(clientPath, resourcePath, sourcePath);
            shouldClose = true;
            ImGui::CloseCurrentPopup();
        }
        else
        {
            currentStep = (WizardStep)((int)currentStep + 1);
        }
    }

    ImGui::EndDisabled();
}

// ======================================================
// File dialogs
// ======================================================

void DialogFirstRun::handleFileDialogs()
{
    IGFD::FileDialogConfig cfg;
    cfg.path = ".";                // Startpfad

    if (openClient)
    {
        ImGuiFileDialog::Instance()->OpenDialog(
            "dlg_client",
            "Select Client Folder",
            nullptr,
            cfg
            );
        openClient = false;
    }

    if (openResource)
    {
        ImGuiFileDialog::Instance()->OpenDialog(
            "dlg_res",
            "Select Resource Folder",
            nullptr,
            cfg
            );
        openResource = false;
    }

    if (openSource)
    {
        ImGuiFileDialog::Instance()->OpenDialog(
            "dlg_src",
            "Select Source Folder",
            nullptr,
            cfg
            );
        openSource = false;
    }

    if (ImGuiFileDialog::Instance()->Display("dlg_client"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
            clientPath = ImGuiFileDialog::Instance()->GetCurrentPath();
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("dlg_res"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
            resourcePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        ImGuiFileDialog::Instance()->Close();
    }

    if (ImGuiFileDialog::Instance()->Display("dlg_src"))
    {
        if (ImGuiFileDialog::Instance()->IsOk())
            sourcePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        ImGuiFileDialog::Instance()->Close();
    }
}

// ======================================================
// Validation
// ======================================================

bool DialogFirstRun::valid() const
{
    return !clientPath.empty() && !resourcePath.empty();
}

bool DialogFirstRun::canGoNext() const
{
    if (currentStep == WizardStep::Paths)
        return valid();
    return true;
}

} // namespace ui
