// src/core/glb/GlbTest.cpp
#include "Glb-Test.h"

#include <cstdint>
#include <vector>
#include <string>
#include <utility>   // std::move
#include <iostream>

// IMPORTANT:
// - tinygltf implementation must live in exactly ONE .cpp
// - Your CMake already defines TINYGLTF_NO_STB_IMAGE (fine).
// - Do NOT include stb_image_write.h yourself here.
//   tinygltf will compile stb_image_write internally (unless you define TINYGLTF_NO_STB_IMAGE_WRITE).
#include "tiny_gltf.h"

namespace glbtest
{
// ============================================================
// Helpers
// ============================================================

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
    model.bufferViews.push_back(v);
    return static_cast<int>(model.bufferViews.size() - 1);
}

static int addAcc(tinygltf::Model& model, int view, int comp, int type, int count)
{
    tinygltf::Accessor a;
    a.bufferView     = view;
    a.componentType  = comp;
    a.type           = type;
    a.count          = count;
    model.accessors.push_back(a);
    return static_cast<int>(model.accessors.size() - 1);
}

// ============================================================
// Debug Atlas (RGBA) -> tinygltf encodes PNG when writing GLB
// ============================================================

struct DebugImage
{
    int width  = 256;
    int height = 256;
    std::vector<unsigned char> rgba;
};

static DebugImage makeDebugAtlas2x1()
{
    // 2x1 Atlas:
    // Left = Red, Right = Blue
    DebugImage img;
    img.width  = 256;
    img.height = 128;
    img.rgba.resize(img.width * img.height * 4);

    for (int y = 0; y < img.height; ++y)
    {
        for (int x = 0; x < img.width; ++x)
        {
            const bool left = (x < img.width / 2);

            unsigned char r = left ? 255 : 0;
            unsigned char g = 0;
            unsigned char b = left ? 0 : 255;
            unsigned char a = 255;

            const int i = (y * img.width + x) * 4;
            img.rgba[i + 0] = r;
            img.rgba[i + 1] = g;
            img.rgba[i + 2] = b;
            img.rgba[i + 3] = a;
        }
    }

    return img;
}

static void addEmbeddedAtlasTexture(tinygltf::Model& model)
{
    DebugImage dbg = makeDebugAtlas2x1();

    // Image (RAW RGBA) – tinygltf will PNG-encode on WriteGltfSceneToFile
    tinygltf::Image image;
    image.width      = dbg.width;
    image.height     = dbg.height;
    image.component  = 4;
    image.bits       = 8;
    image.pixel_type = TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    image.image      = std::move(dbg.rgba);
    image.mimeType   = "image/png";
    model.images.push_back(std::move(image));

    // Sampler
    tinygltf::Sampler sampler;
    sampler.minFilter = TINYGLTF_TEXTURE_FILTER_NEAREST;
    sampler.magFilter = TINYGLTF_TEXTURE_FILTER_NEAREST;
    sampler.wrapS     = TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE;
    sampler.wrapT     = TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE;
    model.samplers.push_back(sampler);

    // Texture
    tinygltf::Texture tex;
    tex.source  = 0; // model.images[0]
    tex.sampler = 0; // model.samplers[0]
    model.textures.push_back(tex);
}

// ============================================================
// Cube Geometry (24 verts: 6 faces * 4 verts) -> correct normals
// ============================================================

static const float CUBE_POSITIONS[] = {
    // Front
    -0.5f,-0.5f, 0.5f,   0.5f,-0.5f, 0.5f,   0.5f, 0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,
    // Back
    0.5f,-0.5f,-0.5f,  -0.5f,-0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,   0.5f, 0.5f,-0.5f,
    // Left
    -0.5f,-0.5f,-0.5f,  -0.5f,-0.5f, 0.5f,  -0.5f, 0.5f, 0.5f,  -0.5f, 0.5f,-0.5f,
    // Right
    0.5f,-0.5f, 0.5f,   0.5f,-0.5f,-0.5f,   0.5f, 0.5f,-0.5f,   0.5f, 0.5f, 0.5f,
    // Top
    -0.5f, 0.5f, 0.5f,   0.5f, 0.5f, 0.5f,   0.5f, 0.5f,-0.5f,  -0.5f, 0.5f,-0.5f,
    // Bottom
    -0.5f,-0.5f,-0.5f,   0.5f,-0.5f,-0.5f,   0.5f,-0.5f, 0.5f,  -0.5f,-0.5f, 0.5f
};

static const float CUBE_NORMALS[] = {
    // Front (0..3)
    0,0,1,  0,0,1,  0,0,1,  0,0,1,
    // Back (4..7)
    0,0,-1, 0,0,-1, 0,0,-1, 0,0,-1,
    // Left (8..11)
    -1,0,0, -1,0,0, -1,0,0, -1,0,0,
    // Right (12..15)
    1,0,0,  1,0,0,  1,0,0,  1,0,0,
    // Top (16..19)
    0,1,0,  0,1,0,  0,1,0,  0,1,0,
    // Bottom (20..23)
    0,-1,0, 0,-1,0, 0,-1,0, 0,-1,0
};

// Full cube indices (single primitive)
static const uint32_t CUBE_INDICES[] = {
    // Front (0–3)
    0,1,2,  0,2,3,
    // Back (4–7)
    4,5,6,  4,6,7,
    // Left (8–11)
    8,9,10, 8,10,11,
    // Right (12–15)
    12,13,14, 12,14,15,
    // Top (16–19)
    16,17,18, 16,18,19,
    // Bottom (20–23)
    20,21,22, 20,22,23
};

// Split indices: 3 faces + 3 faces = 18 + 18 indices
static const uint32_t CUBE_INDICES_A[] = {
    // Front
    0,1,2,  0,2,3,
    // Back
    4,5,6,  4,6,7,
    // Left
    8,9,10, 8,10,11
};

static const uint32_t CUBE_INDICES_B[] = {
    // Right
    12,13,14, 12,14,15,
    // Top
    16,17,18, 16,18,19,
    // Bottom
    20,21,22, 20,22,23
};

// Build 24 UVs for a face-mapped cube into either left or right atlas half.
// (All faces use same rect; good enough as a clean atlas test.)
static std::vector<float> buildCubeUvsAtlasHalf(bool leftHalf)
{
    // leave a tiny margin to avoid bleeding
    const float u0 = leftHalf ? 0.01f : 0.51f;
    const float u1 = leftHalf ? 0.49f : 0.99f;
    const float v0 = 0.01f;
    const float v1 = 0.99f;

    std::vector<float> uvs;
    uvs.reserve(24 * 2);

    // 6 faces * 4 verts:
    for (int face = 0; face < 6; ++face)
    {
        // quad UVs (same winding for every face)
        uvs.push_back(u0); uvs.push_back(v0);
        uvs.push_back(u1); uvs.push_back(v0);
        uvs.push_back(u1); uvs.push_back(v1);
        uvs.push_back(u0); uvs.push_back(v1);
    }
    return uvs;
}

// ============================================================
// Tests
// ============================================================

bool writeTriangle(const std::string& outPath)
{
    tinygltf::Model model;
    tinygltf::Scene scene;
    scene.name = "Scene";

    const float positions[9] = {
        0.0f, 0.5f, 0.0f,
        -0.5f,-0.5f, 0.0f,
        0.5f,-0.5f, 0.0f
    };

    const float normals[9] = {
        0,0,1, 0,0,1, 0,0,1
    };

    const float uvs[6] = {
        0.5f,1.0f,
        0.0f,0.0f,
        1.0f,0.0f
    };

    const uint32_t indices[3] = {0,1,2};

    tinygltf::Buffer buffer;
    buffer.data.reserve(512);

    const size_t posOff = buffer.data.size(); appendBytes(buffer.data, positions, sizeof(positions));
    const size_t nrmOff = buffer.data.size(); appendBytes(buffer.data, normals,   sizeof(normals));
    const size_t uvOff  = buffer.data.size(); appendBytes(buffer.data, uvs,       sizeof(uvs));
    align4(buffer.data);
    const size_t idxOff = buffer.data.size(); appendBytes(buffer.data, indices,   sizeof(indices));

    model.buffers.push_back(std::move(buffer));

    const int posView = addView(model, posOff, sizeof(positions), TINYGLTF_TARGET_ARRAY_BUFFER);
    const int nrmView = addView(model, nrmOff, sizeof(normals),   TINYGLTF_TARGET_ARRAY_BUFFER);
    const int uvView  = addView(model, uvOff,  sizeof(uvs),       TINYGLTF_TARGET_ARRAY_BUFFER);
    const int idxView = addView(model, idxOff, sizeof(indices),   TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

    const int posAcc = addAcc(model, posView, TINYGLTF_COMPONENT_TYPE_FLOAT,        TINYGLTF_TYPE_VEC3,   3);
    model.accessors[posAcc].minValues = {-0.5, -0.5, 0.0};
    model.accessors[posAcc].maxValues = { 0.5,  0.5, 0.0};

    const int nrmAcc = addAcc(model, nrmView, TINYGLTF_COMPONENT_TYPE_FLOAT,        TINYGLTF_TYPE_VEC3,   3);
    const int uvAcc  = addAcc(model, uvView,  TINYGLTF_COMPONENT_TYPE_FLOAT,        TINYGLTF_TYPE_VEC2,   3);
    const int idxAcc = addAcc(model, idxView, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR, 3);

    tinygltf::Material mat;
    mat.name = "DefaultMat";
    mat.pbrMetallicRoughness.baseColorFactor = {1,1,1,1};
    mat.pbrMetallicRoughness.metallicFactor  = 0.0;
    mat.pbrMetallicRoughness.roughnessFactor = 1.0;
    model.materials.push_back(std::move(mat));

    tinygltf::Primitive prim;
    prim.mode = TINYGLTF_MODE_TRIANGLES;
    prim.indices = idxAcc;
    prim.material = 0;
    prim.attributes["POSITION"]   = posAcc;
    prim.attributes["NORMAL"]     = nrmAcc;
    prim.attributes["TEXCOORD_0"] = uvAcc;

    tinygltf::Mesh mesh;
    mesh.name = "TriangleMesh";
    mesh.primitives.push_back(std::move(prim));
    model.meshes.push_back(std::move(mesh));

    tinygltf::Node node;
    node.name = "TriangleNode";
    node.mesh = 0;
    model.nodes.push_back(std::move(node));

    scene.nodes.push_back(0);
    model.scenes.push_back(std::move(scene));
    model.defaultScene = 0;

    tinygltf::TinyGLTF gltf;
    std::string err, warn;
    const bool ok = gltf.WriteGltfSceneToFile(&model, outPath, true, true, false, true);
    if (!warn.empty()) std::cout << "[tinygltf warn] " << warn << "\n";
    if (!err.empty())  std::cout << "[tinygltf err ] " << err << "\n";
    return ok;
}

bool writeCubeSinglePrimitive(const std::string& outPath)
{
    tinygltf::Model model;
    tinygltf::Scene scene;
    scene.name = "Scene";

    // Optional: embed atlas so you can see UVs
    addEmbeddedAtlasTexture(model);

    // One material using atlas texture
    tinygltf::Material mat;
    mat.name = "AtlasMat";
    mat.pbrMetallicRoughness.baseColorTexture.index = 0; // model.textures[0]
    mat.pbrMetallicRoughness.metallicFactor  = 0.0;
    mat.pbrMetallicRoughness.roughnessFactor = 1.0;
    model.materials.push_back(std::move(mat));

    // UVs: use left half as default
    std::vector<float> uvs = buildCubeUvsAtlasHalf(true);

    tinygltf::Buffer buffer;
    buffer.data.reserve(4096);

    const size_t posOff = buffer.data.size(); appendBytes(buffer.data, CUBE_POSITIONS, sizeof(CUBE_POSITIONS));
    const size_t nrmOff = buffer.data.size(); appendBytes(buffer.data, CUBE_NORMALS,   sizeof(CUBE_NORMALS));
    const size_t uvOff  = buffer.data.size(); appendBytes(buffer.data, uvs.data(),     uvs.size() * sizeof(float));
    align4(buffer.data);
    const size_t idxOff = buffer.data.size(); appendBytes(buffer.data, CUBE_INDICES,   sizeof(CUBE_INDICES));

    model.buffers.push_back(std::move(buffer));

    const int posView = addView(model, posOff, sizeof(CUBE_POSITIONS),             TINYGLTF_TARGET_ARRAY_BUFFER);
    const int nrmView = addView(model, nrmOff, sizeof(CUBE_NORMALS),               TINYGLTF_TARGET_ARRAY_BUFFER);
    const int uvView  = addView(model, uvOff,  uvs.size() * sizeof(float),         TINYGLTF_TARGET_ARRAY_BUFFER);
    const int idxView = addView(model, idxOff, sizeof(CUBE_INDICES),               TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

    const int posAcc = addAcc(model, posView, TINYGLTF_COMPONENT_TYPE_FLOAT,        TINYGLTF_TYPE_VEC3,   24);
    model.accessors[posAcc].minValues = {-0.5, -0.5, -0.5};
    model.accessors[posAcc].maxValues = { 0.5,  0.5,  0.5};

    const int nrmAcc = addAcc(model, nrmView, TINYGLTF_COMPONENT_TYPE_FLOAT,        TINYGLTF_TYPE_VEC3,   24);
    const int uvAcc  = addAcc(model, uvView,  TINYGLTF_COMPONENT_TYPE_FLOAT,        TINYGLTF_TYPE_VEC2,   24);
    const int idxAcc = addAcc(model, idxView, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR, 36);

    tinygltf::Primitive prim;
    prim.mode = TINYGLTF_MODE_TRIANGLES;
    prim.indices = idxAcc;
    prim.material = 0;
    prim.attributes["POSITION"]   = posAcc;
    prim.attributes["NORMAL"]     = nrmAcc;
    prim.attributes["TEXCOORD_0"] = uvAcc;

    tinygltf::Mesh mesh;
    mesh.name = "CubeMesh";
    mesh.primitives.push_back(std::move(prim));
    model.meshes.push_back(std::move(mesh));

    tinygltf::Node node;
    node.name = "CubeNode";
    node.mesh = 0;
    model.nodes.push_back(std::move(node));

    scene.nodes.push_back(0);
    model.scenes.push_back(std::move(scene));
    model.defaultScene = 0;

    tinygltf::TinyGLTF gltf;
    std::string err, warn;
    const bool ok = gltf.WriteGltfSceneToFile(&model, outPath, true, true, false, true);
    if (!warn.empty()) std::cout << "[tinygltf warn] " << warn << "\n";
    if (!err.empty())  std::cout << "[tinygltf err ] " << err << "\n";
    return ok;
}

bool writeCubeTwoPrimitivesTwoMaterials(const std::string& outPath)
{
    tinygltf::Model model;
    tinygltf::Scene scene;
    scene.name = "Scene";

    // 1 atlas texture shared by both materials
    addEmbeddedAtlasTexture(model);

    // Material A uses same texture (atlas)
    tinygltf::Material matA;
    matA.name = "Atlas_Left";
    matA.pbrMetallicRoughness.baseColorTexture.index = 0;
    matA.pbrMetallicRoughness.metallicFactor  = 0.0;
    matA.pbrMetallicRoughness.roughnessFactor = 1.0;
    model.materials.push_back(std::move(matA));

    // Material B uses same texture (atlas)
    tinygltf::Material matB;
    matB.name = "Atlas_Right";
    matB.pbrMetallicRoughness.baseColorTexture.index = 0;
    matB.pbrMetallicRoughness.metallicFactor  = 0.0;
    matB.pbrMetallicRoughness.roughnessFactor = 1.0;
    model.materials.push_back(std::move(matB));

    // UVs differ per primitive (Option A)
    std::vector<float> uvsA = buildCubeUvsAtlasHalf(true);   // left half (red)
    std::vector<float> uvsB = buildCubeUvsAtlasHalf(false);  // right half (blue)

    tinygltf::Buffer buffer;
    buffer.data.reserve(8192);

    const size_t posOff  = buffer.data.size(); appendBytes(buffer.data, CUBE_POSITIONS, sizeof(CUBE_POSITIONS));
    const size_t nrmOff  = buffer.data.size(); appendBytes(buffer.data, CUBE_NORMALS,   sizeof(CUBE_NORMALS));

    const size_t uvAOff  = buffer.data.size(); appendBytes(buffer.data, uvsA.data(), uvsA.size() * sizeof(float));
    const size_t uvBOff  = buffer.data.size(); appendBytes(buffer.data, uvsB.data(), uvsB.size() * sizeof(float));

    align4(buffer.data);
    const size_t idxAOff = buffer.data.size(); appendBytes(buffer.data, CUBE_INDICES_A, sizeof(CUBE_INDICES_A));
    align4(buffer.data);
    const size_t idxBOff = buffer.data.size(); appendBytes(buffer.data, CUBE_INDICES_B, sizeof(CUBE_INDICES_B));

    model.buffers.push_back(std::move(buffer));

    const int posView  = addView(model, posOff,  sizeof(CUBE_POSITIONS),               TINYGLTF_TARGET_ARRAY_BUFFER);
    const int nrmView  = addView(model, nrmOff,  sizeof(CUBE_NORMALS),                 TINYGLTF_TARGET_ARRAY_BUFFER);
    const int uvAView  = addView(model, uvAOff,  uvsA.size() * sizeof(float),          TINYGLTF_TARGET_ARRAY_BUFFER);
    const int uvBView  = addView(model, uvBOff,  uvsB.size() * sizeof(float),          TINYGLTF_TARGET_ARRAY_BUFFER);
    const int idxAView = addView(model, idxAOff, sizeof(CUBE_INDICES_A),               TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);
    const int idxBView = addView(model, idxBOff, sizeof(CUBE_INDICES_B),               TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER);

    const int posAcc = addAcc(model, posView, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, 24);
    model.accessors[posAcc].minValues = {-0.5, -0.5, -0.5};
    model.accessors[posAcc].maxValues = { 0.5,  0.5,  0.5};

    const int nrmAcc  = addAcc(model, nrmView, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC3, 24);
    const int uvAAcc  = addAcc(model, uvAView, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2, 24);
    const int uvBAcc  = addAcc(model, uvBView, TINYGLTF_COMPONENT_TYPE_FLOAT, TINYGLTF_TYPE_VEC2, 24);

    // 18 indices per primitive
    const int idxAAcc = addAcc(model, idxAView, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR, 18);
    const int idxBAcc = addAcc(model, idxBView, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT, TINYGLTF_TYPE_SCALAR, 18);

    // Primitive A
    tinygltf::Primitive primA;
    primA.mode = TINYGLTF_MODE_TRIANGLES;
    primA.indices = idxAAcc;
    primA.material = 0; // Atlas_Left
    primA.attributes["POSITION"]   = posAcc;
    primA.attributes["NORMAL"]     = nrmAcc;
    primA.attributes["TEXCOORD_0"] = uvAAcc;

    // Primitive B
    tinygltf::Primitive primB;
    primB.mode = TINYGLTF_MODE_TRIANGLES;
    primB.indices = idxBAcc;
    primB.material = 1; // Atlas_Right
    primB.attributes["POSITION"]   = posAcc;
    primB.attributes["NORMAL"]     = nrmAcc;
    primB.attributes["TEXCOORD_0"] = uvBAcc;

    tinygltf::Mesh mesh;
    mesh.name = "CubeMesh";
    mesh.primitives.push_back(std::move(primA));
    mesh.primitives.push_back(std::move(primB));
    model.meshes.push_back(std::move(mesh));

    tinygltf::Node node;
    node.name = "CubeNode";
    node.mesh = 0;
    model.nodes.push_back(std::move(node));

    scene.nodes.push_back(0);
    model.scenes.push_back(std::move(scene));
    model.defaultScene = 0;

    tinygltf::TinyGLTF gltf;
    std::string err, warn;
    const bool ok = gltf.WriteGltfSceneToFile(&model, outPath, true, true, false, true);
    if (!warn.empty()) std::cout << "[tinygltf warn] " << warn << "\n";
    if (!err.empty())  std::cout << "[tinygltf err ] " << err << "\n";
    return ok;
}

} // namespace glbtest
