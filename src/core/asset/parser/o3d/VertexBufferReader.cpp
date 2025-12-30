#include "VertexBufferReader.h"
#include "Log.h"
#include "asset/decoder/BinaryReader.h"

#include <cmath>
#include <algorithm>

namespace asset::parser::o3d
{
static bool isFinite(float v) { return std::isfinite(v) != 0; }

static bool readF32(const std::vector<std::uint8_t>& bytes, std::size_t off, float& out)
{
    if (off + 4 > bytes.size()) return false;
    std::uint32_t u =
        (std::uint32_t)bytes[off + 0] |
        ((std::uint32_t)bytes[off + 1] << 8) |
        ((std::uint32_t)bytes[off + 2] << 16) |
        ((std::uint32_t)bytes[off + 3] << 24);
    float f;
    std::memcpy(&f, &u, 4);
    out = f;
    return true;
}

static int scoreAsPositionTriplet(const std::vector<std::uint8_t>& bytes, std::size_t off)
{
    float x=0,y=0,z=0;
    if (!readF32(bytes, off + 0, x)) return -999;
    if (!readF32(bytes, off + 4, y)) return -999;
    if (!readF32(bytes, off + 8, z)) return -999;

    if (!isFinite(x) || !isFinite(y) || !isFinite(z)) return -999;

    // Flyff scale grob: sollte nicht astronomisch sein
    const float ax = std::fabs(x), ay = std::fabs(y), az = std::fabs(z);
    if (ax > 1.0e6f || ay > 1.0e6f || az > 1.0e6f) return -50;

    // Null-Triplet eher unwahrscheinlich für "echte" Vertexpos (kommt vor, aber selten massiv)
    if (x == 0.0f && y == 0.0f && z == 0.0f) return 0;

    // Bonus wenn Werte "normal" aussehen
    int s = 5;
    if (ax < 10000 && ay < 10000 && az < 10000) s += 5;
    if (ax < 1000  && ay < 1000  && az < 1000 ) s += 5;
    return s;
}

static std::size_t clampAdd(std::size_t a, std::size_t b, std::size_t maxv)
{
    if (a > maxv) return maxv;
    if (maxv - a < b) return maxv;
    return a + b;
}

bool VertexBufferReader::read(VertexBufferResult& out,
                              const std::vector<std::uint8_t>& bytes,
                              std::size_t startCursor,
                              std::string* outError)
{
    out = {};

    if (startCursor >= bytes.size())
    {
        if (outError) *outError = "VertexBufferReader: startCursor out of range.";
        return false;
    }

    core::Log::pipelineInfo("[VB]");
    core::Log::pipelineInfo(" startCursor=" + std::to_string(startCursor));

    // typische Vertex-Strides (pos+normal+uv etc.)
    const std::size_t strideCandidates[] = {
        12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 64
    };

    // wir suchen innerhalb eines kleinen Fensters nach "echtem" Vertexstart
    const std::size_t scanEnd = std::min(bytes.size(), startCursor + 256);

    int bestScore = -999999;
    std::size_t bestStart = 0;
    std::size_t bestStride = 0;

    for (std::size_t candStart = startCursor; candStart + 64 < scanEnd; candStart += 4)
    {
        for (std::size_t stride : strideCandidates)
        {
            // mindestens 3 vertices checken
            const std::size_t v0 = candStart;
            const std::size_t v1 = candStart + stride;
            const std::size_t v2 = candStart + stride * 2;

            if (v2 + 12 > bytes.size()) continue;

            int s0 = scoreAsPositionTriplet(bytes, v0);
            int s1 = scoreAsPositionTriplet(bytes, v1);
            int s2 = scoreAsPositionTriplet(bytes, v2);

            if (s0 < 5 || s1 < 5 || s2 < 5) continue;

            // Bonus wenn Positionen sich ändern (nicht 3x gleich)
            float x0=0,y0=0,z0=0,x1=0,y1=0,z1=0;
            readF32(bytes, v0+0, x0); readF32(bytes, v0+4, y0); readF32(bytes, v0+8, z0);
            readF32(bytes, v1+0, x1); readF32(bytes, v1+4, y1); readF32(bytes, v1+8, z1);

            int deltaBonus = (x0!=x1 || y0!=y1 || z0!=z1) ? 10 : 0;

            int total = s0 + s1 + s2 + deltaBonus;

            // kleiner Bonus für “gängige” Strides
            if (stride == 32 || stride == 36 || stride == 40) total += 3;

            if (total > bestScore)
            {
                bestScore = total;
                bestStart = candStart;
                bestStride = stride;
            }
        }
    }

    if (bestScore < 20)
    {
        out.found = false;
        if (outError) *outError = "VertexBufferReader: no plausible vertex stream found near startCursor.";
        core::Log::pipelineInfo("[VB] no plausible vertex stream found (score=" + std::to_string(bestScore) + ")");
        return false;
    }

    // count bestimmen: solange vertices plausibel bleiben
    std::size_t count = 0;
    const std::size_t maxVertsToProbe = 200000; // Schutz
    std::size_t cur = bestStart;

    while (count < maxVertsToProbe)
    {
        if (cur + 12 > bytes.size()) break;

        int s = scoreAsPositionTriplet(bytes, cur);
        if (s < 5) break;

        ++count;
        cur += bestStride;

        // Minimal: wir brauchen wenigstens ein paar vertices
        if (count >= 3 && cur > bytes.size()) break;
    }

    if (count < 3)
    {
        out.found = false;
        if (outError) *outError = "VertexBufferReader: found start/stride but vertexCount too small.";
        return false;
    }

    const std::size_t byteCount = count * bestStride;
    if (bestStart + byteCount > bytes.size())
    {
        // clamp
        const std::size_t remain = bytes.size() - bestStart;
        count = remain / bestStride;
    }

    out.found          = true;
    out.vertexDataStart= bestStart;
    out.vertexStride   = bestStride;
    out.vertexCount    = count;
    out.raw.assign(bytes.begin() + bestStart,
                   bytes.begin() + bestStart + (count * bestStride));

    core::Log::pipelineInfo("[VB] FOUND start=" + std::to_string(out.vertexDataStart) +
                            " stride=" + std::to_string(out.vertexStride) +
                            " count=" + std::to_string(out.vertexCount) +
                            " bytes=" + std::to_string(out.raw.size()));

    return true;
}
}
