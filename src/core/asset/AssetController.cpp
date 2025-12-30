#include "AssetController.h"
#include "core/Log.h"

// #include "converter/DdsConverter.h"
// #include "converter/ImageConverter.h"
// #include "converter/ModelConverter.h"

#include <fstream>

namespace fs = std::filesystem;

namespace resource
{
void AssetController::ensureDir(const fs::path& p)
{
    fs::path dir = p.has_extension() ? p.parent_path() : p;
    if (!dir.empty())
        fs::create_directories(dir);
}

fs::path AssetController::withExtension(const fs::path& p, const std::string& ext)
{
    fs::path out = p;
    out.replace_extension(ext);
    return out;
}

// bool AssetController::doTextureToPng(const fs::path& src, const fs::path& dst, std::string* err)
// {
//     // DDS->PNG (oder TGA->PNG via ImageConverter falls du willst; hier: DDS-only, TGA handled in ImageConverter)
//     return resource::DdsConverter::ddsToPng(src.string(), dst.string(), err);
// }

// bool AssetController::doImageToPng(const fs::path& src, const fs::path& dst, std::string* err)
// {
//     // BMP/PNG/JPG/TGA -> PNG
//     return resource::ImageConverter::convertToPng(src.string(), dst.string(), err);
// }

// bool AssetController::doModelToGltf(const fs::path& src, const fs::path& dst, std::string* err)
// {
//     // NOTE:
//     // Du hast ModelConverter hochgeladen. Je nach deiner Signatur:
//     //  - o3dToGltf(src,dst,err)
//     //  - convert(src,dst,err)
//     // Passe NUR diese Zeile an, falls bei dir anders benannt:
//     return resource::ModelConverter::o3dToGltf(src.string(), dst.string(), err);
// }

// bool AssetController::doCopy(const fs::path& src, const fs::path& dst, std::string* err)
// {
//     try
//     {
//         ensureDir(dst);
//         fs::copy_file(src, dst, fs::copy_options::overwrite_existing);
//         return true;
//     }
//     catch (const std::exception& e)
//     {
//         if (err) *err = e.what();
//         return false;
//     }
// }

// bool AssetController::doSfxCopyWithSidecar(const fs::path& src, const fs::path& dst, std::string* err)
// {
//     // Minimal “professionell nutzbar”:
//     // 1) copy .sfx (roh)
//     // 2) write sidecar json placeholder for engine flags/shader hints
//     if (!doCopy(src, dst, err))
//         return false;

//     try
//     {
//         fs::path side = dst;
//         side.replace_extension(dst.extension().string() + ".json"); // e.g. file.sfx.json

//         std::ofstream f(side.string(), std::ios::binary);
//         if (!f.is_open())
//             return true; // Sidecar optional

//         f <<
//             "{\n"
//             "  \"source\": \"" << src.filename().generic_string() << "\",\n"
//                                                 "  \"note\": \"Placeholder sidecar for future Effekseer/engine flags mapping.\",\n"
//                                                 "  \"engine\": {\n"
//                                                 "    \"shader\": \"\",\n"
//                                                 "    \"flags\": []\n"
//                                                 "  }\n"
//                                                 "}\n";
//         return true;
//     }
//     catch (...)
//     {
//         return true; // optional
//     }
// }

// bool AssetController::processOne(const Settings& s, const AssetRecord& rec)
// {
//     // choose src preference:
//     // - if exists in client: use client as truth for mirroring
//     // - else use resource path
//     fs::path src;
//     if (rec.existsInClient) src = rec.clientPath;
//     else if (rec.existsInResource) src = rec.resourcePath;
//     else return true;

//     // compute output target by kind
//     fs::path dst = rec.outPath;

//     // skip logic
//     if (s.onlyMissing && fs::exists(dst))
//         return true;
//     if (!s.overwrite && fs::exists(dst))
//         return true;

//     std::string err;

//     switch (rec.kind)
//     {
//     case AssetKind::Texture:
//     case AssetKind::Icon:
//     case AssetKind::UiSprite:
//     {
//         // normalize to PNG
//         fs::path out = withExtension(dst, ".png");
//         ensureDir(out);

//         const std::string ext = src.extension().string();
//         if (ext == ".dds" || ext == ".DDS")
//         {
//             if (!doTextureToPng(src, out, &err))
//             {
//                 core::Log::warn("[assets] DDS->PNG failed: " + src.string() + " :: " + err);
//                 return false;
//             }
//         }
//         else
//         {
//             if (!doImageToPng(src, out, &err))
//             {
//                 core::Log::warn("[assets] IMG->PNG failed: " + src.string() + " :: " + err);
//                 return false;
//             }
//         }

//         return true;
//     }

//     case AssetKind::Image:
//     {
//         // optional normalization -> png (you can also choose “copy as-is”)
//         fs::path out = withExtension(dst, ".png");
//         ensureDir(out);

//         if (!doImageToPng(src, out, &err))
//         {
//             core::Log::warn("[assets] IMG->PNG failed: " + src.string() + " :: " + err);
//             return false;
//         }
//         return true;
//     }

//     case AssetKind::Model:
//     case AssetKind::Animation:
//     case AssetKind::Skeleton:
//     {
//         fs::path out = withExtension(dst, ".gltf");
//         ensureDir(out);

//         if (!doModelToGltf(src, out, &err))
//         {
//             core::Log::warn("[assets] O3D->GLTF failed: " + src.string() + " :: " + err);
//             return false;
//         }
//         return true;
//     }

//     case AssetKind::Sound:
//     {
//         ensureDir(dst);
//         if (!doCopy(src, dst, &err))
//         {
//             core::Log::warn("[assets] WAV copy failed: " + src.string() + " :: " + err);
//             return false;
//         }
//         return true;
//     }

//     case AssetKind::Sfx:
//     {
//         ensureDir(dst);
//         if (!doSfxCopyWithSidecar(src, dst, &err))
//         {
//             core::Log::warn("[assets] SFX copy failed: " + src.string() + " :: " + err);
//             return false;
//         }
//         return true;
//     }

//     default:
//         // Unknown: copy raw to keep sync (optional)
//         ensureDir(dst);
//         doCopy(src, dst, nullptr);
//         return true;
//     }
// }

// bool AssetController::run(const Settings& s)
// {
//     core::Log::info("[assets] Building asset index...");
//     AssetIndexBuilder builder;
//     auto idx = builder.build(s.index);

//     core::Log::info("[assets] Processing " + std::to_string(idx.size()) + " indexed files...");

//     int ok = 0;
//     int fail = 0;

//     for (auto const& kv : idx)
//     {
//         const AssetRecord& rec = kv.second;

//         // only handle files that exist in at least one source
//         if (!rec.existsInClient && !rec.existsInResource)
//             continue;

//         bool success = processOne(s, rec);
//         if (success) ok++;
//         else fail++;
//     }

//     core::Log::info("[assets] Done. ok=" + std::to_string(ok) + " fail=" + std::to_string(fail));
//     return fail == 0;
// }

} // namespace resource
