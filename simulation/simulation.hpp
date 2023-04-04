#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <memory>

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
    int widthHeightResolution_current;
    int agentCount;

    float offsetX = 0;
    float offsetY = 0;
    float zoomMultiplier = 1;

    float sensorDistance = 60;
    float sensorAngle = 1.5;
    float turnSpeed = 2;
    float diffuse = 0.7;
    float fade = 0.1;
    float speed = 1;
    bool drawSensors = false;
    float* mainAgentColour = new float[3]{0.0f, 0.1f, 0.9f};
    float* agentXDirectionColour = new float[3]{0.0f, 0.7f, 0.2f};
    float* agentYDirectionColour = new float[3]{0.0f, 0.1f, 0.8f};
    float* sensorColour = new float[3]{0.8f, 0.1f, 0.9f};

    float offsetX_inShader = offsetX;
    float offsetY_inShader = offsetY;
    float zoomMultiplier_inShader = zoomMultiplier;

    float sensorDistance_inShader = sensorDistance;
    float sensorAngle_inShader = sensorAngle;
    float turnSpeed_inShader = turnSpeed;
    float diffuse_inShader = diffuse;
    float fade_inShader = fade;
    float speed_inShader = speed;
    bool drawSensors_inShader = drawSensors;
    float mainAgentColour_inShader[3];
    float agentXDirectionColour_inShader[3];
    float agentYDirectionColour_inShader[3];
    float sensorColour_inShader[3];

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
        std::cout << "Generating agents" << std::endl;
        this->computeShaderData.clear();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);
        // Fill the computeShaderData vector with data
        for(int i = 0; i < this->agentCount; i++){
            computeShaderStruct temp;
            // Pick random x and y inside 100 pixel radius of the center of the grid with size this->widthHeightResolution
            temp.xPos = dis(gen) * this->widthHeightResolution_current;
            temp.yPos = dis(gen) * this->widthHeightResolution_current;
            temp.angle = dis(gen) * 2 * 3.14159265359;
            this->computeShaderData.push_back(temp);
        }
    }

    template<typename T>
    bool arryCmp(T* arr1, T* arr2, int size){
        for(int i = 0; i < size; i++){
            if(arr1[i] != arr2[i]){
                return false;
            }
        }
        return true;
    }

    void checkSet1f(const char* name, float& value, float& value_inShader, openGLComponents::computeShader& shader){
        this->shader.use();
        if(value != value_inShader){
            shader.setUniform1f(name, value);
            value_inShader = value;
        }
    }

public:
    main(unsigned int n_agents=10000, unsigned int n_widthHeightResolution=1024){
        this->agentCount = n_agents;
        this->widthHeightResolution = n_widthHeightResolution;
        for(int i = 0; i < 3; i++){
            this->mainAgentColour_inShader[i] = this->mainAgentColour[i];
            this->agentXDirectionColour_inShader[i] = this->agentXDirectionColour[i];
            this->agentYDirectionColour_inShader[i] = this->agentYDirectionColour[i];
            this->sensorColour_inShader[i] = this->sensorColour[i];
        }
        this->generateAgents();
    }

    ~main(){
        delete[] this->sensorColour;
        delete[] this->mainAgentColour;
        delete[] this->agentXDirectionColour;
        delete[] this->agentYDirectionColour;
    }
    
    void setup(){
        this->vbo.generate(this->quadVertices, this->quadVertices.size() * sizeof(float));
        this->layout.pushFloat(3);
        this->layout.pushFloat(2);
        this->vao.addBuffer(this->vbo, this->layout); // Add the buffer "vbo" that has the layout defined by "layout"
        this->widthHeightResolution_current = this->widthHeightResolution;
        this->texture.init(this->widthHeightResolution_current);
        this->texture.clear();
        this->texture.bind();
        this->shader.createShaderFromDisk("GLSL/quadShader.vert.glsl", "GLSL/quadShader.frag.glsl");
        this->shader.use();
        this->shader.setUniform1f("textureRatio", this->textureRatio);
        this->shader.setUniform1f("offsetX", this->offsetX_inShader);
        this->shader.setUniform1f("offsetY", this->offsetY_inShader);
        this->shader.setUniform1f("zoomMultiplier", this->zoomMultiplier_inShader);
        this->computeShader.createShaderFromDisk("GLSL/agent.compute.glsl");
        this->computeShader.use();
        this->computeShader.setUniform1i("size", this->widthHeightResolution_current);
        this->computeShader.setUniform1f("sensorDistance", this->sensorDistance_inShader);
        this->computeShader.setUniform1f("sensorAngle", this->sensorAngle_inShader);
        this->computeShader.setUniform1f("turnSpeed", this->turnSpeed_inShader);
        this->computeShader.setUniform1f("speed", this->speed_inShader);
        this->computeShader.setUniform1i("drawSensors", this->drawSensors_inShader);
        this->computeShader.setUniform3f("sensorColour", this->sensorColour_inShader[0], this->sensorColour_inShader[1], this->sensorColour_inShader[2]);
        this->computeShader.setUniform3f("mainAgentColour", this->mainAgentColour_inShader[0], this->mainAgentColour_inShader[1], this->mainAgentColour_inShader[2]);
        this->computeShader.setUniform3f("agentXDirectionColour", this->agentXDirectionColour_inShader[0], this->agentXDirectionColour_inShader[1], this->agentXDirectionColour_inShader[2]);
        this->computeShader.setUniform3f("agentYDirectionColour", this->agentYDirectionColour_inShader[0], this->agentYDirectionColour_inShader[1], this->agentYDirectionColour_inShader[2]);
        this->diffuseFadeShader.createShaderFromDisk("GLSL/diffuseFade.compute.glsl");
        this->diffuseFadeShader.use();
        this->diffuseFadeShader.setUniform1f("size", this->widthHeightResolution_current);
        this->diffuseFadeShader.setUniform1f("diffuse", this->diffuse_inShader);
        this->diffuseFadeShader.setUniform1f("fade", this->fade_inShader);
        this->generateAgents();
        this->SSBO.generate(this->computeShaderData);
        this->SSBO.bind(this->computeShader.getID(), "agentData", 0);
    }

    void restart(){
        this->widthHeightResolution_current = this->widthHeightResolution;
        this->texture.destroy();
        this->texture.init(this->widthHeightResolution_current);
        this->texture.clear();
        this->generateAgents();
        this->SSBO.generate(this->computeShaderData);
        this->SSBO.bind(this->computeShader.getID(), "agentData", 0);
        this->diffuseFadeShader.use();
        this->diffuseFadeShader.setUniform1f("size", this->widthHeightResolution_current);
        this->computeShader.use();
        this->computeShader.setUniform1i("size", this->widthHeightResolution_current);
    }

    void render(){
        this->texture.bind();
        this->diffuseFadeShader.execute(this->widthHeightResolution_current, this->widthHeightResolution_current, 1);
        this->computeShader.execute(this->computeShaderData.size(), 1, 1);
        this->shader.use();
        this->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, this->quadVertices.size() / 5);
    }

    void update(){
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(600, 420), ImGuiCond_Always);
        ImGui::Begin("Simulation");
        ImGui::SliderFloat("Sensor Distance", &this->sensorDistance, 0, 300);
        ImGui::SliderFloat("Sensor Angle", &this->sensorAngle, 0, 3.1416);
        ImGui::SliderFloat("Turn Speed", &this->turnSpeed, 0, 5);
        ImGui::SliderFloat("Speed", &this->speed, 0.01, 25);
        ImGui::SliderFloat("Diffuse", &this->diffuse, 0, 1);
        ImGui::SliderFloat("Fade", &this->fade, 0, 0.2);
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::ColorEdit3("Main Agent Colour", this->mainAgentColour);
        ImGui::ColorEdit3("Agent X Direction Colour", this->agentXDirectionColour);
        ImGui::ColorEdit3("Agent Y Direction Colour", this->agentYDirectionColour);
        ImGui::Checkbox("Draw Sensors", &this->drawSensors);
        ImGui::ColorEdit3("Sensor Colour", this->sensorColour);
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Text("Restart required for the following settings:");
        ImGui::SliderInt("Agent Count", &this->agentCount, 0, 5000000);
        ImGui::SliderInt("Texture Resolution", &this->widthHeightResolution, 0, 4096*2);
        ImGui::Dummy(ImVec2(0, 10));
        if(ImGui::Button("Restart")){
            this->restart();
        }
        ImGui::End();

        this->checkSet1f("sensorDistance", this->sensorDistance, this->sensorDistance_inShader, this->computeShader);
        this->checkSet1f("sensorAngle", this->sensorAngle, this->sensorAngle_inShader, this->computeShader);
        this->checkSet1f("turnSpeed", this->turnSpeed, this->turnSpeed_inShader, this->computeShader);
        this->checkSet1f("diffuse", this->diffuse, this->diffuse_inShader, this->diffuseFadeShader);
        this->checkSet1f("fade", this->fade, this->fade_inShader, this->diffuseFadeShader);
        this->checkSet1f("speed", this->speed, this->speed_inShader, this->computeShader);

        if(!arryCmp(this->mainAgentColour_inShader, this->mainAgentColour, 3)){
            for(int i = 0; i < 3; i++){
                this->mainAgentColour_inShader[i] = this->mainAgentColour[i];
            }
            this->computeShader.setUniform3f("mainAgentColour", this->mainAgentColour_inShader[0], this->mainAgentColour_inShader[1], this->mainAgentColour_inShader[2]);
        }

        if(!arryCmp(this->agentXDirectionColour_inShader, this->agentXDirectionColour, 3)){
            for(int i = 0; i < 3; i++){
                this->agentXDirectionColour_inShader[i] = this->agentXDirectionColour[i];
            }
            this->computeShader.setUniform3f("agentXDirectionColour", this->agentXDirectionColour_inShader[0], this->agentXDirectionColour_inShader[1], this->agentXDirectionColour_inShader[2]);
        }

        if(!arryCmp(this->agentYDirectionColour_inShader, this->agentYDirectionColour, 3)){
            for(int i = 0; i < 3; i++){
                this->agentYDirectionColour_inShader[i] = this->agentYDirectionColour[i];
            }
            this->computeShader.setUniform3f("agentYDirectionColour", this->agentYDirectionColour_inShader[0], this->agentYDirectionColour_inShader[1], this->agentYDirectionColour_inShader[2]);
        }
        
        if(this->drawSensors_inShader != this->drawSensors){
            this->drawSensors_inShader = this->drawSensors;
            this->computeShader.setUniform1i("drawSensors", this->drawSensors_inShader);
        }
        
        if(!arryCmp(this->sensorColour_inShader, this->sensorColour, 3)){
            for(int i = 0; i < 3; i++){
                this->sensorColour_inShader[i] = this->sensorColour[i];
            }
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