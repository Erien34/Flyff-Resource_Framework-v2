#pragma once

#include <string>
#include <vector>
#include <unordered_map>

namespace modules::descriptors
{

struct ResourceFile
{
    std::string file;     // Dateiname, z.B. "textClient.txt.txt"
    std::string domain;   // Domain-Tag, z.B. "ui", "item", "" = global
};

struct Descriptor
{
    // Grundidentität
    std::string id;       // z.B. "text", "defines", "items"
    std::string name;     // Anzeigename
    std::string scope;    // "core" | "plugin"

    // Eingabedateien (domainfähig)
    std::vector<ResourceFile> resourceFiles;

    // Parser-Regeln
    struct ParserRule
    {
        std::string pattern; // z.B. "*.txt"
        int parserId = 0;
        int priority = 0;
    };
    std::vector<ParserRule> parserRules;

    // Pipeline-Zuordnung
    std::string serializerKey;
    std::string contextBuilderKey;
    std::vector<std::string> dependencies;

    // Typ / Ausgabe
    std::string type;         // z.B. "text", "defines", "items"
    std::string outputModel;  // z.B. "TextData"

    // Asset-Anforderungen (nur deklarativ)
    struct AssetsRequired
    {
        bool icon      = false;
        bool model     = false;
        bool animation = false;
        bool effect    = false;
        bool sound     = false;
    } assetsRequired;

    // Engine-Hook (optional)
    std::string engineHook;

    bool valid = true;
};

} // namespace modules::descriptors
