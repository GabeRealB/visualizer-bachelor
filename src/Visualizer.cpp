#include <visualizer/Visualizer.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <visualizer/Scene.hpp>
#include <visualizer/Shader.hpp>

GLFWwindow* createWindow(Visualizer::VisualizerConfiguration& config);

namespace Visualizer {

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
    glfwSetWindowAspectRatio(window, config->resolution[0], config->resolution[1]);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

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
        tick(*scene);
        draw(*scene);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
}

GLFWwindow* createWindow(Visualizer::VisualizerConfiguration& config)
{
    auto windowName{ "Visualizer" };
    auto monitor{ glfwGetPrimaryMonitor() };
    auto videoMode{ glfwGetVideoMode(monitor) };

    if (config.resolution[0] == 0 || config.resolution[1] == 0) {
        config.resolution[0] = videoMode->width;
        config.resolution[1] = videoMode->height;
    }

    auto screenWidth{ config.resolution[0] };
    auto screenHeight{ config.resolution[1] };

    if (config.fullscreen) {
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

std::shared_ptr<Visualizer::ShaderProgram> createShaderProgram(
    const std::filesystem::path& vs, const std::filesystem::path& fs)
{
    auto vertexShader{ Visualizer::Shader::create(vs, Visualizer::ShaderType::VertexShader) };
    if (!vertexShader) {
        return nullptr;
    }

    auto fragmentShader{ Visualizer::Shader::create(fs, Visualizer::ShaderType::FragmentShader) };
    if (!fragmentShader) {
        return nullptr;
    }

    return Visualizer::ShaderProgram::create(*vertexShader, *fragmentShader);
}