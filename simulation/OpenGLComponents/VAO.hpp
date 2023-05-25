#pragma once
#include <glad/gl.h>

#include "VBO.hpp"
#include "debugging.hpp"

namespace openGLComponents{
    class VAO{
        private:
            unsigned int ID;
            unsigned int attribOffset = 0; // For when using multiple VBOs with the same VAO

        public:
            VAO(){
                GLCall(glGenVertexArrays(1, &this->ID));
            }

            ~VAO(){
                GLCall(glDeleteVertexArrays(1, &this->ID));
            }

            void bind(){
                GLCall(glBindVertexArray(this->ID));
            }

            void addBuffer(VBO& vbo, const VBOLayout& layout){
                this->bind();
                vbo.bind();
                const auto& elements = layout.getElements();
                unsigned int offset = 0;
                for(int i=0; i<elements.size(); i++){
                    const auto& element = elements[i]; // Get the element
                    GLCall(glEnableVertexAttribArray(i + attribOffset)); // Enable the vertex attribute
                    // Then set tthe attribute:
                    GLCall(glVertexAttribPointer(i + attribOffset, element.count, element.type, element.normalized, layout.getStride(), reinterpret_cast<const void*>(offset)));
                    offset += element.count * VBOElement::getSizeOfType(element.type); // Increment the offset
                }
                this->attribOffset += elements.size();
            }
    };
}
