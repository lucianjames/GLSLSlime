#pragma once
#include <glad/gl.h>

#include "debugging.hpp"

namespace openGLComponents{
    class SSBO{
        private:
            unsigned int ID;
        
        public:
            /*
                Create new SSBO from an std::vector
                Automatically deletes buffer if called multiple times 
                (no need to worry about manually destroying it)
            */
            template<typename T>
            void generate(std::vector<T>& data){
                if(this->ID != 0){
                    GLCall(glDeleteBuffers(1, &this->ID));
                    this->ID = 0;
                }
                GLCall(glGenBuffers(1, &this->ID));
                GLCall(glBindBuffer(GL_SHADER_STORAGE_BUFFER, this->ID));
                GLCall(glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(T) * data.size(), data.data(), GL_DYNAMIC_COPY));
            }

            void bind(unsigned int shaderID, const char name[], unsigned int bindingPoint) const{
                unsigned int block_index = glGetProgramResourceIndex(shaderID, GL_SHADER_STORAGE_BLOCK, name);
                glShaderStorageBlockBinding(shaderID, block_index, 0);
                glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, this->ID);
            }

            ~SSBO(){
                if(this->ID != 0){
                    GLCall(glDeleteBuffers(1, &this->ID));
                }
            }

    };
}
