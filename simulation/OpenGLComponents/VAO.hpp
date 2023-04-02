#pragma once
#include <glad/gl.h>

#include "VBO.hpp"

namespace openGLComponents{
    class VAO{
        private:
            unsigned int ID;
            unsigned int attribOffset = 0; // For when using multiple VBOs with the same VAO

        public:
            VAO(){
                glGenVertexArrays(1, &this->ID);
            }

            ~VAO(){
                glDeleteVertexArrays(1, &this->ID);
            }

            void bind(){
                glBindVertexArray(this->ID);
            }

            void addBuffer(VBO& v, const VBOLayout& l){
                this->bind();
                v.bind();
                const auto& elements = l.getElements();
                unsigned int offset = 0;
                for(int i=0; i<elements.size(); i++){
                    const auto& element = elements[i]; // Get the element
                    glEnableVertexAttribArray(i + attribOffset); // Enable the vertex attribute
                    // Then set tthe attribute:
                    glVertexAttribPointer(i + attribOffset, element.count, element.type, element.normalized, l.getStride(), (const void*)offset);
                    offset += element.count * VBOElement::getSizeOfType(element.type); // Increment the offset
                }
                this->attribOffset += elements.size();
            }
    };
}
