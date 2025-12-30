#pragma once

#include <string>

namespace core
{
struct ProjectData
{
    // ================= PROJECT ROOT =================
    std::string projectRoot;

    // ================= USER CONFIGURED PATHS =================
    std::string clientPath;
    std::string resourcePath;
    std::string sourcePath;

    // ================= DERIVED INTERNAL PATHS =================
    std::string configPath;
    std::string dataPath;
    std::string internalDataPath;
    std::string externalDataPath;
    std::string assetCachePath;
    std::string logPath;
    std::string pluginPath;

    // ================= DESCRIPTORS =================
    // ---- Resource descriptors ----
    std::string descriptorResourceCorePath;
    std::string descriptorResourcePluginPath;

    // ---- Source descriptors ----
    std::string descriptorSourceCorePath;
    std::string descriptorSourcePluginPath;

    // ---- Runtime descriptors (future) ----
    std::string descriptorRuntimeCorePath;
    std::string descriptorRuntimePluginPath;

    // ================= VALIDATION =================
    bool valid = false;
};
}
