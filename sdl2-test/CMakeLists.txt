cmake_minimum_required(VERSION 3.16)

set(GAME_TITLE SDL2Test)
project(SDL2Test)

set(SDL_X11_XTEST OFF)

find_package(SDL2)
find_package(SDL2_ttf)

set(CMAKE_CXX_STANDARD 17)

# Add your executable target
add_executable(${GAME_TITLE} src/main.cpp)

target_link_libraries(${GAME_TITLE} PRIVATE SDL2::SDL2 SDL2_ttf)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets/")