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

set(OLC_SOUND_WAVE_ENGINE_BRANCH main)
set(OLC_SOUND_WAVE_ENGINE_URL "https://raw.githubusercontent.com/OneLoneCoder/olcSoundWaveEngine/${OLC_SOUND_WAVE_ENGINE_BRANCH}/olcPixelGameEngine.h")

set(SDL2_DOWNLOAD_VERSION 2.30.4)
set(SDL2_DOWNLOAD_URL "https://github.com/libsdl-org/SDL/releases/download/release-${SDL2_DOWNLOAD_VERSION}/SDL2-${SDL2_DOWNLOAD_VERSION}.tar.gz")

set(SDLMIXER_DOWNLOAD_VERSION 2.6.3)
set(SDLMIXER_DOWNLOAD_URL "https://github.com/libsdl-org/SDL_mixer/releases/download/release-${SDLMIXER_DOWNLOAD_VERSION}/SDL2_mixer-${SDLMIXER_DOWNLOAD_VERSION}.tar.gz")

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/olcPixelGameEngine.h")
    Message("Downloading olcPixelGameEngine.h")
    file(DOWNLOAD "${PIXEL_GAME_ENGINE_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/include/olcPixelGameEngine.h")
endif()

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/olcUTIL_Geometry2D.h")
    Message("Downloading olcUTIL_Geometry2D.h")
    file(DOWNLOAD "${PIXEL_GAME_ENGINE_UTIL_GEOMETRY2D_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/include/olcUTIL_Geometry2D.h")
endif()

# Include olcPixelGameEngine dependencies
find_package(OpenGL REQUIRED)
target_link_libraries(QuestForTrueColor PRIVATE OpenGL::GL)

if (APPLE)
    find_package(PNG REQUIRED)
    find_package(GLUT REQUIRED)

    target_link_libraries(QuestForTrueColor PRIVATE PNG::PNG)
    target_link_libraries(QuestForTrueColor PRIVATE GLUT::GLUT)
endif()

FetchContent_Declare(
  ldtkloader
  GIT_REPOSITORY https://github.com/Madour/LDtkLoader.git
  GIT_TAG        1.5.3
)

FetchContent_MakeAvailable(ldtkloader)
target_link_libraries(QuestForTrueColor PRIVATE LDtkLoader)

target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core)

# SDL2 and SDL2_mixer are required only for macOS
if (APPLE)
    # Download and build SDL2
    FetchContent_Declare(
      SDL2
      URL ${SDL2_DOWNLOAD_URL}
    )
    set(SDL2_DISABLE_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL2)
    target_link_libraries(QuestForTrueColor PRIVATE SDL2)

    # Download and build SDL2_mixer
    FetchContent_Declare(
      SDL2_mixer
      URL ${SDLMIXER_DOWNLOAD_URL}
    )
    set(SDL2MIXER_OPUS OFF CACHE BOOL "Disable Opus support" FORCE)
    set(SDL2MIXER_FLAC OFF CACHE BOOL "Disable FLAC support" FORCE)
    set(SDL2MIXER_MOD OFF CACHE BOOL "Disable MOD support" FORCE)
    set(SDL2MIXER_MIDI OFF CACHE BOOL "Disable MIDI support" FORCE)

    FetchContent_MakeAvailable(SDL2_mixer)    
    target_link_libraries(QuestForTrueColor PRIVATE SDL2_mixer)
    add_compile_definitions(_THREAD_SAFE)
  endif()

# Download soundWaveEngine
if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/olcSoundWaveEngine.h")
    Message("Downloading olcSoundWaveEngine.h")
    file(DOWNLOAD "${OLC_SOUND_WAVE_ENGINE_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/include/olcSoundWaveEngine.h")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-O3)  # Example of adding further optimizations
  add_definitions(-DNDEBUG)  # Ensure assertions are disabled
  message("Release build")
endif()

# Copy assets to build directory on build done
add_custom_command(TARGET QuestForTrueColor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:QuestForTrueColor>/assets)
