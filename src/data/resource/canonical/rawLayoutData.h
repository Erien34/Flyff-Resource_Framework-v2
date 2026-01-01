#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace data { struct Token; } // kommt aus deinem TokenData System

namespace data::resource::canonical
{
// ------------------------------------------------------------
// Versioning / Schema
// ------------------------------------------------------------
struct RawLayoutVersion
{
    uint32_t major = 1;
    uint32_t minor = 0;
};

// ------------------------------------------------------------
// Provenance (woher kommt ein Element)
// ------------------------------------------------------------
struct RawProvenance
{
    std::string sourceFile;
    uint32_t line = 0;
    uint32_t column = 0;
};

// ------------------------------------------------------------
// Rect / geometry (roh)
// Achtung: Bei Flyff Controls sind es oft 4 Koordinaten:
// x y x1 y1  (oder x y x2 y2). Wir speichern es 1:1 roh.
// ------------------------------------------------------------
struct RawRect
{
    int32_t x  = 0;
    int32_t y  = 0;
    int32_t x1 = 0;
    int32_t y1 = 0;
};

// ------------------------------------------------------------
// Raw Color (optional)
// ------------------------------------------------------------
struct RawColor
{
    bool hasRgb = false;
    uint8_t r = 255, g = 255, b = 255;

    bool hasPacked = false;
    uint32_t packed = 0; // falls aus einem int extrahiert

    std::string mode; // "rgb" | "packed" | "default"
};

// ------------------------------------------------------------
// Control (roh) – alles, was du später für AssetRefs + Semantik brauchst
// ------------------------------------------------------------
struct RawControl
{
    // Header basics
    std::string rawHeaderLine;  // komplette Zeile (Debug + später Re-Parse möglich)
    std::string rawType;        // WTYPE_*
    std::string id;             // WIDC_*
    std::string rawVisualRef;   // "WndEditTile00.tga" oder ""

    // Control param layout (wie in deinem Editor)
    int32_t mod0 = 0;           // token[3]
    RawRect rect;               // token[4..7] (x y x1 y1)

    // flags
    std::string flagsHex;       // token[8] z.B. 0x20000
    uint32_t flagsMask = 0;     // parsed
    uint16_t lowFlags = 0;
    uint8_t  midFlags = 0;
    uint8_t  highFlags = 0;

    // optional mods
    int32_t mod1 = 0;
    int32_t mod2 = 0;
    int32_t mod3 = 0;
    int32_t mod4 = 0;
    bool hasMods1to4 = false;

    // optional color
    RawColor color;

    // Text refs (aus nachfolgenden IDS_* Blöcken)
    std::string titleId;
    std::string tooltipId;

    RawProvenance prov;
};

// ------------------------------------------------------------
// Window (roh) – analog zu deinem Editor
// ------------------------------------------------------------
struct RawWindow
{
    std::string rawHeaderLine;    // komplette Header-Zeile
    std::string id;               // APP_*

    std::string tile;             // p[1] unquoted
    std::string titleTextRaw;     // p[2] (kann "" oder IDS_ sein) 1:1 roh
    int32_t mode = 0;             // p[3]
    int32_t w = 0;                // p[4]
    int32_t h = 0;                // p[5]
    std::string flagsHex;         // p[6]
    uint32_t flagsMask = 0;       // parsed
    int32_t mod = 0;              // p[7]

    // optional: window text IDs (die Blöcke nach Header, bevor Controls starten)
    std::string windowTitleId;
    std::string windowHelpId;

    std::vector<RawControl> controls;

    RawProvenance prov;
};

// ------------------------------------------------------------
// Stream = eine Quelldatei / TokenStream
// ------------------------------------------------------------
struct RawLayoutStream
{
    std::string moduleId;
    std::string sourceFile;
    std::string domain;

    // Snapshot: Roh-Tokens (für späteres Reprocessing/Debug)
    std::vector<data::Token> rawTokens;

    // Parsed output
    std::vector<RawWindow> windows;
};

// ------------------------------------------------------------
// ROOT
// ------------------------------------------------------------
struct rawLayoutData
{
    RawLayoutVersion schema;

    bool valid = false;
    std::vector<RawLayoutStream> streams;

    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

} // namespace data::resource::canonical
