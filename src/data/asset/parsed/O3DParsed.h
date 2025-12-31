#pragma once
#include "parser/o3d/MeshParamBlockReader.h"
#include <cstdint>
#include <string>
#include <vector>

namespace asset
{
    struct O3DChunk
    {
        std::uint64_t offset = 0;   // absolute offset in raw
        std::uint64_t size   = 0;   // payload size (ohne header)
        std::uint32_t tag    = 0;   // chunk id / tag (roh)
        std::uint32_t headerSize = 0;

        // optional: nested (wenn wir später Container-Chunks erkennen)
        std::vector<O3DChunk> children;
    };


    struct O3DParsedPart
    {
        bool exists = false;
        std::string extension;
        std::size_t sizeBytes = 0;
        std::vector<std::uint8_t> raw;
        std::string signatureHex;

        // ---- legacy (für DataController Logging / alte Pipeline) ----
        std::vector<O3DChunk> chunks;     // kann leer bleiben
        float coverage = 0.0f;            // bleibt 0
        std::string chosenScheme;         // bleibt ""

        // ---- NEW (2A/2B/2C) ----

        std::size_t indexOffset = 0;

        bool indexIsU32 = false;
        std::int32_t headerVersion = 0;
        std::string  headerNameLower;
        std::size_t  headerCursor = 0;
        bool hasMeshPoolHeader = false;
        std::size_t meshPoolStart = 0;
        std::size_t meshPoolCursorAfterHeader = 0;

        bool hasVertexBuffer = false;
        std::size_t vertexBufferCursor = 0;
        std::vector<std::uint8_t> vertexBufferBytes;

        bool hasIndexBuffer = false;
        std::size_t indexBufferCursor = 0;
        std::uint32_t indexStride = 0;
        std::vector<std::uint8_t> indexBufferBytes;
        size_t   meshParamCursor   = 0;

        uint32_t vertexCount       = 0;
        uint32_t indexCount        = 0;
        uint32_t vertexStride      = 0;

        uint32_t vertexOffsetRel   = 0;
        uint32_t indexOffsetRel    = 0;

        uint32_t primitiveType     = 0;
        uint32_t materialIndex     = 0;

        size_t   vertexStartAbs    = 0;
        size_t   indexStartAbs     = 0;
        std::vector<asset::parser::o3d::MeshParamBlockResult> meshParamBlocks;
    };


    struct O3DParsed
    {
        O3DParsedPart skeleton;
        O3DParsedPart mesh;

        bool hasSkeleton() const { return skeleton.exists; }
        bool hasMesh() const { return mesh.exists; }
    };
}

