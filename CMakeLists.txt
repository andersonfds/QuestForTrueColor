cmake_minimum_required(VERSION 3.1...3.28)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)

project(
    QuestForTrueColor
    VERSION 0.1
    LANGUAGES CXX)


file(GLOB QuestForTrueColor_SRC
"src/*.h"
"src/*.cpp"
)

add_executable(QuestForTrueColor ${QuestForTrueColor_SRC})

# Include olcPixelGameEngine
set(PIXEL_GAME_ENGINE_VERSION v2.23)
set(PIXEL_GAME_ENGINE_URL "https://github.com/OneLoneCoder/olcPixelGameEngine/releases/download/${PIXEL_GAME_ENGINE_VERSION}/olcPixelGameEngine.h")

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/include/olcPixelGameEngine.h")
    file(DOWNLOAD "${PIXEL_GAME_ENGINE_URL}" "${CMAKE_CURRENT_SOURCE_DIR}/include/olcPixelGameEngine.h")
endif()

# Include olcPixelGameEngine dependencies
find_package(PNG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

target_link_libraries(QuestForTrueColor PRIVATE OpenGL::GL)
target_link_libraries(QuestForTrueColor PRIVATE PNG::PNG)
target_link_libraries(QuestForTrueColor PRIVATE GLUT::GLUT)

target_include_directories(QuestForTrueColor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)