#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "OpenGLComponents/VAO.hpp"
#include "OpenGLComponents/VBO.hpp"
#include "OpenGLComponents/shader.hpp"
#include "OpenGLComponents/simulationTexture.hpp"


namespace simulation{
    namespace globalsForBufferSizeCallback{
        const int windowStartWidth = 1024;
        const int windowStartHeight = 1024;
        int newWidth = windowStartWidth;
        int newHeight = windowStartHeight;
    }

void framebufferSizeCallback(GLFWwindow* window, int width, int height){
    glViewport(0, 0, width, height);
}

class main{
private:
    // Settings (dummy for now)
    int widthHeightResolution = 1024;
    float offsetX = 0;
    float offsetY = 0;
    float zoomMultiplier = 1;
    float sensorDistance = 1;
    float sensorAngle = 1;
    float turnSpeed = 1;
    float pixelMultiplier = 0.1;
    float newPixelMultiplier = 0.89;

    // More important stuff
    float textureRatio = 1.0f; // Used to ensure that the texture is always square regardless of the window size

    // Some basic shaders for testing (cant be bothered putting them into a separate file yet)
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

        uniform vec3 ourColor;

        void main(){
            FragColor = vec4(ourColor, 1.0);
        }
    )glsl";

    // The quad which the simulation is rendered to
    std::vector<float> quadVertices = {
        // - positions -  - texture coords -
        // First triangle
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
        0.5f, -0.5f, 0.0f,   1.0f, 0.0f,
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
        // Second triangle
        0.5f,  0.5f, 0.0f,   1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f

    };

    // The opengl components required to render the quad, and perform the simulation
    openGLComponents::VAO vao;
    openGLComponents::VBO vbo;
    openGLComponents::VBOLayout layout;
    openGLComponents::shader shader;
    openGLComponents::simulationTexture texture;

public:
    main(unsigned int textureResolution = 1024){
        this->texture.init(textureResolution);
        this->texture.bind(0, 0, 0);
    }

    // Testing stuff
    void setupHelloTriangle(){
        this->shader.createShaderFromSource(this->basicVertexShaderSource, this->basicFragmentShaderSource);
        this->shader.use(); // Otherwise the uniform won't be set
        this->shader.setUniform3f("ourColor", 1.0f, 0.2f, 0.6f);
        this->vbo.generate(this->quadVertices, this->quadVertices.size() * sizeof(float));
        this->layout.pushFloat(3);
        this->layout.pushFloat(2);
        this->vao.addBuffer(this->vbo, this->layout); // Add the buffer "vbo" that has the layout defined by "layout"
    }
    void drawHelloTriangle(){
        this->shader.use();
        this->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, this->quadVertices.size() / 5);
    }

    // Just a dummy menu thing for now
    void update(){
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(600, 240));
        ImGui::Begin("Simulation Settings");
        ImGui::SliderInt("Width/Height Resolution", &widthHeightResolution, 0, 8192);
        ImGui::SliderFloat("Offset X", &offsetX, -100, 100);
        ImGui::SliderFloat("Offset Y", &offsetY, -100, 100);
        ImGui::SliderFloat("Zoom Multiplier", &zoomMultiplier, 0, 100);
        ImGui::SliderFloat("Sensor Distance", &sensorDistance, 0, 100);
        ImGui::SliderFloat("Sensor Angle", &sensorAngle, 0, 2);
        ImGui::SliderFloat("Turn Speed", &turnSpeed, 0, 10);
        ImGui::SliderFloat("Pixel Multiplier", &pixelMultiplier, 0, 1);
        ImGui::SliderFloat("New Pixel Multiplier", &newPixelMultiplier, 0, 1);
        ImGui::End();
    }

};

}