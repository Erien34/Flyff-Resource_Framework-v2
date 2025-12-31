#include "AniDecoder.h"
#include "data/asset/decoded/DecodedAniData.h"
#include "asset/source/BinaryData.h"

#include <cctype>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>

namespace asset
{
    static std::string toLowerLocal(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static std::string normalizePathLike(std::string s)
    {
        for (char& c : s)
            if (c == '\\') c = '/';
        return s;
    }

    static bool isPrintableAscii(std::uint8_t c)
    {
        return (c >= 32 && c <= 126);
    }

    static bool endsWith(const std::string& s, const char* suffix)
    {
        const std::size_t sl = s.size();
        const std::size_t tl = std::strlen(suffix);
        if (sl < tl) return false;
        return s.compare(sl - tl, tl, suffix) == 0;
    }

    static void pushUnique(std::vector<std::string>& out, std::unordered_set<std::string>& seen, const std::string& v)
    {
        if (v.empty()) return;
        if (seen.insert(v).second)
            out.push_back(v);
    }

    static bool looksLikeRef(const std::string& s)
    {
        if (s.size() < 4) return false;
        if (s.find('.') == std::string::npos) return false;
        return true;
    }

    // Strategy 1: ASCII runs
    static void extractAsciiRuns(const std::vector<std::uint8_t>& buf, std::vector<std::string>& out)
    {
        std::string cur;
        cur.reserve(256);

        auto flush = [&]()
        {
            if (cur.size() >= 4)
                out.push_back(cur);
            cur.clear();
        };

        for (std::uint8_t c : buf)
        {
            if (c == 0)
            {
                flush();
                continue;
            }

            if (isPrintableAscii(c))
            {
                cur.push_back((char)c);
                if (cur.size() > 512)
                    flush();
            }
            else
            {
                flush();
            }
        }
        flush();
    }

    static void classify(
        const std::string& s,
        DecodedAniData& out,
        std::unordered_set<std::string>& seenSkel,
        std::unordered_set<std::string>& seenModel,
        std::unordered_set<std::string>& seenOther)
    {
        if (endsWith(s, ".o3d") || endsWith(s, ".ase"))
        {
            pushUnique(out.refModels, seenModel, s);
            return;
        }

        // typische Skeleton-/Bone-Namen (sehr vorsichtig!)
        if (s.find("bone") != std::string::npos ||
            s.find("skel") != std::string::npos ||
            s.find("skeleton") != std::string::npos)
        {
            pushUnique(out.refSkeletons, seenSkel, s);
            return;
        }

        pushUnique(out.refOther, seenOther, s);
    }

    bool AniDecoder::decode(DecodedAniData& out, const BinaryData& bytes, std::string* outError)
    {
        out = DecodedAniData{};
        out.raw = bytes.bytes;

        if (bytes.bytes.empty())
        {
            if (outError) *outError = "ANI: empty byte buffer.";
            return false;
        }

        // --------------------------------------------------
        // 1) Container detection (best effort)
        // --------------------------------------------------
        out.containerKind = "Unknown";
        out.version = 0;

        if (bytes.bytes.size() >= 3)
        {
            if (bytes.bytes[0] == 'A' && bytes.bytes[1] == 'N' && bytes.bytes[2] == 'I')
                out.containerKind = "ANI";
        }

        // --------------------------------------------------
        // 2) String extraction
        // --------------------------------------------------
        std::vector<std::string> found;
        extractAsciiRuns(bytes.bytes, found);

        std::unordered_set<std::string> seenAll;
        std::unordered_set<std::string> seenSkel, seenModel, seenOther;

        for (auto& s : found)
        {
            s = normalizePathLike(s);
            s = toLowerLocal(s);

            if (s.size() < 4)
                continue;

            if (seenAll.insert(s).second)
                out.allStrings.push_back(s);

            if (looksLikeRef(s))
                classify(s, out, seenSkel, seenModel, seenOther);
        }

        // Decoder ist erfolgreich, auch wenn keine Referenzen erkannt wurden
        return true;
    }
}
