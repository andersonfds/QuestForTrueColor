cmake_minimum_required(VERSION 3.1...3.28)
include(FetchContent)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

project(
    QuestForTrueColor
    VERSION 0.1
    LANGUAGES CXX)


file(GLOB QuestForTrueColor_SRC
"main.cc"
)

add_executable(QuestForTrueColor ${QuestForTrueColor_SRC})

# Include olcPixelGameEngine
set(PIXEL_GAME_ENGINE_VERSION v2.23)
set(PIXEL_GAME_ENGINE_URL "https://github.com/OneLoneCoder/olcPixelGameEngine/releases/download/${PIXEL_GAME_ENGINE_VERSION}/olcPixelGameEngine.h")

# Include olcUTIL_Geometry2D
set(PIXEL_GAME_ENGINE_UTIL_GEOMETRY2D_VERSION main)
set(PIXEL_GAME_ENGINE_UTIL_GEOMETRY2D_URL "https://raw.githubusercontent.com/OneLoneCoder/olcUTIL_Geometry2D/${PIXEL_GAME_ENGINE_UTIL_GEOMETRY2D_VERSION}/olcUTIL_Geometry2D.h")

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/olcPixelGameEngine.h")
    Message("Downloading olcPixelGameEngine.h")
    file(DOWNLOAD "${PIXEL_GAME_ENGINE_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/include/olcPixelGameEngine.h")
endif()

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/olcUTIL_Geometry2D.h")
    Message("Downloading olcUTIL_Geometry2D.h")
    file(DOWNLOAD "${PIXEL_GAME_ENGINE_UTIL_GEOMETRY2D_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/include/olcUTIL_Geometry2D.h")
endif()

# Include olcPixelGameEngine dependencies
find_package(PNG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

FetchContent_Declare(
  ldtkloader
  GIT_REPOSITORY https://github.com/Madour/LDtkLoader.git
  GIT_TAG        1.5.3
)

FetchContent_MakeAvailable(ldtkloader)

# Include olcPixelGameEngine dependencies
find_package(OpenGL REQUIRED)
target_link_libraries(QuestForTrueColor PRIVATE OpenGL::GL)

if (APPLE)
    find_package(PNG REQUIRED)
    find_package(GLUT REQUIRED)

    target_link_libraries(QuestForTrueColor PRIVATE PNG::PNG)
    target_link_libraries(QuestForTrueColor PRIVATE GLUT::GLUT)
endif()


target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
target_link_libraries(QuestForTrueColor PRIVATE LDtkLoader)

# Fetch content for SDL2
FetchContent_Declare(
  sdl2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG release-2.30.3
)

FetchContent_GetProperties(sdl2)
if(NOT sdl2_POPULATED)
  FetchContent_Populate(sdl2)
  add_subdirectory(${sdl2_SOURCE_DIR} ${sdl2_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
target_include_directories(QuestForTrueColor PRIVATE ${sdl2_SOURCE_DIR}/include)
target_link_libraries(QuestForTrueColor PRIVATE SDL2)

if(NOT TARGET SDL2::SDL2)
  add_library(SDL2::SDL2 ALIAS SDL2)
endif()

# Fetch content for SDL2_mixer
FetchContent_Declare(
  sdl2_mixer
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
  GIT_TAG release-2.8.0
)
FetchContent_GetProperties(sdl2_mixer)
set(SDL2MIXER_OPUS OFF CACHE BOOL "Disable Opus support" FORCE)

if(NOT sdl2_mixer_POPULATED)
  FetchContent_Populate(sdl2_mixer)
  add_subdirectory(${sdl2_mixer_SOURCE_DIR} ${sdl2_mixer_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
target_include_directories(QuestForTrueColor PRIVATE ${sdl2_mixer_SOURCE_DIR}/include)
target_link_libraries(QuestForTrueColor PRIVATE SDL2_mixer)
add_compile_definitions(_THREAD_SAFE)
