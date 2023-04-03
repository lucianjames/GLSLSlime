#include <vector>
#include <chrono>
#include <thread>
#include <random>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "OpenGLComponents/VAO.hpp"
#include "OpenGLComponents/VBO.hpp"
#include "OpenGLComponents/shader.hpp"
#include "OpenGLComponents/simulationTexture.hpp"
#include "OpenGLComponents/computeShader.hpp"
#include "OpenGLComponents/SSBO.hpp"

namespace simulation{
    namespace winGlobals{
        const int windowStartWidth = 1920;
        const int windowStartHeight = 1080;
        int currentWidth = windowStartWidth;
        int currentHeight = windowStartHeight;
        int newWidth = windowStartWidth;
        int newHeight = windowStartHeight;
    }

    namespace controlGlobals{
        double scrollYOffset = 0;
        double mouseClickXPos = 0;
        double mouseClickYPos = 0;
        double prevMouseClickXPos = -1;
        double prevMouseClickYPos = -1;
        bool rmbClicked = false;
    }

    namespace callbacks{
        void framebufferSizeCallback(GLFWwindow* window, int width, int height){
            winGlobals::newWidth = width;
            winGlobals::newHeight = height;
            GLCall(glViewport(0, 0, width, height));
        }

        void scrollCallback(GLFWwindow* window, double xOffset, double yOffset){
            controlGlobals::scrollYOffset = yOffset;
        }

        void cursorPositionCallback(GLFWwindow*window, double xPos, double yPos){
            if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS){
                controlGlobals::mouseClickXPos = xPos;
                controlGlobals::mouseClickYPos = yPos;
                controlGlobals::rmbClicked = true;
            }else{
                controlGlobals::prevMouseClickXPos = xPos;
                controlGlobals::prevMouseClickYPos = yPos;
            }
        }
    }

class main{
private:
    float textureRatio = (float)winGlobals::windowStartWidth/winGlobals::windowStartHeight;
    int widthHeightResolution;
    int agentCount;

    float offsetX = 0;
    float offsetY = 0;
    float zoomMultiplier = 1;

    float sensorDistance = 60;
    float sensorAngle = 1.5;
    float turnSpeed = 2;
    float pixelMultiplier = 0.1;
    float newPixelMultiplier = 0.89;
    bool drawSensors = false;
    float* sensorColour = new float[3]{0.4f, 0.3f, 0.7f};

    float offsetX_inShader = offsetX;
    float offsetY_inShader = offsetY;
    float zoomMultiplier_inShader = zoomMultiplier;

    float sensorDistance_inShader = sensorDistance;
    float sensorAngle_inShader = sensorAngle;
    float turnSpeed_inShader = turnSpeed;
    float pixelMultiplier_inShader = pixelMultiplier;
    float newPixelMultiplier_inShader = newPixelMultiplier;
    bool drawSensors_inShader = drawSensors;
    float sensorColour_inShader[3] = {0.4f, 0.3f, 0.7f};

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

    openGLComponents::computeShader computeShader;
    struct computeShaderStruct{
        float xPos = 0;
        float yPos = 0;
        float angle = 0;
        float compatibility = 1234; // Struct layouts are stupid
        // Basically, NEVER EVER create a layout (std140, std430, whatever) that holds a vec3
        // It just wont work as expected
    };
    std::vector<computeShaderStruct> computeShaderData;
    openGLComponents::SSBO SSBO; // Above vector will be stored in this SSBO

    openGLComponents::computeShader diffuseFadeShader;
    
    void generateAgents(){
        this->computeShaderData.clear();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);
        // Fill the computeShaderData vector with data
        for(int i = 0; i < this->agentCount; i++){
            computeShaderStruct temp;
            // Pick random x and y inside 100 pixel radius of the center of the grid with size this->widthHeightResolution
            temp.xPos = dis(gen) * this->widthHeightResolution;
            temp.yPos = dis(gen) * this->widthHeightResolution;
            temp.angle = dis(gen) * 2 * 3.14159265359;
            this->computeShaderData.push_back(temp);
        }
    }

public:
    main(unsigned int n_agents=10000, unsigned int n_widthHeightResolution=1024){
        this->agentCount = n_agents;
        this->widthHeightResolution = n_widthHeightResolution;
        this->generateAgents();
    }

    ~main(){
        delete[] this->sensorColour;
        delete[] this->sensorColour_inShader;
    }
    
    void setup(){
        this->vbo.generate(this->quadVertices, this->quadVertices.size() * sizeof(float));
        this->layout.pushFloat(3);
        this->layout.pushFloat(2);
        this->vao.addBuffer(this->vbo, this->layout); // Add the buffer "vbo" that has the layout defined by "layout"
        this->texture.init(this->widthHeightResolution);
        this->texture.bind();
        this->shader.createShaderFromDisk("GLSL/quadShader.vert.glsl", "GLSL/quadShader.frag.glsl");
        this->shader.use();
        this->shader.setUniform1f("textureRatio", this->textureRatio);
        this->shader.setUniform1f("offsetX", this->offsetX_inShader);
        this->shader.setUniform1f("offsetY", this->offsetY_inShader);
        this->shader.setUniform1f("zoomMultiplier", this->zoomMultiplier_inShader);
        this->computeShader.createShaderFromDisk("GLSL/agent.compute.glsl");
        this->computeShader.use();
        this->computeShader.setUniform1i("size", this->widthHeightResolution);
        this->computeShader.setUniform1f("sensorDistance", this->sensorDistance_inShader);
        this->computeShader.setUniform1f("sensorAngle", this->sensorAngle_inShader);
        this->computeShader.setUniform1f("turnSpeed", this->turnSpeed_inShader);
        this->computeShader.setUniform1i("drawSensors", this->drawSensors_inShader);
        this->computeShader.setUniform3f("sensorColour", this->sensorColour_inShader[0], this->sensorColour_inShader[1], this->sensorColour_inShader[2]);
        this->diffuseFadeShader.createShaderFromDisk("GLSL/diffuseFade.compute.glsl");
        this->diffuseFadeShader.use();
        this->diffuseFadeShader.setUniform1f("size", this->widthHeightResolution);
        this->diffuseFadeShader.setUniform1f("pixelMultiplier", this->pixelMultiplier_inShader);
        this->diffuseFadeShader.setUniform1f("newPixelMultiplier", this->newPixelMultiplier_inShader);
        this->generateAgents();
        this->SSBO.generate(this->computeShaderData);
        this->SSBO.bind(this->computeShader.getID(), "agentData", 0);
    }

    void render(){
        this->texture.bind();
        this->diffuseFadeShader.execute(this->widthHeightResolution, this->widthHeightResolution, 1);
        this->computeShader.execute(this->computeShaderData.size(), 1, 1);
        this->shader.use();
        this->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, this->quadVertices.size() / 5);
    }

    void update(){
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(600, 240), ImGuiCond_Once);
        ImGui::Begin("Simulation");
        ImGui::SliderFloat("Sensor Distance", &this->sensorDistance, 0, 300);
        ImGui::SliderFloat("Sensor Angle", &this->sensorAngle, 0, 3.1416);
        ImGui::SliderFloat("Turn Speed", &this->turnSpeed, 0, 20);
        ImGui::SliderFloat("Pixel Multiplier", &this->pixelMultiplier, 0, 1);
        ImGui::SliderFloat("New Pixel Multiplier", &this->newPixelMultiplier, 0, 1);
        ImGui::Checkbox("Draw Sensors", &this->drawSensors);
        ImGui::ColorEdit3("Sensor Colour", this->sensorColour);
        ImGui::End();

        this->computeShader.use();

        if(this->sensorDistance_inShader != this->sensorDistance){
            this->sensorDistance_inShader = this->sensorDistance;
            this->computeShader.setUniform1f("sensorDistance", this->sensorDistance_inShader);
        }

        if(this->sensorAngle_inShader != this->sensorAngle){
            this->sensorAngle_inShader = this->sensorAngle;
            this->computeShader.setUniform1f("sensorAngle", this->sensorAngle_inShader);
        }

        if(this->turnSpeed_inShader != this->turnSpeed){
            this->turnSpeed_inShader = this->turnSpeed;
            this->computeShader.setUniform1f("turnSpeed", this->turnSpeed_inShader);
        }

        if(this->pixelMultiplier_inShader != this->pixelMultiplier){
            this->pixelMultiplier_inShader = this->pixelMultiplier;
            this->diffuseFadeShader.setUniform1f("pixelMultiplier", this->pixelMultiplier_inShader);
        }

        if(this->newPixelMultiplier_inShader != this->newPixelMultiplier){
            this->newPixelMultiplier_inShader = this->newPixelMultiplier;
            this->diffuseFadeShader.setUniform1f("newPixelMultiplier", this->newPixelMultiplier_inShader);
        }

        if(this->drawSensors_inShader != this->drawSensors){
            this->drawSensors_inShader = this->drawSensors;
            this->computeShader.setUniform1i("drawSensors", this->drawSensors_inShader);
        }

        bool sensorColourChanged = false;
        for(int i = 0; i < 3; i++){
            if(this->sensorColour_inShader[i] != this->sensorColour[i]){
                this->sensorColour_inShader[i] = this->sensorColour[i];
                sensorColourChanged = true;
            }
        }
        if(sensorColourChanged){
            this->computeShader.setUniform3f("sensorColour", this->sensorColour_inShader[0], this->sensorColour_inShader[1], this->sensorColour_inShader[2]);
        }
        

        this->shader.use();

        if(winGlobals::currentHeight != winGlobals::newHeight
        || winGlobals::currentWidth != winGlobals::newWidth){
            winGlobals::currentHeight = winGlobals::newHeight;
            winGlobals::currentWidth = winGlobals::newWidth;
            this->textureRatio = (float)winGlobals::currentWidth/winGlobals::currentHeight;
            this->shader.setUniform1f("textureRatio", this->textureRatio);
            
        }

        if(controlGlobals::scrollYOffset != 0){
            this->zoomMultiplier -= (controlGlobals::scrollYOffset / 40.0f) * this->zoomMultiplier*5;
            this->zoomMultiplier = (this->zoomMultiplier < 0)? 0 : this->zoomMultiplier;
            controlGlobals::scrollYOffset = 0;
            this->zoomMultiplier_inShader = this->zoomMultiplier;
            this->shader.setUniform1f("zoomMultiplier", this->zoomMultiplier_inShader);
        }

        if(controlGlobals::rmbClicked){
            double xr = controlGlobals::prevMouseClickXPos / controlGlobals::mouseClickXPos;
            double yr = controlGlobals::prevMouseClickYPos / controlGlobals::mouseClickYPos;
            this->offsetX += (1-xr)*this->zoomMultiplier;
            this->offsetY -= (1-yr)*this->zoomMultiplier;
            controlGlobals::rmbClicked = false;
            controlGlobals::prevMouseClickXPos = controlGlobals::mouseClickXPos;
            controlGlobals::prevMouseClickYPos = controlGlobals::mouseClickYPos;
            this->offsetX_inShader = this->offsetX;
            this->shader.setUniform1f("offsetX", this->offsetX_inShader);
            this->offsetY_inShader = this->offsetY;
            this->shader.setUniform1f("offsetY", this->offsetY_inShader);
        }

    }

};

}