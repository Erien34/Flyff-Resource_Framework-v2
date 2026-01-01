#pragma once
#include <string>
#include <any>
#include <unordered_map>
#include <mutex>
#include <stdexcept>
#include <typeinfo>

class GlobalStore
{
public:
    static GlobalStore& instance()
    {
        static GlobalStore s;
        return s;
    }

    // Store/overwrite model by key
    void storeModel(const std::string& key, std::any value)
    {
        std::scoped_lock lock(m_mutex);
        m_models[key] = std::move(value);
    }

    bool has(const std::string& key) const
    {
        std::scoped_lock lock(m_mutex);
        return m_models.find(key) != m_models.end();
    }

    void clear()
    {
        std::scoped_lock lock(m_mutex);
        m_models.clear();
    }

    template<typename T>
    const T& get(const std::string& key) const
    {
        std::scoped_lock lock(m_mutex);

        auto it = m_models.find(key);
        if (it == m_models.end())
            throw std::runtime_error("GlobalStore::get - missing key: " + key);

        const std::any& a = it->second;
        if (!a.has_value())
            throw std::runtime_error("GlobalStore::get - empty value for key: " + key);

        const T* ptr = std::any_cast<T>(&a);
        if (!ptr)
        {
            throw std::runtime_error(
                "GlobalStore::get - type mismatch for key: " + key +
                " (stored=" + std::string(a.type().name()) +
                ", requested=" + std::string(typeid(T).name()) + ")"
            );
        }
        return *ptr;
    }

private:
    GlobalStore() = default;

    mutable std::mutex m_mutex;
    std::unordered_map<std::string, std::any> m_models;
};
