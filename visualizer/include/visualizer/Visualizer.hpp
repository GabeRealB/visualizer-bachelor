#pragma once

#include <filesystem>

namespace Visualizer {

void quit();
bool shouldQuit();

void attach(bool attached);
bool isDetached();

void getRelativeMousePosition(double& xPos, double& yPos);

int run(const std::filesystem::path& configurationPath);

}
