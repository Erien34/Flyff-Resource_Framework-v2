#include "MeshReader.h"
#include "Log.h"
#include "asset/decoder/BinaryReader.h"
#include "parser/o3d/PoolDirectoryReader.h"

#include <sstream>
#include <iomanip>

namespace asset::parser::o3d
{
static uint32_t readU32LE(const std::vector<uint8_t>& raw, std::size_t off)
{
    return (uint32_t)raw[off + 0]
           | ((uint32_t)raw[off + 1] << 8)
           | ((uint32_t)raw[off + 2] << 16)
           | ((uint32_t)raw[off + 3] << 24);
}

bool MeshReader::read(MeshReadResult& out,
                      const std::vector<std::uint8_t>& bytes,
                      std::size_t startCursor,
                      const PoolDirectoryResult* dirOpt,
                      std::string* outError)
{
    out = {};

    if (startCursor >= bytes.size())
    {
        if (outError) *outError = "MeshReader: startCursor out of range.";
        return false;
    }

    core::Log::pipelineInfo("[MESH]");
    core::Log::pipelineInfo(" startCursor=" + std::to_string(startCursor));

    // ------------------------------------------------------------
    // PHASE1-PROBE: Packed Mesh Headers im MeshPool scannen
    // Pattern (wie bei deinem ASE Match): vc, unk, faceCount, indexCount
    // ------------------------------------------------------------
    const std::size_t scanBegin = startCursor;
    const std::size_t scanEnd   = std::min(startCursor + (std::size_t)4096, bytes.size());

    int found = 0;

    for (std::size_t off = scanBegin; off + 16 <= scanEnd; ++off)
    {
        // wir erlauben absichtlich unaligned (Babykargo hatte 1-Byte shift)
        uint32_t vc   = readU32LE(bytes, off + 0);
        uint32_t unk  = readU32LE(bytes, off + 4);
        uint32_t fc   = readU32LE(bytes, off + 8);
        uint32_t ic   = readU32LE(bytes, off + 12);

        if (vc == 0 || fc == 0 || ic == 0)
            continue;

        // grob plausibel (breit lassen!)
        if (vc > 500000) continue;
        if (fc > 2000000) continue;

        // ASE typisch: ic == fc*3
        if (ic != fc * 3)
            continue;

        ++found;

        core::Log::pipelineInfo(
            "[MESH][PACKED] hdrAt=" + std::to_string(off) +
            " vc=" + std::to_string(vc) +
            " fc=" + std::to_string(fc) +
            " ic=" + std::to_string(ic) +
            " unk=" + std::to_string(unk)
            );

        // nur die ersten ~32 loggen, sonst spam
        if (found >= 32)
            break;
    }

    core::Log::pipelineInfo("[MESH] packedHeadersFound=" + std::to_string(found));

    // Phase1: wir parsen noch keine Entries final
    out.meshCount = 0;
    out.meshes.clear();
    out.cursorAfterMeshes = startCursor;

    if (dirOpt && dirOpt->found)
    {
        core::Log::pipelineInfo("[MESH] poolDir: meshPoolStart=" + std::to_string(dirOpt->meshPoolStart) +
                                " entries=" + std::to_string(dirOpt->absCursors.size()));
    }

    return true;
}
} // namespace asset::parser::o3d
