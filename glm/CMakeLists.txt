cmake_minimum_required(VERSION 3.16)

project(SimplicialVisualizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------
# Fetch GLM
# ---------------------------
include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.3
)

FetchContent_MakeAvailable(glm)

# ---------------------------
# Executable
# ---------------------------
add_executable(viewer
    ../src/main.cpp
)

# ---------------------------
# Link GLM
# ---------------------------
target_link_libraries(viewer PRIVATE glm)