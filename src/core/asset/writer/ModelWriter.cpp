#include "core/asset/writer/ModelWriter.h"

#include <cstdint>
#include <vector>
#include <algorithm>

// tinygltf implementation must live in exactly ONE .cpp
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_USE_CPP14
#include "tiny_gltf.h"

namespace
{
    static void appendBytes(std::vector<unsigned char>& dst, const void* src, size_t size)
    {
        const auto* p = reinterpret_cast<const unsigned char*>(src);
        dst.insert(dst.end(), p, p + size);
    }

    static void align4(std::vector<unsigned char>& dst)
    {
        while (dst.size() % 4) dst.push_back(0);
    }

    static int addView(tinygltf::Model& model, size_t offset, size_t size, int target)
    {
        tinygltf::BufferView v;
        v.buffer     = 0;
        v.byteOffset = static_cast<int>(offset);
        v.byteLength = static_cast<int>(size);
        v.target     = target;
        model.bufferViews.push_back(std::move(v));
        return static_cast<int>(model.bufferViews.size() - 1);
    }

    static int addAcc(tinygltf::Model& model, int view, int comp, int type, int count, int byteOffset = 0)
    {
        tinygltf::Accessor a;
        a.bufferView    = view;
        a.byteOffset    = byteOffset;
        a.componentType = comp;
        a.type          = type;
        a.count         = count;
        model.accessors.push_back(std::move(a));
        return static_cast<int>(model.accessors.size() - 1);
    }

    static void setMinMaxVec3(tinygltf::Accessor& acc, const std::vector<asset::normalized::Vec3>& pos)
    {
        if (pos.empty())
            return;

        float minx = pos[0].x, miny = pos[0].y, minz = pos[0].z;
        float maxx = pos[0].x, maxy = pos[0].y, maxz = pos[0].z;

        for (const auto& p : pos)
        {
            minx = std::min(minx, p.x); miny = std::min(miny, p.y); minz = std::min(minz, p.z);
            maxx = std::max(maxx, p.x); maxy = std::max(maxy, p.y); maxz = std::max(maxz, p.z);
        }

        acc.minValues = { minx, miny, minz };
        acc.maxValues = { maxx, maxy, maxz };
    }
}

namespace asset::writer
{
    bool ModelWriter::writeGlb(const asset::normalized::NormalizedMesh& mesh,
                              const std::filesystem::path& outFile,
                              std::string* outError)
    {
        if (mesh.positions.empty())
        {
            if (outError) *outError = "ModelWriter: mesh.positions empty.";
            return false;
        }
        if (!mesh.indices.empty() && (mesh.indices.size() % 3) != 0)
        {
            if (outError) *outError = "ModelWriter: indices not multiple of 3.";
            return false;
        }

        // If no primitives were provided, create one primitive for whole index buffer.
        std::vector<asset::normalized::NormalizedPrimitive> prims = mesh.primitives;
        if (prims.empty())
        {
            asset::normalized::NormalizedPrimitive p;
            p.indexOffset = 0;
            p.indexCount  = static_cast<uint32_t>(mesh.indices.size());
            p.materialSlot = 0;
            p.debugName = "AutoPrim";
            prims.push_back(std::move(p));
        }

        // material count = max(materialSlot)+1 (at least 1)
        int maxSlot = 0;
        for (const auto& p : prims) maxSlot = std::max(maxSlot, p.materialSlot);
        const int materialCount = std::max(1, maxSlot + 1);

        tinygltf::Model model;
        tinygltf::Scene scene;
        scene.name = "Scene";

        // --------------------------
        // Buffers (positions/normals/uvs/indices)
        // --------------------------
        tinygltf::Buffer buffer;
        buffer.data.reserve(1024 * 16);

        const size_t posOff = buffer.data.size();
        appendBytes(buffer.data, mesh.positions.data(), mesh.positions.size() * sizeof(asset::normalized::Vec3));

        size_t nrmOff = 0;
        if (mesh.hasNormals())
        {
            nrmOff = buffer.data.size();
            appendBytes(buffer.data, mesh.normals.data(), mesh.normals.size() * sizeof(asset::normalized::Vec3));
        }

        size_t uvOff = 0;
        if (mesh.hasUvs0())
        {
            uvOff = buffer.data.size();
            appendBytes(buffer.data, mesh.uvs0.data(), mesh.uvs0.size() * sizeof(asset::normalized::Vec2));
        }

        align4(buffer.data);

        const size_t idxOff = buffer.data.size();
        if (!mesh.indices.empty())
            appendBytes(buffer.data, mesh.indices.data(), mesh.indices.size() * sizeof(uint32_t));

        model.buffers.push_back(std::move(buffer));

        // --------------------------
        // BufferViews
        // --------------------------
        const int posView = addView(model, posOff, mesh.positions.size() * sizeof(asset::normalized::Vec3),
                                    TINYGLTF_TARGET_ARRAY_BUFFER);

        int nrmView = -1;
        if (mesh.hasNormals())
            nrmView = addView(model, nrmOff, mesh.normals.size() * sizeof(asset::normalized::Vec3),
                              TINYGLTF_TARGET_ARRAY_BUFFER);

        int uvView = -1;
        if (mesh.hasUvs0())
            uvView = addView(model, uvOff, mesh.uvs0.size() * sizeof(asset::normalized::Vec2),
                             TINYGLTF_TARGET_ARRAY_BUFFER);

        int idxView = -1;
        if (!mesh.indices.empty())
            idxView = addView(model, idxOff, mesh.indices.size() * sizeof(uint32_t),
                              TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

        // --------------------------
        // Accessors (shared attrib)
        // --------------------------
        const int posAcc = addAcc(model, posView,
                                  TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3,
                                  static_cast<int>(mesh.positions.size()));
        setMinMaxVec3(model.accessors[posAcc], mesh.positions);

        int nrmAcc = -1;
        if (mesh.hasNormals())
        {
            nrmAcc = addAcc(model, nrmView,
                            TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3,
                            static_cast<int>(mesh.normals.size()));
        }

        int uvAcc = -1;
        if (mesh.hasUvs0())
        {
            uvAcc = addAcc(model, uvView,
                           TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2,
                           static_cast<int>(mesh.uvs0.size()));
        }

        // --------------------------
        // Materials (placeholder – textures später)
        // --------------------------
        model.materials.reserve(materialCount);
        for (int i = 0; i < materialCount; ++i)
        {
            tinygltf::Material m;
            m.name = "Mat_" + std::to_string(i);
            m.pbrMetallicRoughness.baseColorFactor = {1.0, 1.0, 1.0, 1.0};
            m.pbrMetallicRoughness.metallicFactor  = 0.0;
            m.pbrMetallicRoughness.roughnessFactor = 1.0;
            model.materials.push_back(std::move(m));
        }

        // --------------------------
        // Mesh + Primitives (per primitive: own index accessor via byteOffset)
        // --------------------------
        tinygltf::Mesh gltfMesh;
        gltfMesh.name = mesh.name.empty() ? "Mesh" : mesh.name;

        for (const auto& p : prims)
        {
            if (idxView < 0 || mesh.indices.empty())
            {
                if (outError) *outError = "ModelWriter: mesh.indices missing but primitives exist.";
                return false;
            }

            // validate range
            const uint64_t end = uint64_t(p.indexOffset) + uint64_t(p.indexCount);
            if (end > mesh.indices.size())
            {
                if (outError) *outError = "ModelWriter: primitive index range out of bounds.";
                return false;
            }

            const int idxAcc = addAcc(model, idxView,
                                      TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR,
                                      static_cast<int>(p.indexCount),
                                      static_cast<int>(p.indexOffset * sizeof(uint32_t)));

            tinygltf::Primitive prim;
            prim.mode     = TINYGLTF_MODE_TRIANGLES;
            prim.indices  = idxAcc;
            prim.material = (p.materialSlot >= 0) ? p.materialSlot : 0;

            prim.attributes["POSITION"] = posAcc;
            if (nrmAcc >= 0) prim.attributes["NORMAL"] = nrmAcc;
            if (uvAcc  >= 0) prim.attributes["TEXCOORD_0"] = uvAcc;

            gltfMesh.primitives.push_back(std::move(prim));
        }

        model.meshes.push_back(std::move(gltfMesh));

        // Node + Scene
        tinygltf::Node node;
        node.name = "Node";
        node.mesh = 0;
        model.nodes.push_back(std::move(node));

        scene.nodes.push_back(0);
        model.scenes.push_back(std::move(scene));
        model.defaultScene = 0;

        // Write
        tinygltf::TinyGLTF gltf;
        std::string err, warn;
        const bool ok = gltf.WriteGltfSceneToFile(
            &model,
            outFile.string(),
            /*embedImages*/ true,
            /*embedBuffers*/ true,
            /*prettyPrint*/ false,
            /*writeBinary*/ true
        );

        if (!warn.empty())
        {
            // optional: warning in error string
        }
        if (!ok)
        {
            if (outError)
                *outError = "ModelWriter: WriteGltfSceneToFile failed. err=" + err + " warn=" + warn;
            return false;
        }
        if (!err.empty())
        {
            // tinygltf may still write but report non-fatal err; treat as failure if you want
        }

        return true;
    }
}
