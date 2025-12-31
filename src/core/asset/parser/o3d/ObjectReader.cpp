#include "ObjectReader.h"
#include "Log.h"

#include <sstream>
#include <iomanip>

#include "asset/decoder/BinaryReader.h"

#include "ObjectReader.h"
#include "Log.h"

#include <sstream>
#include <iomanip>

#include "asset/decoder/BinaryReader.h"

namespace asset::parser::o3d
{

static bool align4(asset::decode::BinaryReader& br)
{
    const std::size_t pos = br.tell();
    const std::size_t aligned = (pos + 3u) & ~std::size_t(3u);
    return br.seek(aligned);
}
bool ObjectReader::read(ObjectReadResult& out,
                        const std::vector<std::uint8_t>& bytes,
                        const O3DHeader& header,
                        std::string* outError)
{
    out = {};

    asset::decode::BinaryReader br(
        reinterpret_cast<const std::vector<unsigned char>&>(bytes));

    if (!br.seek(header.cursorAfterHeader))
    {
        if (outError)
            *outError = std::string("ObjectReader: cannot seek to header cursor ")
                        + std::to_string(header.cursorAfterHeader);
        return false;
    }

    auto groupCountOpt = br.readLE<std::int32_t>();
    if (!groupCountOpt)
    {
        if (outError) *outError = "ObjectReader: cannot read groupCount.";
        return false;
    }

    out.groupCount = *groupCountOpt;
    if (out.groupCount < 0 || out.groupCount > 100000)
    {
        if (outError)
            *outError = std::string("ObjectReader: invalid groupCount=") + std::to_string(out.groupCount);
        return false;
    }

    out.groups.resize((std::size_t)out.groupCount);

    auto align4 = [&]() -> bool {
        const std::size_t pos = br.tell();
        const std::size_t aligned = (pos + 3u) & ~std::size_t(3u);
        return br.seek(aligned);
    };

    const bool force34 = (header.hasForce34 != 0);
    const int  floatCount = force34 ? 12 : 16;

    for (int g = 0; g < out.groupCount; ++g)
    {
        auto objectCountOpt = br.readLE<std::int32_t>();
        if (!objectCountOpt)
        {
            if (outError)
                *outError = std::string("ObjectReader: cannot read objectCount (group ")
                            + std::to_string(g) + ")";
            return false;
        }

        auto& grp = out.groups[(std::size_t)g];
        grp.objectCount = *objectCountOpt;

        if (grp.objectCount < 0 || grp.objectCount > 100000)
        {
            if (outError)
                *outError = std::string("ObjectReader: invalid objectCount=")
                            + std::to_string(grp.objectCount) + " group=" + std::to_string(g);
            return false;
        }

        grp.objects.resize((std::size_t)grp.objectCount);

        for (int o = 0; o < grp.objectCount; ++o)
        {
            auto& obj = grp.objects[(std::size_t)o];

            // wichtig: align VOR dem Start-Cursor fürs Objekt setzen
            if (!align4())
            {
                if (outError)
                    *outError = std::string("ObjectReader: align4 failed group=")
                                + std::to_string(g) + " obj=" + std::to_string(o);
                return false;
            }

            obj.cursorAtObjectStart = br.tell();

            // unk0 / flags
            auto unk0Opt = br.readLE<std::int32_t>();
            if (!unk0Opt)
            {
                if (outError)
                    *outError = std::string("ObjectReader: cannot read unk0 group=")
                                + std::to_string(g) + " obj=" + std::to_string(o);
                return false;
            }
            obj.unk0 = *unk0Opt;

            // ---- MATRIX ----
            for (int i = 0; i < floatCount; ++i)
            {
                auto f = br.readLE<float>();
                if (!f)
                {
                    if (outError)
                        *outError = std::string("ObjectReader: cannot read matrix[")
                                    + std::to_string(i) + "] group=" + std::to_string(g)
                                    + " obj=" + std::to_string(o);
                    return false;
                }
                obj.matrix[i] = *f;
            }

            if (force34)
            {
                obj.matrix[12] = 0.0f;
                obj.matrix[13] = 0.0f;
                obj.matrix[14] = 0.0f;
                obj.matrix[15] = 1.0f;
            }

            obj.cursorAfterMatrix = br.tell();

            // ---- 36 BYTES ParamBlock (2x i32, 6x f32, 1x u32) ----
            auto unkAOpt = br.readLE<std::int32_t>();
            auto unkBOpt = br.readLE<std::int32_t>();
            if (!unkAOpt || !unkBOpt)
            {
                if (outError)
                    *outError = std::string("ObjectReader: cannot read unkA/unkB group=")
                                + std::to_string(g) + " obj=" + std::to_string(o);
                return false;
            }
            obj.unkA = *unkAOpt;
            obj.unkB = *unkBOpt;

            for (int i = 0; i < 6; ++i)
            {
                auto fOpt = br.readLE<float>();
                if (!fOpt)
                {
                    if (outError)
                        *outError = std::string("ObjectReader: cannot read params[")
                                    + std::to_string(i) + "] group=" + std::to_string(g)
                                    + " obj=" + std::to_string(o);
                    return false;
                }
                obj.params[i] = *fOpt;
            }

            auto poolOpt = br.readLE<std::uint32_t>();
            if (!poolOpt)
            {
                if (outError)
                    *outError = std::string("ObjectReader: cannot read poolIndex group=")
                                + std::to_string(g) + " obj=" + std::to_string(o);
                return false;
            }
            obj.poolIndex = *poolOpt;

            obj.cursorAfterParams = br.tell();

            // -----------------------------------------------------
            // hasMeshCandidate: NUR aus Flags/PoolIndex (ohne meshPoolStart)
            // -----------------------------------------------------
            int score = 0;

            if (obj.poolIndex != 0 && obj.poolIndex != 0xFFFFFFFFu)
                score++;

            if ((obj.unk0 & 0x00010000) != 0) // render-ish flag
                score++;

            if ((obj.unk0 & 0x00020000) != 0) // oft auch relevant
                score++;

            obj.hasMeshCandidate = (score >= 1);
            obj.hasMesh = obj.hasMeshCandidate; // finale Entscheidung macht ModelParser (mit meshPoolStart)
        }
    }

    out.cursorAfterObjects = br.tell();
    return true;
}
} // namespace asset::parser::o3d
