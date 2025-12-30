#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace resource::decode { class BinaryReader; }

namespace asset::parser::o3d
{
// Minimaler Vec3, damit wir 1:1 die Header-Felder transportieren können
struct Vec3
{
    float x = 0, y = 0, z = 0;
};

struct O3DHeader
{
    std::string embeddedFileNameLower; // im File gespeicherter Name (XOR 0xCD decoded, lowercase im client)

    std::int32_t version   = 0;   // nVer
    std::int32_t serialId  = 0;   // m_nID

    Vec3 force1{};
    Vec3 force2{};
    Vec3 force3{};
    Vec3 force4{};
    bool hasForce34 = false;      // version >= 22

    float scrollU = 0.f;
    float scrollV = 0.f;

    Vec3 bbMin{};
    Vec3 bbMax{};
    float perSlerp = 0.f;

    std::int32_t maxFrame = 0;
    std::int32_t maxEvent = 0;
    std::vector<Vec3> events;

    bool hasCollMesh = false;     // nTemp != 0 (wir skippen die GMObject-Daten hier nicht komplett 1:1, kommt im ObjectReader)
    std::int32_t lodFlag = 0;     // m_bLOD (int)

    std::int32_t maxBone = 0;     // m_nMaxBone
    bool hasBones = false;
    bool hasMotion = false;       // (maxBone>0 && maxFrame>0) -> Motion-Block folgt (lesen wir hier NICHT)
    std::int32_t sendVS = 0;      // m_bSendVS (int) wenn bones vorhanden

    std::int32_t poolSize = 0;    // nPoolSize

    // Cursor nach dem Header (d.h. dort wo im GameSource danach Group/Object-Loop beginnt)
    // Achtung: Wenn maxBone>0 und maxFrame>0, steht der Cursor NACH dem Motion-Block (wenn wir ihn skippen können).
    std::size_t cursorAfterHeader = 0;
};

class HeaderReader
{
public:
    // expectedFileNameLower ist optional (z.B. "mvr_maske.o3d" oder nur basename).
    // Wenn gesetzt, wird wie GameSource geprüft: embedded == expected (case-insensitiv).
    static bool read(O3DHeader& out,
                     const std::vector<std::uint8_t>& bytes,
                     const std::optional<std::string>& expectedFileNameLower,
                     std::string* outError = nullptr);
};
}
