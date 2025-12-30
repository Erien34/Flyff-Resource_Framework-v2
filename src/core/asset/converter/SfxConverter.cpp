#include "SfxConverter.h"

namespace resource
{
ConvertResult SfxConverter::convert(const SfxSource& src) const
{
    ConvertResult r;

    // 🔒 Zielpfad ableiten (niemals src.outPath direkt verändern)
    auto out = src.outPath;

    // Optional: Effekt-Seeker-Extension (z.B. ".efs")
    if (!settings().sfxTargetExtension.empty())
        out.replace_extension(settings().sfxTargetExtension);

    // Skip, wenn Datei existiert und overwrite=false
    if (shouldSkipWrite(out))
    {
        r.ok = true;
        return r;
    }

    std::string err;

    // 1) Bevorzugt: bereits geladene Bytes schreiben
    if (!src.bytes.empty())
    {
        if (!writeAllBytes(out, src.bytes.bytes, &err))
        {
            r.ok = false;
            r.error = "SfxConverter: writeAllBytes failed: " + err;
            return r;
        }

        r.ok = true;
        return r;
    }

    // 2) Fallback: Datei direkt kopieren
    if (src.sourcePath.empty())
    {
        r.ok = false;
        r.error =
            "SfxConverter: no data and no sourcePath for relPath=" +
            src.relPath.string();
        return r;
    }

    if (!copyFile(src.sourcePath, out, &err))
    {
        r.ok = false;
        r.error = "SfxConverter: copyFile failed: " + err;
        return r;
    }

    r.ok = true;
    return r;
}
} // namespace resource
