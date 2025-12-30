#pragma once
#include <string>
#include <vector>

namespace data { struct TokenData; }

namespace modules::serializer
{
class SerializerBase
{
public:
    virtual ~SerializerBase() = default;

    virtual std::string moduleId() const = 0;
    virtual std::string outputModel() const = 0;

    void run(const std::vector<data::TokenData>& tokens);

protected:
    virtual void serialize(const std::vector<data::TokenData>& tokens) = 0;

    std::string normalizeWhitespace(const std::string& line) const;
    std::vector<std::string> splitByTab(const std::string& normalized) const;
};
}
