cmake_minimum_required(VERSION 3.16)
project(SDLTest)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/deps/SDL3")

set(BUILD_SHARED_LIBS OFF)
set(SDL_X11_XTEST OFF)

add_subdirectory(deps/SDL3)
add_subdirectory(deps/SDL3_ttf)

set(CMAKE_CXX_STANDARD 17)

add_executable(SDL3Test src/main.cpp)

target_link_libraries(SDL3Test PRIVATE SDL3::SDL3-static SDL3_ttf-static)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets/")