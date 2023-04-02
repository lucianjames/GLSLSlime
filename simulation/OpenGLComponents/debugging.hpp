#pragma once
#include <iostream>
#include <glad/gl.h>

// Debugging macro
#define GLCall(x) GLClearError(); x; GLLogCall(#x, __FILE__, __LINE__)

// Debugging functions
static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (0x" << std::hex << error << ") " << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

static void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}
