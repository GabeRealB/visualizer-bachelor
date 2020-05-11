#include <visualizer/Visualizer.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <iostream>

#include <visualizer/Scene.hpp>
#include <visualizer/SceneObject.hpp>
#include <visualizer/VisualizerConfiguration.hpp>

GLFWwindow* createWindow(const Visualizer::VisualizerConfiguration& config);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

namespace Visualizer {

struct t {
    int a;
};

int run(const std::filesystem::path& configurationPath)
{
    auto config{ loadConfig(configurationPath) };
    if (!config) {
        std::cerr << "ERROR: Could not load the configuration file!" << std::endl;
        return 1;
    }

    if (!glfwInit()) {
        std::cerr << "ERROR: Could not initialize GLFW!" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    auto window{ createWindow(*config) };
    if (!window) {
        std::cerr << "ERROR: Could not create a window!" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "ERROR: Could not setup glad!" << std::endl;
        glfwTerminate();
        return 1;
    }

    auto scene{ loadScene(*config) };
    if (!scene) {
        std::cerr << "ERROR: Could not load the scene!" << std::endl;
        glfwTerminate();
        return 1;
    }

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

}

GLFWwindow* createWindow(const Visualizer::VisualizerConfiguration& config)
{
    auto screenWidth{ config.resolution[0] };
    auto screenHeight{ config.resolution[1] };
    auto fullscreen{ config.fullscreen };
    auto windowName{ "Visualizer" };
    auto monitor{ glfwGetPrimaryMonitor() };
    auto videoMode{ glfwGetVideoMode(monitor) };

    if (screenWidth == 0 || screenHeight == 0) {
        screenWidth = videoMode->width;
        screenHeight = videoMode->height;
    }

    if (fullscreen) {
        return glfwCreateWindow(screenWidth, screenHeight, windowName, monitor, nullptr);
    } else {
        glfwWindowHint(GLFW_RED_BITS, videoMode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, videoMode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, videoMode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, videoMode->refreshRate);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        return glfwCreateWindow(screenWidth, screenHeight, windowName, nullptr, nullptr);
    }
}

void framebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}
