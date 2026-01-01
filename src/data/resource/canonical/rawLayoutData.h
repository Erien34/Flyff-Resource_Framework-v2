#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace data { struct Token; } // forward (Token kommt aus deinem TokenData Header)

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
// (LayoutSerializer erwartet "RawProvenance" + Member "prov")
// ------------------------------------------------------------
struct RawProvenance
{
    std::string sourceFile;
    uint32_t line = 0;
    uint32_t column = 0;
};

// ------------------------------------------------------------
// Rect / geometry (roh)
// ------------------------------------------------------------
struct RawRect
{
    int32_t x = 0;
    int32_t y = 0;
    int32_t w = 0;
    int32_t h = 0;
};

// ------------------------------------------------------------
// Control (roh)
// ------------------------------------------------------------
struct RawControl
{
    std::string id;          // z.B. WIDC_OK
    std::string rawType;     // z.B. WTYPE_BUTTON
    uint32_t rawFlags = 0;

    std::string rawVisualRef;   // z.B. "but_normal_01.tga" (roh)
    RawRect rect;

    RawProvenance prov;      // ✅ vom Serializer genutzt
};

// ------------------------------------------------------------
// Window (roh)
// ------------------------------------------------------------
struct RawWindow
{
    std::string id;          // z.B. "wndFoo"
    uint32_t rawFlags = 0;

    RawRect rect;

    std::vector<RawControl> controls;

    RawProvenance prov;      // ✅ vom Serializer genutzt
};

// ------------------------------------------------------------
// Stream = eine Quelldatei / TokenStream
// ------------------------------------------------------------
struct RawLayoutStream
{
    std::string moduleId;    // ✅ vom Serializer genutzt
    std::string sourceFile;
    std::string domain;

    // Token Snapshot (roh) – LayoutSerializer erwartet rawTokens
    std::vector<data::Token> rawTokens; // ✅ vom Serializer genutzt

    // Parsed output (falls LayoutSerializer schon Fenster/Controls baut)
    std::vector<RawWindow> windows;
};

// ------------------------------------------------------------
// ROOT
// ------------------------------------------------------------
struct rawLayoutData
{
    int32_t streamIndex = -1;
    int32_t windowIndex = -1;
    uint32_t version = 1; // ✅ vom Serializer genutzt

    bool valid = false;
    std::vector<RawLayoutStream> streams;

    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};
}
