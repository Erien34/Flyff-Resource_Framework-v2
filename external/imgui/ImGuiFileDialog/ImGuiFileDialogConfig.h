#pragma once

// ============================================================================
//  Windows Support aktivieren  (sonst fehlen dirent.h etc.)
// ============================================================================
#define IMGUIFILEDIALOG_USE_NATIVE_WINDOWS_API      // <-- wichtig auf Windows
#define IGFD_USE_STD_FILESYSTEM                     // für std::filesystem

// Optional (empfohlen)
#define IMGUI_DEFINE_MATH_OPERATORS
#define IMGUIFILEDIALOG_MAX_PATH_HISTORY 30         // History Funktion
#define IMGUI_FILE_DIALOG_SELECT_MULTIPLE           // Multi-Select möglich