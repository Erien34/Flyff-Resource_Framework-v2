#include "ModelParser.h"

#include "Log.h"
#include "data/asset/decoded/O3DDecoded.h"
#include "data/asset/parsed/O3DParsed.h"

#include "asset/parser/o3d/HeaderReader.h"
#include "asset/parser/o3d/ObjectReader.h"
#include "asset/parser/o3d/MeshReader.h"
#include "asset/parser/o3d/MeshParamBlockReader.h"
#include "asset/parser/o3d/PoolDirectoryReader.h"
#include "asset/parser/o3d/VertexBufferReader.h"
#include <algorithm>
#include <utility>

namespace
{
static void fillPart(resource::O3DParsedPart& out,
                     const resource::O3DDecodedPart& in,
                     const char* name)
{
    out = {};

    if (!in.exists)
    {
        core::Log::info(std::string("[ModelParser] Part missing: ") + name);
        return;
    }

    out.exists       = true;
    out.extension    = in.extension;
    out.sizeBytes    = in.sizeBytes;
    out.raw          = in.raw;
    out.signatureHex = in.signatureHex;

    core::Log::info(
        std::string("[ModelParser] Part detected name=") + name +
        " size=" + std::to_string(out.sizeBytes)
        );

    // ============================================================
    // HEADER
    // ============================================================
    asset::parser::o3d::O3DHeader header;
    std::string headerErr;

    if (!asset::parser::o3d::HeaderReader::read(
            header, out.raw, std::nullopt, &headerErr))
    {
        core::Log::error(
            "[ModelParser] HeaderReader failed part=" +
            std::string(name) + " err=" + headerErr
            );
        return;
    }

    out.headerVersion   = header.version;
    out.headerNameLower = header.embeddedFileNameLower;
    out.headerCursor    = header.cursorAfterHeader;

    core::Log::info(
        "[ModelParser] Header OK part=" + std::string(name) +
        " ver=" + std::to_string(header.version) +
        " cursor=" + std::to_string(header.cursorAfterHeader)
        );

    // ============================================================
    // OBJECTS
    // ============================================================
    asset::parser::o3d::ObjectReadResult objRes;
    std::string objErr;

    if (!asset::parser::o3d::ObjectReader::read(objRes, out.raw, header, &objErr))
    {
        core::Log::error(
            "[ModelParser] ObjectReader failed part=" +
            std::string(name) + " err=" + objErr
            );
        return;
    }

    core::Log::info(
        "[ModelParser] ObjectReader OK groupCount=" +
        std::to_string(objRes.groupCount) +
        " cursorAfterObjects=" +
        std::to_string(objRes.cursorAfterObjects)
        );

    // ============================================================
    // POOL DIRECTORY (nur meshPoolStart merken)
    // ============================================================
    asset::parser::o3d::PoolDirectoryResult dir;
    std::string dirErr;

    asset::parser::o3d::PoolDirectoryReader::read(
        dir,
        out.raw,
        objRes.cursorAfterObjects,
        header,
        &dirErr
        );

    if (!dir.found || dir.meshPoolStart == 0)
    {
        core::Log::pipelineInfo(
            "[ModelParser] PoolDir NOT FOUND or meshPoolStart invalid"
            );
        return;
    }

    out.meshPoolStart = dir.meshPoolStart;

    core::Log::pipelineInfo(
        "[ModelParser] PoolDir APPLY | meshPoolStart=" +
        std::to_string(out.meshPoolStart)
        );

    // ============================================================
    // MESH POOL PROBE (Packed Mesh Headers) – optional/logging
    // ============================================================
    asset::parser::o3d::MeshReadResult meshRes;
    std::string meshErr;
    asset::parser::o3d::MeshReader::read(
        meshRes,
        out.raw,
        out.meshPoolStart, // Start = meshPoolStart
        &dir,
        &meshErr
        );

    // ============================================================
    // MeshParamBlocks – ALLE sammeln, KEIN APPLY hier
    // ============================================================
    out.meshParamBlocks.clear();

    for (const auto& grp : objRes.groups)
    {
        for (const auto& obj : grp.objects)
        {
            // FINALER Filter: nur Kandidaten + Objektblock muss VOR dem MeshPool enden
            // (wenn cursorAfterParams >= meshPoolStart, ist das sehr wahrscheinlich KEIN render-mesh-object)
            const bool isMeshObj =
                obj.hasMeshCandidate &&
                (obj.cursorAfterParams > 0) &&
                (obj.cursorAfterParams <= out.meshPoolStart);

            if (!isMeshObj)
                continue;

            asset::parser::o3d::MeshParamBlockResult pb;
            std::string pbErr;

            const bool ok = asset::parser::o3d::MeshParamBlockReader::read(
                pb,
                out.raw,
                obj.cursorAfterMatrix,
                obj.cursorAfterParams,
                out.meshPoolStart,
                static_cast<uint32_t>(out.headerVersion),
                &pbErr
                );

            if (!ok)
            {
                core::Log::pipelineInfo(
                    std::string("[ModelParser] MeshParam FAILED (mesh-object) | at=") +
                    std::to_string(obj.cursorAfterMatrix) + " err=" + pbErr
                    );
                continue;
            }

            out.meshParamBlocks.push_back(pb);

            core::Log::pipelineInfo(
                std::string("[ModelParser] MeshParam FOUND | ") +
                (pb.isRenderable ? "RENDER" : "DUMMY") +
                " pbAt=" + std::to_string(pb.paramBlockStartAbs) +
                " vc=" + std::to_string(pb.block.vertexCount) +
                " ic=" + std::to_string(pb.block.indexCount) +
                " stride=" + std::to_string(pb.block.vertexStride) +
                " subMeshes=" + std::to_string(pb.subMeshes.size())
                );
        }
    }

    core::Log::pipelineInfo(
        std::string("[ModelParser] MeshParamBlock TOTAL = ") +
        std::to_string(out.meshParamBlocks.size())
        );
}
}
namespace asset::parser
{
    bool ModelParser::parse(resource::O3DParsed& out, const resource::O3DDecoded& in, std::string* err)
    {
        out = {};

        if (!in.mesh.exists && !in.skeleton.exists)
        {
            if (err) *err = "ModelParser: input has no parts (neither skeleton nor mesh).";
            return false;
        }

        fillPart(out.skeleton, in.skeleton, "skeleton");
        fillPart(out.mesh, in.mesh, "mesh");
        return true;
    }
}
