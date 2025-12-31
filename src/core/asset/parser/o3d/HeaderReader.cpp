#include "HeaderReader.h"

#include "asset/decoder/BinaryReader.h"
#include "Log.h"

#include <cctype>
#include <cstring>

namespace asset::parser::o3d
{
static std::string toLowerCopy(std::string s)
{
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

static bool readVec3(asset::decode::BinaryReader& br, Vec3& out)
{
    auto x = br.readLE<float>();
    auto y = br.readLE<float>();
    auto z = br.readLE<float>();
    if (!x || !y || !z) return false;
    out.x = *x; out.y = *y; out.z = *z;
    return true;
}

static bool skip(asset::decode::BinaryReader& br, std::size_t n)
{
    const std::size_t pos = br.tell();
    return br.seek(pos + n);
}

bool HeaderReader::read(O3DHeader& out,
                        const std::vector<std::uint8_t>& bytes,
                        const std::optional<std::string>& expectedFileNameLower,
                        std::string* outError)
{
    out = O3DHeader{};

    if (bytes.size() < 8)
    {
        if (outError) *outError = "HeaderReader: buffer too small.";
        core::Log::pipelineError("HeaderReader: buffer too small.");
        return false;
    }

    asset::decode::BinaryReader br(reinterpret_cast<const std::vector<unsigned char>&>(bytes));

    // --- GameSource ---
    // resFp.Read(&cLen, 1, 1);
    auto nameLenOpt = br.readLE<std::uint8_t>();
    if (!nameLenOpt)
    {
        if (outError) *outError = "HeaderReader: cannot read nameLen.";
        core::Log::pipelineError("HeaderReader: cannot read nameLen.");
        return false;
    }
    const std::uint8_t nameLen = *nameLenOpt;

    if (nameLen == 0 || nameLen >= 64)
    {
        if (outError) *outError = "HeaderReader: embedded filename length invalid (0 or >=64).";
        core::Log::pipelineError("HeaderReader: embedded filename length invalid: " + std::to_string((int)nameLen));
        return false;
    }

    // resFp.Read(buff, cLen, 1);
    auto nameBytes = br.readBytes(nameLen);
    if (nameBytes.size() != nameLen)
    {
        if (outError) *outError = "HeaderReader: cannot read embedded filename bytes.";
        core::Log::pipelineError("HeaderReader: cannot read embedded filename bytes.");
        return false;
    }

    // for j: buff[j] ^= 0xCD;
    std::string embedded;
    embedded.resize(nameLen);
    for (std::size_t i = 0; i < nameBytes.size(); ++i)
    {
        embedded[i] = static_cast<char>(nameBytes[i] ^ 0xCD);
    }

    // GameSource lowercases internal filename; wir normalisieren ebenfalls
    out.embeddedFileNameLower = toLowerCopy(embedded);

    // optional compare like GameSource: strcmpi(basename(expected), embedded)
    if (expectedFileNameLower.has_value())
    {
        const std::string expLower = toLowerCopy(*expectedFileNameLower);
        if (out.embeddedFileNameLower != expLower)
        {
            if (outError)
                *outError = "HeaderReader: embedded filename mismatch. expected='" + expLower +
                            "' got='" + out.embeddedFileNameLower + "'";
            core::Log::pipelineError("HeaderReader: embedded filename mismatch expected='" + expLower +
                                     "' got='" + out.embeddedFileNameLower + "'");
            return false;
        }
    }

    // --- Common header ---
    // resFp.Read(&nVer, 4, 1);
    auto ver = br.readLE<std::int32_t>();
    if (!ver)
    {
        if (outError) *outError = "HeaderReader: cannot read version.";
        core::Log::pipelineError("HeaderReader: cannot read version.");
        return false;
    }
    out.version = *ver;

    // if (nVer < VER_MESH) FAIL;  (VER_MESH = 20 im GameSource)
    if (out.version < 20)
    {
        if (outError) *outError = "HeaderReader: version < VER_MESH (20). got=" + std::to_string(out.version);
        core::Log::pipelineError("HeaderReader: version < 20 got=" + std::to_string(out.version));
        return false;
    }

    // resFp.Read(&m_nID, 4, 1);
    auto id = br.readLE<std::int32_t>();
    if (!id)
    {
        if (outError) *outError = "HeaderReader: cannot read serialId.";
        core::Log::pipelineError("HeaderReader: cannot read serialId.");
        return false;
    }
    out.serialId = *id;

    // resFp.Read(&m_vForce1, sizeof(D3DXVECTOR3), 1);
    // resFp.Read(&m_vForce2, sizeof(D3DXVECTOR3), 1);
    if (!readVec3(br, out.force1) || !readVec3(br, out.force2))
    {
        if (outError) *outError = "HeaderReader: cannot read force1/force2.";
        core::Log::pipelineError("HeaderReader: cannot read force1/force2.");
        return false;
    }

    // if (nVer >= 22) force3/force4
    if (out.version >= 22)
    {
        out.hasForce34 = true;
        if (!readVec3(br, out.force3) || !readVec3(br, out.force4))
        {
            if (outError) *outError = "HeaderReader: cannot read force3/force4 (ver>=22).";
            core::Log::pipelineError("HeaderReader: cannot read force3/force4 (ver>=22).");
            return false;
        }
    }

    // scrollU/V
    auto su = br.readLE<float>();
    auto sv = br.readLE<float>();
    if (!su || !sv)
    {
        if (outError) *outError = "HeaderReader: cannot read scrollU/scrollV.";
        core::Log::pipelineError("HeaderReader: cannot read scrollU/scrollV.");
        return false;
    }
    out.scrollU = *su;
    out.scrollV = *sv;

    // resFp.Seek(16, SEEK_CUR); // reserved
    if (!skip(br, 16))
    {
        if (outError) *outError = "HeaderReader: cannot skip reserved(16).";
        core::Log::pipelineError("HeaderReader: cannot skip reserved(16).");
        return false;
    }

    // bbMin/bbMax
    if (!readVec3(br, out.bbMin) || !readVec3(br, out.bbMax))
    {
        if (outError) *outError = "HeaderReader: cannot read BBMin/BBMax.";
        core::Log::pipelineError("HeaderReader: cannot read BBMin/BBMax.");
        return false;
    }

    // perSlerp
    auto ps = br.readLE<float>();
    if (!ps)
    {
        if (outError) *outError = "HeaderReader: cannot read perSlerp.";
        core::Log::pipelineError("HeaderReader: cannot read perSlerp.");
        return false;
    }
    out.perSlerp = *ps;

    // maxFrame
    auto mf = br.readLE<std::int32_t>();
    if (!mf)
    {
        if (outError) *outError = "HeaderReader: cannot read maxFrame.";
        core::Log::pipelineError("HeaderReader: cannot read maxFrame.");
        return false;
    }
    out.maxFrame = *mf;

    // maxEvent + event vectors
    auto me = br.readLE<std::int32_t>();
    if (!me)
    {
        if (outError) *outError = "HeaderReader: cannot read maxEvent.";
        core::Log::pipelineError("HeaderReader: cannot read maxEvent.");
        return false;
    }
    out.maxEvent = *me;

    if (out.maxEvent < 0 || out.maxEvent > 1'000'000)
    {
        if (outError) *outError = "HeaderReader: maxEvent out of range: " + std::to_string(out.maxEvent);
        core::Log::pipelineError("HeaderReader: maxEvent out of range: " + std::to_string(out.maxEvent));
        return false;
    }

    if (out.maxEvent > 0)
    {
        out.events.resize(static_cast<std::size_t>(out.maxEvent));
        for (int i = 0; i < out.maxEvent; ++i)
        {
            if (!readVec3(br, out.events[static_cast<std::size_t>(i)]))
            {
                if (outError) *outError = "HeaderReader: cannot read event vec3 list.";
                core::Log::pipelineError("HeaderReader: cannot read event vec3 list.");
                return false;
            }
        }
    }

    // nTemp collision mesh flag
    auto nTemp = br.readLE<std::int32_t>();
    if (!nTemp)
    {
        if (outError) *outError = "HeaderReader: cannot read collision flag (nTemp).";
        core::Log::pipelineError("HeaderReader: cannot read collision flag (nTemp).");
        return false;
    }
    out.hasCollMesh = (*nTemp != 0);

    // LOD flag (int)
    auto lod = br.readLE<std::int32_t>();
    if (!lod)
    {
        if (outError) *outError = "HeaderReader: cannot read LOD flag.";
        core::Log::pipelineError("HeaderReader: cannot read LOD flag.");
        return false;
    }
    out.lodFlag = *lod;

    // maxBone
    auto mb = br.readLE<std::int32_t>();
    if (!mb)
    {
        if (outError) *outError = "HeaderReader: cannot read maxBone.";
        core::Log::pipelineError("HeaderReader: cannot read maxBone.");
        return false;
    }
    out.maxBone = *mb;
    out.hasBones = (out.maxBone > 0);

    // ============================
    // 🔥 PIPELINE DUMP (header summary)
    // ============================
    core::Log::pipelineInfo(
        "[HEADER] name=" + out.embeddedFileNameLower +
        " ver=" + std::to_string(out.version) +
        " force34=" + std::string(out.hasForce34 ? "1" : "0") +
        " poolSize=" + std::to_string(out.poolSize) +
        " cursor=" + std::to_string(out.cursorAfterHeader)
        );

#ifdef O3D_DEBUG_VERBOSE_HEADER
    core::Log::pipelineInfo(" serialId=" + std::to_string(out.serialId));
    core::Log::pipelineInfo(" scrollU=" + std::to_string(out.scrollU) + " scrollV=" + std::to_string(out.scrollV));
    core::Log::pipelineInfo(" bbMin=(" + ... + ")");
    core::Log::pipelineInfo(" bbMax=(" + ... + ")");
    core::Log::pipelineInfo(" perSlerp=" + std::to_string(out.perSlerp));
    core::Log::pipelineInfo(" maxFrame=" + std::to_string(out.maxFrame));
    core::Log::pipelineInfo(" maxEvent=" + std::to_string(out.maxEvent));
    core::Log::pipelineInfo(" hasCollMesh=" + std::string(out.hasCollMesh ? "1" : "0"));
    core::Log::pipelineInfo(" lodFlag=" + std::to_string(out.lodFlag));
    core::Log::pipelineInfo(" maxBone=" + std::to_string(out.maxBone));
    core::Log::pipelineInfo(" hasBones=" + std::string(out.hasBones ? "1" : "0"));
#endif

    if (out.version >= 22)
        core::Log::pipelineInfo(" hasForce34=1");
    else
        core::Log::pipelineInfo(" hasForce34=0");

    if (out.hasBones)
    {
        // Wir SKIPPEN die Matrizen+Motion hier NICHT blind.
        out.hasMotion = (out.maxFrame > 0);
        out.cursorAfterHeader = br.tell();

        core::Log::pipelineInfo(" hasMotion=" + std::string(out.hasMotion ? "1" : "0"));
        core::Log::pipelineInfo(" cursorAfterHeader=" + std::to_string(out.cursorAfterHeader));
        return true;
    }

    // Wenn keine Bones: direkt folgt nPoolSize
    auto pool = br.readLE<std::int32_t>();
    if (!pool)
    {
        if (outError) *outError = "HeaderReader: cannot read poolSize.";
        core::Log::pipelineError("HeaderReader: cannot read poolSize.");
        return false;
    }
    out.poolSize = *pool;

    out.cursorAfterHeader = br.tell();

    core::Log::pipelineInfo(" poolSize=" + std::to_string(out.poolSize));
    core::Log::pipelineInfo(" cursorAfterHeader=" + std::to_string(out.cursorAfterHeader));

    return true;
}
}
