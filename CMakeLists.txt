cmake_minimum_required(VERSION 3.16)

project(SimplicialVisualizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(glfw)

add_library(glad glad/src/glad.c)

target_include_directories(glad PUBLIC glad/include)

#main cpp
add_executable(viewer src/main.cpp)

#linkers
target_link_libraries(viewer PRIVATE glm)
target_link_libraries(viewer PRIVATE glad)
target_link_libraries(viewer PRIVATE glfw)
