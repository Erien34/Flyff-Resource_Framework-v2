#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace data::raw::rawdefines
{

// ===============================
// RAW DEFINE KIND
// ===============================
enum class RawDefineKind
{
    MacroDefine,
    EnumDecl,
    EnumMember,
    Const,
    Constexpr,
    Directive
};

// ===============================
// RAW RECORD
// ===============================
struct RawDefineRecord
{
    RawDefineKind kind;

    std::string name;
    std::string rawValue;
    std::string rawType;
    std::string scope;

    std::string sourceFile;
    uint32_t line = 0;
};

// ===============================
// FILE DATA
// ===============================
struct RawDefineFileData
{
    std::vector<RawDefineRecord> records;
};

// ===============================
// MODULE DATA
// ===============================
struct RawDefineData
{
    bool valid = false;

    // domain -> file -> records
    std::unordered_map<
        std::string,
        std::unordered_map<std::string, RawDefineFileData>
        > files;
};

} // namespace data::raw::rawdefines
