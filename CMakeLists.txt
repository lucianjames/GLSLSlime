include(FetchContent)
cmake_minimum_required(VERSION 3.8)


project(GLSLSlime)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")


# This is a really nice and easy way to include all the openGL stuff i need
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
        imgui-glfw-glad-glm
        GIT_REPOSITORY https://github.com/LJ3D/imgui-glfw-glad-glm
)
FetchContent_MakeAvailable(imgui-glfw-glad-glm)


# Add all the .cpp files in ./headers/
file(GLOB_RECURSE HEADERS "headers/*.cpp")
add_executable(GLSLSlime main.cpp ${HEADERS})
target_link_libraries(
        GLSLSlime
        imgui
        glm
)

# Get all .glsl files in ./simulation/GLSL and copy them to the build directory (under ./GLSL)
file(GLOB_RECURSE GLSL_FILES "simulation/GLSL/*.glsl")
file(COPY ${GLSL_FILES} DESTINATION ${CMAKE_BINARY_DIR}/GLSL)