#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

class simulationSettings{
private:
    int widthHeightResolution = 1024;
    float offsetX = 0;
    float offsetY = 0;
    float zoomMultiplier = 1;
    float sensorDistance = 1;
    float sensorAngle = 1;
    float turnSpeed = 1;
    float pixelMultiplier = 0.1;
    float newPixelMultiplier = 0.89;

public:

    /*
        This is just a dummy menu for testing purposes
        Ill set things up so that all the settings actually get retrieved once ive got some basic rendering working
    */
    void draw(){
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