#include "PoolDirectoryReader.h"
#include "Log.h"
#include "asset/decoder/BinaryReader.h"
#include "parser/o3d/HeaderReader.h"

#include <algorithm>

namespace asset::parser::o3d
{
struct Candidate
{
    bool found = false;
    int score = -1;
    std::size_t dirCursor = 0;
    std::vector<std::uint32_t> rel;
    std::vector<std::size_t> abs;
};

static bool readU32(const std::vector<std::uint8_t>& b, std::size_t at, std::uint32_t& out)
{
    if (at + 4 > b.size()) return false;
    out =  (std::uint32_t)b[at]
          | ((std::uint32_t)b[at+1] << 8)
          | ((std::uint32_t)b[at+2] << 16)
          | ((std::uint32_t)b[at+3] << 24);
    return true;
}

static Candidate tryFixed(const std::vector<std::uint8_t>& bytes,
                          std::size_t startCursor,
                          std::size_t dirCursor,
                          std::size_t count)
{
    Candidate c;
    c.dirCursor = dirCursor;

    for (std::size_t i = 0; i < count; ++i)
    {
        std::uint32_t rel;
        if (!readU32(bytes, dirCursor + i * 4, rel)) return c;

        std::size_t abs = startCursor + rel;
        if (abs >= bytes.size()) return c;
        if ((abs & 3) != 0) return c;

        c.rel.push_back(rel);
        c.abs.push_back(abs);
    }

    c.found = true;
    c.score = (int)count * 10;
    return c;
}

bool PoolDirectoryReader::read(PoolDirectoryResult& out,
                               const std::vector<std::uint8_t>& bytes,
                               std::size_t startCursor,
                               const O3DHeader& header,
                               std::string*)
{
    out = {};
    out.scanStartCursor = startCursor;

    if (header.poolSize <= 0)
        return true;

    Candidate best;

    for (std::size_t cur = startCursor; cur + header.poolSize * 4 < bytes.size(); cur += 4)
    {
        Candidate c = tryFixed(bytes, startCursor, cur, header.poolSize);
        if (c.found && c.score > best.score)
            best = c;
    }

    if (!best.found)
        return true;

    out.found = true;
    out.dirCursor = best.dirCursor;
    out.dirAfterCursor = best.dirCursor + best.rel.size() * 4;
    out.relValues = best.rel;
    out.absCursors = best.abs;

    // Phase 1: erster gültiger Pool
    out.meshPoolStart = best.abs.empty() ? 0 : best.abs[0];

    // core::Log::pipelineInfo("[POOL-DIR] FOUND dir=" +
    //                         std::to_string(out.dirCursor) +
    //                         " meshPoolStart=" +
    //                         std::to_string(out.meshPoolStart));

    return true;
}
}
