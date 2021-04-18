#include <visualizer/Visualizer.hpp>

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <filesystem>
#include <fstream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

#include <visconfig/Config.hpp>
#include <visualizer/AssetDatabase.hpp>
#include <visualizer/Scene.hpp>

GLFWwindow* createWindow(Visconfig::Config& config);
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
    const GLchar* message, const void* userParam);

namespace Visualizer {

GLFWwindow* g_window{ nullptr };
bool g_shouldQuit{ false };
bool g_detached{ false };
std::size_t g_freeze_counter{ 0 };

void quit() { g_shouldQuit = true; }

bool shouldQuit() { return g_shouldQuit || glfwWindowShouldClose(g_window); }

void attach(bool attached)
{
    static double lastPosX{ 0.0L };
    static double lastPosY{ 0.0L };

    if (attached) {
        glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(g_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        glfwSetCursorPos(g_window, lastPosX, lastPosY);
    } else {
        glfwGetCursorPos(g_window, &lastPosX, &lastPosY);

        glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        if (glfwRawMouseMotionSupported()) {
            glfwSetInputMode(g_window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
        }

        GLint width;
        GLint height;
        glfwGetWindowSize(g_window, &width, &height);
        glfwSetCursorPos(g_window, width / 2.0, height / 2.0);
    }
    g_detached = !attached;
}

void freeze(bool frozen)
{
    if (frozen) {
        g_freeze_counter++;
    } else {
        if (g_freeze_counter != 0) {
            g_freeze_counter--;
        }
    }
}

bool isDetached() { return g_detached; }

bool is_frozen() { return g_freeze_counter != 0; }

void getRelativeMousePosition(double& xPos, double& yPos)
{
    double x;
    double y;
    glfwGetCursorPos(g_window, &x, &y);

    GLint width;
    GLint height;
    glfwGetWindowSize(g_window, &width, &height);

    xPos = x / width;
    yPos = (static_cast<double>(height) - y) / height;

    xPos *= 2.0L;
    yPos *= 2.0L;

    xPos -= 1.0L;
    yPos -= 1.0L;

    if (xPos < -1.0L) {
        xPos = -1.0L;
    } else if (xPos > 1.0L) {
        xPos = 1.0L;
    }

    if (yPos < -1.0L) {
        yPos = -1.0L;
    } else if (yPos > 1.0L) {
        yPos = 1.0L;
    }
}

int run(const std::filesystem::path& configurationPath)
{
    {
        auto config{ Visconfig::from_file(configurationPath) };

        if (!glfwInit()) {
            std::cerr << "ERROR: Could not initialize GLFW!" << std::endl;
            glfwTerminate();
            return 1;
        }

        glfwSetErrorCallback([](int code, const char* desc) {
            std::cerr << "Error: " << code << ". Description: " << desc << std::endl;
        });

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, config.options.screenMSAASamples);
        g_window = createWindow(config);
        if (!g_window) {
            std::cerr << "ERROR: Could not create a window!" << std::endl;
            glfwTerminate();
            return 1;
        }
        glfwSetWindowAspectRatio(g_window, config.options.screenWidth, config.options.screenHeight);
        attach(true);

        glfwMakeContextCurrent(g_window);
        glfwSetKeyCallback(g_window, [](GLFWwindow*, int key, int, int action, int) {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
                quit();
            } else if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE) {
                attach(isDetached());
            }
        });

        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            std::cerr << "ERROR: Could not setup glad!" << std::endl;
            glfwTerminate();
            return 1;
        }
        glEnable(GL_MULTISAMPLE);

#ifdef DEBUG_OPENGL
        glEnable(GL_DEBUG_OUTPUT);
        glDebugMessageCallback(MessageCallback, 0);
#endif // DEBUG_OPENGL

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(g_window, true);
        ImGui_ImplOpenGL3_Init("#version 410");

        Scene scene = initialize_scene(config);
        while (!shouldQuit()) {
            glfwPollEvents();

            // Start the Dear ImGui frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            tick(scene);
            draw(scene);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Update and Render additional Platform Windows
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
            }

            glfwSwapBuffers(g_window);
        }
    }

    AssetDatabase::clear();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g_window);
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

void GLAPIENTRY MessageCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei, const GLchar* message, const void*)
{
    std::cerr << "GL CALLBACK: " << (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "") << " source = 0x" << source
              << ", type = 0x" << type << ", id = 0x" << id << ", severity = 0x" << severity
              << ", message = " << message << std::endl;
}