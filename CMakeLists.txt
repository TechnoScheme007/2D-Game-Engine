cmake_minimum_required(VERSION 3.16)
project(DungeonBlaster2D VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Engine library ----
add_library(Engine2D STATIC
    src/engine/Renderer.cpp
    src/engine/Collision.cpp
    src/engine/TileMap.cpp
    src/engine/ParticleSystem.cpp
    src/engine/Scene.cpp
    src/engine/Engine.cpp
)

target_include_directories(Engine2D PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# ---- Find SDL2 ----
# Try multiple methods to find SDL2
if(DEFINED ENV{SDL2_DIR})
    list(APPEND CMAKE_PREFIX_PATH $ENV{SDL2_DIR})
endif()

# Try pkg-config first (Linux/Mac)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SDL2 QUIET sdl2)
endif()

if(NOT SDL2_FOUND)
    find_package(SDL2 QUIET)
endif()

if(SDL2_FOUND)
    if(TARGET SDL2::SDL2)
        target_link_libraries(Engine2D PUBLIC SDL2::SDL2 SDL2::SDL2main)
    else()
        target_include_directories(Engine2D PUBLIC ${SDL2_INCLUDE_DIRS})
        target_link_libraries(Engine2D PUBLIC ${SDL2_LIBRARIES})
    endif()
else()
    message(STATUS "SDL2 not found via find_package, trying manual detection...")
    # Manual fallback for common install locations
    if(WIN32)
        # Check common Windows SDL2 locations
        find_path(SDL2_INCLUDE_DIR SDL.h
            PATHS
            "C:/SDL2/include/SDL2"
            "C:/SDL2/include"
            "C:/msys64/mingw64/include/SDL2"
            "C:/msys64/ucrt64/include/SDL2"
            "$ENV{USERPROFILE}/vcpkg/installed/x64-windows/include/SDL2"
        )
        find_library(SDL2_LIBRARY
            NAMES SDL2
            PATHS
            "C:/SDL2/lib"
            "C:/msys64/mingw64/lib"
            "C:/msys64/ucrt64/lib"
            "$ENV{USERPROFILE}/vcpkg/installed/x64-windows/lib"
        )
        find_library(SDL2MAIN_LIBRARY
            NAMES SDL2main
            PATHS
            "C:/SDL2/lib"
            "C:/msys64/mingw64/lib"
            "C:/msys64/ucrt64/lib"
            "$ENV{USERPROFILE}/vcpkg/installed/x64-windows/lib"
        )
        if(SDL2_INCLUDE_DIR AND SDL2_LIBRARY)
            target_include_directories(Engine2D PUBLIC ${SDL2_INCLUDE_DIR}/..)
            target_link_libraries(Engine2D PUBLIC ${SDL2MAIN_LIBRARY} ${SDL2_LIBRARY})
        else()
            message(FATAL_ERROR "SDL2 not found! Install SDL2:\n"
                "  vcpkg: vcpkg install sdl2\n"
                "  MSYS2: pacman -S mingw-w64-ucrt-x86_64-SDL2\n"
                "  Manual: download from libsdl.org and set SDL2_DIR")
        endif()
    else()
        # Unix fallback
        target_include_directories(Engine2D PUBLIC /usr/include/SDL2)
        target_link_libraries(Engine2D PUBLIC SDL2 SDL2main)
    endif()
endif()

# ---- Demo game executable ----
add_executable(DungeonBlaster
    src/demo/DemoGame.cpp
)

target_link_libraries(DungeonBlaster PRIVATE Engine2D)

# On Windows with MSVC, set subsystem to console for debugging
if(MSVC)
    set_target_properties(DungeonBlaster PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()

# ---- Tests ----
enable_testing()

# Headless tests (no window, no SDL init required)
set(HEADLESS_TESTS test_math test_ecs test_collision test_tilemap test_particles test_scene)

foreach(TEST_NAME ${HEADLESS_TESTS})
    add_executable(${TEST_NAME} tests/${TEST_NAME}.cpp)
    target_link_libraries(${TEST_NAME} PRIVATE Engine2D)
    target_compile_definitions(${TEST_NAME} PRIVATE SDL_MAIN_HANDLED)
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# ---- Install ----
install(TARGETS DungeonBlaster RUNTIME DESTINATION bin)
install(DIRECTORY assets/ DESTINATION bin/assets OPTIONAL)

message(STATUS "=== 2D Game Engine ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
