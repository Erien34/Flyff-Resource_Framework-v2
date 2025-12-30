cmake_minimum_required(VERSION 3.16)

message(STATUS "Loading SDL2 external...")

# SHARED library (DLL)
add_library(SDL2::SDL2 SHARED IMPORTED GLOBAL)

set_target_properties(SDL2::SDL2 PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2.dll"
    IMPORTED_IMPLIB   "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# OPTIONAL: SDL2main
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2main.lib")
    add_library(SDL2::SDL2main UNKNOWN IMPORTED)
    set_target_properties(SDL2::SDL2main PROPERTIES
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL2main.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
    )
endif()

message(STATUS "SDL2 loaded successfully.")

