#pragma once

#include <string>

namespace core
{
class ConfigManager
{
public:
    explicit ConfigManager(const std::string& projectRoot);

    // ---- Load / Save ----
    bool load();            // lädt default config
    bool save() const;      // speichert default config

    // ---- Validation ----
    bool validate(std::string& errorOut) const;

    // ---- Setters (First-Run) ----
    void setClientPath(const std::string& path);
    void setResourcePath(const std::string& path);
    void setSourcePath(const std::string& path);

    // ---- Getters ----
    const std::string& clientPath() const;
    const std::string& resourcePath() const;
    const std::string& sourcePath() const;

private:
    std::string configFilePath() const;

private:
    std::string m_projectRoot;

    std::string m_clientPath;
    std::string m_resourcePath;
    std::string m_sourcePath;
};
}
