#include <visualizer/Visualizer.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <visconfig/Config.hpp>

#include <visualizer/Scene.hpp>
#include <visualizer/Shader.hpp>

GLFWwindow* createWindow(Visconfig::Config& config);

namespace Visualizer {

int run(const std::filesystem::path& configurationPath)
{
    auto config{ Visconfig::from_file(configurationPath) };

    if (!glfwInit()) {
        std::cerr << "ERROR: Could not initialize GLFW!" << std::endl;
        glfwTerminate();
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    auto window{ createWindow(config) };
    if (!window) {
        std::cerr << "ERROR: Could not create a window!" << std::endl;
        glfwTerminate();
        return 1;
    }
    glfwSetWindowAspectRatio(window, config.options.screenWidth, config.options.screenHeight);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "ERROR: Could not setup glad!" << std::endl;
        glfwTerminate();
        return 1;
    }

    auto scene{ initializeScene(config) };

    while (!glfwWindowShouldClose(window)) {
        tick(scene);
        draw(scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
}

GLFWwindow* createWindow(Visconfig::Config& config)
{
    auto windowName{ "Visualizer" };
    auto monitor{ glfwGetPrimaryMonitor() };
    auto videoMode{ glfwGetVideoMode(monitor) };

    if (config.options.screenWidth == 0 || config.options.screenHeight == 0) {
        config.options.screenWidth = videoMode->width;
        config.options.screenHeight = videoMode->height;
    }

    auto screenWidth{ config.options.screenWidth };
    auto screenHeight{ config.options.screenHeight };

    if (config.options.screenFullscreen) {
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