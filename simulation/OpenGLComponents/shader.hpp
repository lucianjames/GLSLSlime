#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/gl.h>

namespace openGLComponents{

class shader{
private:
    unsigned int compileShader(unsigned int type, const std::string& source){
        // Create and compile a shader:
        unsigned int id = glCreateShader(type); // Create shader
        const char* src = source.c_str(); // Get the source code
        GLCall(glShaderSource(id, 1, &src, nullptr)); // Set shader source
        GLCall(glCompileShader(id)); // Compile shader
        // Give us some information if compilation fails:
        int result; // Create an integer to store the compile status
        GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result)); // Get the compile status
        if (result == GL_FALSE) { // If the compile failed:
            char info[1024]; // Create a buffer for the error message
            GLCall(glGetShaderInfoLog(id, 1024, NULL, info)); // Get the error message
            std::cout << "Failed to compile shader\n" << info << std::endl; // Print the error message
            GLCall(glDeleteShader(id)); // Delete the shader
            return 0;
        }
        return id; // Return the shader id
    }

public:
    unsigned int ID; // Shader ID, Ideally this wouldnt need to be public

    void createShaderFromDisk(const char* vertexPath, const char* fragmentPath){
        // ========= 1. Retrieve the vertex/fragment source code from filePath
        std::string vertexCode; // Create a string to store the vertex shader source code
        std::string fragmentCode; // Create a string to store the fragment shader source code
        std::ifstream vShaderFile; // Create an input filestream to read the vertex shader
        std::ifstream fShaderFile; // Create an input filestream to read the fragment shader
        // Ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        try{
            // Open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream; // Create string streams to store the contents of the files
            // Read file's buffer contents into streams:
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // Close file handlers:
            vShaderFile.close();
            fShaderFile.close();
            // Convert stream into string
            vertexCode = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure e){
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
        }
        const char* vShaderCode = vertexCode.c_str(); // Get the vertex shader code in a c-string
        const char* fShaderCode = fragmentCode.c_str(); // Get the fragment shader code in a c-string
        // ========== 2. Compile shaders:
        createShaderFromSource(vShaderCode, fShaderCode);
    }

    void createShaderFromSource(const char* vShaderCode, const char* fShaderCode){
        int success;
        char infoLog[512];
        unsigned int vs = compileShader(GL_VERTEX_SHADER, vShaderCode); // Compile the vertex shader and store the id
        unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fShaderCode); // Compile the fragment shader and store the id
        // Create Shader Program:
        ID = glCreateProgram();
        GLCall(glAttachShader(ID, vs));
        GLCall(glAttachShader(ID, fs));
        GLCall(glLinkProgram(ID));
        // Print linking errors if any
        GLCall(glGetProgramiv(ID, GL_LINK_STATUS, &success));
        if (!success){
            GLCall(glGetProgramInfoLog(ID, 512, NULL, infoLog));
            std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        // ========== 3. Clean up shaders
        // Delete the shaders as they're linked into our program now and no longer necessary
        GLCall(glDeleteShader(vs));
        GLCall(glDeleteShader(fs));
    }

    void use(){
        GLCall(glUseProgram(this->ID));
    }

    void setUniform4f(const std::string& name, float x, float y, float z, float w){
        this->use();
        glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    void setUniform3f(const std::string& name, float x, float y, float z){
        this->use();
        glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    void setUniform2f(const std::string& name, float x, float y){
        this->use();
        glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    void setUniform1f(const std::string& name, float x){
        this->use();
        glUniform1f(glGetUniformLocation(ID, name.c_str()), x);
    }

    void setUniform4i(const std::string& name, int x, int y, int z, int w){
        this->use();
        glUniform4i(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
    }
    void setUniform3i(const std::string& name, int x, int y, int z){
        this->use();
        glUniform3i(glGetUniformLocation(ID, name.c_str()), x, y, z);
    }
    void setUniform2i(const std::string& name, int x, int y){
        this->use();
        glUniform2i(glGetUniformLocation(ID, name.c_str()), x, y);
    }
    void setUniform1i(const std::string& name, int x){
        this->use();
        glUniform1i(glGetUniformLocation(ID, name.c_str()), x);
    }
    
    void setUniformMat4fv(const std::string& name, const float* matrix){
        this->use();
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, matrix);
    }
    void setUniformMat3fv(const std::string& name, const float* matrix){
        this->use();
        glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, matrix);
    }
    void setUniformMat2fv(const std::string& name, const float* matrix){
        this->use();
        glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, matrix);
    }
};  

}