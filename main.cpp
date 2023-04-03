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

#define N_AGENTS 1000000
#define TEXTURE_SIZE 3000

int main(){
    /*
        ===== GLFW/GLAD/IMGUI setup
        will probably move this into some function later
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
    simulation::main sim(N_AGENTS, TEXTURE_SIZE);
    sim.setup();

    /*
        ===== Main loop
    */
    // Variables for performance tracking
    double lastFrameMicros = 0;
    while(!glfwWindowShouldClose(window)){
        auto start = std::chrono::high_resolution_clock::now();
        // ===== Process input and start a new imgui frame
        glfwPollEvents();
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){ glfwSetWindowShouldClose(window, true); }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 310), ImGuiCond_Once);
        ImGui::Begin("Performance");
        ImGui::Text("Last frame took %f ms", lastFrameMicros / 1000.0);
        ImGui::Text("FPS: %f", 1000000.0 / lastFrameMicros);
        ImGui::Text("Agents: %d", N_AGENTS);
        ImGui::Text("Texture size: %d. (Total %d pixels)", TEXTURE_SIZE, TEXTURE_SIZE * TEXTURE_SIZE);
        ImGui::End();

        // ===== Draw imgui window, update+render simulation
        sim.update();
        sim.render();

        // ===== Render imgui and swap buffers
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
        std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Not sure why this is here but someone clearly added it for a reason so ill keep it lol
        auto end = std::chrono::high_resolution_clock::now();
        lastFrameMicros = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
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