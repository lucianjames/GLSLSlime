#pragma once
#include <glad/gl.h>

#include "debugging.hpp"

namespace openGLComponents{
    class SSBO{
        private:
            unsigned int ID;
            bool bufferCreated = false;
        
        public:
            /*
                Create new SSBO from an std::vector
                Automatically deletes buffer if called multiple times 
                (no need to worry about manually destroying it)
            */
            template<typename T>
            void generate(std::vector<T>& data){
                if(bufferCreated){
                    GLCall(glDeleteBuffers(1, &ID));
                }
                GLCall(glGenBuffers(1, &ID));
                GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID));
                GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * data.size(), data.data(), GL_DYNAMIC_COPY));
                bufferCreated = true;	
            }

            void bind(unsigned int shaderID, const char name[], unsigned int bindingPoint) const{
                unsigned int block_index;
                block_index = glGetProgramResourceIndex(shaderID, GL_SHADER_STORAGE_BLOCK, name);
                glShaderStorageBlockBinding(shaderID, block_index, 0);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, ID);
            }

            ~SSBO(){
                if(bufferCreated){
                    GLCall(glDeleteBuffers(1, &ID));
                }
            }

    };
}


/*

SSBO::SSBO(const void* data, unsigned int size){
	GLCall(glGenBuffers(1, &ID));
	GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, ID));
	GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_COPY));	
}

SSBO::~SSBO(){
	GLCall(glDeleteBuffers(1, &ID));
}

void SSBO::bind(unsigned int shaderID, const char name[]) const{
	unsigned int block_index;
	block_index = glGetProgramResourceIndex(shaderID, GL_SHADER_STORAGE_BLOCK, name);
	glShaderStorageBlockBinding(shaderID, block_index, 0);
	unsigned int binding_point_index = 0;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding_point_index, ID);
}

*/