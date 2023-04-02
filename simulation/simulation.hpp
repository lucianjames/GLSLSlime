#include "OpenGLComponents/VAO.hpp"
#include "OpenGLComponents/VBO.hpp"
#include "OpenGLComponents/shader.hpp"

#include <vector>


class simulation{
private:

    const char* basicVertexShaderSource = R"glsl(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aColor;

        out vec3 ourColor;

        void main(){
            gl_Position = vec4(aPos, 1.0);
            ourColor = aColor;
        }
    )glsl";

    const char* basicFragmentShaderSource = R"glsl(
        #version 330 core
        out vec4 FragColor;

        in vec3 ourColor;

        void main(){
            FragColor = vec4(ourColor, 1.0);
        }
    )glsl";

    std::vector<float> helloTriangleVertices = {
        -0.5f, -0.5f, 0.0f,  // left
        0.5f, -0.5f, 0.0f,  // right
        0.0f,  0.5f, 0.0f   // top
    };

    openGLComponents::VAO vao;
    openGLComponents::VBO vbo;
    openGLComponents::VBOLayout layout;
    openGLComponents::shader shader;

public:
    simulation(){

    }

    void setupHelloTriangle(){
        this->shader.createShaderFromSource(this->basicVertexShaderSource, this->basicFragmentShaderSource);
        this->shader.setUniform3f("ourColor", 1.0f, 0.5f, 0.2f);
        this->vbo.generate(this->helloTriangleVertices, this->helloTriangleVertices.size());
        this->layout.pushFloat(3);
        this->vao.addBuffer(this->vbo, this->layout); // Add the buffer "vbo" that has the layout defined by "layout"
    }

    void drawHelloTriangle(){
        this->shader.use();
        this->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }


};
