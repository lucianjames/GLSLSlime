#pragma once
#include <glad/gl.h>
#include <vector>

#include "debugging.hpp"

namespace openGLComponents{
    class VBO{
        private:
            unsigned int ID;

        public:
            ~VBO(){
                GLCall(glDeleteBuffers(1, &this->ID));
            }
            template<typename T>
            void generate(std::vector<T> data, unsigned int size){
                GLCall(glGenBuffers(1, &this->ID));
                GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->ID)); // Bind the buffer to the GL_ARRAY_BUFFER target
                GLCall(glBufferData(GL_ARRAY_BUFFER, size, &data[0], GL_STATIC_DRAW)); // Copy the data to the buffer
            }
            void bind(){
                GLCall(glBindBuffer(GL_ARRAY_BUFFER, this->ID));
            }
    };

    /*
        This struct is used within the VBOLayout class to store data about how the data in the VBO is structured
    */
    struct VBOElement{
        unsigned int type;
        unsigned int count;
        unsigned char normalized;
        static unsigned int getSizeOfType(unsigned int type){
            switch(type){
                case GL_FLOAT: return 4;
                case GL_UNSIGNED_INT: return 4;
                case GL_UNSIGNED_BYTE: return 1;
            }
            return 0;
        }
    };

    class VBOLayout{
        private:
            std::vector<VBOElement> elements;
            unsigned int stride;

        public:
            VBOLayout(){
                this->stride = 0;
            }

            void push(unsigned int type, unsigned int count, unsigned char normalized){
                this->elements.push_back({type, count, normalized});
                this->stride += count * VBOElement::getSizeOfType(type);
            }

            void pushFloat(unsigned int count){
                this->push(GL_FLOAT, count, GL_FALSE);
            }

            void pushUnsignedInt(unsigned int count){
                this->push(GL_UNSIGNED_INT, count, GL_FALSE);
            }

            void pushUnsignedByte(unsigned int count){
                this->push(GL_UNSIGNED_BYTE, count, GL_TRUE);
            }

            inline const std::vector<VBOElement> getElements() const { return this->elements; }

            inline unsigned int getStride() const { return this->stride; }
    };
}