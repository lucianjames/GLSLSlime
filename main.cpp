#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <chrono>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <thread>

#include "debugMessageCallback.hpp"

int main(int argc, char *argv[]){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    auto window = glfwCreateWindow(800, 600, "Simulation", nullptr, nullptr);
    if(!window){
        throw std::runtime_error("Error creating glfw window");
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if(!gladLoaderLoadGL()){
        throw std::runtime_error("Error initializing glad");
    }

    glDebugMessageCallback(debug::messageCallback, nullptr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450 core");
    ImGui::StyleColorsClassic();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    while(!glfwWindowShouldClose(window)){
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool showDemo = false;
        ImGui::Begin("Example");
        if(ImGui::Button("Show/Hide ImGui demo")){
            showDemo = !showDemo;
        }
        ImGui::End();
        if(showDemo){
            ImGui::ShowDemoWindow(&showDemo);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        glClear(GL_COLOR_BUFFER_BIT);
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Not sure why this is here but someone clearly added it for a reason so ill keep it lol
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}