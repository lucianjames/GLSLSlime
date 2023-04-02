#include <vector>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "OpenGLComponents/VAO.hpp"
#include "OpenGLComponents/VBO.hpp"
#include "OpenGLComponents/shader.hpp"
#include "OpenGLComponents/simulationTexture.hpp"


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
        layout (location = 0) in vec3 position;
        layout (location = 1) in vec2 texCoord;

        out vec2 v_texCoord;

        uniform float textureRatio;

        void main(){
            gl_Position = vec4(position, 1.0f);
            v_texCoord = texCoord;
            v_texCoord *= vec2(textureRatio, 1.0f);
        }
    )glsl";
    const char* basicFragmentShaderSource = R"glsl(
        #version 330 core
        out vec4 FragColor;
        in vec2 v_texCoord;

        uniform sampler2D textureSampler;

        void main(){
            vec4 texColor = texture(textureSampler, v_texCoord);
	        FragColor = texColor;
        }
    )glsl";

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

public:
    main(unsigned int textureResolution = 128){
        this->texture.init(textureResolution);
        this->texture.bind();
        std::cout << "Updating texture" << std::endl;
        float* data = new float[textureResolution * textureResolution * 4]; // RGBA = 4
        // Make it all white
        for(int i = 0; i < textureResolution * textureResolution * 4; i+=4){
            data[i] = 1.0f;
            data[i + 1] = i / (float)(textureResolution * textureResolution * 4);
            data[i + 2] = i%textureResolution / (float)textureResolution;
            data[i + 3] = 1.0f;
        }
        this->texture.update(data);
        delete[] data;
    }

    // Testing stuff
    void setupHelloTriangle(){
        this->shader.createShaderFromSource(this->basicVertexShaderSource, this->basicFragmentShaderSource);
        this->shader.use();
        this->shader.setUniform1f("textureRatio", this->textureRatio);
        this->vbo.generate(this->quadVertices, this->quadVertices.size() * sizeof(float));
        this->layout.pushFloat(3);
        this->layout.pushFloat(2);
        this->vao.addBuffer(this->vbo, this->layout); // Add the buffer "vbo" that has the layout defined by "layout"
    }
    
    void render(){
        this->shader.use();
        this->texture.bind();
        this->vao.bind();
        glDrawArrays(GL_TRIANGLES, 0, this->quadVertices.size() / 5);
    }

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