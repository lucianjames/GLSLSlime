#include <vector>
#include <random>
#include <chrono>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <opencv2/opencv.hpp>

#include "OpenGLComponents/VAO.hpp"
#include "OpenGLComponents/VBO.hpp"
#include "OpenGLComponents/shader.hpp"
#include "OpenGLComponents/simulationTexture.hpp"
#include "OpenGLComponents/computeShader.hpp"
#include "OpenGLComponents/SSBO.hpp"

// ! Important, must be same as in diffuseFade.compute.glsl
#define DF_GROUPSIZE 32
#define AG_GROUPSIZE 1024

namespace simulation{

    /*
        Global variables for managing window size updates
    */
    namespace winGlobals{
        const int windowStartWidth = 1920;
        const int windowStartHeight = 1080;
        int currentWidth = windowStartWidth;
        int currentHeight = windowStartHeight;
        int newWidth = windowStartWidth;
        int newHeight = windowStartHeight;
    }

    /*
        These variables are used to control the simulation
        They have to be global because they are accessed by both the callbacks and the simulation class
    */
    namespace controlGlobals{
        double scrollYOffset = 0;
        double mouseClickXPos = 0;
        double mouseClickYPos = 0;
        double prevMouseClickXPos = -1;
        double prevMouseClickYPos = -1;
        bool rmbClicked = false;
    }

    /*
        These callbacks are set in main
    */
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
private: // ==================================================== PRIVATE ====================================================
    /*
        Window/texture/agent count
    */
    float textureRatio = (float)winGlobals::windowStartWidth/winGlobals::windowStartHeight;
    int agentCount;
    int widthHeightResolution;
    int widthHeightResolution_current;


    /*
        Controls
    */
    float offsetX = 0;
    float offsetY = 0;
    float zoomMultiplier = 1;
    float offsetX_inShader = offsetX; // These three are passed to the vertex shader in quadShader.vert.glsl
    float offsetY_inShader = offsetY;
    float zoomMultiplier_inShader = zoomMultiplier;


    /*
        Simulation settings (excl agent count and tex size, these are just settings that dont require a reset)
    */
    float sensorDistance = 60;
    float sensorAngle = 1.5;
    float turnSpeed = 2;
    float speed = 1;
    bool drawSensors = false;
    float diffuse = 0.7;
    float fade = 0.1;
    float sensorDistance_inShader = sensorDistance; // This and the following four are passed to agent.compute.glsl
    float sensorAngle_inShader = sensorAngle;
    float turnSpeed_inShader = turnSpeed;
    float speed_inShader = speed;
    bool drawSensors_inShader = drawSensors;
    float diffuse_inShader = diffuse; // This is and fade are passed to diffuseFade.compute.glsl
    float fade_inShader = fade;


    /*
        Pretty colours
    */
    // These need to be pointers because they are passed to ImGUI
    float* mainAgentColour = new float[3]{0.0f, 0.1f, 0.9f};
    float* agentXDirectionColour = new float[3]{0.0f, 0.7f, 0.2f};
    float* agentYDirectionColour = new float[3]{0.0f, 0.1f, 0.8f};
    float* sensorColour = new float[3]{0.8f, 0.1f, 0.9f};
    // These dont have to be pointers because they are only used to set shader uniforms
    // Their value is copied from the pointers above
    float mainAgentColour_inShader[3];
    float agentXDirectionColour_inShader[3];
    float agentYDirectionColour_inShader[3];
    float sensorColour_inShader[3];


    /*
        Animation rendering
    */
    bool renderFrames = false;
    int animFrameCount = 0;
    int renderedFrameCount = 0;
    int frameInterval = 1;


    /*
        Geometry
        positions (3) + texture coords (2), total 5 floats per vertex
        First three sets of 5 are the first triangle, second three sets of 5 are the second triangle
    */
    std::vector<float> quadVertices = { // The quad which the simulation texture is rendered on to
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f,
         1.0f, -1.0f, 0.0f,    1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,    1.0f, 1.0f,
         1.0f,  1.0f, 0.0f,    1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f,    0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,    0.0f, 0.0f

    };


    /*
        OpenGL components and agent data
    */
    openGLComponents::VAO vao;
    openGLComponents::VBO vbo;
    openGLComponents::VBOLayout layout;
    openGLComponents::shader shader;
    openGLComponents::simulationTexture simTexture;
    openGLComponents::computeShader agentComputeShader;
    openGLComponents::computeShader diffuseFadeShader;
    struct computeShaderStruct{
        float xPos = 0;
        float yPos = 0;
        float angle = 0;
        float compatibility = 1234; // Struct layouts are stupid, cant send vec3 to shader, so have to make it a vec4.
        // I might add something to the simulation that uses this float for something cool, but for now it just acts as padding to make the struct work properly
    };
    std::vector<computeShaderStruct> agentData;
    openGLComponents::SSBO SSBO; // Above vector will be stored in this SSBO


    /*
        Useful functions
    */
    void generateAgents(){ // Fills this->agentData with some randomly generated (but valid) data for the simulation to start with
        this->agentData.clear();
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1);
        for(int i = 0; i < this->agentCount; i++){
            computeShaderStruct temp;
            temp.xPos = dis(gen) * this->widthHeightResolution_current; // Random position across the whole texture
            temp.yPos = dis(gen) * this->widthHeightResolution_current;
            temp.angle = dis(gen) * 2 * 3.14159265359;
            this->agentData.push_back(temp);
        }
    }

    template<typename T> bool arryCmp(T* arr1, T* arr2, int size){
        for(int i = 0; i < size; i++){
            if(arr1[i] != arr2[i]){
                return false;
            }
        }
        return true;
    }


    /*
        These functions helps get rid of some repetitive code in the update function
    */
    void checkSet1f_compute(const char* name, float& value, float& value_inShader, openGLComponents::computeShader& shader){
        if(value != value_inShader){
            shader.setUniform1f(name, value);
            value_inShader = value;
        }
    }

    void checkSet1i_compute(const char* name, bool& value, bool& value_inShader, openGLComponents::computeShader& shader){
        if(value != value_inShader){
            shader.setUniform1i(name, value);
            value_inShader = value;
        }
    }

    void checkSet3f_compute(const char* name, float* value, float* value_inShader, openGLComponents::computeShader& shader){
        if(!this->arryCmp(value_inShader, value, 3)){
            for(int i = 0; i < 3; i++){
                value_inShader[i] = value[i];
            }
            shader.setUniform3f(name, value_inShader[0], value_inShader[1], value_inShader[2]);
        }
    }
    

public: // ==================================================== PUBLIC ====================================================
    /*
        Constructor/destructor
    */
    main(unsigned int n_agents=10000, unsigned int n_widthHeightResolution=1024){
        this->agentCount = n_agents;
        this->widthHeightResolution = n_widthHeightResolution;
        // Setting the inShader values to the values of the pointers, as it is assumed that they will be set very soon by this->setup():
        for(int i = 0; i < 3; i++){ 
            this->mainAgentColour_inShader[i] = this->mainAgentColour[i];
            this->agentXDirectionColour_inShader[i] = this->agentXDirectionColour[i];
            this->agentYDirectionColour_inShader[i] = this->agentYDirectionColour[i];
            this->sensorColour_inShader[i] = this->sensorColour[i];
        }
    }

    ~main(){
        // Free the pesky pointers
        delete[] this->sensorColour;
        delete[] this->mainAgentColour;
        delete[] this->agentXDirectionColour;
        delete[] this->agentYDirectionColour;
    }
    

    /*
        Creates all the opengl objects and sets up the starting data and parameters for the simulation
    */
    void setup(){
        // vbo+layout+vao to render the quad
        this->vbo.generate(this->quadVertices, this->quadVertices.size() * sizeof(float));
        this->layout.pushFloat(3);
        this->layout.pushFloat(2);
        this->vao.addBuffer(this->vbo, this->layout); // Add the buffer "vbo" that has the layout defined by "layout"
        
        // Create the texture to render the simulation on to
        this->widthHeightResolution_current = this->widthHeightResolution;
        this->simTexture.init(this->widthHeightResolution_current);
        this->simTexture.clear();
        this->simTexture.bind();

        // Create the shader program to render the quad
        this->shader.createShaderFromDisk("GLSL/quadShader.vert.glsl", "GLSL/quadShader.frag.glsl");
        this->shader.use();
        this->shader.setUniform1f("textureRatio", this->textureRatio);
        this->shader.setUniform1f("offsetX", this->offsetX_inShader);
        this->shader.setUniform1f("offsetY", this->offsetY_inShader);
        this->shader.setUniform1f("zoomMultiplier", this->zoomMultiplier_inShader);
        
        // Create the compute shader to simulate the agents
        this->agentComputeShader.createShaderFromDisk("GLSL/agent.compute.glsl");
        this->agentComputeShader.use();
        this->agentComputeShader.setUniform1i("size", this->widthHeightResolution_current);
        this->agentComputeShader.setUniform1f("sensorDistance", this->sensorDistance_inShader);
        this->agentComputeShader.setUniform1f("sensorAngle", this->sensorAngle_inShader);
        this->agentComputeShader.setUniform1f("turnSpeed", this->turnSpeed_inShader);
        this->agentComputeShader.setUniform1f("speed", this->speed_inShader);
        this->agentComputeShader.setUniform1i("drawSensors", this->drawSensors_inShader);
        this->agentComputeShader.setUniform3f("sensorColour", this->sensorColour_inShader[0], this->sensorColour_inShader[1], this->sensorColour_inShader[2]);
        this->agentComputeShader.setUniform3f("mainAgentColour", this->mainAgentColour_inShader[0], this->mainAgentColour_inShader[1], this->mainAgentColour_inShader[2]);
        this->agentComputeShader.setUniform3f("agentXDirectionColour", this->agentXDirectionColour_inShader[0], this->agentXDirectionColour_inShader[1], this->agentXDirectionColour_inShader[2]);
        this->agentComputeShader.setUniform3f("agentYDirectionColour", this->agentYDirectionColour_inShader[0], this->agentYDirectionColour_inShader[1], this->agentYDirectionColour_inShader[2]);
        
        // Create the compute shader to diffuse and fade the texture over time
        this->diffuseFadeShader.createShaderFromDisk("GLSL/diffuseFade.compute.glsl");
        this->diffuseFadeShader.use();
        this->diffuseFadeShader.setUniform1f("size", this->widthHeightResolution_current);
        this->diffuseFadeShader.setUniform1f("diffuse", this->diffuse_inShader);
        this->diffuseFadeShader.setUniform1f("fade", this->fade_inShader);
        
        // Generate starting agent data, create an SSBO from it, and bind it to the compute shader
        this->generateAgents();
        this->SSBO.generate(this->agentData);
        this->SSBO.bind(this->agentComputeShader.getID(), "agentData", 0);
    }


    /*
        Reset the simulation back to a starting state
        This assumes that setup() has already been called
    */
    void restart(){
        // Reset the texture
        this->widthHeightResolution_current = this->widthHeightResolution;
        this->simTexture.destroy();
        this->simTexture.init(this->widthHeightResolution_current);
        this->simTexture.clear();

        // Reset agent SSBO
        this->generateAgents();
        this->SSBO.generate(this->agentData);
        this->SSBO.bind(this->agentComputeShader.getID(), "agentData", 0);

        // Ensure that the size uniform in both of the compute shaders is set to the correct value
        this->diffuseFadeShader.use();
        this->diffuseFadeShader.setUniform1f("size", this->widthHeightResolution_current);
        this->agentComputeShader.use();
        this->agentComputeShader.setUniform1i("size", this->widthHeightResolution_current);
    }


    /*
        Perform a single step of the simulation
    */
    void step(){
        this->simTexture.bind();
        this->diffuseFadeShader.execute((this->widthHeightResolution_current+DF_GROUPSIZE-1)/DF_GROUPSIZE, (this->widthHeightResolution_current+DF_GROUPSIZE-1)/DF_GROUPSIZE, 1);
        this->agentComputeShader.execute(this->agentData.size()/AG_GROUPSIZE, 1, 1);
    }


    /*
        Render the quad and texture
    */
    void render(){
        this->simTexture.bind();
        this->shader.use();
        this->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, this->quadVertices.size() / 5);
    }


    /*
        ImGUI + setting various uniforms based on the values in the ImGUI window
    */
    void update(){
        // Draw the ImGui window for the simulation settings
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(600, 520), ImGuiCond_Always);
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
        if(ImGui::Button("Toggle texture repeat")){
            this->simTexture.toggleRepeat();
        }
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Checkbox("Render frames to disk", &this->renderFrames);
        ImGui::SliderInt("Frame interval", &this->frameInterval, 0, 10);
        ImGui::Dummy(ImVec2(0, 10));
        ImGui::Text("Restart required for the following settings:");
        ImGui::SliderInt("Agent Count", &this->agentCount, 0, 5000000);
        ImGui::SliderInt("Texture Resolution", &this->widthHeightResolution, 0, 4096*2);
        ImGui::Dummy(ImVec2(0, 10));
        if(ImGui::Button("Restart")){
            this->restart();
        }
        ImGui::End();

        // Draw the window for displaying info
        ImGui::SetNextWindowPos(ImVec2(0, 520), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(600, 340), ImGuiCond_Always);
        ImGui::Begin("Info");
        ImGui::Text("FPS: %f", ImGui::GetIO().Framerate);
        ImGui::Text("Rendered Frames: %d", this->renderedFrameCount);
        ImGui::Text("Anim Frames: %d", this->animFrameCount);
        ImGui::Text("OffsetX_inShader: %f", this->offsetX_inShader);
        ImGui::Text("OffsetY_inShader: %f", this->offsetY_inShader);
        ImGui::Text("ZoomMultiplier_inShader: %f", this->zoomMultiplier_inShader);
        ImGui::Text("SensorDistance_inShader: %f", this->sensorDistance_inShader);
        ImGui::Text("SensorAngle_inShader: %f", this->sensorAngle_inShader);
        ImGui::Text("TurnSpeed_inShader: %f", this->turnSpeed_inShader);
        ImGui::Text("Speed_inShader: %f", this->speed_inShader);
        ImGui::Text("DrawSensors_inShader: %d", this->drawSensors_inShader);
        ImGui::Text("Diffuse_inShader: %f", this->diffuse_inShader);
        ImGui::Text("Fade_inShader: %f", this->fade_inShader);
        ImGui::Text("MainAgentColour_inShader: %f, %f, %f", this->mainAgentColour_inShader[0], this->mainAgentColour_inShader[1], this->mainAgentColour_inShader[2]);
        ImGui::Text("AgentXDirectionColour_inShader: %f, %f, %f", this->agentXDirectionColour_inShader[0], this->agentXDirectionColour_inShader[1], this->agentXDirectionColour_inShader[2]);
        ImGui::Text("AgentYDirectionColour_inShader: %f, %f, %f", this->agentYDirectionColour_inShader[0], this->agentYDirectionColour_inShader[1], this->agentYDirectionColour_inShader[2]);
        ImGui::Text("SensorColour_inShader: %f, %f, %f", this->sensorColour_inShader[0], this->sensorColour_inShader[1], this->sensorColour_inShader[2]);
        ImGui::End();

        // Check if any of the uniforms need to be updated, and if so, update them
        this->checkSet1f_compute("sensorDistance", this->sensorDistance, this->sensorDistance_inShader, this->agentComputeShader);
        this->checkSet1f_compute("sensorAngle", this->sensorAngle, this->sensorAngle_inShader, this->agentComputeShader);
        this->checkSet1f_compute("turnSpeed", this->turnSpeed, this->turnSpeed_inShader, this->agentComputeShader);
        this->checkSet1f_compute("diffuse", this->diffuse, this->diffuse_inShader, this->diffuseFadeShader);
        this->checkSet1f_compute("fade", this->fade, this->fade_inShader, this->diffuseFadeShader);
        this->checkSet1f_compute("speed", this->speed, this->speed_inShader, this->agentComputeShader);
        this->checkSet3f_compute("mainAgentColour", this->mainAgentColour, this->mainAgentColour_inShader, this->agentComputeShader);
        this->checkSet3f_compute("agentXDirectionColour", this->agentXDirectionColour, this->agentXDirectionColour_inShader, this->agentComputeShader);
        this->checkSet3f_compute("agentYDirectionColour", this->agentYDirectionColour, this->agentYDirectionColour_inShader, this->agentComputeShader);
        this->checkSet1i_compute("drawSensors", this->drawSensors, this->drawSensors_inShader, this->agentComputeShader);
        this->checkSet3f_compute("sensorColour", this->sensorColour, this->sensorColour_inShader, this->agentComputeShader);

        // Check if any input needs to be processed
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

        // Check if window size has changed, and if so, update the texture ratio to ensure it doesnt get distorted
        if(winGlobals::currentHeight != winGlobals::newHeight || winGlobals::currentWidth != winGlobals::newWidth){
            winGlobals::currentHeight = winGlobals::newHeight;
            winGlobals::currentWidth = winGlobals::newWidth;
            this->textureRatio = (float)winGlobals::currentWidth/winGlobals::currentHeight;
            this->shader.setUniform1f("textureRatio", this->textureRatio);
        }

        if(this->renderFrames && this->renderedFrameCount % this->frameInterval == 0){
            float* pixels = this->simTexture.getTexImage();
            cv::Mat img(this->widthHeightResolution_current, this->widthHeightResolution_current, CV_32FC4, pixels);
            img *= 255;
            cv::cvtColor(img, img, cv::COLOR_RGBA2BGRA);
            cv::imwrite("animFrame_" + std::to_string(this->animFrameCount) + ".png", img);
            this->animFrameCount++;
            free(pixels);
        }
        this->renderedFrameCount++;        
    }
    
};

}