cmake_minimum_required(VERSION 3.16)

set(GAME_TITLE SDL2Test)
project(SDL2Test)

set(SDL_X11_XTEST OFF)

find_package(SDL2)
find_package(SDL2_ttf)
find_package(OpenGL)
find_package(GLEW)
find_package(glm)

add_subdirectory(deps/tinygltf)

set(CMAKE_CXX_STANDARD 17)

# Add your executable target
add_executable(${GAME_TITLE} src/main.cpp)

target_link_libraries(${GAME_TITLE} PRIVATE SDL2::SDL2 SDL2_ttf OpenGL::GL GLEW::GLEW glm::glm tinygltf)

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/assets/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/assets/")