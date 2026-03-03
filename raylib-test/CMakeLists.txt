# Specify the minimum required CMake version
cmake_minimum_required(VERSION 3.16)

set(GAME_TITLE raylib-test)

# Add an executable target and specify its source file(s)
add_executable(${GAME_TITLE} src/main.cpp)

find_package(SDL2)
find_package(raylib)

# Define the project name and enable C++ support
project(${GAME_TITLE} VERSION 1.0 LANGUAGES CXX)

target_link_libraries(${GAME_TITLE} PRIVATE raylib SDL2::SDL2)

# Specify the C++ standard to use (optional, but recommended)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

