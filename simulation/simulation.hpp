#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "OpenGLComponents/VAO.hpp"
#include "OpenGLComponents/VBO.hpp"
#include "OpenGLComponents/shader.hpp"
#include "OpenGLComponents/simulationTexture.hpp"
#include "OpenGLComponents/computeShader.hpp"


namespace simulation{
    namespace winGlobals{
        const int windowStartWidth = 1024;
        const int windowStartHeight = 1024;
        int currentWidth = windowStartWidth;
        int currentHeight = windowStartHeight;
        int newWidth = windowStartWidth;
        int newHeight = windowStartHeight;
    }

void framebufferSizeCallback(GLFWwindow* window, int width, int height){
    GLCall(glViewport(0, 0, width, height));
    winGlobals::newWidth = width;
    winGlobals::newHeight = height;
}

class main{
private:
    // Settings (dummy for now)
    int widthHeightResolution = 1024;
    float offsetX = 0;
    float offsetX_inShader = 0;
    float offsetY = 0;
    float offsetY_inShader = 0;
    float zoomMultiplier = 1;
    float zoomMultiplier_inShader = 1;
    float sensorDistance = 1;
    float sensorDistance_inShader = 1;
    float sensorAngle = 1;
    float sensorAngle_inShader = 1;
    float turnSpeed = 1;
    float turnSpeed_inShader = 1;
    float pixelMultiplier = 0.1;
    float pixelMultiplier_inShader = 0.1;
    float newPixelMultiplier = 0.89;
    float newPixelMultiplier_inShader = 0.89;

    // More important stuff
    float textureRatio = 1.0f; // Used to ensure that the texture is always square regardless of the window size

    // The quad which the simulation is rendered to
    std::vector<float> quadVertices = {
        // - positions -  - texture coords -
        // First triangle
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
        1.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
        // Second triangle
        1.0f,  1.0f, 0.0f,   1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f

    };

    // The opengl components required to render the quad, and perform the simulation
    openGLComponents::VAO vao;
    openGLComponents::VBO vbo;
    openGLComponents::VBOLayout layout;
    openGLComponents::shader shader;
    openGLComponents::simulationTexture texture;
    openGLComponents::computeShader testComputeShader;

public:
    main(unsigned int textureResolution = 128){
        this->texture.init(textureResolution);
        this->texture.bind();

        std::cout << "Updating texture" << std::endl;
        float* data = new float[textureResolution * textureResolution * 4]; // RGBA = 4
        // Make it all white
        for(int i = 0; i < textureResolution * textureResolution * 4; i+=4){
            data[i] = 1.0f;
            data[i + 1] = i / 4 % textureResolution / (float)textureResolution;
            data[i + 2] = i / 4 / textureResolution / (float)textureResolution;
            data[i + 3] = 1.0f;
        }
        this->texture.update(data);
        delete[] data;

        this->shader.createShaderFromDisk("GLSL/quadShader.vert.glsl", "GLSL/quadShader.frag.glsl");
        this->shader.use();
        this->shader.setUniform1f("textureRatio", this->textureRatio);
        this->shader.setUniform1f("offsetX", this->offsetX_inShader);
        this->shader.setUniform1f("offsetY", this->offsetY_inShader);
        this->shader.setUniform1f("zoomMultiplier", this->zoomMultiplier_inShader);
        this->vbo.generate(this->quadVertices, this->quadVertices.size() * sizeof(float));
        this->layout.pushFloat(3);
        this->layout.pushFloat(2);
        this->vao.addBuffer(this->vbo, this->layout); // Add the buffer "vbo" that has the layout defined by "layout"
        
        this->testComputeShader.createShaderFromDisk("GLSL/test.compute.glsl");
    }
    
    void render(){
        this->texture.bind();
        this->testComputeShader.execute(this->widthHeightResolution, this->widthHeightResolution, 1);
        this->shader.use();
        this->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, this->quadVertices.size() / 5);
    }

    void update(){
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(600, 240), ImGuiCond_Once);
        ImGui::Begin("Simulation Settings");
        //ImGui::SliderInt("Width/Height Resolution", &widthHeightResolution, 0, 8192);
        ImGui::SliderFloat("Offset X", &offsetX, -2, 2);
        ImGui::SliderFloat("Offset Y", &offsetY, -2, 2);
        ImGui::SliderFloat("Zoom Multiplier", &zoomMultiplier, 0, 10);
        //ImGui::SliderFloat("Sensor Distance", &sensorDistance, 0, 100);
        //ImGui::SliderFloat("Sensor Angle", &sensorAngle, 0, 2);
        //ImGui::SliderFloat("Turn Speed", &turnSpeed, 0, 10);
        //ImGui::SliderFloat("Pixel Multiplier", &pixelMultiplier, 0, 1);
        //ImGui::SliderFloat("New Pixel Multiplier", &newPixelMultiplier, 0, 1);
        ImGui::End();

        /*
            Now update any settings (or whatever) that need to be updated
        */

        if(this->offsetX_inShader != this->offsetX){
            this->offsetX_inShader = this->offsetX;
            this->shader.use();
            this->shader.setUniform1f("offsetX", this->offsetX_inShader);
        }

        if(this->offsetY_inShader != this->offsetY){
            this->offsetY_inShader = this->offsetY;
            this->shader.use();
            this->shader.setUniform1f("offsetY", this->offsetY_inShader);
        }

        if(this->zoomMultiplier_inShader != this->zoomMultiplier){
            this->zoomMultiplier_inShader = this->zoomMultiplier;
            this->shader.use();
            this->shader.setUniform1f("zoomMultiplier", this->zoomMultiplier_inShader);
        }

        // ... Rest are TODO once they actually do something

        if(winGlobals::currentHeight != winGlobals::newHeight
        || winGlobals::currentWidth != winGlobals::newWidth){
            this->textureRatio = (float)winGlobals::currentWidth/winGlobals::currentHeight;
            winGlobals::currentHeight = winGlobals::newHeight;
            winGlobals::currentWidth = winGlobals::newWidth;
            this->shader.use();
            this->shader.setUniform1f("textureRatio", textureRatio);
        }
    }

};

}