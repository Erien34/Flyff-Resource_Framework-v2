#pragma once

#include <string>

namespace resource::decode
{
    // Einheitlicher Status für alle Decoder
    struct DecodeStatus
    {
        bool ok = false;
        std::string error;

        static DecodeStatus success()
        {
            return { true, {} };
        }

        static DecodeStatus failure(std::string msg)
        {
            return { false, std::move(msg) };
        }
    };

    // Marker-/Basisklasse für alle Decoder (memory-only!)
    //
    // Wichtig:
    // - KEIN Filesystem
    // - KEIN File-I/O
    // - KEINE Asset-Logik
    // - Nur gemeinsamer Nenner + zukünftige Erweiterbarkeit
    class DecoderBase
    {
    protected:
        DecoderBase() = default;
        virtual ~DecoderBase() = default;
    };
} // namespace resource::decode
