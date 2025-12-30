cmake_minimum_required(VERSION 3.16)
project(FlyFF_Framework)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC OFF)    # <- Qt Automoc NICHT aktiv (wir nutzen kein Qt UI)
set(CMAKE_AUTORCC OFF)
set(CMAKE_AUTOUIC OFF)

# ----------------------------------------
# SOURCE FILES
# ----------------------------------------
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/*.h
)

# ----------------------------------------
# CREATE EXECUTABLE
# ----------------------------------------
add_executable(FlyFF_Framework ${SRC_FILES})

# ----------------------------------------
# SDL2
# ----------------------------------------
set(SDL2_INCLUDE "${PROJECT_SOURCE_DIR}/external/sdl2/include")
set(SDL2_LIB     "${PROJECT_SOURCE_DIR}/external/sdl2/lib")

target_include_directories(FlyFF_Framework PRIVATE ${SDL2_INCLUDE})
target_link_directories(FlyFF_Framework PRIVATE ${SDL2_LIB})
target_link_libraries(FlyFF_Framework PRIVATE SDL2 SDL2main)

# ----------------------------------------
# OpenGL / GLAD
# ----------------------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(FlyFF_Framework PRIVATE OpenGL::GL)

target_include_directories(FlyFF_Framework PRIVATE
    ${PROJECT_SOURCE_DIR}/external/glad/include
)
target_sources(FlyFF_Framework PRIVATE
    ${PROJECT_SOURCE_DIR}/external/glad/src/glad.c
)

# ----------------------------------------
# IMGUI Core + Backends
# ----------------------------------------
file(GLOB IMGUI_SRC
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/imgui_demo.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_sdl2.cpp
    ${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})

target_include_directories(imgui PUBLIC
    ${PROJECT_SOURCE_DIR}/external/imgui
    ${PROJECT_SOURCE_DIR}/external/imgui/backends
    ${SDL2_INCLUDE}
)

target_link_libraries(FlyFF_Framework PRIVATE imgui)

# ----------------------------------------
# 🔥 ImGuiFileDialog Integration
# ----------------------------------------
set(IMGUI_FILE_DIALOG_SRC
    ${PROJECT_SOURCE_DIR}/external/ImGuiFileDialog/ImGuiFileDialog.cpp
)

target_sources(FlyFF_Framework PRIVATE ${IMGUI_FILE_DIALOG_SRC})
target_include_directories(FlyFF_Framework PRIVATE ${PROJECT_SOURCE_DIR}/external/ImGuiFileDialog)

# ----------------------------------------
# DONE
# ----------------------------------------
message(STATUS "FlyFF Framework build configured successfully!")

