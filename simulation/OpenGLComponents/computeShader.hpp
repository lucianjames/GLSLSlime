#pragma once
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/gl.h>

#include "debugging.hpp"

namespace openGLComponents{

class computeShader{
    private:
        unsigned int ID;

    public:
        void createShaderFromSource(const char* cShaderCode){
            int success;
            char infoLog[512];
            // Create compute shader program
            unsigned int compute_shader = glCreateShader(GL_COMPUTE_SHADER);
            GLCall(glShaderSource(compute_shader, 1, &cShaderCode, NULL));
            GLCall(glCompileShader(compute_shader));
            // Print compiling errors if any
            int result;
            glGetShaderiv(compute_shader, GL_COMPILE_STATUS, &result);
            if (result == GL_FALSE) {
                char info[1024];
                GLCall(glGetShaderInfoLog(compute_shader, 1024, NULL, info));
                std::cout << "Failed to compile shader\n" << info << std::endl;
                GLCall(glDeleteShader(compute_shader));
            }
            // Create program:
            this->ID = glCreateProgram();
            GLCall(glAttachShader(this->ID, compute_shader));
            GLCall(glLinkProgram(this->ID));
            // Print linking errors if any
            GLCall(glGetProgramiv(this->ID, GL_LINK_STATUS, &success));
            if (!success){
                GLCall(glGetProgramInfoLog(this->ID, 512, NULL, infoLog));
                std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            }
            // Shader isnt needed after its linked to a program
            GLCall(glDeleteShader(compute_shader));
        }

        void createShaderFromDisk(const char* cShaderPath){
            std::string cShaderCodeStr;
            std::ifstream cShaderFile;
            try{
                cShaderFile.open(cShaderPath);
                std::stringstream cShaderStream;
                cShaderStream << cShaderFile.rdbuf();
            }catch(std::ifstream::failure e){
                std::cout << "ERROR::COMPUTE_SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
            }
            const char* cShaderCode = cShaderCodeStr.c_str();
            this->createShaderFromSource(cShaderCode);
        }

        void use(){
            GLCall(glUseProgram(this->ID));
        }

        void execute(unsigned int ngx, unsigned int ngy, unsigned int ngz){
            this->use();
            GLCall(glDispatchCompute(ngx, ngy, ngz));
	        GLCall(glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
        }



    
};


}