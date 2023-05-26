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

#define N_AGENTS 100000
#define TEXTURE_SIZE 1024
#define DEBUG true

int main(){
    /*
        ===== GLFW/GLAD/IMGUI setup
    */
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    auto window = glfwCreateWindow(simulation::winGlobals::windowStartWidth, simulation::winGlobals::windowStartHeight, "Simulation", nullptr, nullptr);
    if(!window){
        throw std::runtime_error("Error creating glfw window");
    }
    glfwSetFramebufferSizeCallback(window, simulation::callbacks::framebufferSizeCallback);
    glfwSetScrollCallback(window, simulation::callbacks::scrollCallback);
    glfwSetCursorPosCallback(window, simulation::callbacks::cursorPositionCallback);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    if(!gladLoaderLoadGL()){
        throw std::runtime_error("Error initializing glad");
    }
    if(DEBUG){
        GLCall(glEnable(GL_DEBUG_OUTPUT));
        GLCall(glDebugMessageCallback(debug::messageCallback, nullptr));
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");
    ImGui::StyleColorsClassic();
    GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));

    /*
        ===== Simulation setup
    */
    simulation::main sim(N_AGENTS, TEXTURE_SIZE);
    sim.setup();

    /*
        ===== Main loop
    */
    while(!glfwWindowShouldClose(window)){
        // ===== Process input and start a new imgui frame
        glfwPollEvents();
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){ glfwSetWindowShouldClose(window, true); }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ===== Draw imgui window, update+render simulation
        sim.update(); // imgui, window, etc
        sim.step(); // Runs compute shaders to update agents
        sim.render(); // Draws quad with simulation texture

        // ===== Render imgui and swap buffers
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
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