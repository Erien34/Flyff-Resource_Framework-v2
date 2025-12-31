#include "O3DDecoder.h"
#include "data/asset/decoded/O3DDecoded.h"

#include "data/asset/source/ModelSource.h"

#include <sstream>
#include <iomanip>

namespace asset
{
    static std::string hexPrefix(const std::vector<std::uint8_t>& bytes, std::size_t n)
    {
        const std::size_t count = (bytes.size() < n) ? bytes.size() : n;

        std::ostringstream oss;
        oss << std::hex << std::setfill('0');

        for (std::size_t i = 0; i < count; ++i)
            oss << std::setw(2) << static_cast<int>(bytes[i]);

        return oss.str();
    }

    static void decodePart(O3DDecodedPart& out, const ModelPartSource& part)
    {
        out = O3DDecodedPart{};
        if (!part.exists)
            return;

        // Memory-only: wir ignorieren part.path vollständig
        out.exists = true;
        out.extension = part.extension; // kommt aus Loader (lower-case)
        out.sizeBytes = static_cast<std::uint32_t>(part.bytes.bytes.size());
        out.signatureHex = hexPrefix(part.bytes.bytes, 32);

        // Raw unverändert durchreichen
        out.raw = part.bytes.bytes;
    }

    bool O3DDecoder::decode(O3DDecoded& out, const ModelSource& src, std::string* outError)
    {
        out = O3DDecoded{};

        decodePart(out.skeleton, src.skeleton);
        decodePart(out.mesh, src.mesh);

        if (!out.skeleton.exists && !out.mesh.exists)
        {
            if (outError) *outError = "O3DDecoder: no model part bytes present (neither skeleton nor mesh).";
            return false;
        }

        return true;
    }
}
