#include "data/asset/decoded/DecodedSfxData.h"
#include "asset/source/BinaryData.h"

#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>
#include <algorithm>

namespace resource
{
    static std::string toLowerLocal(std::string s)
    {
        for (char& c : s)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return s;
    }

    static bool isPrintableAscii(std::uint8_t c)
    {
        // erlaubtes Set für Pfade/Dateinamen (relativ tolerant)
        return (c >= 32 && c <= 126);
    }

    static std::string normalizePathLike(std::string s)
    {
        for (char& c : s)
        {
            if (c == '\\') c = '/';
        }
        return s;
    }

    static bool endsWith(const std::string& s, const char* suffix)
    {
        const std::size_t sl = s.size();
        const std::size_t tl = std::strlen(suffix);
        if (sl < tl) return false;
        return s.compare(sl - tl, tl, suffix) == 0;
    }

    static bool looksLikeAssetRef(const std::string& s)
    {
        // Minimal: muss einen Punkt haben und nicht zu kurz sein
        if (s.size() < 5) return false;
        if (s.find('.') == std::string::npos) return false;

        // Häufige Flyff-Referenzen
        static const char* exts[] = {
            ".dds",".tga",".png",".jpg",".jpeg",".bmp",
            ".wav",".ogg",".mp3",".bgm",
            ".o3d",".ase",".ani",
            ".sfx"
        };

        for (auto* e : exts)
            if (endsWith(s, e)) return true;

        return false;
    }

    static void pushUnique(std::vector<std::string>& out, std::unordered_set<std::string>& seen, const std::string& v)
    {
        if (v.empty()) return;
        if (seen.insert(v).second)
            out.push_back(v);
    }

    static void classifyAndStore(
        const std::string& vLower,
        DecodedSfxData& out,
        std::unordered_set<std::string>& seenImages,
        std::unordered_set<std::string>& seenSounds,
        std::unordered_set<std::string>& seenModels,
        std::unordered_set<std::string>& seenSfx,
        std::unordered_set<std::string>& seenOther)
    {
        if (endsWith(vLower, ".dds") || endsWith(vLower, ".tga") || endsWith(vLower, ".png") ||
            endsWith(vLower, ".jpg") || endsWith(vLower, ".jpeg") || endsWith(vLower, ".bmp"))
        {
            pushUnique(out.refImages, seenImages, vLower);
            return;
        }
        if (endsWith(vLower, ".wav") || endsWith(vLower, ".ogg") || endsWith(vLower, ".mp3") || endsWith(vLower, ".bgm"))
        {
            pushUnique(out.refSounds, seenSounds, vLower);
            return;
        }
        if (endsWith(vLower, ".o3d") || endsWith(vLower, ".ase") || endsWith(vLower, ".ani"))
        {
            pushUnique(out.refModels, seenModels, vLower);
            return;
        }
        if (endsWith(vLower, ".sfx"))
        {
            pushUnique(out.refSfx, seenSfx, vLower);
            return;
        }

        // Fallback: irgendwas mit dot
        pushUnique(out.refOther, seenOther, vLower);
    }

    static std::uint32_t readU32LE(const std::uint8_t* p)
    {
        return (std::uint32_t)p[0] |
               ((std::uint32_t)p[1] << 8) |
               ((std::uint32_t)p[2] << 16) |
               ((std::uint32_t)p[3] << 24);
    }

    // --- Strategy 1: Null-terminated / raw ASCII sequences scan ---
    static void extractAsciiRuns(const std::vector<std::uint8_t>& buf, std::vector<std::string>& outStrings)
    {
        std::string cur;
        cur.reserve(256);

        auto flush = [&]()
        {
            if (cur.size() >= 4)
                outStrings.push_back(cur);
            cur.clear();
        };

        for (std::size_t i = 0; i < buf.size(); ++i)
        {
            const std::uint8_t c = buf[i];
            if (c == 0)
            {
                flush();
                continue;
            }

            if (isPrintableAscii(c))
            {
                cur.push_back((char)c);
                // harte Grenze, damit wir nicht riesige Blöcke sammeln
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

    // --- Strategy 2: Length-prefixed strings (legacy) ---
    static void extractLengthPrefixedStrings(const std::vector<std::uint8_t>& buf, std::vector<std::string>& outStrings)
    {
        // Wir scannen "sliding window" nach plausiblen [u32 len][len bytes printable]
        // len darf nicht zu groß sein, und muss path-like sein.
        for (std::size_t i = 0; i + 8 <= buf.size(); ++i)
        {
            const std::uint32_t len = readU32LE(buf.data() + i);
            if (len < 4 || len > 260)
                continue;

            const std::size_t start = i + 4;
            const std::size_t end = start + (std::size_t)len;
            if (end > buf.size())
                continue;

            bool ok = true;
            for (std::size_t k = start; k < end; ++k)
            {
                if (!isPrintableAscii(buf[k]))
                {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;

            std::string s((const char*)buf.data() + start, (const char*)buf.data() + end);
            if (s.size() >= 4)
                outStrings.push_back(s);
        }
    }

    static bool startsWithSfxHeader(const std::vector<std::uint8_t>& buf, std::string& outVer)
    {
        if (buf.size() < 6) return false;
        if (!(buf[0] == 'S' && buf[1] == 'F' && buf[2] == 'X' && buf[3] == '0' && buf[4] == '.'))
            return false;

        // typischerweise: "SFX0.3  " (mit spaces)
        // wir lesen bis max 8 bytes und trimmen spaces/0.
        std::size_t n = std::min<std::size_t>(8, buf.size());
        std::string v((const char*)buf.data(), (const char*)buf.data() + n);

        // trim right spaces / null
        while (!v.empty() && (v.back() == ' ' || v.back() == '\0' || v.back() == '\r' || v.back() == '\n' || v.back() == '\t'))
            v.pop_back();

        outVer = v;
        return true;
    }

    bool SfxDecoder::decode(DecodedSfxData& out, const BinaryData& inBytes, std::string* outError)
    {
        out = DecodedSfxData{};
        out.raw = inBytes.bytes;

        const auto& buf = inBytes.bytes;
        if (buf.empty())
        {
            if (outError) *outError = "SFX: empty file.";
            return false;
        }

        // 1) Container/Version erkennen
        std::string ver;
        if (startsWithSfxHeader(buf, ver))
        {
            out.containerKind = "SFX0x";
            out.version = ver;
        }
        else
        {
            out.containerKind = "Legacy";
            out.version.clear(); // unknown
        }

        // 2) Strings extrahieren (beide Strategien)
        std::vector<std::string> found;
        found.reserve(256);

        extractAsciiRuns(buf, found);
        extractLengthPrefixedStrings(buf, found);

        // 3) Normalize + dedupe + classify
        std::unordered_set<std::string> seenAll;
        std::unordered_set<std::string> seenImages, seenSounds, seenModels, seenSfx, seenOther;

        out.allStrings.reserve(found.size());

        for (auto& s : found)
        {
            s = normalizePathLike(s);
            s = toLowerLocal(s);

            // Mini-Filter: zu viele Spaces/komische Sequenzen rauswerfen
            // (aber nicht zu aggressiv sein)
            if (s.size() < 4)
                continue;

            // allStrings (dedupe)
            if (seenAll.insert(s).second)
                out.allStrings.push_back(s);

            // refs
            if (looksLikeAssetRef(s))
                classifyAndStore(s, out, seenImages, seenSounds, seenModels, seenSfx, seenOther);
        }

        // Decoder-Definition: auch wenn keine refs gefunden wurden, ist das NICHT zwingend ein Fehler.
        // Manche Effekte könnten intern sein oder ohne externe Dateien.
        return true;
    }
}
