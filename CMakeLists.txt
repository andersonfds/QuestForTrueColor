cmake_minimum_required(VERSION 3.1...3.28)
include(FetchContent)

set(CMAKE_CXX_STANDARD 20)

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

add_compile_options(-lSDL2 -lSDL2_mixer)

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

target_link_libraries(QuestForTrueColor PRIVATE OpenGL::GL)
target_link_libraries(QuestForTrueColor PRIVATE PNG::PNG)
target_link_libraries(QuestForTrueColor PRIVATE GLUT::GLUT)

target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
target_link_libraries(QuestForTrueColor PRIVATE LDtkLoader)

# SOUND PROCESSING
find_program(SDL2_CONFIG sdl2-config REQUIRED)
execute_process(
    COMMAND ${SDL2_CONFIG} --cflags
    OUTPUT_VARIABLE SDL2_INCLUDE_DIRS
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${SDL2_CONFIG} --libs
    OUTPUT_VARIABLE SDL2_LIBRARIES
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_library(SDL2_MIXER_LIBRARY NAMES SDL2_mixer REQUIRED)
find_path(SDL2_MIXER_INCLUDE_DIR NAME SDL_mixer.h HINTS /opt/homebrew/include/SDL2)
include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIR})
target_link_libraries(QuestForTrueColor PRIVATE ${SDL2_LIBRARIES} ${SDL2_MIXER_LIBRARY})

