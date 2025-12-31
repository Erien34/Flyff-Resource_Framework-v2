cmake_minimum_required(VERSION 3.16)
project(FlyFF_Framework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =======================
# Source Files
# =======================
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/*.h
)

add_executable(${PROJECT_NAME} ${SRC_FILES})

# =======================
# External Paths
# =======================
set(IMGUI_PATH        ${CMAKE_SOURCE_DIR}/external/imgui)
set(IMGUIDIALOG_PATH  ${CMAKE_SOURCE_DIR}/external/imgui/ImGuiFileDialog)
set(IMGUI_BACKENDS    ${IMGUI_PATH}/backends)

set(JSON_PATH ${CMAKE_SOURCE_DIR}/external/nlohmann)

set(SDL2_INCLUDE      ${CMAKE_SOURCE_DIR}/external/SDL2/include)
set(SDL2_LIB          ${CMAKE_SOURCE_DIR}/external/SDL2/lib)

find_package(OpenGL REQUIRED)

# =======================
# Includes
# =======================
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/app
    src/core
    src/core/asset
    src/core/controller
    src/core/source
    src/core/resource
    src/core/runtime
    src/data
    src/data/asset
    src/data/context
    src/data/editor
    src/data/global
    src/data/raw/module
    src/data/module
    src/data/project
    src/data/state
    src/plugins
    src/plugins/api
    src/plugins/host
    src/ui
    ${JSON_PATH}
    ${IMGUI_PATH}
    ${IMGUI_BACKENDS}
    ${IMGUIDIALOG_PATH}
    ${SDL2_INCLUDE}
    ${CMAKE_SOURCE_DIR}/external/stb
)

target_link_directories(${PROJECT_NAME} PRIVATE
    ${SDL2_LIB}
)

# =======================
# Linking
# =======================
target_link_libraries(${PROJECT_NAME}
    OpenGL::GL
    SDL2
    SDL2main
)

# =======================
# ImGui SRC Bind
# =======================
target_sources(${PROJECT_NAME} PRIVATE
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
    ${IMGUI_PATH}/imgui_demo.cpp

    ${IMGUI_BACKENDS}/imgui_impl_sdl2.cpp
    ${IMGUI_BACKENDS}/imgui_impl_opengl3.cpp

    ${IMGUIDIALOG_PATH}/ImGuiFileDialog.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    TINYGLTF_NO_STB_IMAGE
)
