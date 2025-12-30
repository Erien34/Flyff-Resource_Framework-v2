#include "core/ConfigManager.h"
#include "core/Log.h"

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace core
{
    ConfigManager::ConfigManager(const std::string& projectRoot)
        : m_projectRoot(projectRoot)
    {
    }

    // =====================================================
    // Load / Save
    // =====================================================

    bool ConfigManager::load()
    {
        const std::string path = configFilePath();

        if (!fs::exists(path))
            return false;

        std::ifstream in(path);
        if (!in.is_open())
            return false;

        std::string key;
        std::string value;

        while (in >> key >> value)
        {
            if (key == "clientPath")
                m_clientPath = value;
            else if (key == "resourcePath")
                m_resourcePath = value;
            else if (key == "sourcePath")
                m_sourcePath = value;
        }

        return true;
    }

    bool ConfigManager::save() const
    {
        const std::string path = configFilePath();

        fs::create_directories(fs::path(path).parent_path());

        std::ofstream out(path, std::ios::trunc);
        if (!out.is_open())
            return false;

        out << "clientPath "   << m_clientPath   << "\n";
        out << "resourcePath " << m_resourcePath << "\n";
        out << "sourcePath "   << m_sourcePath   << "\n";

        return true;
    }

    // =====================================================
    // Validation
    // =====================================================

    bool ConfigManager::validate(std::string& errorOut) const
    {
        if (m_clientPath.empty())
        {
            errorOut = "Client path is empty";
            return false;
        }

        if (!fs::exists(m_clientPath))
        {
            errorOut = "Client path does not exist";
            return false;
        }

        if (m_resourcePath.empty())
        {
            errorOut = "Resource path is empty";
            return false;
        }

        if (!fs::exists(m_resourcePath))
        {
            errorOut = "Resource path does not exist";
            return false;
        }

        if (!m_sourcePath.empty() && !fs::exists(m_sourcePath))
        {
            errorOut = "Source path does not exist";
            return false;
        }

        return true;
    }

    // =====================================================
    // Setters
    // =====================================================

    void ConfigManager::setClientPath(const std::string& path)
    {
        m_clientPath = path;
    }

    void ConfigManager::setResourcePath(const std::string& path)
    {
        m_resourcePath = path;
    }

    void ConfigManager::setSourcePath(const std::string& path)
    {
        m_sourcePath = path;
    }

    // =====================================================
    // Getters
    // =====================================================

    const std::string& ConfigManager::clientPath() const
    {
        return m_clientPath;
    }

    const std::string& ConfigManager::resourcePath() const
    {
        return m_resourcePath;
    }

    const std::string& ConfigManager::sourcePath() const
    {
        return m_sourcePath;
    }

    // =====================================================
    // Helpers
    // =====================================================

    std::string ConfigManager::configFilePath() const
    {
        return m_projectRoot + "/config/config.ini";
    }
}
