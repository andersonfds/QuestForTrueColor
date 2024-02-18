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
    file(DOWNLOAD "${PIXEL_GAME_ENGINE_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/include/olcPixelGameEngine.h")
endif()

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/olcUTIL_Geometry2D.h")
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

target_link_libraries(QuestForTrueColor PRIVATE OpenGL::GL)
target_link_libraries(QuestForTrueColor PRIVATE PNG::PNG)
target_link_libraries(QuestForTrueColor PRIVATE GLUT::GLUT)

target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/core)
target_link_libraries(QuestForTrueColor PRIVATE LDtkLoader)
