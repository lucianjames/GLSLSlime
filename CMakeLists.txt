# Basic project settings
cmake_minimum_required(VERSION 3.8)
project(GLSLSlime VERSION 1.0.0)

# Set some build settings
set(CMAKE_BUILD_TYPE Release)
set(BUILD_SHARED_LIBS OFF)

# Create executable for GLSLSlime
add_executable(GLSLSlime main.cpp)

# Ensure that optimisation is enabled
target_compile_features(GLSLSlime PRIVATE cxx_std_17)
if(UNIX)
    target_compile_options(GLSLSlime PRIVATE -O3) # Assume GCC or Clang
elseif(WIN32)
    target_compile_options(GLSLSlime PRIVATE /O2) # Assume MSVC
endif()

# Fetch imgui/glfw/glad/glm
include(FetchContent)
FetchContent_Declare(
    imgui-glfw-glad-glm
    GIT_REPOSITORY https://github.com/LJ3D/imgui-glfw-glad-glm
    GIT_SHALLOW ON
    UPDATE_DISCONNECTED ON
)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(FETCHCONTENT_QUIET OFF)
FetchContent_MakeAvailable(imgui-glfw-glad-glm)

# Find OpenCV
# Because building from source will take too long
find_package(OpenCV REQUIRED)

# Link everything together
target_include_directories(GLSLSlime PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(
    GLSLSlime
    imgui
    glm
    ${OpenCV_LIBS}
)

# Copy GLSL files to build directory
add_custom_target(
    copy_glsl_files ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/simulation/GLSL
        ${CMAKE_BINARY_DIR}/GLSL
    COMMENT "Copying GLSL files to build directory"
)
add_dependencies(GLSLSlime copy_glsl_files)