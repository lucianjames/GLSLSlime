#define GLFW_INCLUDE_NONE

#include <GLFW/glfw3.h>
#include <chrono>
#include <glad/gl.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <thread>

#include "misc/debugMessageCallback.hpp"
#include "simulation/simulation.hpp"

int main(){
    /*
        ===== GLFW/GLAD/IMGUI setup
        will probably move this into some function later
    */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    auto window = glfwCreateWindow(simulation::globalsForBufferSizeCallback::windowStartWidth, simulation::globalsForBufferSizeCallback::windowStartHeight, "Hello Triangle", nullptr, nullptr);
    if(!window){
        throw std::runtime_error("Error creating glfw window");
    }
    glfwSetFramebufferSizeCallback(window, simulation::framebufferSizeCallback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if(!gladLoaderLoadGL()){
        throw std::runtime_error("Error initializing glad");
    }
    GLCall(glDebugMessageCallback(debug::messageCallback, nullptr));
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 450 core");
    ImGui::StyleColorsClassic();
    GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));

    /*
        ===== Simulation setup
    */
    simulation::main sim;
    sim.setupHelloTriangle();


    /*
        ===== Main loop
    */
    while(!glfwWindowShouldClose(window)){
        // ===== Process input and start a new imgui frame
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ===== Draw imgui window, update+render simulation
        sim.render();
        sim.update(); // just draws imgui dummy menu stuff for now

        // ===== Render imgui and swap buffers
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Not sure why this is here but someone clearly added it for a reason so ill keep it lol
    }

    /*
        ===== Cleanup
    */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}