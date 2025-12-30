#include "MeshParamBlockReader.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

#include "Log.h"

namespace asset::parser::o3d
{
static uint32_t readU32LE(const std::vector<uint8_t>& raw, std::size_t off)
{
    return (uint32_t)raw[off + 0]
           | ((uint32_t)raw[off + 1] << 8)
           | ((uint32_t)raw[off + 2] << 16)
           | ((uint32_t)raw[off + 3] << 24);
}

static std::string hex36(const std::vector<uint8_t>& raw, std::size_t off)
{
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    const std::size_t end = std::min(off + (std::size_t)36, raw.size());
    for (std::size_t i = off; i < end; ++i)
        ss << std::setw(2) << (int)raw[i];
    return ss.str();
}

static bool readBlockAt(const std::vector<uint8_t>& raw, std::size_t off, MeshParamBlock& p, std::string* err)
{
    if (off + 36 > raw.size())
    {
        if (err) *err = "MeshParamBlock: out of range (need 36 bytes).";
        return false;
    }

    p.vertexCount   = readU32LE(raw, off + 0x00);
    p.indexCount    = readU32LE(raw, off + 0x04);
    p.vertexStride  = readU32LE(raw, off + 0x08);
    p.vertexOffset  = readU32LE(raw, off + 0x0C);
    p.indexOffset   = readU32LE(raw, off + 0x10);
    p.primitiveType = readU32LE(raw, off + 0x14);
    p.vertexFormat  = readU32LE(raw, off + 0x18);
    p.materialIndex = readU32LE(raw, off + 0x1C);
    p.unkExtra      = readU32LE(raw, off + 0x20);
    return true;
}

static bool validateBlock(const MeshParamBlock& p,
                          const std::vector<uint8_t>& raw,
                          std::size_t meshPoolStart,
                          uint32_t version,
                          bool* outRenderable,
                          std::string* err)
{
    if (outRenderable) *outRenderable = false;

    // ✅ Zero-mesh (Dummy) erlauben: gültiger Block, aber nicht renderbar
    if (p.vertexCount == 0 && p.indexCount == 0)
    {
        if (err) *err = "MeshParamBlock: zero-mesh (dummy).";
        return true;
    }

    // ab hier: renderbar nur wenn beide >0
    if (p.vertexCount == 0 || p.indexCount == 0)
    {
        if (err) *err = "MeshParamBlock: vertexCount/indexCount is zero.";
        return false;
    }

    if ((p.indexCount % 3) != 0)
    {
        if (err) *err = "MeshParamBlock: indexCount not divisible by 3.";
        return false;
    }

    if (p.vertexStride < 12 || p.vertexStride > 128)
    {
        if (err) *err = "MeshParamBlock: vertexStride out of plausible range.";
        return false;
    }

    const std::size_t vbStart = meshPoolStart + (std::size_t)p.vertexOffset;
    const std::size_t ibStart = meshPoolStart + (std::size_t)p.indexOffset;

    const std::size_t vbBytes = (std::size_t)p.vertexCount * (std::size_t)p.vertexStride;
    const std::size_t ibBytes = (std::size_t)p.indexCount * (std::size_t)2; // uint16

    if (vbStart >= raw.size() || vbStart + vbBytes > raw.size())
    {
        if (err) *err = "MeshParamBlock: vertex buffer range out of file.";
        return false;
    }

    if (ibStart >= raw.size() || ibStart + ibBytes > raw.size())
    {
        if (err) *err = "MeshParamBlock: index buffer range out of file.";
        return false;
    }

    if (outRenderable) *outRenderable = true;
    return true;
}

static void readSubMeshes(
    MeshParamBlockResult& out,
    const std::vector<uint8_t>& raw,
    std::size_t meshPoolStart)
{
    out.subMeshes.clear();

    if (!out.isRenderable)
        return;

    std::size_t cursor =
        meshPoolStart +
        out.block.indexOffset +
        out.block.indexCount * 2;

    const std::size_t maxEnd = raw.size();

    while (cursor + 16 <= maxEnd)
    {
        SubMesh sm{};
        sm.indexStart     = readU32LE(raw, cursor + 0);
        sm.indexCount     = readU32LE(raw, cursor + 4);
        sm.materialIndex  = readU32LE(raw, cursor + 8);
        sm.primitiveType  = readU32LE(raw, cursor + 12);

        // Plausibilitätscheck
        if (sm.indexCount == 0)
            break;

        if (sm.indexStart + sm.indexCount > out.block.indexCount)
            break;

        out.subMeshes.push_back(sm);
        cursor += 16;
    }
}

bool MeshParamBlockReader::read(
    MeshParamBlockResult& out,
    const std::vector<uint8_t>& raw,
    std::size_t cursorAfterMatrix,
    std::size_t cursorAfterParams,
    std::size_t meshPoolStart,
    uint32_t version,
    std::string* err)
{
    out = {};
    out.isRenderable = false;

    if (meshPoolStart == 0 || meshPoolStart >= raw.size())
    {
        if (err) *err = "MeshParamBlockReader: invalid meshPoolStart.";
        return false;
    }

    if (cursorAfterParams > raw.size())
    {
        if (err) *err = "MeshParamBlockReader: cursorAfterParams invalid.";
        return false;
    }

    std::string bestErr;

    // -----------------------------
    // PASS A: base = afterParams - 36 (dein aktuelles Verhalten)
    // -----------------------------
    if (cursorAfterParams >= 36)
    {
        const std::size_t base = cursorAfterParams;

        for (int delta = 0; delta < 4; ++delta)
        {
            const std::size_t start = base + (std::size_t)delta;

            MeshParamBlock p{};
            std::string rErr;
            if (!readBlockAt(raw, start, p, &rErr))
            {
                bestErr = rErr;
                continue;
            }

            bool renderable = false;
            std::string vErr;
            if (!validateBlock(p, raw, meshPoolStart, version, &renderable, &vErr))
            {
                std::ostringstream ss;
                ss << vErr
                   << " | start=" << start
                   << " base=" << base
                   << " delta=" << delta
                   << " afterMatrix=" << cursorAfterMatrix
                   << " afterParams=" << cursorAfterParams
                   << " vc=" << p.vertexCount
                   << " ic=" << p.indexCount
                   << " stride=" << p.vertexStride
                   << " vOff=" << p.vertexOffset
                   << " iOff=" << p.indexOffset
                   << " hex36=" << hex36(raw, start);
                bestErr = ss.str();
                continue;
            }

            out.found = true;
            out.isRenderable = renderable;
            out.paramBlockStartAbs = start;
            out.cursorAfter = start + 36;
            out.block = p;
            readSubMeshes(out, raw, meshPoolStart);

            core::Log::pipelineInfo(
                "[MeshParamBlock] SubMeshes count=" + std::to_string(out.subMeshes.size())
                );

            for (size_t i = 0; i < out.subMeshes.size() && i < 3; ++i)
            {
                const auto& sm = out.subMeshes[i];
                core::Log::pipelineInfo(
                    "[MeshParamBlock][SubMesh " + std::to_string(i) + "] " +
                    "iStart=" + std::to_string(sm.indexStart) +
                    " iCount=" + std::to_string(sm.indexCount) +
                    " mat=" + std::to_string(sm.materialIndex) +
                    " prim=" + std::to_string(sm.primitiveType)
                    );
            }
            return true;
        }
    }

    // -----------------------------
    // PASS B: Fallback direkt nach Matrix (+0..8)
    // (minimaler Zusatz, rettet force34/padding Fälle)
    // -----------------------------
    {
        const std::size_t base = cursorAfterMatrix;

        for (int delta = 0; delta <= 8; ++delta)
        {
            const std::size_t start = base + (std::size_t)delta;

            MeshParamBlock p{};
            std::string rErr;
            if (!readBlockAt(raw, start, p, &rErr))
            {
                bestErr = rErr;
                continue;
            }

            bool renderable = false;
            std::string vErr;
            if (!validateBlock(p, raw, meshPoolStart, version, &renderable, &vErr))
            {
                std::ostringstream ss;
                ss << vErr
                   << " | start=" << start
                   << " base=" << base
                   << " delta=" << delta
                   << " afterMatrix=" << cursorAfterMatrix
                   << " afterParams=" << cursorAfterParams
                   << " vc=" << p.vertexCount
                   << " ic=" << p.indexCount
                   << " stride=" << p.vertexStride
                   << " vOff=" << p.vertexOffset
                   << " iOff=" << p.indexOffset
                   << " hex36=" << hex36(raw, start);
                bestErr = ss.str();
                continue;
            }

            out.found = true;
            out.isRenderable = renderable;
            out.paramBlockStartAbs = start;
            out.cursorAfter = start + 36;
            out.block = p;

            readSubMeshes(out, raw, meshPoolStart);

            core::Log::pipelineInfo(
                "[MeshParamBlock] SubMeshes count=" + std::to_string(out.subMeshes.size())
                );

            for (size_t i = 0; i < out.subMeshes.size() && i < 3; ++i)
            {
                const auto& sm = out.subMeshes[i];
                core::Log::pipelineInfo(
                    "[MeshParamBlock][SubMesh " + std::to_string(i) + "] " +
                    "iStart=" + std::to_string(sm.indexStart) +
                    " iCount=" + std::to_string(sm.indexCount) +
                    " mat=" + std::to_string(sm.materialIndex) +
                    " prim=" + std::to_string(sm.primitiveType)
                    );
            }
            return true;
        }
    }

    if (err) *err = bestErr.empty()
                   ? "MeshParamBlockReader: no valid alignment candidate."
                   : bestErr;
    return false;
}
}
